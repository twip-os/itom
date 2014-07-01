/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2013, Institut f�r Technische Optik (ITO),
    Universit�t Stuttgart, Germany

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

#include "dialogLoadedPlugins.h"

#include "../AppManagement.h"
#include "../organizer/addInManager.h"
#include "../organizer/designerWidgetOrganizer.h"
#include "qmath.h"

#include <qfileinfo.h>

namespace ito {

//----------------------------------------------------------------------------------------------------------------------------------
DialogLoadedPlugins::DialogLoadedPlugins(QWidget *parent) :
    QDialog(parent),
    m_fileIconProvider(NULL)
{
    ui.setupUi(this);
    m_fileIconProvider = new QFileIconProvider();

    ui.cmdError->setIcon(QIcon(":/application/icons/dialog-error-4.png"));
    ui.cmdWarning->setIcon(QIcon(":/application/icons/dialog-warning-4.png"));
    ui.cmdMessage->setIcon(QIcon(":/application/icons/dialog-information-4.png"));
    ui.cmdIgnored->setIcon(QIcon(":/plugins/icons_m/ignored.png"));

    init();
    filter();

    ui.tree->setSortingEnabled(true);
    ui.tree->sortByColumn(5);

    ui.tree->collapseAll();
}

//----------------------------------------------------------------------------------------------------------------------------------
DialogLoadedPlugins::~DialogLoadedPlugins()
{
    DELETE_AND_SET_NULL(m_fileIconProvider);
}

//----------------------------------------------------------------------------------------------------------------------------------
void DialogLoadedPlugins::init()
{
    ito::AddInManager *AIM = qobject_cast<ito::AddInManager*>(AppManagement::getAddInManager());
    if (AIM)
    {
        m_content = AIM->getPluginLoadStatus();
    }

    ito::DesignerWidgetOrganizer *dwo = qobject_cast<ito::DesignerWidgetOrganizer*>(AppManagement::getDesignerWidgetOrganizer());
    if (dwo)
    {
        m_content.append(dwo->getPluginLoadStatus());
    }

    ui.tree->setColumnWidth(0, 42);
    QTreeWidgetItem *header = new QTreeWidgetItem();
    header->setIcon(1, QIcon(":/application/icons/dialog-information-4.png"));
    header->setIcon(2, QIcon(":/application/icons/dialog-warning-4.png"));
    header->setIcon(3, QIcon(":/application/icons/dialog-error-4.png"));
    header->setIcon(4, QIcon(":/plugins/icons_m/ignored.png"));
    header->setText(5, "Test");
    ui.tree->setHeaderItem(header);

    m_windowTitle = ui.groupBox_2->title();
    m_cmdMessage = ui.cmdMessage->text();
    m_cmdWarning = ui.cmdWarning->text();
    m_cmdError = ui.cmdError->text();
    m_cmdIgnored = ui.cmdIgnored->text();

    foreach (const PluginLoadStatus& item, m_content)
    {
        int overallStatus = 0;
        QFileInfo info(item.filename);
        const QPair<ito::PluginLoadStatusFlags, QString> *message;
        QTreeWidgetItem *child = NULL;

        QTreeWidgetItem *plugin = new QTreeWidgetItem();
        plugin->setData(5, Qt::DisplayRole, info.fileName());
        plugin->setData(5, Qt::ToolTipRole, info.absoluteFilePath());
        plugin->setData(0, Qt::DecorationRole, m_fileIconProvider->icon(info));
        QColor bckRnd = QColor(0xB4,0xCD,0xCD);
        plugin->setBackgroundColor(0, bckRnd);
        plugin->setBackgroundColor(1, bckRnd);
        plugin->setBackgroundColor(2, bckRnd);
        plugin->setBackgroundColor(3, bckRnd);
        plugin->setBackgroundColor(4, bckRnd);
        plugin->setBackgroundColor(5, bckRnd);
        //bool pluginOK = true;

        QChar sortElement = ' '; // This character is only in a column if there is an icon... this makes it easy to sort with the standard function

        for (int i = 0; i < item.messages.size(); i++)
        {
            message = &(item.messages[i]);
            
            overallStatus |= message->first;

            child = new QTreeWidgetItem(plugin);
            child->setData(5, Qt::DisplayRole, message->second);
            child->setData(5, Qt::ToolTipRole, message->second);
            child->setIcon(5, QIcon());
            if (message->first & ito::plsfOk)
            {
                child->setIcon(1, QIcon(":/application/icons/dialog-information-4.png"));
                setSortChar(1, *child);
                m_items.append(QPair<int,QTreeWidgetItem*>(ito::plsfOk, child)); //plsfOk is 0, that is bad, therefore use another value for retOk
                plugin->setIcon(1, QIcon(":/application/icons/dialog-information-4.png"));
                setSortChar(1, *plugin);
            }
            else if (message->first & ito::plsfWarning)
            {
                child->setIcon(2,  QIcon(":/application/icons/dialog-warning-4.png"));
                setSortChar(2, *child);
                if (message->first & ito::plsfRelDbg)
                {
                    m_items.append(QPair<int,QTreeWidgetItem*>(ito::plsfWarning | ito::plsfRelDbg, child));
                }
                else
                {
                    m_items.append(QPair<int,QTreeWidgetItem*>(ito::plsfWarning, child));
                }
                plugin->setIcon(2, QIcon(":/application/icons/dialog-warning-4.png"));
                setSortChar(2, *plugin);
                //pluginOK = false;
            }
            else if (message->first & ito::plsfIgnored)
            {
                child->setIcon(4,  QIcon(":/plugins/icons_m/ignored.png"));
                setSortChar(4, *child);
                m_items.append(QPair<int,QTreeWidgetItem*>(ito::plsfIgnored, child));
                plugin->setIcon(4, QIcon(":/plugins/icons_m/ignored.png"));
                setSortChar(4, *plugin);
                //pluginOK = false;
            }
            else
            {
                child->setIcon(3, QIcon(":/application/icons/dialog-error-4.png"));
                setSortChar(3, *child);
                m_items.append(QPair<int,QTreeWidgetItem*>(ito::plsfError, child));
                plugin->setIcon(3, QIcon(":/application/icons/dialog-error-4.png"));
                setSortChar(3, *plugin);
                //pluginOK = false;
            }
        }

        //if (pluginOK)
        {

        }
        m_items.append(QPair<int,QTreeWidgetItem*>(overallStatus, plugin));
        ui.tree->addTopLevelItem(plugin);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void DialogLoadedPlugins::setSortChar(int column, QTreeWidgetItem &item)
{
    // This is a little workaround to use the standard sort functions 
    // of QTreeWidget. The sort-function normally does not sort icons. 
    // By adding a space character to all columns without an icon
    // the column with an icon is above the ones with spaces.
    QChar sortElement = ' ';
    switch(column)
    {
        case 1:
        {
            //item.setText(1, sortElement);
            item.setText(2, sortElement);
            item.setText(3, sortElement);
            item.setText(4, sortElement);
            break;
        }
        case 2:
        {
            item.setText(1, sortElement);
            //item.setText(2, sortElement);
            item.setText(3, sortElement);
            item.setText(4, sortElement);
            break;
        }
        case 3:
        {
            item.setText(1, sortElement);
            item.setText(2, sortElement);
            //item.setText(3, sortElement);
            item.setText(4, sortElement);
            break;
        }
        case 4:
        {
            item.setText(1, sortElement);
            item.setText(2, sortElement);
            item.setText(3, sortElement);
            //item.setText(4, sortElement);
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void DialogLoadedPlugins::filter()
{
    int stateCount[4] = { 0, 0, 0, 0}; // we need 1: plsfOK, 2: plsfWarning, 4: plsfError, 8: plsfIgnored

    int flag = 0;
    flag = ui.cmdMessage->isChecked() * ito::plsfOk      + 
           ui.cmdWarning->isChecked() * ito::plsfWarning +
           ui.cmdError->isChecked()   * ito::plsfError   +
           ui.cmdIgnored->isChecked() * ito::plsfIgnored;

    bool filterEditNotEmpty = ui.filterEdit->text() != "";
    QString filterEditText = "*" + ui.filterEdit->text() + "*";
    QRegExp rx(filterEditText, Qt::CaseInsensitive, QRegExp::Wildcard);

    for (int i = 0; i < m_items.size(); i++)
    {
        int first = m_items[i].first;
        bool show = false; 
        // check if button is active for this type of message
        if ((first & flag) != 0)
        { 
            // Is compability checkbox set?
            if (ui.onlyComnpatibelCheck->checkState())
            {
                if ((first & ito::plsfRelDbg) != 0) // if reldgb flag is set it�s incompatibel, if 0 it�s compatible
                {
                    //show = false; // show stays false: Debug is incompatibel
                }
                else
                {
                    show = true;
                }
            }
            // It�s not checked, show the item
            else
            {
                show = true;
            }
        }
        else
        { 
            //show = false;// show stays false
        }

        m_items[i].second->setHidden(!show);

        // Count the item if it has a parent and is not hidden
        if (m_items[i].second->parent() && show)
        {
            stateCount[int(log(double((first & 0xF)))/log(double(2)))]++;
        }



        //// show compatible items if ( ... && 0 = no conflict, 1 = conflict)
        //if (!ui.onlyComnpatibelCheck->checkState() || !(m_items[i].first & ito::plsfRelDbg) != 0) 
        //{
        //    hideItem = false;
        //}
        //else if (filterEditNotEmpty && hideItem) // If TextSearch is used and item is not hidden
        //{
        //    if (m_items[i].second->parent())
        //    {
        //        // if item has parent, set the parent hide status
        //        if (!m_items[i].second->parent()->isHidden())
        //        {
        //            hideItem = false;
        //        }
        //    }
        //    else
        //    {
        //        // if not, check if the filter search matches ... if not, hide it 
        //        if (rx.exactMatch(m_items[i].second->text(5)))
        //        {
        //            hideItem = false;
        //        }
        //    }
        //}



        //// Hide the item
        //m_items[i].second->setHidden(hideItem);


    }
    ui.groupBox_2->setTitle(QString("%1 (%2)").arg(m_windowTitle).arg(stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3]));
    ui.cmdMessage->setText(QString("%1 (%2)").arg(m_cmdMessage).arg  (stateCount[0]));
    ui.cmdWarning->setText(QString("%1 (%2)").arg(m_cmdWarning).arg  (stateCount[1]));
    ui.cmdError->setText(QString("%1 (%2)").arg(m_cmdError).arg      (stateCount[2]));
    ui.cmdIgnored->setText(QString("%1 (%2)").arg(m_cmdIgnored).arg  (stateCount[3]));
}

//----------------------------------------------------------------------------------------------------------------------------------
void DialogLoadedPlugins::on_tree_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> items = ui.tree->selectedItems();
    if (items.size() >= 1)
    {
        ui.lblText->setText(items[0]->data(5, Qt::ToolTipRole).toString());
    }
    else
    {
        ui.lblText->setText("");
    }
}

} //end namespace ito