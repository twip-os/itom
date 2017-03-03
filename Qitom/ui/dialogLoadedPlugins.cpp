/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2016, Institut fuer Technische Optik (ITO),
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

#include "dialogLoadedPlugins.h"

#define _USE_MATH_DEFINES

#include "../AppManagement.h"
#include "../../AddInManager/addInManager.h"
#include "../organizer/designerWidgetOrganizer.h"
#include "qmath.h"

#include <qfileinfo.h>

namespace ito {

//----------------------------------------------------------------------------------------------------------------------------------
DialogLoadedPlugins::DialogLoadedPlugins(QWidget *parent) :
    QDialog(parent),
    m_fileIconProvider(NULL),
    m_pluginBackgroundColor(QColor(0xB4, 0xCD, 0xCD))
{
    ui.setupUi(this);
    m_fileIconProvider = new QFileIconProvider();

    ui.cmdError->setIcon(QIcon(":/application/icons/dialog-error-4.png"));
    ui.cmdWarning->setIcon(QIcon(":/application/icons/dialog-warning-4.png"));
    ui.cmdMessage->setIcon(QIcon(":/application/icons/dialog-information-4.png"));
    ui.cmdIgnored->setIcon(QIcon(":/plugins/icons/ignored.png"));

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
    header->setIcon(4, QIcon(":/plugins/icons/ignored.png"));
    header->setText(5, "Library / Status");
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
        
        plugin->setBackgroundColor(0, m_pluginBackgroundColor);
        plugin->setBackgroundColor(1, m_pluginBackgroundColor);
        plugin->setBackgroundColor(2, m_pluginBackgroundColor);
        plugin->setBackgroundColor(3, m_pluginBackgroundColor);
        plugin->setBackgroundColor(4, m_pluginBackgroundColor);
        plugin->setBackgroundColor(5, m_pluginBackgroundColor);
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
                child->setIcon(4,  QIcon(":/plugins/icons/ignored.png"));
                setSortChar(4, *child);
                m_items.append(QPair<int,QTreeWidgetItem*>(ito::plsfIgnored, child));
                plugin->setIcon(4, QIcon(":/plugins/icons/ignored.png"));
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
void DialogLoadedPlugins::setPluginBackgroundColor(const QColor &color)
{
    m_pluginBackgroundColor = color;
    QTreeWidgetItem* t;

    for (int i = 0; i < ui.tree->topLevelItemCount(); ++i)
    {
        t = ui.tree->topLevelItem(i);
        t->setBackgroundColor(0, m_pluginBackgroundColor);
        t->setBackgroundColor(1, m_pluginBackgroundColor);
        t->setBackgroundColor(2, m_pluginBackgroundColor);
        t->setBackgroundColor(3, m_pluginBackgroundColor);
        t->setBackgroundColor(4, m_pluginBackgroundColor);
        t->setBackgroundColor(5, m_pluginBackgroundColor);
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

    int flag =  ui.cmdMessage->isChecked() * ito::plsfOk      + 
                ui.cmdWarning->isChecked() * ito::plsfWarning +
                ui.cmdError->isChecked()   * ito::plsfError   +
                ui.cmdIgnored->isChecked() * ito::plsfIgnored;

    QRegExp rx("*" + ui.filterEdit->text() + "*", Qt::CaseInsensitive, QRegExp::Wildcard);

    for (int i = 0; i < m_items.size(); i++)
    {
        int first = m_items[i].first;
        bool show = (first & flag) != 0 &&      // check if button is active for this type of message
                    (!ui.onlyCompatibleCheck->checkState() || (first & ito::plsfRelDbg) == 0) &&    // Isn't compability checkbox set OR if reldgb flag is set it�s incompatibel, if 0 it�s compatible
                    ((m_items[i].second->childCount() == 0) || ui.filterEdit->text() == "" || rx.exactMatch(m_items[i].second->text(5))); // has no child OR filter text is empty OR filter text matches node text
        m_items[i].second->setHidden(!show);
    }


        // check if button is active for this type of message
/*        if ((first & flag) != 0)
        {
            // Is compability checkbox set?
            if (ui.onlyCompatibelCheck->checkState())
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

            if (show && (m_items[i].second->childCount() > 0) && filterEditNotEmpty && !rx.exactMatch(m_items[i].second->text(5)))
            {
                show = false;
            }
        }
        else
        { 
            //show = false;// show stays false
        }*/

    // to count the visible items we need a second loop
    for (int i = 0; i < m_items.size(); i++)
    {
        // Count the item if it has a parent and is not hidden and parent is also not hidden
        if (m_items[i].second->parent() && !m_items[i].second->isHidden() && !m_items[i].second->parent()->isHidden())
        {
            stateCount[int(log(double((m_items[i].first & 0xF)))/log(double(2)))]++;
        }
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