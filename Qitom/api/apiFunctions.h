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

#ifndef APIFUNCTIONS_H
#define APIFUNCTINOS_H

#include "../../common/apiFunctionsInc.h"

namespace ito 
{
    class AbstractAddInConfigDialog;

    class ApiFunctions 
    {
        public:
            ito::RetVal setFPointer(void);
            ApiFunctions();
            ~ApiFunctions();

            //! function called by apiFilterGetFunc
            static ito::RetVal mfilterGetFunc(const QString &name, ito::AddInAlgo::FilterDef *&FilterDef);

            //! function called by apiFilterCall
            static ito::RetVal mfilterCall(const QString &name, QVector<ito::ParamBase> *paramsMand, QVector<ito::ParamBase> *paramsOpt, QVector<ito::ParamBase> *paramsOut);

            //! function called by apiFilterParamBase
            static ito::RetVal mfilterParamBase(const QString &name, QVector<ito::ParamBase> *paramsMand, QVector<ito::ParamBase> *paramsOpt, QVector<ito::ParamBase> *paramsOut);

            //! function called by apiFilterParam
            static ito::RetVal mfilterParam(const QString &name, QVector<ito::Param> *paramsMand, QVector<ito::Param> *paramsOpt, QVector<ito::Param> *paramsOut);

            //! function called by apiAddInGetInitParams
            static ito::RetVal maddInGetInitParams(const QString &name, const int pluginType, int *pluginNum, QVector<ito::Param> *&paramsMand, QVector<ito::Param> *&paramsOpt);

            //! function called by apiAddInOpenActuator
            static ito::RetVal maddInOpenActuator(const QString &name, const int pluginNum, const bool autoLoadParams, QVector<ito::ParamBase> *paramsMand, QVector<ito::ParamBase> *paramsOpt, ito::AddInActuator *&actuator);

            //! function called by apiAddInOpenDataIO
            static ito::RetVal maddInOpenDataIO(const QString &name, const int pluginNum, const bool autoLoadParams, QVector<ito::ParamBase> *paramsMand, QVector<ito::ParamBase> *paramsOpt, ito::AddInDataIO *&dataIO);

            //! function called by apiCreateFromDataObject
            static ito::DataObject* mcreateFromDataObject(const ito::DataObject *dObj, int nrDims, ito::tDataType type, int *sizeLimits = NULL, ito::RetVal *retval = NULL);

            //! function called by apiCreateFromNamedDataObject
            static ito::DataObject* mcreateFromNamedDataObject(const ito::DataObject *dObj, int nrDims, ito::tDataType type, const char *name = NULL, int *sizeLimits = NULL, ito::RetVal *retval = NULL);

            //! function called by apiGetCurrentWorkingDir
            static QString getCurrentWorkingDir(void);

            //! function called by apiShowConfigurationDialog
            static ito::RetVal mshowConfigurationDialog(ito::AddInBase *plugin, ito::AbstractAddInConfigDialog *configDialogInstance);
            
            //! function called by apiSendParamToPyWorkspace
            static ito::RetVal sendParamToPyWorkspaceThreadSafe(const QString &varname, const QSharedPointer<ito::ParamBase> &value);

            //! function called by apiSendParamsToPyWorkspace
            static ito::RetVal sendParamsToPyWorkspaceThreadSafe(const QStringList &varnames, const QVector<QSharedPointer<ito::ParamBase> > &values);

        private:
            int m_loadFPointer;
    };
}

#endif //APIFUNTIONS_H
