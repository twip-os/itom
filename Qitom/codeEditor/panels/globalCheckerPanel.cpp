/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2023, Institut fuer Technische Optik (ITO),
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

#include "globalCheckerPanel.h"

#include "../codeEditor.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qicon.h>
#include <qtooltip.h>
#include <qmenu.h>
#include <qaction.h>
#include "../textBlockUserData.h"
#include "../codeEditor.h"
#include "../delayJobRunner.h"

#include "helper/guiHelper.h"

#include <qdebug.h>
namespace ito {

//----------------------------------------------------------
/*
*/
GlobalCheckerPanel::GlobalCheckerPanel(const QString &description /*= ""*/, QWidget *parent /*= nullptr */) :
    Panel("GlobalCheckerPanel", false, description, parent)
{
    setScrollable(true);
}

//----------------------------------------------------------
/*
*/
GlobalCheckerPanel::~GlobalCheckerPanel()
{

}

//------------------------------------------------------------
/*
Returns the panel size hint. (fixed with of 16px)
*/
QSize GlobalCheckerPanel::sizeHint() const
{   
    int dpi = GuiHelper::getScreenLogicalDpi();
    int size = 16 * dpi / 96;

    QFontMetricsF metrics(editor()->font());
    QSize size_hint(metrics.height(), metrics.height());
    if (size_hint.width() > size)
    {
        size_hint.setWidth(size);
    }
    return size_hint;
}

//----------------------------------------------------------
/*Paints the messages and the visible area on the panel.

*/
void GlobalCheckerPanel::paintEvent(QPaintEvent *e)
{
    Panel::paintEvent(e);

    if (isVisible())
    {
        m_backgroundBrush = QBrush(editor()->background());
        QPainter painter(this);
        painter.fillRect(e->rect(), m_backgroundBrush);
        drawMessages(painter);
        drawVisibleArea(painter);
    }
}

//----------------------------------------------------------
/*
Draw messages from all subclass of CheckerMode currently
installed on the editor.
*/
void GlobalCheckerPanel::drawMessages(QPainter &painter)
{
    TextBlockUserData *tbud;
    QIcon icon;
    CodeCheckerItem::CheckerType worstStatus = CodeCheckerItem::Info;
    bool hasCheckerMessage;
    QRect rect;
    QBrush brushInfo(Qt::blue);
    QBrush brushWarning(QColor(255,165,0));
    QBrush brushError(Qt::red);

    const QTextDocument* td = editor()->document();
    QTextBlock b = td->firstBlock();

    while (b.isValid())
    {
        worstStatus = CodeCheckerItem::Info;
        hasCheckerMessage = false;
        tbud = dynamic_cast<TextBlockUserData*>(b.userData());

        if (tbud)
        {
            foreach (const CodeCheckerItem &cci,tbud->m_checkerMessages)
            {
                hasCheckerMessage = true;

                if (cci.type() > worstStatus)
                {
                    worstStatus = cci.type();
                }
            }

            if (hasCheckerMessage)
            {
                rect = QRect();
                rect.setX(sizeHint().width() / 4);
                rect.setY(b.blockNumber() * getMarkerHeight());
                rect.setSize(getMarkerSize());

                switch (worstStatus)
                {
                case ito::CodeCheckerItem::Info:
                    painter.fillRect(rect, brushInfo);
                    break;
                case ito::CodeCheckerItem::Warning:
                    painter.fillRect(rect, brushWarning);
                    break;
                case ito::CodeCheckerItem::Error:
                    painter.fillRect(rect, brushError);
                    break;
                }
                
            }
        }

        b = b.next();
    }
}


//----------------------------------------------------------
/*
Draw the visible area.
This method does not take folded blocks into account.
*/
void GlobalCheckerPanel::drawVisibleArea(QPainter &painter)
{
    if (editor()->visibleBlocks().size() > 0)
    {
        auto startBlock = editor()->visibleBlocks()[0].textBlock;

        auto endBlock = editor()->visibleBlocks().last().textBlock;
        QRect rect;
        rect.setX(0);
        rect.setY(startBlock.blockNumber() * getMarkerHeight());
        rect.setWidth(sizeHint().width());
        rect.setBottom(endBlock.blockNumber() * getMarkerHeight());
        QColor c;

        if (editor()->background().lightness() < 128)
        {
            c = editor()->background().darker(150);
        }
        else
        {
            c = editor()->background().darker(110);
        }

        c.setAlpha(128);
        painter.fillRect(rect, c);
    }
}


//----------------------------------------------------------
/* Gets the height of message marker.
*/
int GlobalCheckerPanel::getMarkerHeight() const
{
    return editor()->viewport()->height() / editor()->lineCount();
}

//---------------------------------------------------------
/*
Gets the size of a message marker.
*/
QSize GlobalCheckerPanel::getMarkerSize() const
{
    int h = getMarkerHeight();

    if (h < 1)
    {
        h = 1;
    }

    return QSize(sizeHint().width() / 2, h);
}

//----------------------------------------------------------
/*
* Moves the editor text cursor to the clicked line.
*/
void GlobalCheckerPanel::mousePressEvent(QMouseEvent *e)
{
    auto height = e->pos().y();
    auto line = height / getMarkerHeight();
    //height = event.pos().y() line = height // self.get_marker_height()
    editor()->gotoLine(line, 0);
}


} //end namespace ito