/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2020, Institut fuer Technische Optik (ITO), 
    Universitaet Stuttgart, Germany

    This file is part of itom.
  
    itom is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public Licence as published by
    the Free Software Foundation; either version 2 of the Licence, or (at
    your option) any later version.

    itom is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
    General Public Licence for more details.

    You should have received a copy of the GNU Library General Public License
    along with itom. If not, see <http://www.gnu.org/licenses/>.
*********************************************************************** */

#include "../python/pythonEngineInc.h"
#include "../widgets/mainWindow.h"
#include "scriptEditorWidget.h"
#include "qpair.h"

#include "../global.h"
#include "../AppManagement.h"
#include "../mainApplication.h"
#include "../helper/guiHelper.h"

#include <qfileinfo.h>
#include "../ui/dialogEditBreakpoint.h"

#include <qmessagebox.h>
#include <QtPrintSupport/qprintpreviewdialog.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qmimedata.h>
#include <qtextcodec.h>
#include <qinputdialog.h>
#include <qdatetime.h>
#include <qcryptographichash.h>
#include <qregularexpression.h>

#include "../codeEditor/managers/panelsManager.h"
#include "../codeEditor/managers/modesManager.h"
#include "../codeEditor/textBlockUserData.h"
#include "scriptEditorPrinter.h"
#include "../organizer/userOrganizer.h"
#include "../widgets/consoleWidget.h"
#include "../python/pythonCommon.h"

namespace ito 
{

//!< static variable initialization
const QString ScriptEditorWidget::lineBreak = QString("\n");
ScriptEditorWidget::CursorPosition ScriptEditorWidget::currentGlobalEditorCursorPos;
QHash<int, ScriptEditorWidget*> ScriptEditorWidget::editorByUID;
int ScriptEditorWidget::currentMaximumUID = 1;

//----------------------------------------------------------------------------------------------------------------------------------
ScriptEditorWidget::ScriptEditorWidget(BookmarkModel *bookmarkModel, QWidget* parent) :
    AbstractCodeEditorWidget(parent),
    m_pFileSysWatcher(NULL), 
    m_filename(QString()),
    m_uid(ScriptEditorWidget::currentMaximumUID++),
    m_pythonBusy(false), 
    m_pythonExecutable(true),
    m_canCopy(false),
    m_codeCheckerCallTimer(NULL),
    m_outlineTimer(NULL),
    m_contextMenu(NULL),
    m_keepIndentationOnPaste(true),
    m_cursorJumpLastAction(false),
    m_pBookmarkModel(bookmarkModel),
    m_currentLineIndex(-1)
{
    qRegisterMetaType<QList<ito::CodeCheckerItem> >("QList<ito::CodeCheckerItem>");

    m_codeCheckerCallTimer = new QTimer(this);
    connect(m_codeCheckerCallTimer, SIGNAL(timeout()), this, SLOT(updateSyntaxCheck()));
    m_codeCheckerCallTimer->setInterval(1000);

    // regular expression for methods              |> this part might be not in the same line due multiple line parameter set
    //the regular expression should detect begin of definitions. This is:
    // 1. the line starts with 0..inf numbers of whitespace characters --> (\\s*)
    // 2. 'def' + 1 whitespace characters is following --> (def)\\s
    // 3. optionally, 0..inf numbers of _ may come (e.g. for private methods) --> (_*)
    // 4. 1..inf arbitrary characters will come (function name) --> (.+)
    // 5. bracket open '(' --> \\(
    // 6. arbitrary characters --> (.*)
    // 7. OR combination --> (cond1|cond2)
    // 7a. cond1: bracket close ')' followed by colon, arbitrary spaces and an optional comment starting with # --> \\):\\s*(#?.*)?
    // 7b. backspace to indicate a newline --> \\\\ 
    m_regExpClass = QRegularExpression("^(\\s*)(class)\\s(?<name>.+)(\\(.*\\))?:\\s*(#?.*)");
    m_regExpClass.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    m_regExpDecorator = QRegularExpression("^(\\s*)(@)(?<name>\\S+)\\s*(#?.*)");
    m_regExpMethodStart = QRegularExpression("^(\\s*)(async\\s*)?def\\s+(?<name>\\w+)\\(");
    m_regExpMethodStart.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    //m_regExpMethod = QRegularExpression("^(\\s*)(?<async>async\\s*)?(def)\\s(?<private>_*)(?<name>.+)\\(?<args>(.*)(\\)(\\s*|\\s->\\s(?<rettype>.+)):\\s*(#?.*)?|\\\\)");

    // named groups in complex OR-cases seems not to work
    m_regExpMethod = QRegularExpression("^(\\s*)(?<async>async\\s*)?(def)\\s(?<name>.+)\\((.*)(\\)(\\s*|\\s->\\s(.+)):\\s*(#?.*)?|\\\\)");
    m_regExpMethod = QRegularExpression("^(\\s*)(?<async>async\\s*)?(def)\\s+(?<name>.+)\\((.*)\\)(\\s*|\\s+->\\s+(.+)):\\s*(#?.*)?");
    m_regExpMethod.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

    m_outlineTimer = new QTimer(this);
    connect(m_outlineTimer, SIGNAL(timeout()), this, SLOT(outlineTimerElapsed()));
    m_outlineTimer->setInterval(2000);

    m_cursorBeforeMouseClick.invalidate();
    
    initEditor();
    initMenus();
    loadSettings();

    m_pFileSysWatcher = new QFileSystemWatcher(this);
    connect(m_pFileSysWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileSysWatcherFileChanged(const QString&)));

    PythonEngine *pyEngine = qobject_cast<PythonEngine*>(AppManagement::getPythonEngine());
    const MainWindow *mainWin = qobject_cast<MainWindow*>(AppManagement::getMainWindow());

    if (pyEngine) 
    {
        m_pythonBusy = pyEngine->isPythonBusy();
        connect(pyEngine, SIGNAL(pythonDebugPositionChanged(QString, int)), this, SLOT(pythonDebugPositionChanged(QString, int)));
        connect(pyEngine, SIGNAL(pythonStateChanged(tPythonTransitions)), this, SLOT(pythonStateChanged(tPythonTransitions)));
    
        connect(this, SIGNAL(pythonRunFile(QString)), pyEngine, SLOT(pythonRunFile(QString)));
        connect(this, SIGNAL(pythonDebugFile(QString)), pyEngine, SLOT(pythonDebugFile(QString)));

        connect(this, SIGNAL(pythonRunSelection(QString)), mainWin, SLOT(pythonRunSelection(QString)));

        const BreakPointModel *bpModel = pyEngine->getBreakPointModel();

        connect(bpModel, SIGNAL(breakPointAdded(BreakPointItem, int)), this, SLOT(breakPointAdd(BreakPointItem, int)));
        connect(bpModel, SIGNAL(breakPointDeleted(QString, int, int)), this, SLOT(breakPointDelete(QString, int, int)));
        connect(bpModel, SIGNAL(breakPointChanged(BreakPointItem, BreakPointItem)), this, SLOT(breakPointChange(BreakPointItem, BreakPointItem)));

        //!< check if BreakPointModel already contains breakpoints for this editor and load them
        if (getFilename() != "")
        {
            QModelIndexList modelIndexList = bpModel->getBreakPointIndizes(getFilename());
            QList<BreakPointItem> bpItems = bpModel->getBreakPoints(modelIndexList);

            for (int i = 0; i < bpItems.size(); i++)
            {
                breakPointAdd(bpItems.at(i), i);
            }
        }
    }    

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(nrOfLinesChanged()));
    connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(copyAvailable(bool)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorPositionChanged()));

    connect(m_pBookmarkModel, SIGNAL(bookmarkAdded(BookmarkItem)), this, SLOT(onBookmarkAdded(BookmarkItem)));
    connect(m_pBookmarkModel, SIGNAL(bookmarkDeleted(BookmarkItem)), this, SLOT(onBookmarkDeleted(BookmarkItem)));

    setAcceptDrops(true);

    editorByUID[m_uid] = this;

#ifndef WIN32
    m_filenameCaseSensitivity = Qt::CaseSensitive;
#else
    m_filenameCaseSensitivity = Qt::CaseInsensitive;
#endif

    // update the outline
    outlineTimerElapsed();
}

