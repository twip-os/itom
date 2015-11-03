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

#include "widgetPropGeneralApplication.h"
#include "../global.h"
#include "../AppManagement.h"

#include <qsettings.h>
#include <qfiledialog.h>

namespace ito
{

//----------------------------------------------------------------------------------------------------------------------------------
WidgetPropGeneralApplication::WidgetPropGeneralApplication(QWidget *parent) :
    AbstractPropertyPageWidget(parent)
{
    ui.setupUi(this);

    ui.btnAdd->setEnabled(true);
    ui.btnRemove->setEnabled(false);
}

//----------------------------------------------------------------------------------------------------------------------------------
WidgetPropGeneralApplication::~WidgetPropGeneralApplication()
{
}

//----------------------------------------------------------------------------------------------------------------------------------
void WidgetPropGeneralApplication::readSettings()
{
    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("MainWindow");
    ui.checkAskBeforeExit->setChecked( settings.value("askBeforeClose", false).toBool() );
    settings.endGroup();

    QListWidgetItem *lwi;
    settings.beginGroup("Application");
    lwi = new QListWidgetItem(QCoreApplication::applicationDirPath() + "/lib", ui.listWidget);
    lwi->setTextColor(Qt::GlobalColor::gray);

    int size = settings.beginReadArray("searchPathes");
    for (int i = 0; i < size; ++i) 
    {
        settings.setArrayIndex(i);
        ui.listWidget->addItem(settings.value("path", QString()).toString());
    }
    settings.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------------------
void WidgetPropGeneralApplication::writeSettings()
{
    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("MainWindow");
    settings.setValue("askBeforeClose", ui.checkAskBeforeExit->isChecked() );
    settings.endGroup();

    QStringList files;
    settings.beginGroup("Application");
    settings.beginWriteArray("searchPathes");
    for (int i = 1; i < ui.listWidget->count(); i++)
    {
        settings.setArrayIndex(i - 1);
        settings.setValue("path", ui.listWidget->item(i)->text());
        files.append(ui.listWidget->item(i)->text());
    }
    settings.endArray();
    settings.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------------------
void WidgetPropGeneralApplication::on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    ui.btnRemove->setEnabled(current != NULL && current->text() != QCoreApplication::applicationDirPath() + "/lib");
}

//----------------------------------------------------------------------------------------------------------------------------------
void WidgetPropGeneralApplication::on_btnAdd_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("load directory"), QDir::currentPath());

    if (dir != "" && ui.listWidget->findItems(dir, Qt::MatchExactly).isEmpty())
    {
        ui.listWidget->addItem(dir);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
void WidgetPropGeneralApplication::on_btnRemove_clicked()
{
    qDeleteAll(ui.listWidget->selectedItems());
}

//----------------------------------------------------------------------------------------------------------------------------------
void WidgetPropGeneralApplication::on_listWidget_itemActivated(QListWidgetItem* item)
{
    if (item)
    {
/*        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.listWidget->editItem(item);*/
    }
}

} //end namespace ito