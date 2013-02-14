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

#include "designerWidgetOrganizer.h"

#include "../global.h"
#include "../AppManagement.h"
#include "common/apiFunctionsGraphInc.h"
#include "plot/AbstractFigure.h"
#include "plot/AbstractDObjFigure.h"
#include "plot/abstractItomDesignerPlugin.h"

#include <QtUiTools/quiloader.h>
#include <qpluginloader.h>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qlibrary.h>
#include <qsettings.h>

/*!
    \class DesignerWidgetOrganizer
    \brief 
*/


namespace ito
{

//----------------------------------------------------------------------------------------------------------------------------------
//! constructor
/*!

*/
DesignerWidgetOrganizer::DesignerWidgetOrganizer()
{
    //create figure categories (for property dialog...)
   ito::PlotDataFormats allFormats = ~(ito::PlotDataFormats(0)); //(~ito::Format_Gray8); // | ito::Format_Gray8; //(ito::PlotDataFormats(0));

    m_figureCategories["DObjLiveLine"] = FigureCategory("Data Object, Line Plot, Live", ito::DataObjLine, allFormats, ito::Live | ito::PlotLine, 0, "itom1DQwtFigure");
    m_figureCategories["DObjLiveImage"] = FigureCategory("Data Object, 2D Image Plot, Live", ito::DataObjPlane, allFormats, ito::Live | ito::PlotImage, 0, "itom2DGVFigure");
    m_figureCategories["DObjStaticLine"] = FigureCategory("Data Object, Line Plot, Static", ito::DataObjLine, allFormats, ito::Static | ito::PlotLine, 0, "itom1DQwtFigure");
    m_figureCategories["DObjStaticImage"] = FigureCategory("Data Object, 2D Image Plot, Static", ito::DataObjPlane | ito::DataObjPlaneStack, allFormats, ito::Static | ito::PlotImage, 0, "itom2DQwtFigure");
    m_figureCategories["DObjStaticGeneralPlot"] = FigureCategory("Data Object, Any Planar Plot, Static", ito::DataObjLine | ito::DataObjPlane | ito::DataObjPlaneStack, allFormats, ito::Static, ito::Plot3D | ito::PlotISO, "itom2DQwtFigure");

    scanDesignerPlugins();
}

//----------------------------------------------------------------------------------------------------------------------------------
//! destructor
/*!

*/
DesignerWidgetOrganizer::~DesignerWidgetOrganizer()
{
    foreach(ito::FigurePlugin p, m_figurePlugins)
    {
        DELETE_AND_SET_NULL(p.factory);
    }
    m_figurePlugins.clear();
}



//------------------------------------------------------------------------------------------------------------------
RetVal DesignerWidgetOrganizer::scanDesignerPlugins()
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cd("designer");
    QStringList candidates = dir.entryList(QDir::Files);
    QString absolutePluginPath;
    FigurePlugin infoStruct;
    PluginLoadStatus status;
    QString message;
    QPluginLoader *loader = NULL;

    foreach(const QString &plugin, candidates)
    {
        absolutePluginPath = QDir::cleanPath(dir.absoluteFilePath(plugin));
        status.filename = absolutePluginPath;
        status.messages.clear();
        if (QLibrary::isLibrary(absolutePluginPath))
        {
            loader = new QPluginLoader(absolutePluginPath);
            
            if (loader->load())
            {
                QDesignerCustomWidgetInterface *iface = NULL;
                // try with a normal plugin, we do not support collections
                if ((iface = qobject_cast<QDesignerCustomWidgetInterface *>(loader->instance())) && 
                    ((loader->instance())->inherits("ito::AbstractItomDesignerPlugin")))
                {
                    ito::AbstractItomDesignerPlugin *absIDP = (ito::AbstractItomDesignerPlugin *)loader->instance();
                    infoStruct.filename = absolutePluginPath;
                    infoStruct.classname = iface->name();
                    infoStruct.plotDataFormats = absIDP->getPlotDataFormats();
                    infoStruct.plotDataTypes = absIDP->getPlotDataTypes();
                    infoStruct.plotFeatures = absIDP->getPlotFeatures();
                    infoStruct.icon = iface->icon();
                    infoStruct.factory = loader; //now, loader is organized by m_figurePlugins-list
                    m_figurePlugins.append(infoStruct);

                    absIDP->setItomSettingsFile(AppManagement::getSettingsFile());

                    message = tr("DesignerWidget '%1' successfully loaded").arg(iface->name());
                    status.messages.append(QPair<ito::tRetValue, QString>(ito::retOk, message));
                }
                else
                {
                    loader->unload();
                    message = tr("Plugin in file '%1' is no Qt DesignerWidget inherited from AbstractItomDesignerPlugin").arg(status.filename);
                    status.messages.append( QPair<ito::tRetValue, QString>(ito::retError, message));
                    DELETE_AND_SET_NULL(loader);
                }
                
            }
            else
            {
                loader->unload();
                message = tr("DesignerWidget in file '%1' could not be loaded. Error message: %2").arg(status.filename).arg(loader->errorString());
                status.messages.append( QPair<ito::tRetValue, QString>( ito::retError, message) );
                DELETE_AND_SET_NULL(loader);
            }

            m_pluginLoadStatus.append(status);
        }
    }

    return ito::retOk;
}


bool DesignerWidgetOrganizer::figureClassExists( const QString &className )
{
    foreach(const FigurePlugin &plugin, m_figurePlugins)
    {
        if(plugin.classname == className)
        {
            return true;
        }
    }
    return false;
}


