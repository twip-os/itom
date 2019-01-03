/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2018, Institut fuer Technische Optik (ITO),
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

#include "abstractCodeEditorWidget.h"
#include "../global.h"
#include "../AppManagement.h"
#include "../helper/guiHelper.h"
#include "../codeEditor/foldDetector/indentFoldDetector.h"
#include "../codeEditor/syntaxHighlighter/pythonSyntaxHighlighter.h"
#include "../codeEditor/modes/occurrences.h"
#include "../codeEditor/managers/modesManager.h"
#include "../codeEditor/modes/pyAutoIndent.h"
#include "../codeEditor/modes/indenter.h"
#include "../codeEditor/syntaxHighlighter/codeEditorStyle.h"

#include <qstring.h>
#include <qsettings.h>
#include <qdebug.h>
#include <qcolor.h>
#include <qfont.h>
#include <qtooltip.h>



namespace ito {

//----------------------------------------------------------------------------------------------------------------------------------
AbstractCodeEditorWidget::AbstractCodeEditorWidget(QWidget* parent) :
    CodeEditor(parent),
    m_userSelectionState(selNo)
{
    init();
    reloadSettings();

    connect(AppManagement::getMainApplication(), SIGNAL(propertiesChanged()), this, SLOT(reloadSettings()));
}

//----------------------------------------------------------------------------------------------------------------------------------
AbstractCodeEditorWidget::~AbstractCodeEditorWidget()
{
}

//----------------------------------------------------------------------------------------------------------------------------------
void AbstractCodeEditorWidget::init()
{
    m_editorStyle = QSharedPointer<CodeEditorStyle>(new CodeEditorStyle());
    //add python syntax highlighter
    m_pythonSyntaxHighlighter = QSharedPointer<SyntaxHighlighterBase>(new PythonSyntaxHighlighter(document(), "PythonSyntaxHighlighter", m_editorStyle));
    m_pythonSyntaxHighlighter->setFoldDetector(QSharedPointer<FoldDetector>(new IndentFoldDetector()));
    modes()->append(m_pythonSyntaxHighlighter.dynamicCast<Mode>());

    OccurrencesHighlighterMode *occHighlighterMode = new OccurrencesHighlighterMode("OccurrencesHighlighterMode");
    occHighlighterMode->setBackground(Qt::green);
    occHighlighterMode->setCaseSensitive(true);
    occHighlighterMode->setSelectOnDoubleClick(true);
    occHighlighterMode->setDelay(100);
    modes()->append(Mode::Ptr(occHighlighterMode));

    m_codeCompletionMode = QSharedPointer<CodeCompletionMode>(new CodeCompletionMode("CodeCompletionMode"));
    modes()->append(Mode::Ptr(m_codeCompletionMode.dynamicCast<Mode>()));

    m_calltipsMode = QSharedPointer<PyCalltipsMode>(new PyCalltipsMode("CalltipsMode"));
    modes()->append(Mode::Ptr(m_calltipsMode.dynamicCast<Mode>()));

    m_pyAutoIndentMode = QSharedPointer<PyAutoIndentMode>(new PyAutoIndentMode("PyAutoIndentMode"));
    modes()->append(Mode::Ptr(m_pyAutoIndentMode.dynamicCast<Mode>()));

    modes()->append(Mode::Ptr(new IndenterMode("IndenterMode")));

    m_symbolMatcher = QSharedPointer<SymbolMatcherMode>(new SymbolMatcherMode("SymbolMatcherMode"));
    modes()->append(m_symbolMatcher.dynamicCast<Mode>());

    m_caretLineHighlighter = QSharedPointer<CaretLineHighlighterMode>(new CaretLineHighlighterMode("CaretLineHighlighterMode"));
    modes()->append(m_caretLineHighlighter.dynamicCast<Mode>());
}

//----------------------------------------------------------------------------------------------------------------------------------
void AbstractCodeEditorWidget::loadSettings()
{
    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("CodeEditor");

    bool updateSyntaxHighlighter = false;

    CodeEditorStyle defaultStyle;

    // ------------ general  --------------------------------------------------------

    //TODO:
    /*QString eolMode = settings.value("eolMode", "EolUnix").toString();

    if (eolMode == "EolUnix")
    {
        setEolMode(QsciScintilla::EolUnix);
    }
    else if (eolMode == "EolWindows")
    {
        setEolMode(QsciScintilla::EolWindows);
    }
    else
    {
        setEolMode(QsciScintilla::EolMac);
    }*/

    QSharedPointer<PyAutoIndentMode> pyAutoIndentMode = modes()->get("PyAutoIndentMode").dynamicCast<PyAutoIndentMode>();
    if (pyAutoIndentMode)
    {
        pyAutoIndentMode->setEnabled(settings.value("autoIndent", true).toBool()); //auto indentation
    }

    setUseSpacesInsteadOfTabs(!settings.value("indentationUseTabs", false).toBool()); //tabs (true) or whitespace (false)
    setTabLength(settings.value("indentationWidth", 4).toInt()); //numbers of whitespaces   
    setShowIndentationGuides(settings.value("showIndentationGuides", true).toBool());

    //TODO
    //spacing above and below each line
    //setExtraAscent(settings.value("extraAscent", 0).toInt());
    //setExtraDescent(settings.value("extraDescent", 0).toInt());

    //TODO
    /*
    QString indentationWarning = settings.value("indentationWarning", "Inconsistent").toString();

    if (eolMode == "Inconsistent")
    {
        qSciLex->setIndentationWarning(QsciLexerPython::Inconsistent);
    }
    else if (eolMode == "NoWarning")
    {
        qSciLex->setIndentationWarning(QsciLexerPython::NoWarning);
    }
    else if (eolMode == "TabsAfterSpaces")
    {
        qSciLex->setIndentationWarning(QsciLexerPython::TabsAfterSpaces);
    }
    else if (eolMode == "Spaces")
    {
        qSciLex->setIndentationWarning(QsciLexerPython::Spaces);
    }
    else //Tabs
    {
        qSciLex->setIndentationWarning(QsciLexerPython::Tabs);
    }*/

    // ------------ calltips --------------------------------------------------------
    m_calltipsMode->setEnabled(settings.value("calltipsEnabled",true).toBool());

    // ------------ auto completion --------------------------------------------------------
    m_codeCompletionMode->setEnabled(settings.value("autoComplEnabled", true).toBool());
    m_codeCompletionMode->setCaseSensitive(settings.value("autoComplCaseSensitive", true).toBool());
    m_codeCompletionMode->setTriggerLength(settings.value("autoComplThreshold", 2).toInt());
    m_codeCompletionMode->setShowTooltips(settings.value("autoComplShowTooltips", false).toBool());
    m_codeCompletionMode->setFilterMode((ito::CodeCompletionMode::FilterMode)settings.value("autoComplFilterMode", CodeCompletionMode::FilterFuzzy).toInt());

    // --------------- styles ------------------------------------------------------------

    if (m_pythonSyntaxHighlighter)
    {
        setBackground(QColor(settings.value("paperBackgroundColor", QColor(Qt::white)).toString()));
        m_pythonSyntaxHighlighter->editorStyle()->setBackground(QColor(settings.value("paperBackgroundColor", QColor(Qt::white)).toString()));

        //TODO
        //setMarginsBackgroundColor(QColor(settings.value("marginBackgroundColor", QColor(224,224,224)).toString()));
        //setMarginsForegroundColor(QColor(settings.value("marginForegroundColor", QColor(0, 0, 0)).toString()));
        //setFoldMarginColors(QColor(settings.value("foldMarginForegroundColor", QColor(233,233,233)).toString()), \
        //    QColor(settings.value("foldMarginBackgroundColor", QColor(Qt::white)).toString()));

        QTextCharFormat keyWhitespaceFormat = m_pythonSyntaxHighlighter->editorStyle()->format(StyleItem::KeyWhitespace);

        if (keyWhitespaceFormat.background() != m_pythonSyntaxHighlighter->editorStyle()->background())
        {
            m_pythonSyntaxHighlighter->editorStyle()->rformat(StyleItem::KeyWhitespace).setBackground(m_pythonSyntaxHighlighter->editorStyle()->background()); //invalid color -> default from lexer is user! //setWhitespaceBackgroundColor(QColor()); 
            updateSyntaxHighlighter = true;
        }

        if (keyWhitespaceFormat.foreground().color() != QColor(settings.value("whitespaceForegroundColor", QColor(Qt::black)).toString()))
        {
            m_pythonSyntaxHighlighter->editorStyle()->rformat(StyleItem::KeyWhitespace).setForeground(QColor(settings.value("whitespaceForegroundColor", QColor(Qt::black)).toString()));
            updateSyntaxHighlighter = true;
        }
    }

    if (m_symbolMatcher)
    {
        m_symbolMatcher->setMatchBackground(QColor(settings.value("matchedBraceBackgroundColor", QColor(Qt::white)).toString()));
        m_symbolMatcher->setMatchForeground(QColor(settings.value("matchedBraceForegroundColor", QColor(Qt::red)).toString()));

        m_symbolMatcher->setUnmatchBackground(QColor(settings.value("unmatchedBraceBackgroundColor", QColor(Qt::white)).toString()));
        m_symbolMatcher->setUnmatchForeground(QColor(settings.value("unmatchedBraceForegroundColor", QColor(Qt::red)).toString()));
    }

    m_caretLineHighlighter->setBackground(QColor(settings.value("caretBackgroundColor", QColor(Qt::white)).toString()));
    m_caretLineHighlighter->setEnabled(settings.value("caretBackgroundShow", false).toBool());
    //todo
    setForeground(QColor(settings.value("caretForegroundColor", QColor(Qt::black)).toString())); //caret color


    Mode::Ptr mode = modes()->get("OccurrencesHighlighterMode");
    if (mode)
    {
        OccurrencesHighlighterMode* occHighlighterMode = (OccurrencesHighlighterMode*)(mode.data());
        occHighlighterMode->setBackground(QColor(settings.value("markerSameStringBackgroundColor", QColor(Qt::green)).toString()));
    }

    setSelectionBackground(QColor(settings.value("selectionBackgroundColor", QColor(51, 153, 255)).toString()));
    setSelectionForeground(QColor(settings.value("selectionForegroundColor", QColor(Qt::white)).toString()));

    settings.endGroup();

    // ------------ styles ---------------------------------------------------------------
    //set font for line numbers (equal to font of default style number)


    //TODO
    //QFont marginFont = qSciLex->font(qSciLex->defaultStyle());
    //setMarginsFont(marginFont);

    QTextCharFormat defaultFormat;
    QTextCharFormat currentFormat;
    

    foreach (StyleItem::StyleType styleType, StyleItem::availableStyleTypes())
    {
        if (styleType == StyleItem::KeyWhitespace)
        {
            continue; //this will be handled separately
        }

        StyleItem &item = m_editorStyle->at(styleType);
        defaultFormat =  defaultStyle[styleType].format();
        currentFormat = item.format();

        if (item.isValid())
        {
            settings.beginGroup("PythonLexerStyle" + QString().setNum(item.type()));

            QColor bgColor = settings.value("backgroundColor", background()).toString();
            if (bgColor.isValid())
            {
                bgColor.setAlpha(settings.value("backgroundColorAlpha", 255).toInt());
                if (currentFormat.background().color() != bgColor)
                {
                    item.rformat().setBackground(bgColor);
                    updateSyntaxHighlighter = true;
                }
            }

            QColor fgColor = settings.value("foregroundColor", defaultFormat.foreground().color()).toString();
            if (fgColor.isValid())
            {
                fgColor.setAlpha(settings.value("foregroundColorAlpha", 255).toInt());
                
                if (currentFormat.foreground().color() != fgColor)
                {
                    item.rformat().setForeground(fgColor);
                    updateSyntaxHighlighter = true;
                }                
            }

            QString fontFamily = settings.value("fontFamily", "").toString();
            if (fontFamily != "")
            {
                if (currentFormat.fontFamily() != fontFamily)
                {
                    item.rformat().setFontFamily(fontFamily);
                    updateSyntaxHighlighter = true;
                }
            }

            int fontPointSize = settings.value("pointSize", 0).toInt();
            if (fontPointSize > 0)
            {
                if (currentFormat.fontPointSize() != fontPointSize)
                {
                    item.rformat().setFontPointSize(fontPointSize);
                    updateSyntaxHighlighter = true;
                }
            }

            int fontWeight = settings.value("weight", defaultFormat.fontWeight()).toInt();
            if (currentFormat.fontWeight() != fontWeight)
            {
                item.rformat().setFontWeight(fontWeight);
                updateSyntaxHighlighter = true;
            }

            bool fontItalic = settings.value("italic", defaultFormat.fontItalic()).toBool();
            if (currentFormat.fontItalic() != fontItalic)
            {
                item.rformat().setFontItalic(fontItalic);
                updateSyntaxHighlighter = true;
            }

            settings.endGroup();
        }

        if (styleType == StyleItem::KeyDefault)
        {
            currentFormat = item.format();

            if (item.isValid())
            {
                //set font of whitespace to default
                QTextCharFormat &whitespaceFormat = m_editorStyle->rformat(ito::StyleItem::KeyWhitespace);
                if (whitespaceFormat.font() != currentFormat.font())
                {
                    whitespaceFormat.setFont(currentFormat.font());
                    updateTabStopAndIndentationWidth();
                    updateSyntaxHighlighter = true;
                }

                if (fontName() != currentFormat.fontFamily())
                {
                    setFontName(currentFormat.fontFamily());
                    updateSyntaxHighlighter = true;
                }
                if (fontSize() != currentFormat.fontPointSize())
                {
                    setFontSize(currentFormat.fontPointSize());
                    updateSyntaxHighlighter = true;
                }
            }
            else
            {
                if (fontName() != defaultFormat.fontFamily())
                {
                    setFontName(defaultFormat.fontFamily());
                    updateSyntaxHighlighter = true;
                }
                if (fontSize() != defaultFormat.fontPointSize())
                {
                    setFontSize(defaultFormat.fontPointSize());
                    updateSyntaxHighlighter = true;
                }
            }
        }
    }

    if (updateSyntaxHighlighter && m_pythonSyntaxHighlighter)
    {
        m_pythonSyntaxHighlighter->refreshEditor(m_editorStyle);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
QPixmap AbstractCodeEditorWidget::loadMarker(const QString &name, int sizeAt96dpi)
{
    int dpi = GuiHelper::getScreenLogicalDpi();
    QPixmap px(name);
    if (dpi != 96 || px.height() != sizeAt96dpi)
    {
        int newSize = sizeAt96dpi * dpi / 96;
        px = px.scaled(newSize, newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return px;
}

//----------------------------------------------------------------------------------------------------------------------------------
QString AbstractCodeEditorWidget::getWordAtPosition(const int &line, const int &index)
{
    return wordAtPosition(line, index, true);
}

//----------------------------------------------------------------------------------------------------------------------------------
//! counts the numbers of leading tabs or spaces of a string
/*!
    \return number of leading tabs or spaces
*/
int AbstractCodeEditorWidget::getSpaceTabCount(const QString &s)
{
    int res = 0;
    if (s.mid(res, 1).indexOf(QRegExp("[\t]")) > -1 || s.mid(res, 1) == " ")
    {
        do
        {
            ++res;
        }
        while (s.mid(res, 1).indexOf(QRegExp("[\t]")) > -1 || s.mid(res, 1) == " ");
    }

    return res;
}

//----------------------------------------------------------------------------------------------------------------------------------
//! checks if text line contains a colon sign as last valid character (only comments or spaces are allowed after the colon)
/*!
    This method is necessary in order to verify if the following text lines must be indented with respect
    to this line in Python syntax.

    \return true if colon is last valid sign, else false
*/
bool AbstractCodeEditorWidget::haveToIndention(QString s)
{
    s = s.trimmed();
    s.replace("'''", "\a");
    s.replace("\"\"\"", "\a");
    int count1 = s.count("\a");
    int count2 = s.count("#");

    if (count1 + count2 > 0)
    {
        if (count1 == 0)
        {
            s = s.mid(1, s.indexOf("#"));
        }
        else if (count2 == 0)
        {
            bool comment = (count1 % 2 == 1);
            if (comment)
            {
                s = s.mid(1, s.lastIndexOf("\a") - 1);
                s = s.trimmed();
                --count1;
            }

            while (count1 > 0)
            {
                int pos1 = s.indexOf("\a");
                int pos2 = pos1 + 1;
                while (s.mid(pos2, 1) != "\a")
                {
                    ++pos2;
                }
                s = s.mid(0, pos1) + s.mid(pos2 + 1);
                --count1;
                --count1;
            }
        }
        else
        {
            s = s.mid(1, s.indexOf("#"));
            s = s.trimmed();

            bool comment = ((count1 & 2) == 1);
            if (comment)
            {
                s = s.mid(1, s.lastIndexOf("\a"));
                s = s.trimmed();
                --count1;
            }

            while (count1 > 0)
            {
                int pos1 = s.indexOf("\a");
                int pos2 = pos1 + 1;
                while (s.mid(pos2, 1) != "\a")
                {
                    ++pos2;
                }
                s = s.mid(1, pos1) + s.mid(pos2 + 1);
                --count1;
                --count1;
            }
        }
    }

    s = s.trimmed();
    return s.mid(s.size() - 1, 1) == ":";
}

//----------------------------------------------------------------------------------------------------------------------------------
QString AbstractCodeEditorWidget::formatPythonCodePart(const QString &text, int &lineCount)
{
    QString res = "";
    lineCount = 0;
    if (text.trimmed() != "")
    {
        QString endlineRegExp = "[\n]";
        QString endline = "\n";

        QStringList commandList = text.split(QRegExp(endlineRegExp));
        lineCount = commandList.size();
        if (lineCount == 1)
        {
            res = text.trimmed();
        }
        else
        {
            int i = 1;
            while (i < lineCount && commandList[i].trimmed() == "")
            {
                ++i;
            }

            if (i < lineCount)
            {
                int spaceTabCount1 = getSpaceTabCount(commandList[i]);
                int spaceTabCount2 = 0;
                int tmp = 0;
                i = 2;
                while (i < lineCount && spaceTabCount2 == 0)
                {
                    tmp = getSpaceTabCount(commandList[i]);
                    if (tmp != spaceTabCount1)
                    {
                        spaceTabCount2 = tmp;
                    }
                    ++i;
                }

                int delCount = 0;
                if (haveToIndention(commandList[0]))
                {
                    int spaceTabDifCount = 0;
                    if (spaceTabCount2 != 0)
                    {
                        if (spaceTabCount1 > spaceTabCount2)
                        {
                            spaceTabDifCount = spaceTabCount1 - spaceTabCount2;
                        }
                        else
                        {
                            spaceTabDifCount = spaceTabCount2 - spaceTabCount1;
                        }
                    }
                    else
                    {
                        if (spaceTabCount1 == 0 || spaceTabCount1 % 4 == 0)
                        {
                            spaceTabDifCount = 4;
                        }
                        else if (spaceTabCount1 % 3 == 0)
                        {
                            spaceTabDifCount = 3;
                        }
                        else if (spaceTabCount1 % 2 == 0)
                        {
                            spaceTabDifCount = 2;
                        }
                        else
                        {
                            spaceTabDifCount = 1;
                        }
                    }

                    delCount = spaceTabCount1 - spaceTabDifCount;
                }
                else
                {
                    delCount = spaceTabCount1;
                }

                res = commandList[0].trimmed() + endline;
                for (i = 1; i < lineCount; ++i)
                {
                    commandList[i].remove(0, delCount);
                    res += commandList[i] + endline;
                }
            }
            else
            {
                res = text.trimmed(); 
            }
        }
    }

    return res;
}

//----------------------------------------------------------------------------------------------------------------------------------
QString AbstractCodeEditorWidget::formatConsoleCodePart(const QString &text)
{
    QString res = "";
    QString temp = "";
    QString endlineRegExp = "[\n]";
    QString endline = "\n";
    QStringList commandList = text.split(QRegExp(endlineRegExp));
    
    for (int i = 0; i < commandList.size(); ++i)
    {
        if (i == commandList.size() - 1)
        {
            endline = "";
        }

        temp = commandList[i];
        while (temp.size() > 0 && temp[0] == '>')
        {
            temp.remove(0, 1);
        }

        res += temp + endline;
    }

    return res;
}



} //end namespace ito