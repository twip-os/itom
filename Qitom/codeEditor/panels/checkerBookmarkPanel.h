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

#ifndef CHECKERBOOKMARKPANEL_H
#define CHECKERBOOKMARKPANEL_H

/*
Checker panels:

- CheckerPanel: draw checker messages in front of each line
- GlobalCheckerPanel: draw all checker markers as colored rectangle to
  offer a global view of all errors
*/

#include "../panel.h"
#include "../utils/utils.h"
#include "../textBlockUserData.h"

#include <qevent.h>
#include <qsize.h>
#include <qcolor.h>
#include <qicon.h>

namespace ito {

class DelayJobRunnerBase;
/*
Shows messages collected by one or more checker modes
*/
class CheckerBookmarkPanel : public Panel
{
    Q_OBJECT
public:
    CheckerBookmarkPanel(const QString &description = "", QWidget *parent = NULL);
    virtual ~CheckerBookmarkPanel();

    virtual QSize sizeHint() const;
    
    virtual void onUninstall();

    QList<CheckerMessage> markersForLine(int line) const;

    static QIcon iconFromMessages(bool hasCheckerMessages, bool hasBookmark, CheckerMessage::CheckerStatus checkerStatus);

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void leaveEvent(QEvent *e);

protected:
    void displayTooltip(QList<QVariant> args);

private:
    int m_previousLine;
    DelayJobRunnerBase *m_pJobRunner;

signals:
    void toggleBookmarkRequested(int line);
};

} //end namespace ito

#endif


