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

    Further hints:
    ------------------------

    This file belongs to the code editor of itom. The code editor is
    in major parts a fork / rewritten version of the python-based source 
    code editor PyQode from Colin Duquesnoy and others 
    (see https://github.com/pyQode). PyQode itself is licensed under 
    the MIT License (MIT).

    Some parts of the code editor of itom are also inspired by the
    source code editor of the Spyder IDE (https://github.com/spyder-ide),
    also licensed under the MIT License and developed by the Spyder Project
    Contributors. 

*********************************************************************** */

#include "pyDocstringGenerator.h"

#include "../codeEditor.h"
#include "../../widgets/scriptEditorWidget.h"

#include <qdebug.h>
#include <qregularexpression.h>

namespace ito {

//-------------------------------------------------------------------------------------
PyDocstringGeneratorMode::PyDocstringGeneratorMode(const QString &name, const QString &description /*= ""*/, QObject *parent /*= NULL*/) :
    Mode(name, description),
    QObject(parent)
{
}

//-------------------------------------------------------------------------------------
/*
*/
PyDocstringGeneratorMode::~PyDocstringGeneratorMode()
{
}


//-------------------------------------------------------------------------------------
/*
*/
void PyDocstringGeneratorMode::onStateChanged(bool state)
{
    if (state)
    {
        // maybe the connection already exists.
        connect(
            editor(), &CodeEditor::keyPressed, 
            this, &PyDocstringGeneratorMode::onKeyPressed,
            Qt::UniqueConnection);
    }
    else
    {
        disconnect(
            editor(), &CodeEditor::keyPressed, 
            this, &PyDocstringGeneratorMode::onKeyPressed);
    }       
}

//-------------------------------------------------------------------------------------
QSharedPointer<OutlineItem> PyDocstringGeneratorMode::getOutlineOfLineIdx(int lineIdx) const
{
    ScriptEditorWidget *sew = qobject_cast<ScriptEditorWidget*>(editor());

    if (!sew)
    {
        return QSharedPointer<OutlineItem>();
    }

    auto current = sew->parseOutline();
    auto result = QSharedPointer<OutlineItem>();
    bool found = true;

    while (found && !current.isNull())
    {
        found = false;

        foreach(const QSharedPointer<OutlineItem> &c, current->m_childs)
        {
            if (lineIdx >= c->m_startLineIdx
                && lineIdx <= c->m_endLineIdx)
            {
                result = c;
                current = c;
                found = true;
                break;
            }
        }
    }

    if (!result.isNull())
    {
        if (lastLineIdxOfDefinition(result) < 0)
        {
            result.clear();
        }
        else if (result->m_type == OutlineItem::typeClass)
        {
            result.clear();
        }
    }

    return result;
}


//-------------------------------------------------------------------------------------
void PyDocstringGeneratorMode::insertDocstring(const QTextCursor &cursor) const
{
    if (cursor.isNull())
    {
        return;
    }

    auto outline = getOutlineOfLineIdx(cursor.blockNumber());

    if (outline.isNull())
    {
        return;
    }

    CodeEditor *e = editor();

    int lineIdx = lastLineIdxOfDefinition(outline);

    if (lineIdx < 0)
    {
        return;
    }

    QTextCursor insertCursor = e->gotoLine(lineIdx, 0);
    insertCursor.movePosition(QTextCursor::EndOfLine);

    // get the indentation for the new docstring
    int initIndent = e->lineIndent(outline->m_startLineIdx);
    QString indent = editor()->useSpacesInsteadOfTabs() ? 
        QString(initIndent + editor()->tabLength(), ' ') : 
        QString(initIndent + 1, '\t');
    
    FunctionInfo finfo = parseFunctionInfo(outline, lineIdx);

    QString docstring = generateGoogleDoc(outline, finfo);
    docstring = QString("\"\"\"%1\n\"\"\"").arg(docstring);

    // add the indentation to all lines of the docstring
    QStringList lines = docstring.split("\n");

    for (int i = 0; i < lines.size(); ++i)
    {
        lines[i] = Utils::rstrip(indent + lines[i]);
    }

    // insert the docstring
    insertCursor.insertText("\n" + lines.join("\n"));

    e->setCursorPosition(cursor.blockNumber() + 1, indent.size() + 3);
}

//-------------------------------------------------------------------------------------
PyDocstringGeneratorMode::FunctionInfo PyDocstringGeneratorMode::parseFunctionInfo(
    const QSharedPointer<OutlineItem> &item,
    int lastLineIdxOfDefinition) const
{
    FunctionInfo info;

    // parse arguments
    parseArgList(item, info);

    // get code
    QStringList codelines;
    const CodeEditor *e = editor();
    int startIdx = std::max(item->m_startLineIdx, lastLineIdxOfDefinition + 1);

    for (int i = startIdx; i <= item->m_endLineIdx; ++i)
    {
        codelines.append(e->lineText(i));
    }

    foreach(const QSharedPointer<OutlineItem> &c, item->m_childs)
    {
        // remove code from subfunctions
        for (int i = c->m_startLineIdx; i <= c->m_endLineIdx; ++i)
        {
            codelines[i - startIdx] = "";
        }
    }

    codelines.removeAll("");

    QString code = codelines.join("\n");

    // raise
    QRegularExpression re("[ \\t]raise ([a-zA-Z0-9_]*)");
    QRegularExpressionMatchIterator reIter = re.globalMatch(code);

    while (reIter.hasNext())
    {
        QRegularExpressionMatch match = reIter.next();
        info.m_raises.append(match.captured(1));
    }
    
    info.m_hasYield = false;

    // generic return type
    if (item->m_returnType != "")
    {
        info.m_returnTypes << item->m_returnType;
    }

    // yield
    QRegularExpressionMatchIterator yieldIter =
        QRegularExpression("[ \\t]yield ").globalMatch(code);

    if (yieldIter.hasNext())
    {
        info.m_hasYield = true;

        if (info.m_returnTypes.size() == 0)
        {
            info.m_returnTypes << "TYPE";
        }
    }
    else
    {
        // return
        QRegularExpressionMatchIterator returnIter =
            QRegularExpression("[ \\t]return ").globalMatch(code);

        if (returnIter.hasNext())
        {
            if (info.m_returnTypes.size() == 0)
            {
                info.m_returnTypes << "TYPE";
            }
        }
    }

    return info;
}

//-------------------------------------------------------------------------------------
void PyDocstringGeneratorMode::parseArgList(
    const QSharedPointer<OutlineItem> &item, FunctionInfo &info) const
{
    bool expectSelfOrCls = 
        item->m_type == OutlineItem::typeClassMethod || 
        item->m_type == OutlineItem::typeMethod;
    QString argstr = item->m_args.trimmed();
    QStringList args;
    int lastpos = 0;
    QList<QChar> specialCharStack;
    int idx1, idx2;

    for (int pos = 0; pos < argstr.size(); ++pos)
    {
        if (specialCharStack.size() == 0)
        {
            if (argstr[pos] == ",")
            {
                if (pos - lastpos > 0)
                {
                    args.append(argstr.mid(lastpos, pos - lastpos));
                }

                lastpos = pos + 1; //ignore the comma
            }
        }
        else
        {
            QChar lastChar = specialCharStack.last();

            if (lastChar != '"' && lastChar != '\'')
            {
                switch (argstr[pos].toLatin1())
                {
                case '(':
                    specialCharStack.append('(');
                    break;
                case ')':
                    if (lastChar == '(')
                    {
                        specialCharStack.removeLast();
                    }
                    break;
                case '[':
                    specialCharStack.append('[');
                    break;
                case ']':
                    if (lastChar == '[')
                    {
                        specialCharStack.removeLast();
                    }
                    break;
                case '{':
                    specialCharStack.append('{');
                    break;
                case '}':
                    if (lastChar == '{')
                    {
                        specialCharStack.removeLast();
                    }
                    break;
                case '"':
                    if (lastChar == '"')
                    {
                        specialCharStack.removeLast();
                    }
                    else
                    {
                        specialCharStack.append('"');
                    }
                    break;
                case '\'':
                    if (lastChar == '\'')
                    {
                        specialCharStack.removeLast();
                    }
                    else
                    {
                        specialCharStack.append('\'');
                    }
                    break;
                }
            }
            else // last char = " or '
            {
                if (argstr[pos] == lastChar)
                {
                    specialCharStack.removeLast();
                }
            }
        }
    }

    if (lastpos < argstr.size() - 1)
    {
        // append last section
        args.append(argstr.mid(lastpos));
    }

    int count = 0;

    foreach(const QString &arg, args)
    {
        idx1 = arg.indexOf(":");
        idx2 = arg.indexOf("=", idx1);

        ArgInfo a;
        a.m_isOptional = (idx2 >= 0);

        if (idx1 >= 0)
        {
            a.m_name = arg.left(idx1).trimmed();
            a.m_type = idx2 >= 0 ?
                arg.mid(idx1 + 1, idx2 - idx1 - 1).trimmed() :
                arg.mid(idx1 + 1).trimmed();
        }
        else if (idx2 >= 0)
        {
            a.m_name = arg.left(idx2).trimmed();
        }
        else
        {
            a.m_name = arg.trimmed();
        }

        if (count == 0 && expectSelfOrCls && (a.m_name == "self" || a.m_name == "cls"))
        {
            //pass
        }
        else
        {
            info.m_args.append(a);
        }

        count++;
    }
}

//-------------------------------------------------------------------------------------
QString PyDocstringGeneratorMode::generateGoogleDoc(
    const QSharedPointer<OutlineItem> &item, const FunctionInfo &info) const
{
    QString docs = "";

    if (info.m_args.size() > 0)
    {
        docs += "\n\nArgs:";

        foreach(const ArgInfo &arg, info.m_args)
        {
            if (arg.m_name != "")
            {
                QString typ = arg.m_type != "" ? arg.m_type : "TYPE";

                if (arg.m_isOptional)
                {
                    docs += QString("\n    %1 (%2, optional): DESCRIPTION")
                        .arg(arg.m_name).arg(typ);
                }
                else
                {
                    docs += QString("\n    %1 (%2): DESCRIPTION")
                        .arg(arg.m_name).arg(typ);
                }
            }
        }
    }

    if (info.m_raises.size() > 0)
    {
        docs += "\n\nRaises:";

        foreach(const QString &exc, info.m_raises)
        {
            docs += QString("\n    %1: DESCRIPTION").arg(exc);
        }
    }

    if (info.m_returnTypes.size() > 0)
    {
        if (info.m_hasYield)
        {
            docs += "\n\nYields:";
        }
        else
        {
            docs += "\n\nReturns:";
        }

        foreach(const QString &returnType, info.m_returnTypes)
        {
            docs += QString("\n    %1: DESCRIPTION").arg(returnType);
        }
    }

    return docs;
}

//-------------------------------------------------------------------------------------
int PyDocstringGeneratorMode::lastLineIdxOfDefinition(const QSharedPointer<OutlineItem> &item) const
{
    CodeEditor *e = editor();
    QString text;

    if (!e)
    {
        return -1;
    }

    QRegularExpression re("^(.*)(:\\s*#)(.*)$");

    for (int idx = item->m_startLineIdx; idx <= item->m_endLineIdx; ++idx)
    {
        text = e->lineText(idx).trimmed();

        if (text.endsWith(":"))
        {
            return idx;
        }

        QRegularExpressionMatch match = re.match(text);

        if (match.hasMatch())
        {
            return idx;
        }
    }

    return -1;
}

} //end namespace ito
