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
#include "addInManager.h"

#include "../api/apiFunctions.h"
#include "../api/apiFunctionsGraph.h"

#include "../../common/sharedFunctionsQt.h"
#include "../../common/helperCommon.h"

#include "../helper/sharedPointerHelper.h"

#include "../AppManagement.h"

#include <qsettings.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qpluginloader.h>
#include <qapplication.h>
#include <QtXml/qdom.h>
#include "../global.h"
#include <qmainwindow.h>
#include <qdockwidget.h>
#include <QDebug>
#include <QDirIterator>
#include <qaction.h>

//#include "./memoryCheck/setDebugNew.h"
//#include "./memoryCheck/reportingHook.h"

//! global variables used by the AddInManager
//! global variable reference used to store AddInManager reference, as the AIM is singleton this variable is principally only
//! accessed by the class itself. Its value is return also by the getReference \ref AddInManager::method of AIM
ito::AddInManager* ito::AddInManager::m_pAddInManager = NULL;
//! list holding all available dataIO plugins
QList<QObject *> ito::AddInManager::m_addInListDataIO = QList<QObject *>();
//! list holding all available actuator plugins
QList<QObject *> ito::AddInManager::m_addInListAct = QList<QObject *>();
//! list holding all available algorithm plugins
QList<QObject *> ito::AddInManager::m_addInListAlgo = QList<QObject *>();
//! hash holding all available algorithms
QHash<QString, ito::AddInAlgo::FilterDef *> ito::AddInManager::m_filterList = QHash<QString, ito::AddInAlgo::FilterDef *>();

QHash<QString, ito::AddInAlgo::AlgoWidgetDef *> ito::AddInManager::m_algoWidgetList = QHash<QString, ito::AddInAlgo::AlgoWidgetDef *>();
QHash<void*, ito::FilterParams *> ito::AddInManager::filterParamHash = QHash<void*, ito::FilterParams*>();
QMultiHash<QString, ito::AddInAlgo::FilterDef *> ito::AddInManager::m_filterListInterfaceTag = QMultiHash<QString, ito::AddInAlgo::FilterDef *>();
QList<ito::PluginLoadStatus> ito::AddInManager::m_pluginLoadStatus = QList<ito::PluginLoadStatus>();

namespace ito
{

