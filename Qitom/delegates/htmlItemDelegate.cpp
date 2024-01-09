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
*********************************************************************** */

#include "htmlItemDelegate.h"

#include <qabstracttextdocumentlayout.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qtextdocument.h>
#include <qtreewidget.h>
#include <qdebug.h>

#include "widgets/itomQWidgets.h"

namespace ito {

//-------------------------------------------------------------------------------------
void HtmlItemDelegate::paint(
    QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem options(option);
    QTextDocument doc;
    prepareTextDocument(options, doc, index);

    QStyle* style = options.widget ? options.widget->style() : QApplication::style();
    options.text = "";

    // Note : We need to pass the options widget as an argument of
    // drawCrontol to make sure the delegate is painted with a style
    // consistent with the widget in which it is used.
    // See spyder - ide / spyder#10677.
    style->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = options.palette;

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &options, nullptr);
    painter->save();

    painter->translate(textRect.topLeft() + QPoint(0, -3));

    // Type check : Prevent error in PySide where using
    // doc.documentLayout().draw() may fail because doc.documentLayout()
    // returns an object of type QtGui.QStandardItem(for whatever reason).
    auto docLayout = doc.documentLayout();
    docLayout->draw(painter, ctx);

    painter->restore();
}

//-------------------------------------------------------------------------------------
void HtmlItemDelegate::prepareTextDocument(QStyleOptionViewItem& option, QTextDocument &doc, const QModelIndex &index) const
{
    // This logic must be shared between paint and sizeHint for consistency
    QStyleOptionViewItem options(option);
    initStyleOption(&options, index);

    doc.setHtml(options.text);
}


//-------------------------------------------------------------------------------------
QSize HtmlItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem options(option);
    QTextDocument doc;
    prepareTextDocument(options, doc, index);
    return QSize(qRound(doc.idealWidth()), qRound(doc.size().height() - 2));
}

//-------------------------------------------------------------------------------------
bool HtmlItemDelegate::editorEvent(
    QEvent* event,
    QAbstractItemModel* model,
    const QStyleOptionViewItem& option,
    const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        QTreeWidgetItom* treeWidget = qobject_cast<QTreeWidgetItom*>(const_cast<QWidget*>(option.widget));
        if (treeWidget)
        {
            QTreeWidgetItem* item = treeWidget->itemFromIndex2(index);
            if (item)
            {
                emit itemDoubleClicked(treeWidget, item);
                return true;
            }
        }
        return true;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

} // end namespace ito
