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

#ifndef LASTCOMMANDDOCKWIDGET_H
#define LASTCOMMANDDOCKWIDGET_H

#include "abstractDockWidget.h"

#include <qwidget.h>
#include <qaction.h>
#include <qtoolbar.h>

#include <qtreewidget.h>

namespace ito
{
    class LastCommandDockWidget : public AbstractDockWidget
    {
        Q_OBJECT

        public:
            LastCommandDockWidget(const QString &title, QWidget *parent = NULL, bool docked = true, bool isDockAvailable = true, tFloatingStyle floatingStyle = floatingNone, tMovingStyle movingStyle = movingEnabled);
            ~LastCommandDockWidget();

        protected:

            void createActions();
            void createMenus();
            void createToolBars();
            void createStatusBar(){};
            void updateActions();
            void updatePythonActions(){ updateActions(); }

        private:
			QTreeWidget *m_lastCommandTreeWidget;
            QMenu* m_pContextMenu;
            ShortcutAction* m_pActClearList;

        signals:
            void runPythonCommand(const QString cmd);

        private slots:
            void itemDoubleClicked(QTreeWidgetItem *item, int column);
            void mnuClearList();
            void treeWidgetContextMenuRequested(const QPoint &pos);

        public slots:
            void addLastCommand(const QString cmd);
    };

} //end namespace ito

#endif
