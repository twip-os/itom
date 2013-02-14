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

#ifndef PYTHONCOMMON_H
#define PYTHONCOMMON_H

//python
//// see http://vtk.org/gitweb?p=VTK.git;a=commitdiff;h=7f3f750596a105d48ea84ebfe1b1c4ca03e0bab3
//#ifdef _DEBUG
//    #undef _DEBUG
//    #ifdef linux
//        #include "Python.h"
//    #else
//        #include "include\Python.h"
//    #endif
//    #define _DEBUG
//#else
//    #ifdef linux
//        #include "Python.h"
//    #else
//        #include "include\Python.h"
//    #endif
//#endif

#include "pythonPlugins.h"
#include "pythonDataObject.h"
#include "pythonPCL.h"

#include "../common/sharedStructures.h"

#include <qvector.h>
#include <qvariant.h>

namespace ito
{

    ito::RetVal checkAndSetParamVal(PyObject *tempObj, ito::Param *param, int *set);
    ito::RetVal checkAndSetParamVal(PyObject *pyObj, const ito::Param *defaultParam, ito::ParamBase &outParam, int *set);
    //!< This function is used to print out parameters to a dictionary and the itom-console
    PyObject* PrntOutParams(const QVector<ito::Param> *params, bool asErr, bool addInfos, const int num);
    void errOutInitParams(const QVector<ito::Param> *params, const int num, const QString reason);
    ito::RetVal parseInitParams(QVector<ito::Param> *initParamListMand, QVector<ito::Param> *initParamListOpt, PyObject *args, PyObject *kwds);
    ito::RetVal parseInitParams(const QVector<ito::Param> *defaultParamListMand, const QVector<ito::Param> *defaultParamListOpt, PyObject *args, PyObject *kwds, QVector<ito::ParamBase> &paramListMandOut, QVector<ito::ParamBase> &paramListOptOut);
    ito::RetVal copyParamVector(const QVector<ito::ParamBase> *paramVecIn, QVector<ito::ParamBase> &paramVecOut);
    ito::RetVal copyParamVector(const QVector<ito::Param> *paramVecIn, QVector<ito::Param> &paramVecOut);
    ito::RetVal copyParamVector(const QVector<ito::Param> *paramVecIn, QVector<ito::ParamBase> &paramVecOut);
    ito::RetVal createEmptyParamBaseFromParamVector(const QVector<ito::Param> *paramVecIn, QVector<ito::ParamBase> &paramVecOut);

    //!< This function searches for reserves Keywords (e.g. autoLoadParams) sets the corresponding bool parameter to the right value and than deletes the keyword
    ito::RetVal findAndDeleteReservedInitKeyWords(PyObject *kwds, bool * enableAutoLoadParams);

    //PyObject* transformQVariant2PyObject(QVariant *value, ito::RetVal &retValue);
    PyObject* buildFilterOutputValues(QVector<QVariant> *outVals, ito::RetVal &retValue);

    class PythonCommon
    {
        public:
            static bool transformRetValToPyException(ito::RetVal &retVal, PyObject *exceptionIfError = PyExc_RuntimeError);
    };

} //end namespace ito

#endif
