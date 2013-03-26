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

#include "AppManagement.h"
#include <qfileinfo.h>

/*!
    \class AppManagement
    \brief static class only administrating QObject-pointers to main organization and management units of the main application
*/

//initialization of members
QObject* AppManagement::m_pe = NULL;
QObject* AppManagement::m_sew = NULL;
QObject* AppManagement::m_dwo = NULL;
QObject* AppManagement::m_plo = NULL;
QObject* AppManagement::m_app = NULL;
QObject* AppManagement::m_mainWin = NULL;
QObject* AppManagement::m_uiOrganizer = NULL;
QObject* AppManagement::m_processOrganizer = NULL;
QString AppManagement::m_userName = QString("itom");    //!< standard user is itom
int AppManagement::m_userRole = 2;                      //!< developer
QMutex AppManagement::m_mutex;
QString AppManagement::m_settingsFile;


QString AppManagement::getUserID(void)
{
    QFileInfo fi(m_settingsFile);
    QString name = fi.baseName();
    return name.right(name.length() - 5);
}