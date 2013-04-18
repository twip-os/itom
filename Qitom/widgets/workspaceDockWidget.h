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

#ifndef WORKSPACEDOCKWIDGET_H
#define WORKSPACEDOCKWIDGET_H

#include "workspaceWidget.h"
#include "abstractDockWidget.h"

#include <qwidget.h>
#include <qaction.h>
#include <qtoolbar.h>
#include <qevent.h>


namespace ito {


class WorkspaceDockWidget : public AbstractDockWidget
{
    Q_OBJECT

public:
    WorkspaceDockWidget(const QString &title, bool globalNotLocal, QWidget *parent = NULL, bool docked = true, bool isDockAvailable = true, tFloatingStyle floatingStyle = floatingNone, tMovingStyle movingStyle = movingEnabled);
    ~WorkspaceDockWidget();

protected:

    //void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar(){}
    void updateActions();
    void updatePythonActions(){ updateActions(); }

private:
    bool m_globalNotLocal;

    WorkspaceWidget *m_pWorkspaceWidget;
    ShortcutAction *m_actDelete;
    ShortcutAction *m_actRename;
    ShortcutAction *m_actExport;
    ShortcutAction *m_actImport;

    QToolBar *m_pMainToolBar;
    QMenu* m_pContextMenu;
    QTreeWidgetItem* m_firstCurrentItem;
    QString m_firstCurrentItemKey;

private slots:
    void mnuDeleteItem();
    void mnuExportItem();
    void mnuImportItem();
    void mnuRenameItem();

    void treeWidgetItemSelectionChanged() { updateActions(); };
    void treeWidgetItemChanged(QTreeWidgetItem * item, int column);
    void treeViewContextMenuRequested(const QPoint &pos);

	

signals:
    void setStatusInformation(QString text, int timeout = 0);
};

} //end namespace ito

#endif