ito::RetVal DesignerWidgetOrganizer::figureClassMinimumRequirementCheck( const QString &className, int plotDataTypesMask, int plotDataFormatsMask, int plotFeaturesMask, bool *ok )
{
    ito::RetVal retVal;
    bool success = false;
    foreach(const FigurePlugin &plugin, m_figurePlugins)
    {
        if(className == plugin.classname)
        {
            if( (plugin.plotDataTypes & plotDataTypesMask) != plotDataTypesMask )
            {
                retVal += ito::RetVal::format(ito::retError, 0, tr("Figure '%s' does not correspond to the minimum requirements").toAscii().data(), className.toAscii().data());
                break;
            }
            if( (plugin.plotDataFormats & plotDataFormatsMask) != plotDataFormatsMask )
            {
                retVal += ito::RetVal::format(ito::retError, 0, tr("Figure '%s' does not correspond to the minimum requirements").toAscii().data(), className.toAscii().data());
                break;
            }
            if( (plugin.plotFeatures & plotFeaturesMask) != plotFeaturesMask )
            {
                retVal += ito::RetVal::format(ito::retError, 0, tr("Figure '%s' does not correspond to the minimum requirements").toAscii().data(), className.toAscii().data());
                break;
            }
            success = true;
        }
    }

    if(retVal == ito::retOk && success == false)
    {
        retVal += ito::RetVal::format(ito::retError, 0, tr("Figure '%s' not found").toAscii().data(), className.toAscii().data());
    }
    
    if(ok) *ok = success;
    return retVal;
}


QList<FigurePlugin> DesignerWidgetOrganizer::getPossibleFigureClasses( int plotDataTypesMask, int plotDataFormatsMask, int plotFeaturesMask )
{
    QList<FigurePlugin> figurePlugins;

    foreach(const FigurePlugin &plugin, m_figurePlugins)
    {
        if( (plugin.plotDataTypes & plotDataTypesMask) == plotDataTypesMask &&
            (plugin.plotDataFormats & plotDataFormatsMask) == plotDataFormatsMask &&
            (plugin.plotFeatures & plotFeaturesMask) == plotFeaturesMask )
        {
            figurePlugins.append( plugin );
        }
    }
    return figurePlugins;
}

QList<FigurePlugin> DesignerWidgetOrganizer::getPossibleFigureClasses( const FigureCategory &figureCat )
{
    QList<FigurePlugin> figurePlugins;

        QString         m_description;
    PlotDataTypes   m_allowedPlotDataTypes;
    PlotDataFormats m_allowedPlotDataFormats;
    PlotFeatures    m_requiredPlotFeatures;
    PlotFeatures    m_excludedPlotFeatures;
    QString         m_defaultClassName;

    foreach(const FigurePlugin &plugin, m_figurePlugins)
    {
        if( (plugin.plotDataTypes & figureCat.m_allowedPlotDataTypes) &&
            (plugin.plotDataFormats & figureCat.m_allowedPlotDataFormats) &&
            (plugin.plotFeatures & figureCat.m_excludedPlotFeatures) == 0 &&
            ((plugin.plotFeatures & figureCat.m_requiredPlotFeatures) == figureCat.m_requiredPlotFeatures) )
        {
            figurePlugins.append(plugin);
        }
    }
    return figurePlugins;
}

QString DesignerWidgetOrganizer::getFigureClass( const QString &figureCategory, const QString &defaultClassName, ito::RetVal &retVal )
{
    if(!m_figureCategories.contains(figureCategory))
    {
        retVal += ito::RetVal::format(ito::retError, 0, tr("The figure category '%s' is unknown").toAscii().data(), figureCategory.data());
        return "";
    }

    FigureCategory figureCat = m_figureCategories[figureCategory];    
    QList<FigurePlugin> figurePlugins;

    foreach(const FigurePlugin &plugin, m_figurePlugins)
    {
        if( (plugin.plotDataTypes & figureCat.m_allowedPlotDataTypes) &&
            (plugin.plotDataFormats & figureCat.m_allowedPlotDataFormats) &&
            (plugin.plotFeatures & figureCat.m_excludedPlotFeatures) == 0 &&
            ((plugin.plotFeatures & figureCat.m_requiredPlotFeatures) == figureCat.m_requiredPlotFeatures) )
        {
            figurePlugins.append(plugin);
        }
    }

    if(defaultClassName != "")
    {
        foreach(const FigurePlugin &plugin, figurePlugins)
        {
            if(plugin.classname == defaultClassName)
            {
                return defaultClassName; //the given class name fits to the figureCategory and exists
            }
        }
    }

    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("DesignerPlotWidgets");
    QString settingsClassName = settings.value(figureCategory, figureCat.m_defaultClassName ).toString();
    settings.endGroup();

    foreach(const FigurePlugin &plugin, figurePlugins)
    {
        if(plugin.classname == settingsClassName)
        {
            return settingsClassName; //the given class name fits to the figureCategory and exists
        }
    }

    if(figurePlugins.count() > 0)
    {
        return figurePlugins[0].classname;
    }

    retVal += ito::RetVal(ito::retError, 0, tr("no plot figure plugin could be found that fits to the given category.").toAscii().data());
    return "";

}

RetVal DesignerWidgetOrganizer::setFigureDefaultClass( const QString &figureCategory, const QString &defaultClassName)
{
    if(!m_figureCategories.contains(figureCategory))
    {
        return ito::RetVal::format(ito::retError, 0, tr("The figure category '%s' is unknown").toAscii().data(), figureCategory.data());
    }


    QSettings settings(AppManagement::getSettingsFile(), QSettings::IniFormat);
    settings.beginGroup("DesignerPlotWidgets");
    settings.setValue(figureCategory, defaultClassName);
    settings.endGroup();
    return retOk;
}


//----------------------------------------------------------------------------------------------------------------------------------

} //end namespace ito