    //----------------------------------------------------------------------------------------------------------------------------------
    /** decrements the reference counter of arguments passed to a plugin if necessary
    *   @param [in] ai          AddIn to which the parameters are passed
    *   @param [in] paramsMand  mandatory argument parameters
    *   @param [in] paramsOpt   optional argument parameters
    *
    *   This function decrements the reference counter of plugins passed to other plugins as parameters, to enable
    *   a closing of the passed plugins when they are no longer used by any other plugin.
    */
    ito::RetVal decRefParamPlugins(ito::AddInBase *ai)
    {
        ito::RetVal retval(ito::retOk);
        QVector<ito::AddInBase::AddInRef *> *argAddInList = ai->getArgAddIns();
        ito::AddInManager *aim = ito::AddInManager::getInstance();

        for (int n = 0; n < (*argAddInList).size(); n++)
        {
            ito::AddInBase::AddInRef *ref = (*argAddInList)[n];
            //if (!(ref->type & ito::ParamBase::Axis))
            //{
                ito::AddInBase *closeAi = reinterpret_cast<ito::AddInBase*>((*argAddInList)[n]->ptr);
                ito::AddInInterfaceBase *aib = ai->getBasePlugin();
                if (aib)
                {
                    aim->decRef(&closeAi);
                }
                else
                {
                    retval += ito::retError;
                }
            //}
            delete ref;
        }
        (*argAddInList).clear();

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** increments the reference counter of arguments passed to a plugin if necessary
    *   @param [in] ai          AddIn to which the parameters are passed
    *   @param [in] paramsMand  mandatory argument parameters
    *   @param [in] paramsOpt   optional argument parameters
    *
    *   This function increments the reference counter of plugins passed to other plugins as parameters, to avoid
    *   the passed plugins are closed while they are still in use by the other plugin.
    */
    void incRefParamPlugins(ito::AddInBase *ai, QVector<ito::ParamBase> *paramsMand, QVector<ito::ParamBase> *paramsOpt)
    {
        void* hwRefPtr = NULL;

        if (paramsMand)
        {
            QVector<ito::AddInBase::AddInRef *> *addInArgList = ai->getArgAddIns();
            for (int n = 0; n < paramsMand->size(); n++)
            {
                ito::ParamBase *param = &((*paramsMand)[n]);
                
                if (param->getType() == (ParamBase::HWRef & ito::paramTypeMask))
                {
                    hwRefPtr = param->getVal<void *>();
                    if (hwRefPtr)
                    {
                        /*if (!(param->getFlags() & ito::ParamBase::Axis))
                        {*/
                            ito::AddInBase *aib = reinterpret_cast<ito::AddInBase*>(hwRefPtr);
                            if (aib)
                            {
                                ito::AddInInterfaceBase *ab = aib->getBasePlugin();
                                ab->incRef(aib);
                                ito::AddInBase::AddInRef *ref = new ito::AddInBase::AddInRef(aib, param->getType() & param->getFlags());
                                (*addInArgList).append(ref);
                            }
                        //}
                    }
                }
            }
        }

        if (paramsOpt)
        {
            QVector<ito::AddInBase::AddInRef*> *addInArgList = ai->getArgAddIns();
            for (int n = 0; n < paramsOpt->size(); n++)
            {
                ito::ParamBase *param = &((*paramsOpt)[n]);
                if (param->getType() == (ParamBase::HWRef & ito::paramTypeMask))
                {
                    hwRefPtr = param->getVal<void *>();
                    if (hwRefPtr)
                    {
                        /*if (!(param->getFlags() & ito::ParamBase::Axis))
                        {*/
                            ito::AddInBase *aib = reinterpret_cast<ito::AddInBase*>(hwRefPtr);
                            if (aib)
                            {
                                ito::AddInInterfaceBase *ab = aib->getBasePlugin();
                                ab->incRef(aib);
                                ito::AddInBase::AddInRef *ref = new ito::AddInBase::AddInRef(aib, param->getType() & param->getFlags());
                                (*addInArgList).append(ref);
                            }
                        //}
                    }
                }
            }
        }
    }

    //! AddInManager implementation
    //----------------------------------------------------------------------------------------------------------------------------------
    /** getInstance method, retrieves Instance of the AddInManager (or opens it if no instance exists)
    *   @return instance of the AddInManager
    *
    *   This method returns the instance of the AddInManager, i.e. if the AddInManager has not been started, it is started then.
    *   Otherwise the reference to the open AddInManager is returned
    */
    AddInManager * AddInManager::getInstance(void)
    {
        if (AddInManager::m_pAddInManager == NULL)
        {
            AddInManager::m_pAddInManager = new ito::AddInManager();
        }
        return AddInManager::m_pAddInManager;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** closeInstance
    *   @return ito::retOk
    *
    *   closes the instance of the AddInManager - should only be called at the very closing of the main program
    */
    RetVal AddInManager::closeInstance(void)
    {
        if (AddInManager::m_pAddInManager)
        {
            delete AddInManager::m_pAddInManager;
        }
        return ito::retOk;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** scanAddInDir
    *   @param path directory path to search in (currently unused)
    *   @return     returns ito::retOk on success otherwise ito::retError
    *
    *   This method searches the plugin directory which is currently assumed to be in the main programs folder
    *   and must habe the name "plugins" for loadable plugins. The found plugins are sorted into the three lists
    *   with the available plugins (ito::AddInManager::m_addInListDataIO, ito::AddInManager::m_addInListAct, ito::AddInManager::m_addInListAlg)
    */
    const RetVal AddInManager::scanAddInDir(const QString &path)
    {
        RetVal retValue = retOk;
        bool firstStart = false;
        bool pluginsFolderExists = true;
        QDir pluginsDir;
        if (path.isEmpty() || path == "")
        {
            firstStart = true;
            m_plugInModel.resetModel(true);

            //search for base plugin folder
            pluginsDir = QDir(qApp->applicationDirPath());

#if defined(WIN32)
            if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
            {
                pluginsDir.cdUp();
            }
#elif defined(__APPLE__)
            if (pluginsDir.dirName() == "MacOS")
            {
                pluginsDir.cdUp();
                pluginsDir.cdUp();
                pluginsDir.cdUp();
            }
#endif
            if (!pluginsDir.cd("plugins"))
            {
                //plugins-folder could not be found.
                pluginsFolderExists = false;
            }
        }
        else
        {
            pluginsDir.setPath(path);
        }

        if (pluginsDir.exists() == false)
        {
            QString dirErr = QObject::tr("directory '%1' could not be found").arg(pluginsDir.canonicalPath());
            retValue += RetVal(retError, 0, dirErr.toLatin1().data());
        }
        else if (!pluginsFolderExists)
        {
            retValue += RetVal(retWarning, 0, QObject::tr("plugins folder could not be found").toLatin1().data());
        }
        else
        {
            QString absoluteAddInPath;

            foreach (const QString &folderName, pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
            {
                absoluteAddInPath = QDir::cleanPath(pluginsDir.absoluteFilePath(folderName));
                retValue += scanAddInDir(absoluteAddInPath);
            }

            QStringList filters;
#ifdef linux
            filters << "*.a" << "*.so";
#elif (defined __APPLE__)
            filters << "*.a" << "*.dylib";            
#else
            filters << "*.dll";
#endif 

            foreach (const QString &fileName, pluginsDir.entryList(filters, QDir::Files))
            {
                absoluteAddInPath = QDir::cleanPath(pluginsDir.absoluteFilePath(fileName));
                if (QLibrary::isLibrary(absoluteAddInPath))
                {
                    retValue += loadAddIn(absoluteAddInPath);
                }
            }
        }

        if (firstStart)
        {
            m_plugInModel.resetModel(false);
        }

        return retValue;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param filename 
    *   @return RetVal    
    */
    RetVal AddInManager::loadAddIn(QString &filename)
    {
        RetVal retValue(retOk);
        QString message;
        QFileInfo finfo(filename);
        ito::PluginLoadStatus pls;

        if (QLibrary::isLibrary(filename) == false)
        {
            message = QObject::tr("filename is no itom plugin library: %1").arg(filename);
            qDebug() << message;
            retValue += RetVal(retError, 1001, message.toLatin1().data());
        }
        else
        {
            emit splashLoadMessage(QObject::tr("scan and load plugins (%1)").arg(finfo.fileName()), Qt::AlignRight | Qt::AlignBottom);
            QCoreApplication::processEvents();

            //load translation file
            QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
            QStringList startupScripts;

            settings.beginGroup("Language");
            QString language = settings.value("language", "en").toString();
            QByteArray codec =  settings.value("codec", "UTF-8" ).toByteArray();
            settings.endGroup();

            QFileInfo fileInfo(filename);
            QDir fileInfoDir = fileInfo.dir();
            fileInfoDir.cdUp();
            if (language != "en_US" && fileInfoDir.absolutePath() == qApp->applicationDirPath() + "/plugins")
            {
                QLocale local = QLocale(language); //language can be "language[_territory][.codeset][@modifier]"
                QString translationPath = fileInfo.path() + "/translation";
                QString languageStr = local.name().left(local.name().indexOf("_", 0, Qt::CaseInsensitive));
                QDirIterator it(translationPath, QStringList("*_" + languageStr + ".qm"), QDir::Files);
                if (it.hasNext())
                {
                    QString translationLocal = it.next();
                    m_Translator.append(new QTranslator);
                    m_Translator.last()->load(translationLocal, translationPath);
                    if (m_Translator.last()->isEmpty())
                    {
                        message = QObject::tr("Unable to load translation file '%1'. Translation file is empty.").arg(translationPath + '/' + translationLocal);
                        qDebug() << message;
                        pls.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(plsfWarning, message));
                    }
                    else
                    {
                        QCoreApplication::instance()->installTranslator(m_Translator.last());
                    }
                }
                else
                {
    //                message = QObject::tr("Unable to find translation file for plugin '%1'.").arg(fileInfo.baseName());
                    message = QObject::tr("Unable to find translation file.");
                    qDebug() << message;
                    pls.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(plsfWarning, message));
                }
            }

            QPluginLoader *loader = new QPluginLoader(filename);
            QObject *plugin = loader->instance();
            if (plugin)
            {
                ito::AddInInterfaceBase *ain = qobject_cast<ito::AddInInterfaceBase *>(plugin);
                pls.filename = filename;

                if (ain)
                {
                    ain->setFilename(filename);
                    ain->setApiFunctions(ITOM_API_FUNCS);
                    ain->setApiFunctionsGraph(ITOM_API_FUNCS_GRAPH);
                    ain->setLoader(loader);
                    //the event User+123 is emitted by AddInManager, if the API has been prepared and can
                    //transmitted to the plugin. This assignment cannot be done directly, since 
                    //the array ITOM_API_FUNCS is in another scope if called from itom. By sending an
                    //event from itom to the plugin, this method is called and ITOM_API_FUNCS is in the
                    //right scope. The methods above only set the pointers in the "wrong"-itom-scope (which
                    //also is necessary if any methods of the plugin are directly called from itom).
                    QEvent evt((QEvent::Type)(QEvent::User+123));
                    QCoreApplication::sendEvent(ain, &evt);

                    switch (ain->getType()&(ito::typeDataIO|ito::typeAlgo|ito::typeActuator))
                    {
                    case typeDataIO:
                        retValue += loadAddInDataIO(plugin, pls);
                        break;

                    case typeActuator:
                        retValue += loadAddInActuator(plugin, pls);
                        break;

                    case typeAlgo:
                        retValue += loadAddInAlgo(plugin, pls);
                        break;

                    default:
                        message = QObject::tr("Plugin with filename '%1' is unknown.").arg(filename);
                        qDebug() << message;
                        //retValue += RetVal(retError, 1003, message.toLatin1().data());
                        pls.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(plsfError, message));
                        break;
                    }
                    m_pluginLoadStatus.append(pls);
                }
                else
                {
                    //check whether this instance is an older or newer version of AddInInterface
                    QObject *obj = qobject_cast<QObject*>(plugin);
                    if (obj)
                    {
                        if (obj->qt_metacast("ito::AddInInterfaceBase") != NULL)
                        {
                            int i = 0;
                            const char* oldName = ito_AddInInterface_OldVersions[0];
                            
                            while(oldName != NULL)
                            {
                                if (obj->qt_metacast(oldName) != NULL)
                                {
                                    message = QObject::tr("AddIn '%1' fits to the obsolete interface %2. The AddIn interface of this version of 'itom' is %3.").arg(filename).arg(oldName).arg(ito_AddInInterface_CurrentVersion);
                                    break;
                                }
                                oldName = ito_AddInInterface_OldVersions[++i];
                            }
                            if (oldName == NULL)
                            {
                                message = QObject::tr("AddIn '%1' fits to a new addIn-interface, which is not supported by this version of itom. The AddIn interface of this version of 'itom' is %2.").arg(filename).arg(ito_AddInInterface_CurrentVersion);
                            }
                        }
                        else
                        {
                            message = QObject::tr("AddIn '%1' does not fit to the general interface AddInInterfaceBase").arg(filename);
                        }
                    }
                    else
                    {
                        message = QObject::tr("AddIn '%1' is not derived from class QObject.").arg(filename).arg(loader->errorString());
                    }
                    qDebug() << message;
                    //retValue += RetVal(retError, 1003, message.toLatin1().data());
                    pls.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(plsfError, message));
                    m_pluginLoadStatus.append(pls);

//                    delete plugin;
                    loader->unload();
                    DELETE_AND_SET_NULL(loader);
                }
            }
            else
            {
                QString notValidQtLibraryMsg = QLibrary::tr("The file '%1' is not a valid Qt plugin.").arg("*");
                QRegExp rx(notValidQtLibraryMsg, Qt::CaseSensitive, QRegExp::Wildcard);
                qDebug() << loader->errorString();
                if (rx.exactMatch(loader->errorString()))
                {
                    message = QObject::tr("Library '%1' was ignored. Message: %2").arg(filename).arg(loader->errorString());
                    qDebug() << message;
                    pls.filename = filename;
                    pls.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(plsfIgnored, message));
                    m_pluginLoadStatus.append(pls);
                }
                else
                {
                    //This regular expression is used to check whether the error message during loading a plugin contains the words
                    //'debug' or 'release'. This means, that a release plugin is tried to be loaded with a debug version of itom or vice-versa
                    QRegExp regExpDebugRelease(".*(release|debug).*", Qt::CaseInsensitive); 
                    if (regExpDebugRelease.exactMatch(loader->errorString()))
                    {
                        message = QObject::tr("AddIn '%1' could not be loaded. Error message: %2").arg(filename).arg(loader->errorString());
                        qDebug() << message;
                        pls.filename = filename;
                        ito::PluginLoadStatusFlags flags(plsfWarning | plsfRelDbg);
                        pls.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(flags, message));
                        m_pluginLoadStatus.append(pls);
                    }
                    else
                    {
                        message = QObject::tr("AddIn '%1' could not be loaded. Error message: %2").arg(filename).arg(loader->errorString());
                        qDebug() << message;
                        //retValue += RetVal(retError, 1003, message.toLatin1().data());
                        pls.filename = filename;
                        pls.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(plsfError, message));
                        m_pluginLoadStatus.append(pls);
                    }
                }
                loader->unload();
                DELETE_AND_SET_NULL(loader);
            }
        }

        return retValue;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param plugin 
    *   @param pluginLoadStatus
    *   @return RetVal    
    */
    RetVal AddInManager::loadAddInDataIO(QObject *plugin, ito::PluginLoadStatus &pluginLoadStatus)
    {
        if (!m_addInListDataIO.contains(plugin))
        {
            m_addInListDataIO.append(plugin);
            pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfOk, QObject::tr("%1 (DataIO) loaded").arg(plugin->objectName())));
            return retOk;
        }
        else
        {
            pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfWarning, QObject::tr("Plugin %1 (DataIO) already exists. Duplicate rejected.").arg(plugin->objectName())));
            return retWarning;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param plugin 
    *   @param pluginLoadStatus
    *   @return RetVal    
    */
    RetVal AddInManager::loadAddInActuator(QObject *plugin, ito::PluginLoadStatus &pluginLoadStatus)
    {
        if (!m_addInListAct.contains(plugin))
        {
            m_addInListAct.append(plugin);
            pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfOk, QObject::tr("%1 (Actuator) loaded").arg(plugin->objectName())));
            return retOk;
        }
        else
        {
            pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfWarning, QObject::tr("Plugin %1 (Actuator) already exists. Duplicate rejected.").arg(plugin->objectName())));
            return retWarning;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param plugin 
    *   @param pluginLoadStatus
    *   @return RetVal    
    */
    RetVal AddInManager::loadAddInAlgo(QObject *plugin, ito::PluginLoadStatus &pluginLoadStatus)
    {
        QString message;
        ito::RetVal retValue;
        if (!m_addInListAlgo.contains(plugin))
        {
            m_addInListAlgo.append(plugin);

            ito::AddInAlgo *algoInst = NULL;
            QVector<ito::ParamBase> paramsMand, paramsOpt;
            initAddIn(m_addInListAlgo.size() - 1, plugin->objectName(), &algoInst, &paramsMand, &paramsOpt, true);
            if (!algoInst)
            {
                message = QObject::tr("error initializing plugin: %1").arg(plugin->objectName());
                qDebug() << message;
                retValue += RetVal(retError, 1002, message.toLatin1().data());
                pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfError, message));
            }
            else
            {
                QHash<QString, ito::AddInAlgo::FilterDef *> funcList;
                algoInst->getFilterList(funcList);

                QHash<QString, ito::AddInAlgo::FilterDef *>::const_iterator it = funcList.constBegin();
                ito::AddInInterfaceBase *ain = qobject_cast<ito::AddInInterfaceBase *>(plugin);

                ito::AddInAlgo::FilterDef *fd;
                ito::RetVal validRet;
                QVector<ito::Param> paramsMand, paramsOpt, paramsOut;
                QStringList tags;
                while (it != funcList.constEnd())
                {
                    fd = *it;
                    if (m_filterList.contains(it.key()))
                    {
                        algoInst->rejectFilter(it.key());
                        message = QObject::tr("Filter '%1' rejected since a filter with the same name already exists in global filter list").arg(it.key());
                        qDebug() << message;
                        retValue += RetVal(retWarning, 1004, message.toLatin1().data());
                        pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfWarning, message));
                    }
                    else
                    {
                        //1. first check if filter has a valid interface (if indicated)
                        validRet = ito::retOk;
                        tags.clear();
                        if (fd->m_interface == 0 || m_algoInterfaceValidator->isValidFilter(*fd,validRet,tags))
                        {

                            //2. hash the mand, opt and out param vectors from the filter (if not yet done, since multiple filters can use the same paramFunc-function.
                            paramsMand.clear();
                            paramsOpt.clear();
                            paramsOut.clear();
                            if (! filterParamHash.contains((void*)fd->m_paramFunc))
                            {
                                validRet += fd->m_paramFunc(&paramsMand, &paramsOpt, &paramsOut);

                                if (!validRet.containsError())
                                {
                                    ito::FilterParams *fp = new ito::FilterParams();
                                    fp->paramsMand = paramsMand;
                                    fp->paramsOpt = paramsOpt;
                                    fp->paramsOut = paramsOut;
                                    filterParamHash[(void*)fd->m_paramFunc] = fp;
                                }
                            }

                            if (!validRet.containsError())
                            {
                                fd->m_pBasePlugin = ain; //put pointer to corresponding AddInInterfaceBase to this filter
                                fd->m_name = it.key();
                                m_filterList.insert(it.key(), fd);
                                pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfOk, QObject::tr("Filter %1 loaded").arg(it.key())));

                                if (tags.size() == 0) tags.append("");
                                foreach (const QString &tag, tags)
                                {
                                    m_filterListInterfaceTag.insert(QString::number(fd->m_interface) + "_" + tag, fd);
                                }
                            }
                            else
                            {
                                algoInst->rejectFilter(it.key() );
                                if (validRet.hasErrorMessage())
                                {
                                    message = "Filter " + it.key() + " rejected. The filter parameters could not be loaded: " + QLatin1String(validRet.errorMessage());
                                }
                                else
                                {
                                    message = "Filter " + it.key() + " rejected. The filter parameters could not be loaded.";
                                }
                                qDebug() << message;
                                pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfError, message));
                            }
                        }
                        else if (validRet.containsError() || fd->m_interface != 0) //the !=0 check is only to make sure that we always get into that case if the filter is somehow wrong
                        {
                            algoInst->rejectFilter(it.key());
                            if (validRet.hasErrorMessage())
                            {
                                message = "Filter " + it.key() + " rejected. It does not correspond to the algorithm interface: " + QLatin1String(validRet.errorMessage());
                            }
                            else
                            {
                                message = "Filter " + it.key() + " rejected. It does not correspond to the algorithm interface.";
                            }
                            qDebug() << message;
                            pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfError, message));
                        }
                    }
                    ++it;
                }

                QHash<QString, ito::AddInAlgo::AlgoWidgetDef *> algoWidgetList;
                ito::AddInAlgo::AlgoWidgetDef *ad;
                algoInst->getAlgoWidgetList(algoWidgetList);

                QHash<QString, ito::AddInAlgo::AlgoWidgetDef *>::const_iterator jt = algoWidgetList.constBegin();
                while (jt != algoWidgetList.constEnd())
                {
                    if (m_algoWidgetList.contains(jt.key()))
                    {
                        algoInst->rejectAlgoWidget(jt.key());
                        message = QObject::tr("Widget '%1' rejected since widget with the same name already exists in global plugin widget list").arg(jt.key());
                        qDebug() << message;
                        retValue += RetVal(retWarning, 1005, message.toLatin1().data());
                    }
                    else
                    {
                        ad = *jt;
                        //1. first check if filter has a valid interface (if indicated)
                        validRet = ito::retOk;
                        tags.clear();
                        if (ad->m_interface == 0 || m_algoInterfaceValidator->isValidWidget(*ad, validRet,tags))
                        {

                            //2. hash the mand, opt and out param vectors from the widget  (if not yet done, since multiple filters can use the same paramFunc-function.
                            paramsMand.clear();
                            paramsOpt.clear();
                            paramsOut.clear();
                            if (! filterParamHash.contains((void*)ad->m_paramFunc))
                            {
                                ad->m_paramFunc(&paramsMand, &paramsOpt, &paramsOut);
                                ito::FilterParams *fp = new ito::FilterParams();
                                fp->paramsMand = paramsMand;
                                fp->paramsOpt = paramsOpt;
                                fp->paramsOut = paramsOut;
                                filterParamHash[(void*)ad->m_paramFunc] = fp;
                            }

                            ad->m_pBasePlugin = ain; //put pointer to corresponding AddInInterfaceBase to this filter
                            ad->m_name = jt.key();
                            m_algoWidgetList.insert(jt.key(), ad);
                            pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfOk, QObject::tr("Widget %1 loaded").arg(jt.key())));
                        }
                        else if (validRet.containsError())
                        {
                            algoInst->rejectAlgoWidget(jt.key());
                            if (validRet.hasErrorMessage())
                            {
                                message = "Widget " + jt.key() + " rejected. It does not correspond to the algorithm interface: " + QLatin1String(validRet.errorMessage());
                            }
                            else
                            {
                                message = "Widget " + jt.key() + " rejected. It does not correspond to the algorithm interface.";
                            }
                            qDebug() << message;
                            pluginLoadStatus.messages.append(QPair<ito::PluginLoadStatusFlags, QString>(ito::plsfError, message));
                        }
                    }
                    ++jt;
                }
            }
        }

        return retValue;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** getInitParams
    *   @param [in]  name        plugin name for which the initialisation parameters should be retrieved
    *   @param [in]  pluginType  plugin type, i.e. in which of the plugin lists should be searched for the plugin
    *   @param [out] pluginNum   number of the plugin in the plugin list, this number is needed later to create an instance of the plugin class
    *   @param [out] paramsMand  mandatory initialisation parameters
    *   @param [out] paramsOpt   optional initialisation parameters
    *   @return      ito::retOk on success ito::retError otherwise
    *
    *   The getInitParams method searchs the plugin list given by plugin type for a plugin with the name 'name'. In case the according
    *   plugin is found its number, mandatory and optional initialisation parameters are returned.
    *
    *   Please consider that this method returns pointers to the original initialization parameter vectors. If you change the value of these elements
    *   consider to copy the complete vector.
    */
    const RetVal AddInManager::getInitParams(const QString &name, const int pluginType, int *pluginNum, QVector<ito::Param> *&paramsMand, QVector<ito::Param> *&paramsOpt)
    {
        ito::RetVal ret;
        *pluginNum = -1;

        if (pluginType & ito::typeActuator)
        {
            for (int n=0; n < m_addInListAct.size(); n++)
            {
                if (QString::compare(m_addInListAct[n]->objectName(), name, Qt::CaseInsensitive) == 0)
                {
                    *pluginNum = n;
                    paramsMand = (qobject_cast<ito::AddInInterfaceBase *>(m_addInListAct[n]))->getInitParamsMand();
                    paramsOpt = (qobject_cast<ito::AddInInterfaceBase *>(m_addInListAct[n]))->getInitParamsOpt();
                    ret = ito::retOk;
                    break;
                }
            }
        }
        else if (pluginType & ito::typeDataIO)
        {
            for (int n=0; n < m_addInListDataIO.size(); n++)
            {
                if (QString::compare(m_addInListDataIO[n]->objectName(), name, Qt::CaseInsensitive) == 0)
                {
                    *pluginNum = n;
                    paramsMand = (qobject_cast<ito::AddInInterfaceBase *>(m_addInListDataIO[n]))->getInitParamsMand();
                    paramsOpt = (qobject_cast<ito::AddInInterfaceBase *>(m_addInListDataIO[n]))->getInitParamsOpt();
                    ret = ito::retOk;
                    break;
                }
            }
        }
        else if (pluginType & ito::typeAlgo)
        {
            for (int n=0; n < m_addInListAlgo.size(); n++)
            {
                if (QString::compare(m_addInListAlgo[n]->objectName(), name, Qt::CaseInsensitive) == 0)
                {
                    *pluginNum = n;
                    ret = ito::retOk;
                    break;
                }
            }
        }
        else
        {
            ret += ito::RetVal(ito::retError, 0, QObject::tr("invalid plugin type. Only typeDataIO, typeActuator or typeAlgo are allowed.").toLatin1().data());
        }

        if (*pluginNum < 0)
        {
            ret += ito::RetVal(ito::retError, 0, QObject::tr("Plugin '%1' not found in list of given type").arg(name).toLatin1().data());
        }

        return ret;
    }
    
    //----------------------------------------------------------------------------------------------------------------------------------
    /** getPlugInInfo
    *   @param [in]  name               plugin name for which type and number should be retrieved
    *   @param [out] pluginType         plugin type, i.e. in which of the plugin lists should be searched for the plugin
    *   @param [out] pluginNum          number of the plugin in the plugin list, this number is needed later to create an instance of the plugin class
    *   @param [out] pluginTypeString   type of the plugin as string
    *   @param [out] author             author name or company
    *   @param [out] description        short discribtion of the plugin
    *   @param [out] detaildescription  detail discription of the plugin
    *   @param [out] version            plugin version number
    *   @return      ito::retOk on success ito::retError otherwise
    *
    *   The getPlugInInfo method searchs in all three plugin lists for a plugin with the name 'name'. In case the according
    *   plugin is found its information about number, name ... returned. For all parameters of type char** provide the address to a char*-variable.
    *   Then, a newly allocated \0-terminated string is returned. Don't forget to free this pointer after using it (free not delete!).
    */
    const RetVal AddInManager::getPluginInfo(const QString &name, int &pluginType, int &pluginNum, int &version, QString &typeString, QString &author, QString &description, QString &detaildescription, QString &license, QString &about)
    {
        ito::RetVal ret = ito::RetVal(ito::retError, 0, QObject::tr("plugin not found").toLatin1().data());
        int found = 0;
        ito::AddInInterfaceBase *aib = NULL;

        //test actuator (objectName)
        for (int n=0; n < m_addInListAct.size(); n++)
        {
            if (QString::compare(m_addInListAct[n]->objectName(),name,Qt::CaseInsensitive) == 0)
            {
                pluginNum = n;
                pluginType = ito::typeActuator;

                typeString = "Actuator";
                aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListAct[n]);
                found = 1;
                break;
            }
        }

        if (!found) //test dataIO (objectName)
        {
            for (int n=0; n < m_addInListDataIO.size(); n++)
            {
                if (QString::compare(m_addInListDataIO[n]->objectName(),name,Qt::CaseInsensitive) == 0)
                {
                    pluginNum = n;
                    pluginType = ito::typeDataIO;

                    typeString = "DataIO";
                    aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListDataIO[n]);
                    found = 1;
                    break;
                }
            }
        }

        if (!found) //test Algorithm (objectName)
        {
            for (int n=0; n < m_addInListAlgo.size(); n++)
            {
                if (QString::compare(m_addInListAlgo[n]->objectName(),name,Qt::CaseInsensitive) == 0)
                {
                    pluginNum = n;
                    pluginType = ito::typeAlgo;

                    typeString = "Algorithm";
                    aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListAlgo[n]);
                    found = 1;
                    break;
                }
            }
        }

        //if nothing found until then, try to find name as filename within the dll-filename of the plugin
        if (!found)
        {
            QFileInfo fi;
            QString name_(name);
#ifdef _DEBUG
            name_ += "d"; //since we are now comparing with the filename, we append 'd' that corresponds to the debug versions of the plugin dll filenames
#endif

            //test actuator (objectName)
            for (int n=0; n < m_addInListAct.size(); n++)
            {
                aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListAct[n]);
                fi.setFile(aib->getFilename());
                if (QString::compare(fi.completeBaseName(),name_,Qt::CaseInsensitive) == 0)
                {
                    pluginNum = n;
                    pluginType = ito::typeActuator;
                    typeString = "Actuator";
                    found = 1;
                    break;
                }
            }

            if (!found) //test dataIO (objectName)
            {
                for (int n=0; n < m_addInListDataIO.size(); n++)
                {
                    aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListDataIO[n]);
                    fi.setFile(aib->getFilename());
                    if (QString::compare(fi.completeBaseName(),name_,Qt::CaseInsensitive) == 0)
                    {
                        pluginNum = n;
                        pluginType = ito::typeDataIO;
                        typeString = "DataIO";
                        found = 1;
                        break;
                    }
                }
            }

            if (!found) //test Algorithm (objectName)
            {
                for (int n=0; n < m_addInListAlgo.size(); n++)
                {
                    aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListAlgo[n]);
                    fi.setFile(aib->getFilename());
                    if (QString::compare(fi.completeBaseName(),name_,Qt::CaseInsensitive) == 0)
                    {
                        pluginNum = n;
                        pluginType = ito::typeAlgo;
                        typeString = "Algorithm";
                        found = 1;
                        break;
                    }
                }
            }
        }

        if (aib && found)
        {
            author = aib->getAuthor();
            description = aib->getDescription();
            detaildescription =aib->getDetailDescription();
            version = aib->getVersion();
            license = aib->getLicenseInfo();
            about = aib->getAboutInfo();
            ret = ito::retOk;
        }

        return ret;
    }
    
    //----------------------------------------------------------------------------------------------------------------------------------
    /** initAddIn initialize new instance of a dataIO addIn class
    *   @param [in]  addIn      pointer to newly initialized pluginIn
    *   @return      on success ito::retOk
    *
    *   checks if addIn has a docking widget and if so, registers this docking widget to mainWindow
    */
    ito::RetVal AddInManager::initDockWidget(const ito::AddInBase *addIn)
    {
        QMainWindow *win = qobject_cast<QMainWindow*>(AppManagement::getMainWindow());
        if (addIn->getDockWidget() && win)
        {
            QDockWidget* dockWidget = addIn->getDockWidget();
            Qt::DockWidgetArea area;
            bool floating;
            bool visible;
            addIn->dockWidgetDefaultStyle(floating, visible, area);
            win->addDockWidget(area, dockWidget);
            dockWidget->setFloating(floating);
            dockWidget->setVisible(visible);

            /*bool restored =*/ win->restoreDockWidget(dockWidget);
        }

        return ito::retOk;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** initAddIn initialize new instance of a dataIO addIn class
    *   @param [in]  pluginNum      number of the plugin in the plugin list, retrieved with \ref getInitParams
    *   @param [in]  name           name of the plugin to be initialized, this just a check that number and name correspond, principally it should not be necessary to pass the name
    *   @param [out] addIn          pointer to the new instance of the plugin class
    *   @param [in]  paramsMand     mandatory initialisation parameters which are required by the initialisation. As this vector should(must) be retrieved from the plugin
    *                               previously with the \ref getInitParams method it should always be filled with meaningful values
    *   @param [in]  paramsOpt      mandatory initialisation parameters which may optionally be passed to the initialisation. As this vector should(must) be retrieved from the plugin
    *                               previously with the \ref getInitParams method it should always be filled with meaningful values
    *   @param [in, out] aimWait    wait condition for calls from other threads. See also \ref ItomSharedSemaphore
    *   @return      on success ito::retOk, ito::retError otherwise
    *
    *   A new instance from the addIn class is created then the newly created object is moved into a new thread. Afterwards the classes init method is invoked with
    *   the passed mandatory and optional parameters. As a last step the plugins parameters are loaded from the plugins parameters xml file \ref loadParamVals.
    */
    ito::RetVal AddInManager::initAddIn(const int pluginNum, const QString &name, ito::AddInDataIO **addIn, QVector<ito::ParamBase> *paramsMand, QVector<ito::ParamBase> *paramsOpt, bool autoLoadPluginParams, ItomSharedSemaphore *aimWait)
    {
        ItomSharedSemaphoreLocker locker(aimWait);
        ito::RetVal retval = ito::retOk;
        ItomSharedSemaphore *waitCond = NULL;
        ito::tAutoLoadPolicy policy = ito::autoLoadNever;
        ito::AddInInterfaceBase *aib = NULL;
        bool callInitInNewThread;
        bool timeoutOccurred = false;

        if (QString::compare((m_addInListDataIO[pluginNum])->objectName(), name, Qt::CaseInsensitive) != 0)
        {
            retval += ito::RetVal(ito::retError, 0, QObject::tr("Wrong plugin name").toLatin1().data());
        }
        else
        {
            aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListDataIO[pluginNum]);
            m_plugInModel.insertInstance(aib, true); //begin insert
            retval += aib->getAddInInst(reinterpret_cast<ito::AddInBase **>(addIn));
            if ((!addIn) || (!*addIn))
            {
                retval += ito::RetVal(ito::retError, 0, tr("Plugin instance is invalid (NULL)").toLatin1().data());
            }
        }

        if (!retval.containsError())
        {
            //ref-count of plugin must be zero (that means one instance is holder a single reference), this is rechecked in the following line
            if (aib->getRef(*addIn) != 0)
            {
                retval += ito::RetVal(ito::retWarning, 0, "reference counter of plugin has to be initialized with zero. This is not the case for this plugin (Please contact the plugin developer).");
            }

            if ((*addIn)->getBasePlugin() == NULL || (*addIn)->getBasePlugin()->getType() == 0)
            {
                retval += ito::RetVal(ito::retError, 2000, QObject::tr("Base plugin or appropriate plugin type not indicated for this plugin.").toLatin1().data());
            }
        }

        if (!retval.containsError())
        {
            retval += initDockWidget(static_cast<ito::AddInBase*>(*addIn));

            waitCond = new ItomSharedSemaphore();

            callInitInNewThread = (*addIn)->getBasePlugin()->getCallInitInNewThread();

            if (callInitInNewThread)
            {
                (*addIn)->MoveToThread();
            }

            QMetaObject::invokeMethod(*addIn, "init", Q_ARG(QVector<ito::ParamBase>*, paramsMand), Q_ARG(QVector<ito::ParamBase>*, paramsOpt), Q_ARG(ItomSharedSemaphore*, waitCond));

            //this gives the plugin's init method to invoke a slot of any gui instance of the plugin within its init method. Else this slot is called after
            //having finished this initAddIn method (since main thread is blocked).
            while (!waitCond->waitAndProcessEvents(AppManagement::timeouts.pluginInitClose, QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers))
            {
                if (!(*addIn)->isAlive())
                {
                    retval += ito::RetVal(ito::retError, 0, QObject::tr("timeout while initializing dataIO").toLatin1().data());
                    timeoutOccurred = true;
                    break;
                }
            }
            retval += waitCond->returnValue;
            waitCond->deleteSemaphore();
            waitCond = NULL;

            if (!callInitInNewThread)
            {
                (*addIn)->MoveToThread();
            }

            if (timeoutOccurred == true)
            {
                //increment depending addIns in order to keep their reference alive while this plugin is in a undefined status.
                incRefParamPlugins(*addIn, paramsMand, paramsOpt);

                retval += registerPluginAsDeadPlugin(*addIn);
                *addIn = NULL;
            }
            else
            {
                //no timeout

                if (!((*addIn)->getBasePlugin()->getType() & ito::typeDataIO) || retval.containsError())
                {
                    if (*addIn != NULL)
                    {
                        m_plugInModel.insertInstance(aib, false); //end insert, since closeAddIn will call beginRemoveRows...
                        retval += closeAddIn(reinterpret_cast<ito::AddInBase*>(*addIn));
                    }
                    *addIn = NULL;
                }
                else
                {
                    incRefParamPlugins(*addIn, paramsMand, paramsOpt);

                    policy = (*addIn)->getBasePlugin()->getAutoLoadPolicy();

                    if (autoLoadPluginParams && policy != ito::autoLoadKeywordDefined)
                    {
                        retval += ito::RetVal(ito::retWarning, 0, QObject::tr("Parameter has own parameter management. Keyword 'autoLoadParams' is ignored.").toLatin1().data());
                    }

                    if (policy == ito::autoLoadAlways || (policy == ito::autoLoadKeywordDefined && autoLoadPluginParams))
                    {
                        retval += loadParamVals(reinterpret_cast<ito::AddInBase*>(*addIn));
                    }
                }
            }

            if (*addIn)
            {
                m_plugInModel.insertInstance(aib, false); //end insert
            }
        }

        if (aimWait)
        {
            aimWait->returnValue = retval;
            aimWait->release();
        }

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** initAddIn initialize new instance of a actuator addIn class
    *   @param [in]  pluginNum      number of the plugin in the plugin list, retrieved with \ref getInitParams
    *   @param [in]  name           name of the plugin to be initialized, this just a check that number and name correspond, principally it should not be necessary to pass the name
    *   @param [out] addIn          pointer to the new instance of the plugin class
    *   @param [in]  paramsMand     mandatory initialisation parameters which are required by the initialisation. As this vector should(must) be retrieved from the plugin
    *                               previously with the \ref getInitParams method it should always be filled with meaningful values
    *   @param [in]  paramsOpt      mandatory initialisation parameters which may optionally be passed to the initialisation. As this vector should(must) be retrieved from the plugin
    *                               previously with the \ref getInitParams method it should always be filled with meaningful values
    *   @param [in, out] aimWait    wait condition for calls from other threads. See also \ref ItomSharedSemaphore
    *   @return      on success ito::retOk, ito::retError otherwise
    *
    *   A new instance from the addIn class is created then the newly created object is moved into a new thread. Afterwards the classes init method is invoked with
    *   the passed mandatory and optional parameters. As a last step the plugins parameters are loaded from the plugins parameters xml file \ref loadParamVals.
    */
    ito::RetVal AddInManager::initAddIn(const int pluginNum, const QString &name, ito::AddInActuator **addIn, QVector<ito::ParamBase> *paramsMand, QVector<ito::ParamBase> *paramsOpt, bool autoLoadPluginParams,  ItomSharedSemaphore *aimWait)
    {
        ItomSharedSemaphoreLocker locker(aimWait);
        ItomSharedSemaphore *waitCond = NULL;
        ito::RetVal retval = ito::retOk;
        ito::tAutoLoadPolicy policy = ito::autoLoadNever;
        ito::AddInInterfaceBase *aib = NULL;
        bool callInitInNewThread;
        bool timeoutOccurred = false;

        if (QString::compare((m_addInListAct[pluginNum])->objectName(), name, Qt::CaseInsensitive) != 0)
        {
            retval += ito::RetVal(ito::retError, 0, QObject::tr("Wrong plugin name").toLatin1().data());
        }
        else
        {
            aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListAct[pluginNum]);
            m_plugInModel.insertInstance(aib, true); //begin insert
            retval += aib->getAddInInst(reinterpret_cast<ito::AddInBase **>(addIn));
            if ((!addIn) || (!*addIn))
            {
                retval += ito::RetVal(ito::retError, 0, tr("Plugin instance is invalid (NULL)").toLatin1().data());
            }
        }

        if (!retval.containsError())
        {
            //ref-count of plugin must be zero (that means one instance is holder a single reference), this is rechecked in the following line
            if (aib->getRef(*addIn) != 0)
            {
                retval += ito::RetVal(ito::retWarning, 0, "reference counter of plugin has to be initialized with zero. This is not the case for this plugin (Please contact the plugin developer).");
            }

            if ((*addIn)->getBasePlugin() == NULL || (*addIn)->getBasePlugin()->getType() == 0)
            {
                retval += ito::RetVal(ito::retError, 2000, QObject::tr("Base plugin or appropriate plugin type not indicated for this plugin.").toLatin1().data());
            }
        }

        if (!retval.containsError())
        {
            retval += initDockWidget(static_cast<ito::AddInBase*>(*addIn));

            waitCond = new ItomSharedSemaphore();

            callInitInNewThread = (*addIn)->getBasePlugin()->getCallInitInNewThread();

            if (callInitInNewThread)
            {
                (*addIn)->MoveToThread();
            }

            QMetaObject::invokeMethod(*addIn, "init", Q_ARG(QVector<ito::ParamBase>*, paramsMand), Q_ARG(QVector<ito::ParamBase>*, paramsOpt), Q_ARG(ItomSharedSemaphore*, waitCond));

            //this gives the plugin's init method to invoke a slot of any gui instance of the plugin within its init method. Else this slot is called after
            //having finished this initAddIn method (since main thread is blocked).
            while (!waitCond->waitAndProcessEvents(AppManagement::timeouts.pluginInitClose, QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers))
            {
                if (!(*addIn)->isAlive())
                {
                    retval += ito::RetVal(ito::retError, 0, QObject::tr("timeout while initializing actuator").toLatin1().data());
                    timeoutOccurred = true;
                    break;
                }
            }
            retval += waitCond->returnValue;
            waitCond->deleteSemaphore();
            waitCond = NULL;

            if (!callInitInNewThread)
            {
                (*addIn)->MoveToThread();
            }

            if (timeoutOccurred == true)
            {
                //increment depending addIns in order to keep their reference alive while this plugin is in a undefined status.
                incRefParamPlugins(*addIn, paramsMand, paramsOpt);

                retval += registerPluginAsDeadPlugin(*addIn);
                *addIn = NULL;
            }
            else
            {
                if (!((*addIn)->getBasePlugin()->getType() & ito::typeActuator) || retval.containsError())
                {
                    if (*addIn != NULL)
                    {
                        m_plugInModel.insertInstance(aib, false); //end insert, since closeAddIn will call beginRemoveRows...
                        retval += closeAddIn(reinterpret_cast<ito::AddInBase*>(*addIn));
                    }
                    *addIn = NULL;
                }
                else
                {
                    incRefParamPlugins(*addIn, paramsMand, paramsOpt);

                    policy = (*addIn)->getBasePlugin()->getAutoLoadPolicy();

                    if (autoLoadPluginParams && policy != ito::autoLoadKeywordDefined)
                    {
                        retval += ito::RetVal(ito::retWarning, 0, QObject::tr("Parameter has own parameter management. Keyword 'autoLoadParams' is ignored.").toLatin1().data());
                    }

                    if (policy == ito::autoLoadAlways || (policy == ito::autoLoadKeywordDefined && autoLoadPluginParams))
                    {
                        retval += loadParamVals(reinterpret_cast<ito::AddInBase*>(*addIn));
                    }
                }
            }

            if (*addIn)
            {
                m_plugInModel.insertInstance(aib, false); //end insert
            }
        }

        if (aimWait)
        {
            aimWait->returnValue = retval;
            aimWait->release();
        }

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** initAddIn initialize new instance of a algo addIn class
    *   @param [in]  pluginNum      number of the plugin in the plugin list, retrieved with \ref getInitParams
    *   @param [in]  name           name of the plugin to be initialized, this just a check that number and name correspond, principally it should not be necessary to pass the name
    *   @param [out] addIn          pointer to the new instance of the plugin class
    *   @param [in]  paramsMand     mandatory initialisation parameters which are required by the initialisation. As this vector should(must) be retrieved from the plugin
    *                               previously with the \ref getInitParams method it should always be filled with meaningful values
    *   @param [in]  paramsOpt      mandatory initialisation parameters which may optionally be passed to the initialisation. As this vector should(must) be retrieved from the plugin
    *                               previously with the \ref getInitParams method it should always be filled with meaningful values
    *   @return      on success ito::retOk, ito::retError otherwise
    *
    *   new instance from the addIn class is created. In contrast to the dataIO and actuator plugins the new object is not moved to a new thread and no init method is called.
    *   As a last step the plugins parameters are loaded from the plugins parameters xml file \ref loadParamVals.
    */
    ito::RetVal AddInManager::initAddIn(const int pluginNum, const QString &name, ito::AddInAlgo **addIn, QVector<ito::ParamBase> * paramsMand, QVector<ito::ParamBase> * paramsOpt, bool autoLoadPluginParams, ItomSharedSemaphore *aimWait)
    {
        ItomSharedSemaphoreLocker locker(aimWait);
        ito::RetVal retval = ito::retOk;
        ito::tAutoLoadPolicy policy = ito::autoLoadNever;
        ito::AddInInterfaceBase *aib = NULL;

        if (QString::compare((m_addInListAlgo[pluginNum])->objectName(), name, Qt::CaseInsensitive) != 0)
        {
            retval += ito::RetVal(ito::retError, 0, QObject::tr("Wrong plugin name").toLatin1().data());
        }
        else
        {
            aib = qobject_cast<ito::AddInInterfaceBase *>(m_addInListAlgo[pluginNum]);
            retval += aib->getAddInInst(reinterpret_cast<ito::AddInBase **>(addIn));
            if ((!addIn) || (!*addIn))
            {
                retval += ito::RetVal(ito::retError, 0, tr("Plugin instance is invalid (NULL)").toLatin1().data());
            }
        }

        if (!retval.containsError())
        {
            //ref-count of plugin must be zero (that means one instance is holder a single reference), this is rechecked in the following line
            if (aib->getRef(*addIn) != 0)
            {
                retval += ito::RetVal(ito::retWarning, 0, "reference counter of plugin has to be initialized with zero. This is not the case for this plugin (Please contact the plugin developer).");
            }

            (*addIn)->init(paramsMand, paramsOpt);

            if (!((*addIn)->getBasePlugin()->getType() & ito::typeAlgo) || retval.containsError())
            {
                if (*addIn != NULL)
                {
                    retval += closeAddIn(reinterpret_cast<ito::AddInBase*>(*addIn));
                }
                *addIn = NULL;
            }
        }

        if (!retval.containsError())
        {
            policy = (*addIn)->getBasePlugin()->getAutoLoadPolicy();

            if (autoLoadPluginParams && policy != ito::autoLoadKeywordDefined)
            {
                retval += ito::RetVal(ito::retWarning, 0, QObject::tr("Parameter has own parameter management. Keyword 'autoLoadParams' is ignored.").toLatin1().data());
            }

            if (policy == ito::autoLoadAlways || (policy == ito::autoLoadKeywordDefined && autoLoadPluginParams))
            {
                retval += loadParamVals(reinterpret_cast<ito::AddInBase*>(*addIn));
            }
        }

        if (aimWait)
        {
            aimWait->returnValue = retval;
            aimWait->release();
        }

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** closeAddIn  close an instance of an actuator addIn object
    *   @param [in]  addIn  the addIn to close
    *   @return      on success ito::retOk, ito::retError otherwise
    *
    *   At first the close method of the plugin class is invoked. Then the \ref closeInst method of the addInInterfaceBase is called.
    */
    ito::RetVal AddInManager::closeAddIn(AddInBase *addIn, ItomSharedSemaphore *aimWait)
    {
        ItomSharedSemaphoreLocker locker(aimWait);
        ito::RetVal retval = ito::retOk;
        ItomSharedSemaphore *waitCond = NULL;
        bool timeout = false;

        ito::AddInInterfaceBase *aib = addIn->getBasePlugin();

        if (aib->getRef(addIn) <= 0) //this instance holds the last reference of the plugin. close it now.
        {
            //we always promised that if the init-method is called in the main thread, the close method is called in the main thread, too.
            //Therefore pull it to the main thread, if necessary.
            if (!aib->getCallInitInNewThread())
            {
                ItomSharedSemaphoreLocker moveToThreadLocker(new ItomSharedSemaphore());
                if (QMetaObject::invokeMethod(addIn, "moveBackToApplicationThread", Q_ARG(ItomSharedSemaphore*, moveToThreadLocker.getSemaphore())))
                {
                    if (moveToThreadLocker->wait(AppManagement::timeouts.pluginInitClose) == false)
                    {
                        retval += ito::RetVal(ito::retWarning, 0, "timeout while pulling plugin back to main thread.");
                    }
                }
                else
                {
                    moveToThreadLocker->deleteSemaphore();
                    retval += ito::RetVal(ito::retWarning, 0, "error invoking method 'moveBackToApplicationThread' of plugin.");
                }
            }

            waitCond = new ItomSharedSemaphore();
            QMetaObject::invokeMethod(addIn, "close", Q_ARG(ItomSharedSemaphore*, waitCond));

            while (waitCond->wait(AppManagement::timeouts.pluginInitClose) == false && !timeout)
            {
                if (addIn->isAlive() == 0)
                {
                    timeout = true;
                    retval += ito::RetVal(ito::retError, 0, "timeout while closing plugin");
                }
            }

            if (!timeout)
            {
                retval += waitCond->returnValue;

                if (aib->getCallInitInNewThread())
                {
                    ItomSharedSemaphoreLocker moveToThreadLocker(new ItomSharedSemaphore());
                    if (QMetaObject::invokeMethod(addIn, "moveBackToApplicationThread", Q_ARG(ItomSharedSemaphore*, moveToThreadLocker.getSemaphore())))
                    {
                        if (moveToThreadLocker->wait(AppManagement::timeouts.pluginInitClose) == false)
                        {
                            retval += ito::RetVal(ito::retWarning, 0, "timeout while pulling plugin back to main thread.");
                        }
                    }
                    else
                    {
                        moveToThreadLocker->deleteSemaphore();
                        retval += ito::RetVal(ito::retWarning, 0, "error invoking method 'moveBackToApplicationThread' of plugin.");
                    }
                }
            
                if (aib->getAutoSavePolicy() == ito::autoSaveAlways)
                {
                    retval += saveParamVals(addIn);
                }

                m_plugInModel.deleteInstance(addIn, true); //begin remove

                retval += decRefParamPlugins(addIn);
                retval += aib->closeInst(&addIn);

                m_plugInModel.deleteInstance(addIn, false); //end remove
            }
            else
            {
                qDebug() << "Plugin could not be safely removed. Unknown state for plugin.";
                //TODO: what happens in the case that the close method does not return???
                //until now, we can not put it to the dead-plugin stack since this stack only handles plugins
                //that could not be initialized.
            }

            waitCond->deleteSemaphore();
            waitCond = NULL;

        }
        else
        {
            aib->decRef(addIn);
        }

        if (aimWait)
        {
            aimWait->returnValue = retval;
            aimWait->release();
        }

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** incRef  increment reference counter of addin
    *   @param [in]  addIn  the addIn to increment reference
    *   @return      on success ito::retOk, ito::retError otherwise
    *
    *   The method increments the reference counter of the addin.
    */
    const ito::RetVal AddInManager::incRef(ito::AddInBase *addIn)
    {
        ito::AddInInterfaceBase *aib = addIn->getBasePlugin();
        aib->incRef(addIn);
        return ito::retOk;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** decRef  decrement reference counter of addin and close it if necessary
    *   @param [in]  addIn  the addIn to increment reference
    *   @return      on success ito::retOk, ito::retError otherwise
    *
    *   The method decrements the reference counter of the addin.
    */
    const ito::RetVal AddInManager::decRef(ito::AddInBase **addIn)
    {
        ito::AddInInterfaceBase *aib = (*addIn)->getBasePlugin();
        if (aib->getRef(*addIn) <= 0) //this instance holds the last reference of the plugin, therefore it is closed now
        {
            ito::RetVal retval(ito::retOk);
            retval += closeAddIn(*addIn);
            return retval;
        }
        else //at least one other instance is holding a reference of the plugin. just decrement the reference counter
        {
            aib->decRef(*addIn);
        }
        return ito::retOk;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** saveParamVals saves the plugins parameter values to the plugin parameter xml file
    *   @param [in] plugin  plugin for which the parameter should be saved
    *
    *   A xml file with the same name as the plugin library in the plugin directory is used to save the plugin parameters. The xml file
    *   is checked for the current plugin-file version and type when opened. The parameters are stored underneath the unique ID of
    *   the instance currently closed. This enables to have a several parameter sets for one plugin. Each parameter is stored with its
    *   name, type and value. The type may be either number or string.
    */
    const ito::RetVal AddInManager::saveParamVals(ito::AddInBase *plugin)
    {
        ito::RetVal ret = ito::retOk;
        QFile paramFile;
        QString pluginUniqueId = plugin->getIdentifier();
        if (pluginUniqueId == "")
        {
            pluginUniqueId = QString::number(plugin->getID());
        }

        // Generate the filename

        ito::AddInInterfaceBase *aib = plugin->getBasePlugin();
        QString fname = aib->getFilename();

        if ((ret = generateAutoSaveParamFile(fname, paramFile)) == ito::retError)
        {
            return ret;
        }

        // Get the paremterlist
        QMap<QString, ito::Param> *paramList;


        if ((ret = plugin->getParamList(&paramList)) == ito::retError)
        {
            return ret;
        }

        QMap<QString, ito::Param> paramListCpy(*paramList);

        // Write parameter list to file
        if ((ret = saveQLIST2XML(&paramListCpy, pluginUniqueId, paramFile)) == ito::retError)
        {
            return ret;
        }

        if (ret.containsWarning())
        {
            return ret;
        }
        else
        {
            return ito::retOk;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** loadParamVals loads the plugins parameter values from the plugin parameter xml file
    *   @param [in] plugin  plugin for which the parameter should be loaded
    *
    *   A xml file with the same name as the plugin library in the plugin directory is used to load the plugin parameters. The xml file
    *   is checked for the current plugin-file version and type when opened. The parameters are set using the invokeMethod function on the
    *   plugins' setParam method.
    */
    const ito::RetVal AddInManager::loadParamVals(ito::AddInBase *plugin)
    {
        ito::RetVal ret = ito::retOk;
        QFile paramFile;
        QString pluginUniqueId = plugin->getIdentifier();
        if (pluginUniqueId == "")
        {
            pluginUniqueId = QString::number(plugin->getID());
        }
        ito::Param param1;

        ito::AddInInterfaceBase *aib = plugin->getBasePlugin();
        QString fname = aib->getFilename();

        if ((ret = generateAutoSaveParamFile(fname, paramFile)) == ito::retError)
        {
            return ito::RetVal(ito::retWarning, 0, ret.errorMessage());
        }

        // Get the paremterlist
        QMap<QString, ito::Param> *paramListPlugin;
        QMap<QString, ito::Param> paramListXML;

        if ((ret = plugin->getParamList(&paramListPlugin)) == ito::retError)
        {
            return ito::RetVal(ito::retWarning, 0, ret.errorMessage());
        }

        QMap<QString, ito::Param> paramListPluginCpy(*paramListPlugin);

        // Write parameter list to file
        if ((ret = loadXML2QLIST(&paramListXML, pluginUniqueId, paramFile)) == ito::retError)
        {
            return ito::RetVal(ito::retWarning, 0, ret.errorMessage());
        }

        if ((ret = mergeQLists(&paramListPluginCpy, &paramListXML, true, true)) == ito::retError)
        {
            return ito::RetVal(ito::retWarning, 0, ret.errorMessage());
        }

        ItomSharedSemaphore *waitCond = NULL;
        foreach (param1, paramListPluginCpy)
        {
            if (!strlen(param1.getName()))
            {
                continue;
            }

            QSharedPointer<ito::ParamBase> qsParam(new ito::ParamBase(param1));

//            if (!param1.isNumeric() &&  (param1.getType() != (ito::ParamBase::String & ito::paramTypeMask)) && (param1.getType() != (ito::ParamBase::String & ito::paramTypeMask)))
//            {
//                ret += ito::RetVal(ito::retWarning, 0, "Paramtype not loadable yet");
//                continue;
//            }
            waitCond = new ItomSharedSemaphore();
            QMetaObject::invokeMethod(plugin, "setParam", Q_ARG(QSharedPointer<ito::ParamBase>, qsParam), Q_ARG(ItomSharedSemaphore*, waitCond));
            ret += waitCond->returnValue;
            waitCond->wait(AppManagement::timeouts.pluginGeneral);
            waitCond->deleteSemaphore();
            waitCond = NULL;
        }

        if (ret.containsError())
        {
            ret = ito::RetVal(ito::retWarning, 0, ret.errorMessage());
        }
        return ret;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    AddInManager::AddInManager(void) :
        m_algoInterfaceValidator(NULL)
    {
        ito::RetVal retValue;
        // this needs to be done in combination with Q_DECLARE_METATYPE to register a user data type
        qRegisterMetaType<char*>("char*");
        qRegisterMetaType<char**>("char**");
        qRegisterMetaType<const char*>("const char*");
        qRegisterMetaType<const char**>("const char**");
        // incomplete type, i.e. void* are illegal in Qt5!
        //qRegisterMetaType<const void *>("const void *");
        qRegisterMetaType<double>("double");
        qRegisterMetaType<double *>("double*");
        //qRegisterMetaType<const double>();
        qRegisterMetaType<const double *>("const double*");
        qRegisterMetaType<int *>("int*");
        qRegisterMetaType<const int *>("const int*");
        //qRegisterMetaType<int>();
        qRegisterMetaType<ItomSharedSemaphore*>("ItomSharedSemaphore*");
        qRegisterMetaType<ito::AddInInterfaceBase*>("ito::AddInInterfaceBase*");
        qRegisterMetaType<ito::AddInBase*>("ito::AddInBase*");
        qRegisterMetaType<ito::AddInBase*>("ito::AddInBase**");
        qRegisterMetaType<ito::AddInDataIO**>("ito::AddInDataIO**");
        qRegisterMetaType<ito::AddInActuator**>("ito::AddInActuator**");
        qRegisterMetaType<ito::AddInAlgo**>("ito::AddInAlgo**");
//        qRegisterMetaType<ito::ActuatorAxis*>("ito::ActuatorAxis**");
        qRegisterMetaType<ito::RetVal>("ito::RetVal");
        qRegisterMetaType<ito::RetVal*>("ito::RetVal*");
//        qRegisterMetaType<const void*>("const void*");
        qRegisterMetaType<QVector<ito::Param>*>("QVector<ito::Param>*");
        qRegisterMetaType<QVector<ito::ParamBase>*>("QVector<ito::ParamBase>*");
        qRegisterMetaType<QVector<int> >("QVector<int>");
        qRegisterMetaType<QVector<double> >("QVector<double>");
        // used in plotItemsChanged do not remove
        qRegisterMetaType<QVector<float> >("QVector<float>");

        qRegisterMetaType<QSharedPointer<double> >("QSharedPointer<double>");
        qRegisterMetaType<QSharedPointer<QVector<double> > >("QSharedPointer<QVector<double>>");
        qRegisterMetaType<QSharedPointer<int> >("QSharedPointer<int>");
        qRegisterMetaType<QSharedPointer<IntVector> >("QSharedPointer<IntVector>");
        qRegisterMetaType<QSharedPointer<char*> >("QSharedPointer<char>");
        qRegisterMetaType<QSharedPointer<QByteArray> >("QSharedPointer<QByteArray>");
        qRegisterMetaType<QSharedPointer<ito::Param> >("QSharedPointer<ito::Param>");
        qRegisterMetaType<QSharedPointer<ito::ParamBase> >("QSharedPointer<ito::ParamBase>");

        qRegisterMetaType<ito::DataObject>("ito::DataObject");
        qRegisterMetaType<QMap<QString, ito::Param> >("QMap<QString, ito::Param>");
        qRegisterMetaType<QMap<QString, ito::Param> >("QMap<QString, ito::ParamBase>");

#if ITOM_POINTCLOUDLIBRARY > 0    
        qRegisterMetaType<ito::PCLPointCloud >("ito::PCLPointCloud");
        qRegisterMetaType<ito::PCLPolygonMesh >("ito::PCLPolygonMesh");
        qRegisterMetaType<ito::PCLPoint >("ito::PCLPoint");
        qRegisterMetaType<QSharedPointer<ito::PCLPointCloud> >("QSharedPointer<ito::PCLPointCloud>");
        qRegisterMetaType<QSharedPointer<ito::PCLPolygonMesh> >("QSharedPointer<ito::PCLPolygonMesh>");
        qRegisterMetaType<QSharedPointer<ito::PCLPoint> >("QSharedPointer<ito::PCLPoint>");
#endif //#if ITOM_POINTCLOUDLIBRARY > 0

        m_deadPlugins.clear();

        connect(&m_deadPluginTimer, SIGNAL(timeout()), this, SLOT(closeDeadPlugins()));

        m_algoInterfaceValidator = new AlgoInterfaceValidator(retValue);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** destructor, closes all instances of plugins and plugins
    *
    *   Before the AddInManager itself is closed it closes all instances of plugins that are in the plugins' instance lists.
    *   Afterwards the AddInInterfaceBase for each plugin (i.e. the library) is closed an it is removed from the plugin list.
    *   This is done for dataIO, actuator and algo plugins.
    */
    AddInManager::~AddInManager(void)
    {
        ito::RetVal retval;
        AddInBase *addInInstance = NULL;
        QList<AddInBase*> addInInstancesCpy;
        AddInInterfaceBase *aib = NULL;

        //if there are still plugins in the "deadPlugin"-stack, try to kill them now
        closeDeadPlugins();
        m_deadPluginTimer.stop();

        foreach (QTranslator *Translator, m_Translator)
        {
            delete Translator;
        }
        m_Translator.clear();

        //we need to apply two steps in order to close all hardware-references
        //1. first -> close all opened instances (instances that keep reference to others must delete them after their deletion)
        //2. second -> delete all AddInInterfaceBase classes


        //step 1:
        foreach(QObject *obj, m_addInListDataIO)
        {
            aib = qobject_cast<ito::AddInInterfaceBase*>(obj);
            addInInstancesCpy = aib->getInstList(); //this copy is necessary in order to close every instance exactly one times (even if one instance is not deleted here but later, since another plugin still holds a reference to it)
            while (addInInstancesCpy.size() > 0)
            {
                addInInstance = (addInInstancesCpy[0]);
                if (addInInstance)
                {
                    retval += closeAddIn(addInInstance, NULL);
                }
                addInInstancesCpy.removeFirst();
            }
        }

        foreach(QObject *obj, m_addInListAct)
        {
            aib = qobject_cast<ito::AddInInterfaceBase*>(obj);
            addInInstancesCpy = aib->getInstList(); //this copy is necessary in order to close every instance exactly one times (even if one instance is not deleted here but later, since another plugin still holds a reference to it)
            while (addInInstancesCpy.size() > 0)
            {
                addInInstance = (addInInstancesCpy[0]);
                if (addInInstance)
                {
                    retval += closeAddIn(addInInstance, NULL);
                }
                addInInstancesCpy.removeFirst();
            }
        }

        //step 2:
        while (m_addInListDataIO.size() > 0)
        {
            QObject *qaib = m_addInListDataIO[0];
            AddInInterfaceBase *aib = (qobject_cast<ito::AddInInterfaceBase *>(qaib));
            m_addInListDataIO.removeFirst();
            QPluginLoader *loader = aib->getLoader();
            //loader->unload(); //under windows, unloading the plugin will sometimes not return. Therefore, no unload() here.
            DELETE_AND_SET_NULL(loader);
        }

        while (m_addInListAct.size() > 0)
        {
            QObject *qaib = m_addInListAct[0];
            AddInInterfaceBase *aib = (qobject_cast<ito::AddInInterfaceBase *>(qaib));
            m_addInListAct.removeFirst();
            QPluginLoader *loader = aib->getLoader();
            //loader->unload(); //under windows, unloading the plugin will sometimes not return. Therefore, no unload() here.
            DELETE_AND_SET_NULL(loader);
        }


        QHashIterator<void*, ito::FilterParams*> i(filterParamHash);
        while (i.hasNext()) 
        {
            i.next();
            delete i.value();
        }
        filterParamHash.clear();

        //remove all algorithms
        while (m_addInListAlgo.size() > 0)
        {
            QObject *qaib = m_addInListAlgo[0];
            AddInInterfaceBase *aib = (qobject_cast<ito::AddInInterfaceBase *>(qaib));
            while (aib->getInstList().size() > 0)
            {
                AddInAlgo *ail = reinterpret_cast<AddInAlgo *>(aib->getInstList()[0]);
                if (ail)
                {
                    QHash<QString, ito::AddInAlgo::FilterDef *> funcList;
                    ail->getFilterList(funcList);
                    QList<QString> keyList = funcList.keys();
                    for (int n = 0; n < keyList.size(); n++)
                    {
                        if (m_filterList.contains(keyList[n]))
                        {
                            m_filterList.remove(keyList[n]);
                        }
                    }

                    aib->closeInst(reinterpret_cast<ito::AddInBase **>(&ail));
                }
            }
            m_addInListAlgo.removeFirst();
            QPluginLoader *loader = aib->getLoader();
            //loader->unload(); //under windows, unloading the plugin will sometimes not return. Therefore, no unload() here.
            DELETE_AND_SET_NULL(loader);
        }

        DELETE_AND_SET_NULL(m_algoInterfaceValidator);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param name 
    *   @return ito::RetVal    
    */
    const ito::RetVal AddInManager::reloadAddIn(const QString &name)
    {
        ito::AddInInterfaceBase *aib = NULL;
        int pluginNum =  getPluginNum(name, aib);
        QString filename = aib->getFilename();
        QList<ito::AddInBase *> instList;
        ito::AddInAlgo *algo = NULL;

        if ((((aib->getType() == ito::typeDataIO) || (aib->getType() == ito::typeActuator)) && (aib->getInstCount() != 0))
            || ((aib->getType() == ito::typeAlgo) && (aib->getInstCount() != 1)))
        {
            return ito::RetVal(ito::retError, 0, "Reference counter not zero. Only unused plugins can be reloaded.");
        }
        
        switch (aib->getType())
        {
            case ito::typeActuator:
                m_addInListAct.removeAt(pluginNum);
            break;

            case ito::typeDataIO:
                m_addInListDataIO.removeAt(pluginNum);
            break;

            case ito::typeAlgo:
                instList = aib->getInstList();
                for (int n = 0; n < instList.length(); n++)
                {
                    algo = (ito::AddInAlgo*)instList.at(n);
                    closeAddIn((ito::AddInBase*)algo);
                }
                m_addInListAlgo.removeAt(pluginNum);
            break;
        } 

        delete aib;
        loadAddIn(filename);

        return ito::retOk;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** showConfigDialog    show the plugin's configuration dialog
    *   @param [in] addin   addin from which the dialog should be called
    *
    *   This method opens the configuration dialog of a plugin. The dialog can be opened using a right click on an instance of the plugin
    *   in the addInModel list or using showConfiguration command in python. An implementation of a configuration dialog is not mandatory, 
    *   so in case there is no dialog implemented nothing happens.
    */
    ito::RetVal AddInManager::showConfigDialog(ito::AddInBase *addin, ItomSharedSemaphore *waitCond /*= NULL*/)
    {
        ItomSharedSemaphoreLocker locker(waitCond);
        ito::RetVal retval;

        if (addin && addin->hasConfDialog())
        {
            retval += addin->showConfDialog();
        }
        else
        {
            retval += ito::RetVal(ito::retWarning, 0, tr("no configuration dialog available").toLatin1().data());
        }

        if (waitCond)
        {
            waitCond->returnValue = retval;
            waitCond->release();
        }

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** showDockWidget              show or hide the plugin's widget
    *   @param [in] addin           addin from which the dialog should be called
    *   @param [in] visible         1=show, 0=hide, -1=toggle
    *   @param [in, out] waitCond   wait condition for calls from other threads. See also \ref ItomSharedSemaphore
    *
    *   This method opens or closes the wodget of a plugin. The widget can be opened or closed using a right click on an instance of the
    *   plugin in the addInModel list or using showToolbox or hideToolbox command in python. An implementation of a configuration dialog
    *   is not mandatory, so in case there is no dialog implemented nothing happens.
    */
    ito::RetVal AddInManager::showDockWidget(ito::AddInBase *addin, int visible, ItomSharedSemaphore *waitCond /*= NULL*/)
    {
        ItomSharedSemaphoreLocker locker(waitCond);
        ito::RetVal retval;

        if (addin)
        {
            QDockWidget *dw = addin->getDockWidget();
            if (dw)
            {
                QAction *toggleAction = dw->toggleViewAction();
                if (visible == 0) //hide
                {
                    if (toggleAction->isChecked()) //dock widget is currently visible -> hide it now
                    {
                        dw->toggleViewAction()->trigger();
                    }
                }
                else if (visible == 1) //show
                {
                    if (toggleAction->isChecked() == false) //dock widget is currently hidden -> show it now
                    {
                        dw->toggleViewAction()->trigger();
                    }
                }
                else //toggle
                {
                    dw->toggleViewAction()->trigger();
                }
            }
            else
            {
                retval += ito::RetVal(ito::retError, 0, QObject::tr("no toolbox available").toLatin1().data());
            }
        }
        else
        {
            retval += ito::RetVal(ito::retError, 0, QObject::tr("plugin not available").toLatin1().data());
        }

        if (waitCond)
        {
            waitCond->returnValue = retval;
            waitCond->release();
        }

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param algoWidgetName algoPluginName
    *   @param algoPluginName
    *   @return ito::AddInAlgo::AlgoWidgetDef    
    */
    const ito::AddInAlgo::AlgoWidgetDef * AddInManager::getAlgoWidgetDef(QString algoWidgetName, QString algoPluginName)
    {
        //at the moment algoPluginName do not really influence the search, but maybe it might become necessary to also search for plugin-widgets by "pluginName.widgetName"

        const QHash<QString, ito::AddInAlgo::AlgoWidgetDef *> *list = getAlgoWidgetList();
        QHash<QString, ito::AddInAlgo::AlgoWidgetDef *>::ConstIterator iter = list->find(algoWidgetName);
        if (iter == list->end() || iter.value() == NULL)
        {
            return NULL;
        }
        else
        {
            ito::AddInInterfaceBase *aib = iter.value()->m_pBasePlugin;
            if (aib && (aib->objectName() == algoPluginName  || algoPluginName.isEmpty() || algoPluginName == ""))
            {
                return iter.value();
            }
            else if (aib == NULL && (algoPluginName.isEmpty() || algoPluginName == ""))
            {
                return iter.value();
            }
        }

        return NULL;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param path filterParam
    *   @return ito::FilterParams    
    */
    const ito::FilterParams* AddInManager::getHashedFilterParams(ito::AddInAlgo::t_filterParam filterParam) const
    {
        QHash<void*,ito::FilterParams*>::ConstIterator it = AddInManager::filterParamHash.constFind((void*)filterParam);
        if (it != AddInManager::filterParamHash.constEnd())
        {
            return *it;
        }
        return NULL;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @return RetVal    
    */
    RetVal AddInManager::closeDeadPlugins()
    {
        RetVal retval(retOk);
        QList< QPointer< ito::AddInBase > >::iterator it;
        it = m_deadPlugins.begin();
        ito::AddInBase *aib = NULL;

        while (it != m_deadPlugins.end())
        {
            aib = it->data();
            if (it->isNull()) //weak pointer does not live any more
            {
                it = m_deadPlugins.erase(it);
            }
            else if (aib->isInitialized()) //plugin finished init-method (late, but finally it did finish ;)), we can kill it now
            {
                retval += closeAddIn(aib, NULL);
                it = m_deadPlugins.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (m_deadPlugins.count() == 0)
        {
            m_deadPluginTimer.stop();
        }
        else if (m_deadPluginTimer.interval() < 30000)
        {
            //the interval is incremented by 2000ms after each run until a max-value of 30secs. 
            //This gives the chance to delete newly added dead plugins fast and decrease the 
            //intents the longer it did not work.
            m_deadPluginTimer.setInterval(m_deadPluginTimer.interval() + 2000);
        }

        return retval;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param addIn 
    *   @return RetVal    
    */
    RetVal AddInManager::registerPluginAsDeadPlugin(ito::AddInBase *addIn)
    {
        QPointer<ito::AddInBase> ptr(addIn);
        m_deadPlugins.push_back(ptr);

        if (m_deadPluginTimer.isActive() == false)
        {
            m_deadPluginTimer.start(2000); //the interval is incremented by 2000ms after each run until a max-value of 30secs. This gives the chance to delete newly added dead plugins fast and decrease the intents the longer it did not work.
        }
        return ito::retOk;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param plugin 
    *   @return bool    
    */
    bool AddInManager::isPluginInstanceDead(const ito::AddInBase *plugin) const
    {
        foreach (const QPointer<ito::AddInBase> ptr, m_deadPlugins)
        {
            if (!ptr.isNull() && ptr.data() == plugin)
            {
                return true;
            }
        }
        return false;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param iface 
    *   @param tag 
    *   @return QList<ito::AddInAlgo::FilterDef *>    
    */
    const QList<ito::AddInAlgo::FilterDef *> AddInManager::getFilterByInterface(ito::AddInAlgo::tAlgoInterface iface, const QString tag) const
    {
        if (tag.isNull())
        {
            QList<ito::AddInAlgo::FilterDef *> res;
            QHash<QString, ito::AddInAlgo::FilterDef *>::const_iterator it = m_filterList.constBegin();
            while(it != m_filterList.constEnd())
            {
                if (it.value()->m_interface == iface) res.append(*it);
                ++it;
            }
            return res;
        }
        else
        {
            QString key = QString::number(iface) + "_" + tag;
            return m_filterListInterfaceTag.values(key);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param cat 
    *   @return QList<ito::AddInAlgo::FilterDef *>   
    */
    const QList<ito::AddInAlgo::FilterDef *> AddInManager::getFiltersByCategory(ito::AddInAlgo::tAlgoCategory cat) const
    {
        QList<ito::AddInAlgo::FilterDef *> res;
        QHash<QString, ito::AddInAlgo::FilterDef *>::const_iterator it = m_filterList.constBegin();
        while(it != m_filterList.constEnd())
        {
            if (it.value()->m_category == cat) res.append(*it);
            ++it;
        }
        return res;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    /** 
    *   @param iface 
    *   @param cat 
    *   @param tag 
    *   @return QList<ito::AddInAlgo::FilterDef    
    */
    const QList<ito::AddInAlgo::FilterDef *> AddInManager::getFilterByInterfaceAndCategory(ito::AddInAlgo::tAlgoInterface iface, ito::AddInAlgo::tAlgoCategory cat, const QString tag) const
    {
        QList<ito::AddInAlgo::FilterDef *> res = getFilterByInterface(iface,tag);
        QList<ito::AddInAlgo::FilterDef *> res2;
        for(int i=0; i<res.size(); i++)
        {
            if (res[i]->m_category == cat) res2.append(res[i]);
        }
        return res2;
    }


    //----------------------------------------------------------------------------------------------------------------------------------
} // namespace ito