//----------------------------------------------------------------------------------------------------------------------------------
ScriptEditorWidget::~ScriptEditorWidget()
{
    const PythonEngine *pyEngine = PythonEngine::getInstance();
    const MainWindow *mainWin = qobject_cast<MainWindow*>(AppManagement::getMainWindow());

    if (pyEngine)
    {
        const BreakPointModel *bpModel = pyEngine->getBreakPointModel();

        disconnect(pyEngine, SIGNAL(pythonDebugPositionChanged(QString, int)), this, SLOT(pythonDebugPositionChanged(QString, int)));
        disconnect(pyEngine, SIGNAL(pythonStateChanged(tPythonTransitions)), this, SLOT(pythonStateChanged(tPythonTransitions)));

        disconnect(this, SIGNAL(pythonRunFile(QString)), pyEngine, SLOT(pythonRunFile(QString)));
        disconnect(this, SIGNAL(pythonDebugFile(QString)), pyEngine, SLOT(pythonDebugFile(QString)));

        disconnect(this, SIGNAL(pythonRunSelection(QString)), mainWin, SLOT(pythonRunSelection(QString)));

        disconnect(bpModel, SIGNAL(breakPointAdded(BreakPointItem, int)), this, SLOT(breakPointAdd(BreakPointItem, int)));
        disconnect(bpModel, SIGNAL(breakPointDeleted(QString, int, int)), this, SLOT(breakPointDelete(QString, int, int)));
        disconnect(bpModel, SIGNAL(breakPointChanged(BreakPointItem, BreakPointItem)), this, SLOT(breakPointChange(BreakPointItem, BreakPointItem)));
    }   

    disconnect(this, SIGNAL(blockCountChanged(int)), this, SLOT(nrOfLinesChanged()));
    disconnect(this, SIGNAL(copyAvailable(bool)), this, SLOT(copyAvailable(bool)));

    DELETE_AND_SET_NULL(m_pFileSysWatcher);

    setContextMenuPolicy(Qt::DefaultContextMenu); //contextMenuEvent is called

    editorByUID.remove(m_uid);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::initEditor()
{
    m_foldingPanel = QSharedPointer<FoldingPanel>(new FoldingPanel(false, "FoldingPanel"));
    panels()->append(m_foldingPanel.dynamicCast<ito::Panel>());
    m_foldingPanel->setOrderInZone(1);

    m_checkerBookmarkPanel = QSharedPointer<CheckerBookmarkPanel>(new CheckerBookmarkPanel(m_pBookmarkModel, "CheckerBookmarkPanel"));
    panels()->append(m_checkerBookmarkPanel.dynamicCast<ito::Panel>());
    m_checkerBookmarkPanel->setOrderInZone(4);

    m_breakpointPanel = QSharedPointer<BreakpointPanel>(new BreakpointPanel("BreakpointPanel"));
    panels()->append(m_breakpointPanel.dynamicCast<ito::Panel>());
    m_breakpointPanel->setOrderInZone(2);

    m_errorLineHighlighterMode = QSharedPointer<ErrorLineHighlighterMode>(new ErrorLineHighlighterMode("ErrorLineHighlighterMode"));
    modes()->append(m_errorLineHighlighterMode.dynamicCast<ito::Mode>());
    m_errorLineHighlighterMode->setBackground(QColor(255, 192, 192));

    m_lineNumberPanel = QSharedPointer<LineNumberPanel>(new LineNumberPanel("LineNumberPanel"));
    panels()->append(m_lineNumberPanel.dynamicCast<ito::Panel>());
    m_lineNumberPanel->setOrderInZone(3);

    m_pyGotoAssignmentMode = QSharedPointer<PyGotoAssignmentMode>(new PyGotoAssignmentMode("PyGotoAssignmentMode"));
    connect(m_pyGotoAssignmentMode.data(), SIGNAL(outOfDoc(PyAssignment)), this, SLOT(gotoAssignmentOutOfDoc(PyAssignment)));
    modes()->append(m_pyGotoAssignmentMode.dynamicCast<ito::Mode>());

    m_wordHoverTooltipMode = QSharedPointer<WordHoverTooltipMode>(new WordHoverTooltipMode("WordHoverTooltipMode"));
    modes()->append(m_wordHoverTooltipMode.dynamicCast<ito::Mode>());

    if (m_symbolMatcher)
    {
        m_symbolMatcher->setMatchBackground(QColor("lightGray"));
        m_symbolMatcher->setMatchForeground(QColor("blue"));
    }

    connect(m_checkerBookmarkPanel.data(), SIGNAL(toggleBookmarkRequested(int)), this, SLOT(toggleBookmarkRequested(int)));
    
    connect(m_breakpointPanel.data(), SIGNAL(toggleBreakpointRequested(int)), this, SLOT(toggleBreakpoint(int)));
    connect(m_breakpointPanel.data(), SIGNAL(toggleEnableBreakpointRequested(int)), this, SLOT(toggleEnableBreakpoint(int)));
    connect(m_breakpointPanel.data(), SIGNAL(editBreakpointRequested(int)), this, SLOT(editBreakpoint(int)));
    connect(m_breakpointPanel.data(), SIGNAL(clearAllBreakpointsRequested()), this, SLOT(clearAllBreakpoints()));
    connect(m_breakpointPanel.data(), SIGNAL(gotoNextBreakPointRequested()), this, SLOT(gotoNextBreakPoint()));
    connect(m_breakpointPanel.data(), SIGNAL(gotoPreviousBreakRequested()), this, SLOT(gotoPreviousBreakPoint()));

    return RetVal(retOk);
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::loadSettings()
{
    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("CodeEditor");

    if (settings.value("showWhitespace", true).toBool())
    {
        setShowWhitespaces(true);
    }
    else
    {
        setShowWhitespaces(false);
    }

    // Code Checker
    m_codeCheckerEnabled = settings.value("codeCheckerMode", 
                            PythonCommon::CodeCheckerPyFlakes).toInt() 
                            != PythonCommon::NoCodeChecker;
    m_codeCheckerInterval = (int)(settings.value("codeCheckerInterval", 1).toDouble()*1000);

    if (m_codeCheckerCallTimer)
    {
        m_codeCheckerCallTimer->stop();
        m_codeCheckerCallTimer->setInterval(m_codeCheckerInterval);
    }

    if (m_codeCheckerEnabled)
    { // empty call: all bugs disappear
        triggerCodeChecker();
    }
    else
    {
        codeCheckerResultsChanged(QList<ito::CodeCheckerItem>());
    }

    // Code Outline
    m_outlineTimerEnabled = settings.value("outlineAutoUpdateEnabled", true).toBool();
    m_outlineTimer->stop();
    m_outlineTimer->setInterval((settings.value("outlineAutoUpdateDelay", 2.00).toDouble() * 1000));

    //todo
    // Fold Style
    QByteArray foldStyle = settings.value("foldStyle", "plus_minus").toByteArray();
    if (foldStyle == "") 
    {
        foldStyle = "none";
    }

    switch (foldStyle[0])
    {
    default:
    case 'n':
        m_foldingPanel->setVisible(false);
        break;
    case 'p':
        m_foldingPanel->setVisible(true);
        break;
    case 's':
        m_foldingPanel->setVisible(true);
        break;
    case 'c':
        m_foldingPanel->setVisible(true);
        break;
    }

    setEdgeMode((CodeEditor::EdgeMode)(settings.value("edgeMode", edgeMode()).toInt()));
    setEdgeColumn(settings.value("edgeColumn", edgeColumn()).toInt());
    setEdgeColor(settings.value("edgeColor", edgeColor()).value<QColor>());

    m_pyGotoAssignmentMode->setEnabled(settings.value("gotoAssignmentEnabled", true).toBool());
    m_pyGotoAssignmentMode->setMouseClickEnabled(settings.value("gotoAssignmentMouseClickEnabled", m_pyGotoAssignmentMode->mouseClickEnabled()).toBool());
    m_pyGotoAssignmentMode->setDefaultWordClickMode(settings.value("gotoAssignmentMouseClickMode", m_pyGotoAssignmentMode->defaultWordClickMode()).toInt());

    Qt::KeyboardModifiers modifiers;
    switch (settings.value("gotoAssignmentMouseClickKey", 1).toInt())
    {
    case 0:
        modifiers = Qt::ControlModifier;
        break;
    case 1:
        modifiers = Qt::ControlModifier | Qt::ShiftModifier;
        break;
    default:
        modifiers = Qt::ControlModifier | Qt::AltModifier;
        break;
    }

    m_pyGotoAssignmentMode->setWordClickModifiers(modifiers);

    m_wordHoverTooltipMode->setEnabled(settings.value("helpTooltipEnabled", true).toBool());

    m_errorLineHighlighterMode->setBackground(QColor(settings.value("markerScriptErrorBackgroundColor", QColor(255, 192, 192)).toString()));

    setSelectLineOnCopyEmpty(settings.value("selectLineOnCopyEmpty", true).toBool());
    setKeepIndentationOnPaste(settings.value("keepIndentationOnPaste", true).toBool());

    m_pyAutoIndentMode->setAutoStripTrailingSpacesAfterReturn(
        settings.value("autoStripTrailingSpacesAfterReturn", true).toBool()
    );

    bool pyCodeFormatEnabled = settings.value("autoCodeFormatEnabled", true).toBool();
    m_autoCodeFormatCmd = settings.value("autoCodeFormatCmd", "black --line-length 88 --quiet -").toString();
    auto pyCodeFormatAction = m_editorMenuActions.find("formatFile");

    if (pyCodeFormatAction != m_editorMenuActions.end())
    {
        pyCodeFormatAction->second->setVisible(pyCodeFormatEnabled);
        pyCodeFormatAction->second->setEnabled(m_autoCodeFormatCmd != "");
    }

    settings.endGroup();

    AbstractCodeEditorWidget::loadSettings();
}

//-------------------------------------------------------------------------------------
void ScriptEditorWidget::initMenus()
{
    QMenu *editorMenu = contextMenu();

    m_editorMenuActions["cut"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/editCut.png"), tr("Cut"), 
            this, SLOT(menuCut()), QKeySequence::Cut
        );


    m_editorMenuActions["copy"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/editCopy.png"), tr("Copy"), 
            this, SLOT(menuCopy()), QKeySequence::Copy
        );

    m_editorMenuActions["paste"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/editPaste.png"), tr("Paste"), 
            this, SLOT(menuPaste()), QKeySequence::Paste
        );

    editorMenu->addSeparator();

    m_editorMenuActions["indent"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/editIndent.png"), tr("Indent"), 
            this, SLOT(menuIndent()), QKeySequence(tr("Tab", "QShortcut"))
        );
    
    m_editorMenuActions["unindent"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/editUnindent.png"), tr("Unindent"), 
            this, SLOT(menuUnindent()), QKeySequence(tr("Shift+Tab", "QShortcut"))
        );
    
    m_editorMenuActions["comment"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/editComment.png"), tr("Comment"), this, 
            SLOT(menuComment()), QKeySequence(tr("Ctrl+R", "QShortcut"))
        );
    
    m_editorMenuActions["uncomment"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/editUncomment.png"), tr("Uncomment"), this, 
            SLOT(menuUncomment()), QKeySequence(tr("Ctrl+Shift+R", "QShortcut"))
        );
    
    m_editorMenuActions["formatFile"] = 
        editorMenu->addAction(
            QIcon(":/editor/icons/leftAlign.png"), tr("Auto Format File"), 
            this, SLOT(menuPyCodeFormatting()), QKeySequence(tr("Ctrl+Alt+I", "QShortcut"))
        );
    m_editorMenuActions["formatFile"]->setVisible(false);
    
    editorMenu->addSeparator();
    
    m_editorMenuActions["runScript"] = 
        editorMenu->addAction(
            QIcon(":/script/icons/runScript.png"), tr("Run Script"), this, 
            SLOT(menuRunScript()), QKeySequence(tr("F5", "QShortcut"))
        );
    
    m_editorMenuActions["runSelection"] = 
        editorMenu->addAction(
            QIcon(":/script/icons/runScript.png"), tr("Run Selection"), this, 
            SLOT(menuRunSelection()), QKeySequence(tr("F9", "QShortcut"))
        );
    
    m_editorMenuActions["debugScript"] = 
        editorMenu->addAction(
            QIcon(":/script/icons/debugScript.png"), tr("Debug Script"), this, 
            SLOT(menuDebugScript()), QKeySequence(tr("F6", "QShortcut"))
        );
    
    m_editorMenuActions["stopScript"] = 
        editorMenu->addAction(
            QIcon(":/script/icons/stopScript.png"), tr("Stop Script"), this, 
            SLOT(menuStopScript()), QKeySequence(tr("Shift+F5", "QShortcut"))
        );
    
    editorMenu->addSeparator();

    QShortcut *tabChangedShortcut  = new QShortcut(QKeySequence(tr("Ctrl+Tab", "QShortcut")), this);
    tabChangedShortcut->setContext(Qt::WidgetShortcut);
    connect(tabChangedShortcut, &QShortcut::activated, this, &ScriptEditorWidget::tabChangeRequest);

    editorMenu->addActions(m_pyGotoAssignmentMode->actions());

    editorMenu->addSeparator();
    
    QMenu *foldMenu = editorMenu->addMenu(tr("Folding"));
    m_editorMenuActions["foldUnfoldToplevel"] = 
        foldMenu->addAction(tr("Fold/Unfold &Toplevel"), this, SLOT(menuFoldUnfoldToplevel()));
    
    m_editorMenuActions["foldUnfoldAll"] = 
        foldMenu->addAction(tr("Fold/Unfold &All"), this, SLOT(menuFoldUnfoldAll()));
    
    m_editorMenuActions["unfoldAll"] = 
        foldMenu->addAction(tr("&Unfold All"), this, SLOT(menuUnfoldAll()));
    
    m_editorMenuActions["foldAll"] = 
        foldMenu->addAction(tr("&Fold All"), this, SLOT(menuFoldAll()));
    
    editorMenu->addSeparator();
    
    m_editorMenuActions["insertCodec"] = 
        editorMenu->addAction(tr("&Insert Codec..."), this, SLOT(menuInsertCodec()));
}

//----------------------------------------------------------------------------------------------------------------------------------
const ScriptEditorStorage ScriptEditorWidget::saveState() const
{
    ScriptEditorStorage storage;
    storage.filename = getFilename();
    storage.firstVisibleLine = firstVisibleLine();

    return storage;
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::restoreState(const ScriptEditorStorage &data)
{
    RetVal retVal = openFile(data.filename, true);

    if (!retVal.containsError())
    {
        setFirstVisibleLine(data.firstVisibleLine);
    }
    
    return retVal;
}

//-----------------------------------------------------------
bool ScriptEditorWidget::keepIndentationOnPaste() const
{
    return m_keepIndentationOnPaste;
}

void ScriptEditorWidget::setKeepIndentationOnPaste(bool value)
{
    m_keepIndentationOnPaste = value;
}

//------------------------------------------------------------
void ScriptEditorWidget::contextMenuAboutToShow(int contextMenuLine)
{
    const PythonEngine *pyEngine = qobject_cast<PythonEngine*>(AppManagement::getPythonEngine());
    int lineFrom, indexFrom, lineTo, indexTo;

    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

    m_editorMenuActions["cut"]->setEnabled(lineFrom != -1 || selectLineOnCopyEmpty());
    m_editorMenuActions["copy"]->setEnabled(lineFrom != -1 || selectLineOnCopyEmpty());
    m_editorMenuActions["paste"]->setEnabled(!m_pythonBusy && contextMenuLine >= 0 && canPaste());
    m_editorMenuActions["runScript"]->setEnabled(!m_pythonBusy);
    m_editorMenuActions["runSelection"]->setEnabled(lineFrom != -1 && pyEngine && (!m_pythonBusy || pyEngine->isPythonDebuggingAndWaiting()));
    m_editorMenuActions["debugScript"]->setEnabled(!m_pythonBusy);
    m_editorMenuActions["stopScript"]->setEnabled(m_pythonBusy);
    m_editorMenuActions["insertCodec"]->setEnabled(!m_pythonBusy);   

    AbstractCodeEditorWidget::contextMenuAboutToShow(contextMenuLine);
}

//----------------------------------------------------------------------------------------------------------------------------------
bool ScriptEditorWidget::canInsertFromMimeData(const QMimeData *source) const
{
    if ((source->hasFormat("FileName") || source->hasFormat("text/uri-list")))
    {
        ito::UserOrganizer *uOrg = (UserOrganizer*)AppManagement::getUserOrganizer();
        if (uOrg->currentUserHasFeature(featDeveloper))
        {
            QList<QUrl> list(source->urls());
            for(int i = 0; i<list.length(); ++i)
            {
                QString fext = QFileInfo(source->urls().at(0).toString()).suffix().toLower();
                if (!((fext == "txt") || (fext == "py") || (fext == "c") || (fext == "cpp")
                    || (fext == "h") || (fext == "hpp") || (fext == "cxx") || (fext == "hxx")))
                {
                    return false;
                }
            }
            return true;
        }
    }
    else
    {
        return AbstractCodeEditorWidget::canInsertFromMimeData(source);
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::insertFromMimeData(const QMimeData *source)
{
    if (keepIndentationOnPaste() &&
        source->hasText())
    {
        int lineCount;
        QString formattedText = formatPythonCodePart(source->text(), lineCount);

        QStringList textlines = formattedText.split("\n");

        if (textlines.size() > 0)
        {
            int currentLine;
            int currentColumn;
            getCursorPosition(&currentLine, &currentColumn);

            QString currentIdent = lineText(currentLine).left(currentColumn);

            if (currentIdent.trimmed().size() != 0)
            {
                //the text in the current line from the beginning to the cursor does not
                //only contain spaces or tabs. Therefore this line is not considered.
                currentIdent = "";
            }

            for (int i = 1; i < textlines.size(); ++i)
            {
                textlines[i] = currentIdent + textlines[i];
            }

            QMimeData mimeData;
            mimeData.setText(textlines.join("\n"));

            AbstractCodeEditorWidget::insertFromMimeData(&mimeData);
        }
        else
        {
            AbstractCodeEditorWidget::insertFromMimeData(source);
        }
    }
    else
    {
        AbstractCodeEditorWidget::insertFromMimeData(source);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::dropEvent(QDropEvent *event)
{
    QObject *sew = AppManagement::getScriptEditorOrganizer();

    if (sew != NULL)
    {
        if ((event->mimeData()->hasFormat("FileName") || event->mimeData()->hasFormat("text/uri-list")))
        {
            bool areAllLocals = true; //type of file is already checked in ScriptEditorWidget::canInsertFromMimeData
            QList<QUrl> list(event->mimeData()->urls());
            for (int i = 0; i < list.length(); ++i)
            {
                if (!list[i].isLocalFile())
                {
                    areAllLocals = false;
                    break;
                }
            }
            if (areAllLocals)
            {
                for (int i = 0; i < list.length(); ++i)
                {
                    QMetaObject::invokeMethod(sew, "openScript", Q_ARG(QString, list[i].toLocalFile()), Q_ARG(ItomSharedSemaphore*, NULL));
                    
                }
                event->accept();
            }

            if (event->isAccepted())
            {
                //fix in order not to freeze the cursor after dropping 
                //see: https://stackoverflow.com/questions/29456366/qtextedit-cursor-becomes-frozen-after-overriding-its-dropevent
                QMimeData mimeData;
                mimeData.setText("");
                QDropEvent dummyEvent(event->posF(), event->possibleActions(), &mimeData, event->mouseButtons(), event->keyboardModifiers());
                AbstractCodeEditorWidget::dropEvent(&dummyEvent);
            }
            else
            {
                AbstractCodeEditorWidget::dropEvent(event);
            }
        }
        else
        {
            AbstractCodeEditorWidget::dropEvent(event);

            QObject *parent = event->source() ? event->source()->parent() : NULL;
            
            //this snipped is based on a QScintilla mailing list thread:
            //http://www.riverbankcomputing.com/pipermail/qscintilla/2014-September/000996.html
            if (parent && qobject_cast<ito::ConsoleWidget*>(parent))
            {
                //we never want to move text out of the console, text should always be copied
                if (event->dropAction() == Qt::MoveAction)
                {
                    event->setDropAction(Qt::CopyAction);
                    event->accept();
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::copyAvailable(const bool yes)
{
    m_canCopy = yes;
}

//----------------------------------------------------------------------------------------------------------------------------------
bool ScriptEditorWidget::getCanCopy() const
{ 
    return m_canCopy || selectLineOnCopyEmpty();
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::setCursorPosAndEnsureVisible(const int line, bool errorMessageClick /*= false*/, bool showSelectedCallstackLine /*= false*/)
{
    CursorPosition temp = currentGlobalEditorCursorPos;

    ensureLineVisible(line);

    QTextCursor c = textCursor();
    int currentLine = c.isNull() ? -1 : c.blockNumber();

    if (!temp.cursor.isNull() && \
        currentLine != -1)
    {
        int bn = temp.cursor.blockNumber();

        if (temp.editorUID != m_uid || \
            std::abs(currentLine - temp.cursor.blockNumber()) > 11)
        {
            reportGoBackNavigationCursorMovement(CursorPosition(c, m_uid), "setCursorPosAndEnsureVisible");
        }
    }

    

    if (errorMessageClick)
    {
        m_errorLineHighlighterMode->setErrorLine(line);
    }

    if (showSelectedCallstackLine)
    {
        m_breakpointPanel->setSelectedCallstackLine(line);
    }

    setFocus();

    return retOk;
}

//----------------------------------------------------------------------------------------------------------------------------------
//!< removes the current-line and current-callstack-line arrows from the breakpoint panel, if currently displayed
void ScriptEditorWidget::removeCurrentCallstackLine()
{
    m_breakpointPanel->setSelectedCallstackLine(-1);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::showLineAndHighlightWord(const int line, const QString &highlightedText, Qt::CaseSensitivity caseSensitivity)
{
    ito::RetVal retval;
    
    if (line >= 0)
    {
        retval += setCursorPosAndEnsureVisible(line);

        QString text = lineText(line);
        int idx = text.indexOf(highlightedText, 0, caseSensitivity);

        if (idx >= 0)
        {
            setSelection(line, idx, line, idx + highlightedText.size());
        }
    }

    return retval;
}


//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::gotoAssignmentOutOfDoc(PyAssignment ref)
{
    QObject *seo = AppManagement::getScriptEditorOrganizer();
    if (seo)
    {
        QMetaObject::invokeMethod(seo, "openScript", Q_ARG(QString, ref.m_modulePath), Q_ARG(ItomSharedSemaphore*, NULL), Q_ARG(int, ref.m_line), Q_ARG(bool, false));
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuCut()
{
    cut();
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuCopy()
{
    copy();
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuPaste()
{
    paste();
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuIndent()
{
    if (isReadOnly() == false)
    {
        indent();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuUnindent()
{
    if (isReadOnly() == false)
    {
        unindent();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuComment()
{
    if (isReadOnly() == false)
    {
        int lineFrom, lineTo, indexFrom, indexTo;
        QString lineTextFull;
        QString lineTextTrimmed;
        int searchIndex;

        getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
        if (lineFrom < 0)
        {
            getCursorPosition(&lineFrom, &indexFrom);
            lineTo = lineFrom;
            indexTo = indexFrom;
        }

        for (int i = lineFrom; i <= lineTo; i++)
        {
            lineTextFull = lineText(i);
            lineTextTrimmed = lineTextFull.trimmed();

            searchIndex = lineTextFull.indexOf(lineTextTrimmed);
            if (searchIndex >= 0)
            {
                QTextCursor cursor = setCursorPosition(i, searchIndex, false);
                cursor.insertText("# ");

                if (i == lineFrom)
                {
                    indexFrom += 2; // 2 is the length of the inserted text
                }
                if (i == lineTo)
                {
                    indexTo += 2; // 2 is the length of the inserted text
                }
            }
        }

        if (lineFrom != lineTo || indexFrom != indexTo)
        {
            setSelection(lineFrom, indexFrom, lineTo, indexTo);
        }
        else
        {
            setCursorPosition(lineTo, indexTo);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuUncomment()
{
    if (isReadOnly() == false)
    {
        int lineFrom, lineTo, indexFrom, indexTo;
        QString lineTextFull;
        int searchIndex;
        QString lineTextTrimmed;

        getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
        if (lineFrom < 0)
        {
            getCursorPosition(&lineFrom, &indexFrom);
            lineTo = lineFrom;
            indexTo = indexFrom;
        }

        for (int i = lineFrom; i <= lineTo; i++)
        {
            lineTextFull = lineText(i);
            lineTextTrimmed = ito::Utils::lstrip(lineTextFull); // remove whitespaces at the beginning (not at the end of the string)

            if (lineTextTrimmed.left(2) == "# ") // delete # with space characters
            {
                searchIndex = lineTextFull.indexOf("# ");
                if (searchIndex >= 0)
                {
                    setSelection(i, searchIndex, i, searchIndex + 2);
                    textCursor().removeSelectedText();

                    if (i == lineFrom && indexFrom >= 2)
                    {
                        indexFrom -= 2; // 2 is the length of the removed text
                    }
                    if (i == lineTo && indexTo >= 2)
                    {
                        indexTo -= 2; // 2 is the length of the removed text
                    }
                }
            }
            else if (lineTextTrimmed.left(1) == "#")
            {
                searchIndex = lineTextFull.indexOf("#");
                if (searchIndex >= 0)
                {
                    setSelection(i, searchIndex, i, searchIndex + 1);
                    textCursor().removeSelectedText();

                    if (i == lineFrom && indexFrom >= 1)
                    {
                        indexFrom -= 1; // 1 is the length of the removed text
                    }
                    if (i == lineTo && indexTo >= 1)
                    {
                        indexTo -= 1; // 1 is the length of the removed text
                    }
                }
            }
            
        }

        if (lineFrom != indexFrom || lineTo != indexTo)
        {
            setSelection(lineFrom, indexFrom, lineTo, indexTo);
        }
        else
        {
            setCursorPosition(lineFrom, indexFrom);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuRunScript()
{
    RetVal retValue(retOk);

    retValue += saveFile(false);

    if (!retValue.containsError())
    {
        //retValue += checkSaveStateForExecution();

        if (!retValue.containsError())
        {
            emit pythonRunFile(getFilename());
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuRunSelection()
{
    int lineFrom = -1;
    int lineTo = -1;
    int indexFrom = -1;
    int indexTo = -1;

    //check whether text has been marked
    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    if (lineFrom >= 0)
    {
        QString defaultText = selectedText();

        //in linux, double-clicking at one line entirely marks this line and sometimes includes a \n to the next line. remove this:
        const QChar *data = defaultText.constData();
        int signsToRemove = 0;
        int len = defaultText.size() - 1;
        
        while (defaultText[len-signsToRemove] == '\n' || defaultText[len-signsToRemove] == '\r' || defaultText[len-signsToRemove] == ' ')
        {
            signsToRemove++;
        }

        defaultText.truncate(len - signsToRemove + 1);
        

        emit pythonRunSelection(defaultText);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuDebugScript()
{
    RetVal retValue(retOk);

    if (getFilename() == "")
    {
        retValue += saveFile(true);
    }

    if (!retValue.containsError())
    {
        //retValue += checkSaveStateForExecution();

        if (!retValue.containsError())
        {
            emit pythonDebugFile(getFilename());
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuStopScript()
{
    PythonEngine* eng = qobject_cast<PythonEngine*>(AppManagement::getPythonEngine());

    if (eng != NULL)
    {
        eng->pythonInterruptExecutionThreadSafe();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuInsertCodec()
{
    QStringList items;
    bool ok;
    items << "ascii (English, us-ascii)" << "latin1 (West Europe, iso-8859-1)" 
          << "iso-8859-15 (Western Europe)" << "utf8 (all languages)";

    QString codec = QInputDialog::getItem(
        this, 
        tr("Insert Codec"), 
        tr("Choose an encoding of the file which is added to the first line of the script"), 
        items, 
        2, 
        false, 
        &ok
    );

    if (codec != "" && ok)
    {
        items = codec.split(" ");

        if (items.size() > 0)
        {
            QString newText = QString("# coding=%1\n").arg(items[0]);
            QTextCursor currentCursor = textCursor();
            currentCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            currentCursor.insertText(newText);
            setModified(true);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuUnfoldAll()
{
    m_foldingPanel->expandAll();
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuFoldAll()
{
    m_foldingPanel->collapseAll();
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuFoldUnfoldToplevel()
{
    m_foldingPanel->toggleFold(true);
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::menuFoldUnfoldAll()
{
    m_foldingPanel->toggleFold(false);
}

//-------------------------------------------------------------------------------------
void doDeleteLater(QObject *obj)
{
    obj->deleteLater();
}

//-------------------------------------------------------------------------------------
void ScriptEditorWidget::menuPyCodeFormatting()
{
    if (m_autoCodeFormatCmd == "")
    {
        QMessageBox::critical(
            this,
            tr("Missing auto code format command"),
            tr("No auto code format call command has been given in the "
                "itom property dialog. Please indicate a command there.")
        );
        return;
    }

    // the PyCodeFormatter should be destroyed using deleteLater
    // since the cancel button event will indirectly lead to 
    // a clear command of m_pyCodeFormatter (via the method pyCodeFormatterDone).
    // This would then destroy the QProgressDialog before the mouseRelease event
    // of the cancel button has been fully terminated.
    m_pyCodeFormatter = QSharedPointer<PyCodeFormatter>(new PyCodeFormatter(this), doDeleteLater);
    connect(m_pyCodeFormatter.data(), &PyCodeFormatter::formattingDone,
        this, &ScriptEditorWidget::pyCodeFormatterDone);
    ito::RetVal retval = m_pyCodeFormatter->startFormatting(m_autoCodeFormatCmd, toPlainText(), this);

    if (retval.containsError())
    {
        const ito::UserOrganizer *userOrg = (UserOrganizer*)AppManagement::getUserOrganizer();
        ito::UserFeatures features = userOrg->getCurrentUserFeatures();

        QString text1 = tr("The code formatting could not be started:\n%1").arg(retval.errorMessage());
        QString text2 = tr("\n\nShould this feature be deactivated? "
            "This can be changed again in the property dialog of itom.");

        if (features & ito::UserFeature::featProperties)
        {
            int btn = QMessageBox::critical(
                this,
                tr("Error starting auto code format"),
                text1 + text2,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );

            if (btn == QMessageBox::Yes)
            {
                QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
                settings.beginGroup("CodeEditor");
                settings.setValue("autoCodeFormatEnabled", false);
                settings.endGroup();

                MainApplication *ma = qobject_cast<MainApplication*>(
                    AppManagement::getMainApplication());

                if (ma)
                {
                    emit ma->propertiesChanged();
                }
            }
        }
        else
        {
            QMessageBox::critical(
                this,
                tr("Error starting auto code format"),
                text1
            );
        }
    }
}

//-------------------------------------------------------------------------------------
void ScriptEditorWidget::pyCodeFormatterDone(bool success, QString code)
{
    if (success && code != toPlainText())
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.beginEditBlock();
        cursor.removeSelectedText();
        cursor.insertText(code);
        cursor.endEditBlock();
        setModified(true);
    }
    else if (!success && code.trimmed() != "")
    {
        const ito::UserOrganizer *userOrg = (UserOrganizer*)AppManagement::getUserOrganizer();
        ito::UserFeatures features = userOrg->getCurrentUserFeatures();

        code = code.trimmed();

        if (code.size() > 200)
        {
            // trim code if too long
            code = code.left(98) + "\n...\n" + code.right(99);
        }

        QString text1 = tr("The code formatting failed:\n%1").arg(code);
        QString text2 = tr("\n\nShould this feature be deactivated? "
            "This can be changed again in the property dialog of itom.");

        if (features & ito::UserFeature::featProperties)
        {
            int btn = QMessageBox::critical(
                this,
                tr("Auto code format error"),
                text1 + text2,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            );

            if (btn == QMessageBox::Yes)
            {
                QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
                settings.beginGroup("CodeEditor");
                settings.setValue("autoCodeFormatEnabled", false);
                settings.endGroup();

                MainApplication *ma = qobject_cast<MainApplication*>(
                    AppManagement::getMainApplication());

                if (ma)
                {
                    emit ma->propertiesChanged();
                }
            }
        }
        else
        {
            QMessageBox::critical(
                this,
                tr("Auto code format error"),
                text1
            );
        }
    }

    m_pyCodeFormatter.clear();
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::openFile(QString fileName, bool ignorePresentDocument)
{
    //!< check for modifications in the present document first
    if (!ignorePresentDocument)
    {
        if (isModified())
        {
            int ret = QMessageBox::information(
                this, 
                tr("Unsaved Changes"), 
                tr("There are unsaved changes in the current document. Do you want to save it first?"), 
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes
            );

            if (ret & QMessageBox::Cancel)
            {
                return RetVal(retOk);
            }
            else if (ret & QMessageBox::Yes)
            {
                saveFile(false);
                setModified(false);
            }
        }
    }

    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error while opening file"), tr("File %1 could not be loaded").arg(fileName));
    }
    else
    {
        //in Qt4, QString(QByteArray) created the string with fromAscii(byteArray), in Qt5 it is fromUtf8(byteArray)
        //therefore there is a setting property telling the encoding of saved python files and the files are loaded assuming
        //this special encoding. If no encoding is given, latin1 is always assumed.
        QByteArray content = file.readAll();
        QString text = AppManagement::getScriptTextCodec()->toUnicode(content);
        file.close();

        clearAllBreakpoints();
        setPlainText("");
        setPlainText(text);

        changeFilename(fileName);

        QList<BookmarkItem> currentBookmarks = m_pBookmarkModel->getBookmarks(fileName);

        foreach(const BookmarkItem &item, currentBookmarks)
        {
            onBookmarkAdded(item);
        }

        QStringList watchedFiles = m_pFileSysWatcher->files();
        if (watchedFiles.size() > 0)
        {
            m_pFileSysWatcher->removePaths(watchedFiles);
        }
        m_pFileSysWatcher->addPath(m_filename);

        //!< check if BreakPointModel already contains breakpoints for this editor and load them
        if (getFilename() != "")
        {
            BreakPointModel *bpModel = PythonEngine::getInstance() ? PythonEngine::getInstance()->getBreakPointModel() : NULL;
            if (bpModel)
            {
                QModelIndexList modelIndexList = bpModel->getBreakPointIndizes(getFilename());
                QList<BreakPointItem> bpItems = bpModel->getBreakPoints(modelIndexList);

                for (int i=0; i<bpItems.size(); i++)
                {
                    breakPointAdd(bpItems.at(i), i);
                }
            }
        }

        setModified(false);

        // update the outline
        outlineTimerElapsed();

        QObject *seo = AppManagement::getScriptEditorOrganizer();
        if (seo)
        {
            QMetaObject::invokeMethod(seo, "fileOpenedOrSaved", Q_ARG(QString, m_filename));
        }
    }

    return RetVal(retOk);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::saveFile(bool askFirst)
{
    if (!isModified())
    {
        return RetVal(retOk);
    }

    if (this->getFilename().isNull())
    {
        return saveAsFile(askFirst);
    }

    if (askFirst)
    {
        int ret = QMessageBox::information(
            this, 
            tr("Unsaved Changes"), 
            tr("There are unsaved changes in the document '%1'. Do you want to save it first?").arg(getFilename()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes
        );

        if (ret & QMessageBox::Cancel)
        {
            return RetVal(retError);
        }
        else if (ret & QMessageBox::No)
        {
            return RetVal(retOk);
        }
    }

    m_pFileSysWatcher->removePath(getFilename());

    QFile file(getFilename());

    if (! file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error while accessing file"), tr("File %1 could not be accessed").arg(getFilename()));
        return RetVal(retError);
    }

    //todo
    //convertEols(QsciScintilla::EolUnix);
    
    QString t = toPlainText();
    file.write(AppManagement::getScriptTextCodec()->fromUnicode(t));
    file.close();
    QFileInfo fi(getFilename());

    if (fi.exists())
    {
        QObject *seo = AppManagement::getScriptEditorOrganizer();

        if (seo)
        {
            QMetaObject::invokeMethod(seo, "fileOpenedOrSaved", Q_ARG(QString, m_filename));
        }
    }

    setModified(false);

    m_pFileSysWatcher->addPath(getFilename());

    return RetVal(retOk);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::saveAsFile(bool askFirst)
{
    if (askFirst)
    {
        int ret = QMessageBox::information(this, tr("Unsaved Changes"), tr("There are unsaved changes in the current document. Do you want to save it first?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
        if (ret & QMessageBox::Cancel)
        {
            return RetVal(retError);
        }
        else if (ret & QMessageBox::No)
        {
            return RetVal(retOk);
        }
    }

    QString defaultPath = QDir::currentPath();
    QFile file;

    //we need to block the signals from the file system watcher, since a crash will occur if this file is renamed 
    //during the save as process (the 'remove file due to rename' dialog will appear during the save-as dialog if the signal is not blocked)
    m_pFileSysWatcher->blockSignals(true); 
    QString tempFileName = QFileDialog::getSaveFileName(this, tr("Save As..."), defaultPath, "Python (*.py)");
    m_pFileSysWatcher->blockSignals(false);
    if (!tempFileName.isEmpty())
    {
        QDir::setCurrent(QFileInfo(tempFileName).path());
        file.setFileName(tempFileName);
    }
    else
    {
        return RetVal(retError);
    }

    if (! file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error while accessing file"), tr("File %1 could not be accessed").arg(getFilename()));
        return RetVal(retError);
    }

    m_pFileSysWatcher->removePath(getFilename());

    //todo
    //convertEols(QsciScintilla::EolUnix);
    
    QString t = toPlainText();
    file.write(AppManagement::getScriptTextCodec()->fromUnicode(t));
    file.close();

    changeFilename(tempFileName);

    QFileInfo fi(getFilename());
    if (fi.exists())
    {
        QObject *seo = AppManagement::getScriptEditorOrganizer();
        if (seo)
        {
            QMetaObject::invokeMethod(seo, "fileOpenedOrSaved", Q_ARG(QString, m_filename));
        }
    }

    setModified(false);

    m_pFileSysWatcher->addPath(tempFileName);

    return RetVal(retOk);
}

//----------------------------------------------------------------------------------------------------------------------------------
//! slot invoked by pythonEnginge::pythonCodeCheck
/*!
    This function is automatically called to deliver the results of the syntax checker

    \sa triggerCodeChecker
*/
void ScriptEditorWidget::codeCheckResultsReady(QList<ito::CodeCheckerItem> codeCheckerItems)
{ 
    codeCheckerResultsChanged(codeCheckerItems);
}

//----------------------------------------------------------------------------------------------------------------------------------
//! Updates the list of code checker results if changes are available
/*!
    \param codeCheckerItems list of all current code checker result messages for this file
*/
void ScriptEditorWidget::codeCheckerResultsChanged(const QList<ito::CodeCheckerItem> &codeCheckerItems)
{ 
    //at first: remove all errors... from existing blocks
    foreach (TextBlockUserData *userData, textBlockUserDataList())
    {
        userData->m_checkerMessages.clear();
    }

    //2nd: add new errors...
    int lineNumber;
    TextBlockUserData *userData;

    foreach(const ito::CodeCheckerItem &item, codeCheckerItems)
    {
        lineNumber = item.lineNumber();

        if (lineNumber > 0)
        {
            userData = getTextBlockUserData(lineNumber - 1);

            if (userData)
            {
                userData->m_checkerMessages.append(item);
            }
        }
    }

    panels()->refresh();
}

//----------------------------------------------------------------------------------------------------------------------------------
//! Sends the code to the Syntax Checker
/*!
    This function is called to send the content of this ScriptEditorWidget to the syntax checker

    \sa codeCheckResultsReady
*/
void ScriptEditorWidget::triggerCodeChecker()
{
    PythonEngine *pyEng = qobject_cast<PythonEngine*>(AppManagement::getPythonEngine());
    if (pyEng && pyEng->pySyntaxCheckAvailable())
    {
        QString filename = getFilename();
        bool savedFile = (isModified() == false) && filename != "";
        QMetaObject::invokeMethod(pyEng, "pythonCodeCheck", 
            Q_ARG(QString, this->toPlainText()), 
            Q_ARG(QString, filename),
            Q_ARG(bool, savedFile),
            Q_ARG(QPointer<QObject>, QPointer<QObject>(this)), 
            Q_ARG(QByteArray, "codeCheckResultsReady"));
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
//! slot invoked by timer
/*!
    This slot is invoked by the timer to trigger the syntax check. The intervall is set in the option dialog.
    \sa codeCheckResultsReady, triggerCodeChecker
*/
void ScriptEditorWidget::updateSyntaxCheck()
{
    if (m_codeCheckerCallTimer)
    {
        m_codeCheckerCallTimer->stop();
        triggerCodeChecker();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
bool ScriptEditorWidget::event(QEvent *event)
{ 
    if (event->type() == QEvent::KeyRelease)
    {
        // SyntaxCheck   
        if (m_pythonExecutable && m_codeCheckerEnabled)
        {
            if (m_codeCheckerCallTimer)
            {
                m_codeCheckerCallTimer->start(); //starts or restarts the timer
            }
        }

        if (m_outlineTimerEnabled)
        {   
            // Class Navigator if Timer is active
            m_outlineTimer->start();
        }

        QKeyEvent *keyEvent = (QKeyEvent*)event;

        //check if a go back navigation item should be signalled
        //here: the cursor was moved by a mouse click, followed by a destructive action (backspace, delete...)
        if (m_cursorJumpLastAction && 
            !textCursor().isNull() && 
            ((keyEvent->key() == Qt::Key_Backspace) ||
                (keyEvent->key() == Qt::Key_Delete))
            )
        {
            reportGoBackNavigationCursorMovement(CursorPosition(textCursor(), m_uid), "DestructiveAfterJump");
        }

        m_cursorJumpLastAction = false;
    }
    else if (m_errorLineHighlighterMode && m_errorLineHighlighterMode->errorLineAvailable())
    {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyPress)
        {
            m_errorLineHighlighterMode->clearErrorLine();
        }
    }

    return AbstractCodeEditorWidget::event(event);
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_errorLineHighlighterMode->errorLineAvailable())
    {
        m_errorLineHighlighterMode->clearErrorLine();
    }

    AbstractCodeEditorWidget::mouseReleaseEvent(event);

    QTextCursor cursor = textCursor();

    if (!cursor.isNull())
    {
        m_cursorJumpLastAction = true;

        QTextCursor c = textCursor();

        int currentLine = c.isNull() ? -1 : c.blockNumber();

        if (currentLine != -1)
        {
            if (m_cursorBeforeMouseClick.cursor.isNull() || \
                m_cursorBeforeMouseClick.editorUID != m_uid || \
                std::abs(currentLine - m_cursorBeforeMouseClick.cursor.blockNumber()) > 11)
            {
                reportGoBackNavigationCursorMovement(CursorPosition(c, m_uid), "onMouseRelease");
            }
        }
    }

    m_cursorBeforeMouseClick.invalidate();
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::mousePressEvent(QMouseEvent *event)
{
    m_cursorBeforeMouseClick = currentGlobalEditorCursorPos; //make a local copy, since the global editor cursor pos is already changed before the release event

    AbstractCodeEditorWidget::mousePressEvent(event);
}

//----------------------------------------------------------------------------------------------------------------------------------
//!< bookmark handling
RetVal ScriptEditorWidget::toggleBookmark(int line)
{
    if (line < 0)
    {
        int index;
        getCursorPosition(&line, &index);
    }

    toggleBookmarkRequested(line);

    return RetVal(retOk);
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::toggleBookmarkRequested(int line)
{
    BookmarkItem item;
    QString filename = hasNoFilename() ? getUntitledName() : getFilename();
    item.filename = filename;
    item.lineIdx = line;
    item.enabled = true;

    if (m_pBookmarkModel->bookmarkExists(item))
    {
        m_pBookmarkModel->deleteBookmark(item);
    }
    else
    {
        m_pBookmarkModel->addBookmark(item);
    }
    
    
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::onBookmarkAdded(const BookmarkItem &item)
{
    QString filename = hasNoFilename() ? getUntitledName() : getFilename();

    if (QString::compare(item.filename, filename, m_filenameCaseSensitivity) == 0)
    {
        TextBlockUserData *userData = getTextBlockUserData(item.lineIdx);

        if (userData)
        {
            userData->m_bookmark = true;

            panels()->refresh();
        }
    }

    emit marginChanged();
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::onBookmarkDeleted(const BookmarkItem &item)
{
    QString filename = hasNoFilename() ? getUntitledName() : getFilename();

    if (QString::compare(item.filename, filename, m_filenameCaseSensitivity) == 0)
    {
        TextBlockUserData *userData = getTextBlockUserData(item.lineIdx);

        if (userData)
        {
            userData->m_bookmark = false;

            panels()->refresh();
        }
    }

    emit marginChanged();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Breakpoint Handling
//----------------------------------------------------------------------------------------------------------------------------------
bool ScriptEditorWidget::lineAcceptsBPs(int line)
{
    // Check if it's a blank or comment line 
    for (int i = 0; i < this->lineLength(line); ++i)
    {
        QChar c = this->lineText(line).at(i);
        if (c != '\t' && c != ' ' && c != '#' && c != '\n')
        { // it must be a character
            return true;
        }
        else if (this->lineText(line)[i] == '#' || i == this->lineLength(line)-1)
        { // up to now there have only been '\t'or' ' if there is a '#' now, return ORend of line reached an nothing found
            return false;
        }
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::toggleBreakpoint(int line)
{
    if (getFilename() == "") return RetVal(retError);

    //!< markerLine(handle) returns -1, if marker doesn't exist any more (because lines have been deleted...)
    std::list<QPair<int, int> >::iterator it;
    const PythonEngine *pyEngine = qobject_cast<PythonEngine*>(AppManagement::getPythonEngine());
    if (pyEngine)
    {
        BreakPointModel *bpModel = pyEngine->getBreakPointModel();
        QModelIndexList indexList = bpModel->getBreakPointIndizes(getFilename(), line);

        if (indexList.size() > 0)
        {
            bpModel->deleteBreakPoints(indexList);
        }
        else if (lineAcceptsBPs(line))
        {
            BreakPointItem bp;
            bp.filename = getFilename();
            bp.lineno = line;
            bp.conditioned = false;
            bp.condition = "";
            bp.enabled = true;
            bp.temporary = false;
            bp.ignoreCount = 0;
            bpModel->addBreakPoint(bp);
        }

        m_breakpointPanel->update();

        return RetVal(retOk);
    }

    return retError;
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::toggleEnableBreakpoint(int line)
{
    if (getFilename() == "") return RetVal(retError);

    const PythonEngine *pyEngine = qobject_cast<PythonEngine*>(AppManagement::getPythonEngine());
    if (pyEngine)
    {
        BreakPointModel *bpModel = pyEngine->getBreakPointModel();
        QModelIndexList indexList = bpModel->getBreakPointIndizes(getFilename(), line);
        BreakPointItem item;

        if (indexList.size() > 0)
        {
            for (int i = 0; i < indexList.size(); i++)
            {
                item = bpModel->getBreakPoint(indexList.at(i));
                item.enabled = !item.enabled;
                bpModel->changeBreakPoint(indexList.at(i), item);
            }

            m_breakpointPanel->update();
            return RetVal(retOk);
        }
    }

    return RetVal(retError);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::editBreakpoint(int line)
{
    if (getFilename() == "") return RetVal(retError);

    const PythonEngine *pyEngine = qobject_cast<PythonEngine*>(AppManagement::getPythonEngine());
    if (pyEngine)
    {
        BreakPointModel *bpModel = pyEngine->getBreakPointModel();
        QModelIndex index;
        BreakPointItem item;
        RetVal retValue(retOk);

        QTextBlock block = document()->findBlockByNumber(line);
        TextBlockUserData *tbud = dynamic_cast<TextBlockUserData*>(block.userData());
        if (block.isValid() && tbud && tbud->m_breakpointType != TextBlockUserData::TypeNoBp)
        {
            index = bpModel->getFirstBreakPointIndex(getFilename(), line);

            if (index.isValid())
            {
                item = bpModel->getBreakPoint(index);

                DialogEditBreakpoint *dlg = new DialogEditBreakpoint(item.filename, line + 1, item.enabled, item.temporary, item.ignoreCount, item.condition, this);
                dlg->setModal(true);
                dlg->exec();
                if (dlg->result() == QDialog::Accepted)
                {
                    dlg->getData(item.enabled, item.temporary, item.ignoreCount, item.condition);
                    item.conditioned = (item.condition != "") || (item.ignoreCount > 0) || item.temporary;

                    bpModel->changeBreakPoint(index, item);
                }

                DELETE_AND_SET_NULL(dlg);

                m_breakpointPanel->update();
                return RetVal(retOk);
            }
        }
    }

    return RetVal(retError);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::clearAllBreakpoints()
{
    if (getFilename() == "") 
    {
        return RetVal(retError);
    }

    BreakPointModel *bpModel = PythonEngine::getInstance() ? PythonEngine::getInstance()->getBreakPointModel() : NULL;

    if (bpModel)
    {
        bpModel->deleteBreakPoints(bpModel->getBreakPointIndizes(getFilename()));
    }

    m_breakpointPanel->update();

    return RetVal(retOk);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::gotoNextBreakPoint()
{
    int line, index;
    int breakPointLine = -1;
    getCursorPosition(&line, &index);

    line += 1;

    if (line == lineCount())
    {
        line = 0;
    }

    const QTextBlock &currentBlock = document()->findBlockByNumber(line);
    QTextBlock block = currentBlock;
    TextBlockUserData *tbud;

    //look from currentBlock to the end
    while (block.isValid())
    {
        tbud = dynamic_cast<TextBlockUserData*>(block.userData());
        if (tbud && tbud->m_breakpointType != TextBlockUserData::TypeNoBp)
        {
            breakPointLine = block.blockNumber();
            break;
        }
        block = block.next();
    }

    if (breakPointLine == -1)
    {
        //start from the beginning to currentBlock
        block = document()->firstBlock();

        while (block.isValid() && block != currentBlock)
        {
            tbud = dynamic_cast<TextBlockUserData*>(block.userData());
            if (tbud && tbud->m_breakpointType != TextBlockUserData::TypeNoBp)
            {
                breakPointLine = block.blockNumber();
                break;
            }
            block = block.next();
        }
    }

    if (breakPointLine >= 0)
    {
        setCursorPosAndEnsureVisible(breakPointLine);
        return RetVal(retOk);
    }

    return RetVal(retError);
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::gotoPreviousBreakPoint()
{
    int line, index;
    int breakPointLine = -1;
    getCursorPosition(&line, &index);

    if (line == 0)
    {
        line = lineCount()-1;
    }
    else
    {
        line -= 1;
    }

    const QTextBlock &currentBlock = document()->findBlockByNumber(line);
    QTextBlock block = currentBlock;
    TextBlockUserData *tbud;

    //look from currentBlock to the beginning
    while (block.isValid())
    {
        tbud = dynamic_cast<TextBlockUserData*>(block.userData());
        if (tbud && tbud->m_breakpointType != TextBlockUserData::TypeNoBp)
        {
            breakPointLine = block.blockNumber();
            break;
        }
        block = block.previous();
    }

    if (breakPointLine == -1)
    {
        //start from the end to currentBlock
        block = document()->lastBlock();

        while (block.isValid() && block != currentBlock)
        {
            tbud = dynamic_cast<TextBlockUserData*>(block.userData());
            if (tbud && tbud->m_breakpointType != TextBlockUserData::TypeNoBp)
            {
                breakPointLine = block.blockNumber();
                break;
            }
            block = block.previous();
        }
    }

    if (breakPointLine >= 0)
    {
        setCursorPosAndEnsureVisible(breakPointLine);
        return RetVal(retOk);
    }
    return RetVal(retError);
}

//----------------------------------------------------------------------------------------------------------------------------------
//!< slot, invoked by BreakPointModel
void ScriptEditorWidget::breakPointAdd(BreakPointItem bp, int /*row*/)
{
    int newHandle = -1;

#ifndef WIN32
    if (bp.filename != "" && bp.filename == getFilename())
#else
    if (bp.filename != "" && QString::compare(bp.filename, getFilename(), Qt::CaseInsensitive) == 0)
#endif
    {
        TextBlockUserData * tbud = getTextBlockUserData(bp.lineno, true);

        if (!tbud) //line does not exist
        {
            return;
        }

        if (tbud->m_breakpointType != TextBlockUserData::TypeNoBp)
        {
            return;//!< there is already a breakpoint in this line, do not add the new one
        }

        TextBlockUserData::BreakpointType markId;
        if (bp.enabled)
        {
            if (bp.conditioned)
            {
                markId = TextBlockUserData::TypeBpEdit;
            }
            else
            {
                markId =  TextBlockUserData::TypeBp;
            }
        }
        else
        {
            if (bp.conditioned)
            {
                markId = TextBlockUserData::TypeBpEditDisabled;
            }
            else
            {
                markId = TextBlockUserData::TypeBpDisabled;
            }
        }

        tbud->m_breakpointType = markId;

        m_breakpointPanel->update();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
//!< slot, invoked by BreakPointModel
void ScriptEditorWidget::breakPointDelete(QString filename, int lineNo, int /*pyBpNumber*/)
{
    bool found = false;

#ifndef WIN32
    if (filename != "" && filename == getFilename())
#else
    if (filename != "" && QString::compare(filename, getFilename(), Qt::CaseInsensitive) == 0)
#endif
    {
        TextBlockUserData *userData = getTextBlockUserData(lineNo, false);
        if (userData && userData->m_breakpointType != TextBlockUserData::TypeNoBp)
        {
            userData->m_breakpointType = TextBlockUserData::TypeNoBp;
            m_breakpointPanel->update();
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
//!< slot, invoked by BreakPointModel
void ScriptEditorWidget::breakPointChange(BreakPointItem oldBp, BreakPointItem newBp)
{
#ifndef WIN32
    if (oldBp.filename == getFilename())
#else
    if (QString::compare(oldBp.filename, getFilename(), Qt::CaseInsensitive) == 0)
#endif
    {
        breakPointDelete(oldBp.filename, oldBp.lineno, oldBp.pythonDbgBpNumber);
    }

#ifndef WIN32
    if (newBp.filename == getFilename())
#else
    if (QString::compare(newBp.filename, getFilename(), Qt::CaseInsensitive) == 0)
#endif
    {
        breakPointAdd(newBp, -1); //!< -1 has no task
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::print()
{
    if (lineCount() == 0 || toPlainText() == "")
    {
        QMessageBox::warning(this, tr("Print"), tr("There is nothing to print"));
    }
    else
    {
        ScriptEditorPrinter printer(QPrinter::HighResolution);
        printer.setPageSize(QPagedPaintDevice::A4);
        printer.setOrientation(QPrinter::Portrait);
        printer.setPageMargins(20, 15, 20, 15, QPrinter::Millimeter);

        if (hasNoFilename() == false)
        {
            printer.setDocName(getFilename());
        }
        else
        {
            printer.setDocName(tr("Unnamed"));
        }

        //todo
        //printer.setMagnification(-1); //size one point smaller than the one displayed in itom.

        QPrintPreviewDialog printPreviewDialog(&printer, this);
        printPreviewDialog.setWindowFlags(Qt::Window);
        connect(&printPreviewDialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printPreviewRequested(QPrinter*)));
        printPreviewDialog.exec();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::printPreviewRequested(QPrinter *printer)
{
    ScriptEditorPrinter *p = static_cast<ScriptEditorPrinter*>(printer);
    if (p)
    {
        p->printRange(this);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
RetVal ScriptEditorWidget::changeFilename(const QString &newFilename)
{
    QString oldFilename = getFilename();
    
    if (oldFilename.isNull())
    {
        if (newFilename == "" || newFilename.isNull())
        {
            m_filename = QString();
        }
        else
        {
            QFileInfo newFileInfo(newFilename);
            m_filename = newFileInfo.canonicalFilePath();
        }
    }
    else
    {
        BreakPointModel *bpModel = PythonEngine::getInstance() ? PythonEngine::getInstance()->getBreakPointModel() : NULL;
        QModelIndexList modelIndexList;

        if (newFilename == "" || newFilename.isNull())
        {
            if (bpModel)
            {
                modelIndexList = bpModel->getBreakPointIndizes(oldFilename);
                bpModel->deleteBreakPoints(modelIndexList);
            }

            m_filename = QString();
        }
        else
        {
            QFileInfo newFileInfo(newFilename);

            if (bpModel)
            {
                modelIndexList = bpModel->getBreakPointIndizes(oldFilename);
                QList<BreakPointItem> lists = bpModel->getBreakPoints(modelIndexList);
                BreakPointItem temp;
                QList<BreakPointItem> newList;

                for (int i = 0; i < lists.size(); i++)
                {
                    temp = lists.at(i);
                    temp.filename = newFileInfo.canonicalFilePath();
                    newList.push_back(temp);
                }

                bpModel->changeBreakPoints(modelIndexList, newList, false);
            }

            m_filename = newFileInfo.canonicalFilePath();
        }
    }

    //change bookmarks
    if (oldFilename == "")
    {
        oldFilename = getUntitledName();
    }

    QList<BookmarkItem> currentBookmarks = m_pBookmarkModel->getBookmarks(oldFilename);
    foreach(const BookmarkItem &item, currentBookmarks)
    {
        m_pBookmarkModel->changeBookmark(item, newFilename, item.lineIdx);
    }

    return RetVal(retOk);
}

//----------------------------------------------------------------------------------------------------------------------------------
/*virtual*/ bool ScriptEditorWidget::removeTextBlockUserData(TextBlockUserData* userData)
{
    if (CodeEditor::removeTextBlockUserData(userData))
    {
        if (userData->m_breakpointType != TextBlockUserData::TypeNoBp)
        {
            BreakPointModel *bpModel = PythonEngine::getInstance() ? PythonEngine::getInstance()->getBreakPointModel() : NULL;
            if (bpModel)
            {
                bpModel->deleteBreakPoint(bpModel->getFirstBreakPointIndex(getFilename(), userData->m_currentLineIdx));
            }
        }
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::nrOfLinesChanged()
{
    BreakPointModel *bpModel = PythonEngine::getInstance() ? PythonEngine::getInstance()->getBreakPointModel() : NULL;
    BookmarkModel *bmModel = m_pBookmarkModel;

    QTextBlock block = document()->firstBlock();
    TextBlockUserData *userData;
    QSet<TextBlockUserData*>::iterator it;
    QModelIndex index;
    ito::BreakPointItem item;
    QModelIndexList changedIndices;
    QList<ito::BreakPointItem> changedBpItems;
    QString filename = getFilename();
    QString bmFilename = filename != "" ? filename : getUntitledName();
    QList<BookmarkItem> currentBookmarks = bmModel->getBookmarks(bmFilename); //all bookmarks for this file
    QHash<int, int> bmLUT;

    for (int idx = 0; idx < currentBookmarks.size(); ++idx)
    {
        bmLUT[currentBookmarks[idx].lineIdx] = idx;
    }

    while (block.isValid())
    {
        userData = dynamic_cast<TextBlockUserData*>(block.userData());

        if (userData)
        {
            it = textBlockUserDataList().find(userData);

            if (it != textBlockUserDataList().end())
            {
                if (block.blockNumber() != userData->m_currentLineIdx)
                {
                    //the block number changed
                    if (bpModel && userData->m_breakpointType != TextBlockUserData::TypeNoBp)
                    {
                        index = bpModel->getFirstBreakPointIndex(filename, userData->m_currentLineIdx);
                        item = bpModel->getBreakPoint(index);
                        item.lineno = block.blockNumber(); //new line
                        changedIndices << index;
                        changedBpItems << item;
                    }

                    if (bmModel && userData->m_bookmark)
                    {
                        int idx = bmLUT.value(userData->m_currentLineIdx, -1);

                        if (idx >= 0)
                        {
                            bmModel->changeBookmark(currentBookmarks[idx], bmFilename, block.blockNumber());
                            bmLUT[userData->m_currentLineIdx] = -1; //handled
                        }
                    }

                    userData->m_currentLineIdx = block.blockNumber();
                }
                else if (bmLUT.value(userData->m_currentLineIdx, -1) >= 0)
                {
                    bmLUT[userData->m_currentLineIdx] = -1; //handled
                }
            }
        }
        block = block.next();
    }

    if (changedIndices.size() > 0 && bpModel)
    {
        bpModel->changeBreakPoints(changedIndices, changedBpItems);
    }

    //check if there are still values in the bookmark hash, that have not been handled yet. If so, delete them
    QHash<int, int>::const_iterator bmLUTit = bmLUT.constBegin();

    while (bmLUTit != bmLUT.constEnd())
    {
        if (bmLUTit.value() >= 0)
        {
            bmModel->deleteBookmark(BookmarkItem(bmFilename, bmLUTit.key()));
        }
        ++bmLUTit;
    }

    // SyntaxCheck   
    if (m_pythonExecutable && m_codeCheckerEnabled)
    {
        if (m_codeCheckerCallTimer)
        {
            m_codeCheckerCallTimer->start(); //starts or restarts the timer
        }
    }
    if (m_outlineTimerEnabled)
    {
        m_outlineTimer->start(); //starts or restarts the timer
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::pythonDebugPositionChanged(QString filename, int lineno)
{
    if (!hasNoFilename() && (QFileInfo(filename) == QFileInfo(getFilename())))
    {
        m_breakpointPanel->setCurrentLine(lineno - 1);
        ensureLineVisible(lineno-1);
        raise();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
//void ScriptEditorWidget::pythonCodeExecContinued()
//{
//    if (markCurrentLineHandle != -1)
//    {
//        markerDeleteHandle(markCurrentLineHandle);
//    }
//}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::pythonStateChanged(tPythonTransitions pyTransition)
{
    switch(pyTransition)
    {
    case pyTransBeginRun:
    case pyTransBeginDebug:
        if (!hasNoFilename())
        {
            setReadOnly(true);
        }
        m_pythonBusy = true;
        m_pythonExecutable = false;
        break;
    case pyTransDebugContinue:
        m_breakpointPanel->removeAllLineSelectors();
        m_pythonExecutable = false;
        break;
    case pyTransEndRun:
    case pyTransEndDebug:
        setReadOnly(false);
        m_breakpointPanel->removeAllLineSelectors();
        m_pythonBusy = false;
        m_pythonExecutable = true;
        break;
    case pyTransDebugWaiting:
        m_pythonExecutable = true;
        break;
    case pyTransDebugExecCmdBegin:
        m_pythonExecutable = false;
        break;
    case pyTransDebugExecCmdEnd:
        m_pythonExecutable = true;
        break;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void ScriptEditorWidget::fileSysWatcherFileChanged(const QString &path) //this signal may be emitted multiple times at once for the same file, therefore the mutex protection is introduced
{
    if (m_fileSystemWatcherMutex.tryLock(1))
    {
        QMessageBox msgBox(this);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        if (path == getFilename())
        {
            QFile file(path);

            if (!file.exists())
            {
                //if git updates a file, the file is deleted and then the modified file is created.
                //this will cause a 'delete' notification, however the 'modified' notification would be correct.
                //try to sleep for a while and recheck the state of the file again...
                QThread::sleep(0.4);
            }

            if (!file.exists()) //file deleted
            {
                msgBox.setText(tr("The file '%1' does not exist any more.").arg(path));
                msgBox.setInformativeText(tr("Keep this file in editor?"));

                int ret = msgBox.exec();

                if (ret == QMessageBox::No)
                {
                    emit closeRequest(this, true);
                }
                else
                {
                    document()->setModified(true);
                }
            }
            else //file changed
            {    
                QCryptographicHash fileHash(QCryptographicHash::Sha1);
                file.open(QIODevice::ReadOnly | QIODevice::Text);
                fileHash.addData(file.readAll());

                QCryptographicHash fileHash2(QCryptographicHash::Sha1);
                fileHash2.addData(toPlainText().toLatin1());

                //fileModified = !(QLatin1String(file.readAll()) == text()); //does not work!?
                
                if (!(fileHash.result() == fileHash2.result())) //does not ask user in the case of same texts
                {
                    msgBox.setText(tr("The file '%1' has been modified by another program.").arg(path));
                    msgBox.setInformativeText(tr("Do you want to reload it?"));
                    int ret = msgBox.exec();

                    if (ret == QMessageBox::Yes)
                    {
                        openFile(path, true);
                    }
                    else
                    {
                        document()->setModified(true);
                    }
                }
                
            }
        }

        m_fileSystemWatcherMutex.unlock();
    }
}

//-------------------------------------------------------------------------------------
// Outline
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
QSharedPointer<OutlineItem> ScriptEditorWidget::checkBlockForOutlineItem(
    int startLineIdx, 
    int endLineIdx) const
{
    int lineIndex = startLineIdx;

    if (endLineIdx >= lineCount())
    {
        endLineIdx = lineCount() - 1;
    }

    // read from settings
    QString line = lineText(startLineIdx).trimmed();

    if (line.endsWith('\\'))
    {
        line = line.left(line.size() - 1);
    }

    QString decoLine;   // @-Decorato(@)r Line in previous line of a function

    //auto classMatch = m_regExpClass.match("class MyClass:  # comment");
    //qDebug() << classMatch.hasMatch() << classMatch.hasPartialMatch() << classMatch.capturedTexts();
        
    auto classMatch = m_regExpClass.match(line);

    if (classMatch.hasMatch())
    {
        QSharedPointer<OutlineItem> item(new OutlineItem(OutlineItem::typeClass));
        item->m_name = classMatch.captured("name");
        item->m_startLineIdx = startLineIdx;
        item->m_endLineIdx = endLineIdx;
        item->m_private = item->m_name.startsWith("_");
        item->m_async = false;
        return item;
    }

    /*auto methodMatch = m_regExpMethod.match("   def test():");
    qDebug() << methodMatch.hasMatch() << methodMatch.hasPartialMatch() << methodMatch.capturedTexts();

    methodMatch = m_regExpMethod.match("   def __sdfsdf__(self, a=[]): # comment");
    qDebug() << methodMatch.hasMatch() << methodMatch.hasPartialMatch() << methodMatch.capturedTexts();

    methodMatch = m_regExpMethod.match("   async   def _sdf(clicked):   ");
    qDebug() << methodMatch.hasMatch() << methodMatch.hasPartialMatch() << methodMatch.capturedTexts();

    methodMatch = m_regExpMethod.match("   async   def _sdf(clicked, test: bla) -> Optional[int]:  # sdf ");
    qDebug() << methodMatch.hasMatch() << methodMatch.hasPartialMatch() << methodMatch.capturedTexts();*/

    auto methodMatch = m_regExpMethodStart.match(line);

    if (methodMatch.hasMatch())
    {
        //it can be that the def signature is spread over multiple lines.
        //then iteratively add more lines to the possible text and try
        //it again (max. 30 lines).
        methodMatch = m_regExpMethod.match(line);

        

        while (!methodMatch.hasMatch() &&
            lineIndex <= endLineIdx)
        {
            QString linenew = lineText(++lineIndex).trimmed();

            if (linenew.endsWith('\\'))
            {
                linenew = linenew.left(linenew.size() - 1);
            }

            line = line + linenew + " ";
            methodMatch = m_regExpMethod.match(line);
        }

        if (methodMatch.hasMatch())
        {
            QSharedPointer<OutlineItem> item(new OutlineItem(OutlineItem::typeFunction));
            item->m_name = methodMatch.captured("name");
            item->m_private = item->m_name.startsWith("_");
            item->m_args = methodMatch.captured(5).trimmed();
            bool hasSelf = item->m_args.startsWith("self", Qt::CaseInsensitive);
            item->m_returnType = methodMatch.captured(7);
            item->m_startLineIdx = startLineIdx;
            item->m_endLineIdx = endLineIdx;
            item->m_async = methodMatch.captured("async")
                .trimmed()
                .startsWith("async", Qt::CaseInsensitive) 
                ? true : false;

            // check for decorator
            if (startLineIdx > 0)
            {
                decoLine = lineText(startLineIdx - 1);
                auto decoMatch = m_regExpDecorator.match(decoLine);

                if (decoMatch.hasMatch())
                {
                    QString decorator = decoMatch.captured("name").trimmed().toLower();

                    if (decorator == "staticmethod")
                    {
                        item->m_type = OutlineItem::typeStaticMethod;
                    }
                    else if (decorator == "classmethod")
                    {
                        item->m_type = OutlineItem::typeClassMethod;
                    }
                    else if (decorator == "property")
                    {
                        item->m_type = OutlineItem::typePropertyGet;
                    }
                    else if (decorator.endsWith(".setter"))
                    {
                        item->m_type = OutlineItem::typePropertySet;
                    }
                    else
                    {
                        item->m_type = hasSelf ? OutlineItem::typeMethod : OutlineItem::typeFunction;
                    }
                }
                else
                {
                    item->m_type = hasSelf ? OutlineItem::typeMethod : OutlineItem::typeFunction;
                }
            }
            else
            {
                item->m_type = hasSelf ? OutlineItem::typeMethod : OutlineItem::typeFunction;
            }

            return item;
        }
    }

    return QSharedPointer<OutlineItem>(); // invalid item
}

//-------------------------------------------------------------------------------------
void ScriptEditorWidget::parseOutlineRecursive(QSharedPointer<OutlineItem> &parent) const
{
    const QTextDocument* doc = document();
    bool valid;

    for (int blockIdx = parent->m_startLineIdx + 1; blockIdx <= parent->m_endLineIdx; ++blockIdx)
    {
        const QTextBlock &block = doc->findBlockByNumber(blockIdx);

        if (Utils::TextBlockHelper::isFoldTrigger(block))
        {
            FoldScope scope(block, valid);

            if (valid)
            {
                auto range = scope.getRange();
                QSharedPointer<OutlineItem> item = checkBlockForOutlineItem(range.first, range.second);

                if (!item.isNull())
                {
                    parseOutlineRecursive(item);
                    item->m_parent = parent.toWeakRef();
                    parent->m_childs.append(item);
                    blockIdx = range.second;
                }
            }
        }
    }
}

//-------------------------------------------------------------------------------------
QSharedPointer<OutlineItem> ScriptEditorWidget::parseOutline() const
{
    const QTextDocument* doc = document();
    bool valid;

    QSharedPointer<OutlineItem> root(new OutlineItem(OutlineItem::typeRoot));

    for (int blockIdx = 0; blockIdx < blockCount(); ++blockIdx)
    {
        const QTextBlock &block = doc->findBlockByNumber(blockIdx);

        if (Utils::TextBlockHelper::isFoldTrigger(block))
        {
            FoldScope scope(block, valid);

            if (valid)
            {
                auto range = scope.getRange();

                QSharedPointer<OutlineItem> item = checkBlockForOutlineItem(range.first, range.second);

                if (!item.isNull())
                {
                    parseOutlineRecursive(item);
                    item->m_parent = root.toWeakRef();
                    root->m_childs.append(item);
                    blockIdx = range.second;
                }
            }
        }
    }

    return root;
}


//-------------------------------------------------------------------------------------
/* slot called if the outline timer expired. This is the case if the document has
been changed a very short time ago. Therefore, the outline has to be parsed again
and sent to the script dock widget. 

This method is also directly called if a new file is opened in this editor.
*/
void ScriptEditorWidget::outlineTimerElapsed()
{
    m_outlineTimer->stop();
    m_rootOutlineItem = parseOutline();

    emit outlineModelChanged(this, m_rootOutlineItem);
}

//----------------------------------------------------------------------------------
void ScriptEditorWidget::dumpFoldsToConsole(bool)
{
    int lvl;
    bool trigger;
    bool valid;

    QTextBlock block = document()->firstBlock();

    std::cout << "block foldings:\n" << std::endl;

    while (block.isValid())
    {
        lvl = Utils::TextBlockHelper::getFoldLvl(block);
        trigger = Utils::TextBlockHelper::isFoldTrigger(block);
        QTextBlock tb = FoldScope::findParentScope(block);

        if (1 || trigger)
        {
            std::cout << QString(4 * lvl, ' ').toLatin1().constData() << "Block " << block.blockNumber() + 1 << ": lvl " << lvl << ", trigger: " << trigger << 
                " parent: valid: " << tb.isValid() << ", nr: " << tb.blockNumber() + 1 << "\n" << std::endl;
        }

        if (trigger)
        {
            FoldScope scope(block, valid);
            QPair<int,int> range = scope.getRange();
            

            std::cout << QString(4 * lvl, ' ').toLatin1().constData() << " --> [" << range.first+1 << 
                "-" << range.second+1 << "] << ," << scope.scopeLevel() << ", " << scope.triggerLevel() << 
                " parent: valid: " << tb.isValid() << ", nr: " << tb.blockNumber() + 1 << "\n" << std::endl;

        }
        
        block = block.next();
    }  
}

//-----------------------------------------------------------
void ScriptEditorWidget::reportGoBackNavigationCursorMovement(const CursorPosition &cursor, const QString &origin) const
{
    int lineIndex = cursor.cursor.blockNumber();

    if (lineIndex >= 0)
    {
        GoBackNavigationItem item;

        item.column = cursor.cursor.columnNumber();
        item.line = lineIndex;
        item.UID = cursor.editorUID;

        if (item.UID == -1)
        {
            item.UID = m_uid;
        }

        item.origin = origin;

        ScriptEditorWidget* editor = editorByUID.value(item.UID, NULL);

        if (editor)
        {
            item.filename = editor->getFilename();
            
            QString shortText = editor->lineText(lineIndex).trimmed();

            if (shortText.size() > 40)
            {
                shortText = shortText.left(37) + "...";
            }

            item.shortText = shortText;
        }
        else
        {
            return;
        }

        /*if (item.filename != "")
        {
            std::cout << "addGoBackNavigationItem: " << item.origin.toLatin1().data() << "::" << 
                item.filename.toLatin1().data() << "(" << lineIndex + 1 << ":" << item.column << ")\n" << std::endl;
        }
        else
        {
            std::cout << "addGoBackNavigationItem: " << item.origin.toLatin1().data() << "::Untitled" <<
                item.UID << "(" << lineIndex + 1 << ":" << item.column << ")\n" << std::endl;
        }*/

        emit const_cast<ScriptEditorWidget*>(this)->addGoBackNavigationItem(item);
    }
}

//-----------------------------------------------------------
void ScriptEditorWidget::reportCurrentCursorAsGoBackNavigationItem(const QString &reason, int UIDFilter /*= -1*/)
{
    if (!currentGlobalEditorCursorPos.cursor.isNull() && \
        (UIDFilter == -1 || currentGlobalEditorCursorPos.editorUID == UIDFilter))
    {
        reportGoBackNavigationCursorMovement(currentGlobalEditorCursorPos, reason);
        currentGlobalEditorCursorPos.invalidate();
    }
}

//-----------------------------------------------------------
void ScriptEditorWidget::onCursorPositionChanged()
{
    QTextCursor c = textCursor();
    int currentLine = c.isNull() ? -1 : c.blockNumber();

    if (currentLine != m_currentLineIndex)
    {
        emit outlineModelChanged(this, m_rootOutlineItem);
        m_currentLineIndex = currentLine;
    }

    // set the current cursor position to the global cursor position variable
    //if (hasFocus())
    {
        currentGlobalEditorCursorPos.cursor = c;
        currentGlobalEditorCursorPos.editorUID = m_uid;
    }
}

//-----------------------------------------------------------
/*static*/ QString ScriptEditorWidget::filenameFromUID(int UID, bool &found)
{
    ScriptEditorWidget *sew = editorByUID.value(UID, NULL);

    if (sew)
    {
        found = true;
        return sew->getFilename();
    }
    else
    {
        found = false;
        return QString();
    }
}

//-------------------------------------------------------------------------------------
void ScriptEditorWidget::tabChangeRequest()
{
    emit tabChangeRequested();
}

} // end namespace ito
