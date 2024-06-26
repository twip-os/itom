/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2020, Institut für Technische Optik (ITO),
    Universität Stuttgart, Germany

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

#ifndef DIALOGLOADEDPLUGINS_H
#define DIALOGLOADEDPLUGINS_H

#include "../global.h"

#include <qcolor.h>
#include <qdialog.h>
#include <qfileiconprovider.h>
#include <qlist.h>
#include <qtreewidget.h>

#include "ui_dialogLoadedPlugins.h"

namespace ito {

struct PluginLoadStatus; // forward declaration

class DialogLoadedPlugins : public QDialog
{
    Q_OBJECT

public:
    DialogLoadedPlugins(QWidget* parent = nullptr);
    ~DialogLoadedPlugins();

protected:
    void init();
    void filter();
    void setSortChar(int column, QTreeWidgetItem& item);

    Ui::DialogLoadedPlugins ui;
    QList<PluginLoadStatus> m_content;
    QList<QPair<int, QTreeWidgetItem*>> m_items;
    QFileIconProvider* m_fileIconProvider;
    QString m_windowTitle;
    QString m_cmdMessage;
    QString m_cmdWarning;
    QString m_cmdError;
    QString m_cmdIgnored;

private slots:
    void on_onlyCompatibleCheck_clicked(bool /*value*/)
    {
        filter();
    };
    void on_cmdError_clicked(bool /*value*/)
    {
        filter();
    };
    void on_cmdWarning_clicked(bool /*value*/)
    {
        filter();
    };
    void on_cmdMessage_clicked(bool /*value*/)
    {
        filter();
    };
    void on_cmdIgnored_clicked(bool /*value*/)
    {
        filter();
    };
    void on_filterEdit_textChanged(const QString /*value*/)
    {
        filter();
    };
    void on_tree_itemSelectionChanged();
};

} // end namespace ito

#endif
