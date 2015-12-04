/* ********************************************************************
    itom software
    URL: http://www.uni-stuttgart.de/ito
    Copyright (C) 2016, Institut fuer Technische Optik (ITO),
    Universitaet Stuttgart, Germany

    This file is part of itom and its software development toolkit (SDK).

    itom is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public Licence as published by
    the Free Software Foundation; either version 2 of the Licence, or (at
    your option) any later version.
   
    In addition, as a special exception, the Institut fuer Technische
    Optik (ITO) gives you certain additional rights.
    These rights are described in the ITO LGPL Exception version 1.0,
    which can be found in the file LGPL_EXCEPTION.txt in this package.

    itom is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
    General Public Licence for more details.

    You should have received a copy of the GNU Library General Public License
    along with itom. If not, see <http://www.gnu.org/licenses/>.
*********************************************************************** */

#ifndef ABSTRACTFIGURE_H
#define ABSTRACTFIGURE_H

#include "../common/commonGlobal.h"
#include "AbstractNode.h"

#include "../common/apiFunctionsGraphInc.h"
#include "../common/apiFunctionsInc.h"

#include <qmainwindow.h>
#include <qlabel.h>
#include <qtoolbar.h>
#include <qevent.h>
#include <qdockwidget.h>

class QPropertyEditorWidget; //forward declaration

#if !defined(Q_MOC_RUN) || defined(ITOMCOMMONQT_MOC) //only moc this file in itomCommonQtLib but not in other libraries or executables linking against this itomCommonQtLib

//place this macro in the header file of the designer plugin widget class right before the first section (e.g. public:)
#define DESIGNER_PLUGIN_ITOM_API \
            protected: \
                void importItomApi(void** apiPtr) \
                {ito::ITOM_API_FUNCS = apiPtr;} \
                void importItomApiGraph(void** apiPtr) \
                { ito::ITOM_API_FUNCS_GRAPH = apiPtr;} \
            public: \
                //.

namespace ito {

class AbstractFigure; //forward declaration
class AbstractFigurePrivate; //forward declaration

class ITOMCOMMONQT_EXPORT AbstractFigure : public QMainWindow, public AbstractNode
{
    Q_OBJECT
    Q_ENUMS(WindowMode)
    Q_ENUMS(UnitLabelStyle)

    Q_PROPERTY(bool toolbarVisible READ getToolbarVisible WRITE setToolbarVisible DESIGNABLE true USER true)
    Q_PROPERTY(bool contextMenuEnabled READ getContextMenuEnabled WRITE setContextMenuEnabled DESIGNABLE true)

    Q_CLASSINFO("prop://toolbarVisible", "Toggles the visibility of the toolbar of the plot.")
    Q_CLASSINFO("prop://contextMenuEnabled", "Defines whether the context menu of the plot should be enabled or not.")
    Q_CLASSINFO("slot://refreshPlot", "Triggers an update of the current plot window.")

    public:
        enum WindowMode 
        { 
            ModeInItomFigure, 
            ModeStandaloneInUi, 
            ModeStandaloneWindow 
        };
        
        enum CompilerFeatures 
        { 
            tOpenCV        = 0x01,
            tPointCloudLib = 0x02
        };

        enum tChildPlotStates
        {
            tNoChildPlot            = 0x00,
            tExternChild            = 0x01,
            tOwnChild               = 0x02,
            tUninitilizedExtern     = 0x10,
            tVisibleOnInit          = 0x20
        };

        enum UnitLabelStyle 
        { 
            UnitLabelSlash,              // x-axis / m  -> corresponds to DIN461
            UnitLabelKeywordIn,          // x-axis in m -> corresponds to DIN461
            UnitLabelSquareBrackets      // x-axis [m]  -> does not correspond to DIN461
        };

        int getCompilerFeatures(void) const 
        {
            int retval = tOpenCV;
            #if defined USEPCL || ITOM_POINTCLOUDLIBRARY
            retval |= tPointCloudLib;
            #endif
            return retval;
        }

        struct ToolBarItem {
            ToolBarItem() : toolbar(NULL), visible(1), section(0), key("") {}
            QToolBar *toolbar;
            bool visible;
            int section;
            Qt::ToolBarArea area;
            QString key;
        };

        struct ToolboxItem {
            ToolboxItem() : toolbox(NULL), key("") {}
            QDockWidget *toolbox;
            Qt::DockWidgetArea area;
            QString key;
        };

        AbstractFigure(const QString &itomSettingsFile, WindowMode windowMode = ModeStandaloneInUi, QWidget *parent = 0);
        virtual ~AbstractFigure();

        virtual bool event(QEvent *e);
        void setApiFunctionGraphBasePtr(void **apiFunctionGraphBasePtr);
        void setApiFunctionBasePtr(void **apiFunctionBasePtr);
        void ** getApiFunctionGraphBasePtr(void) { return m_apiFunctionsGraphBasePtr; }
        void ** getApiFunctionBasePtr(void) { return m_apiFunctionsBasePtr; }

        virtual RetVal addChannel(AbstractNode *child, ito::Param* parentParam, ito::Param* childParam, Channel::ChanDirection direction, bool deleteOnParentDisconnect, bool deleteOnChildDisconnect);
        virtual RetVal addChannel(Channel *newChannel);
        virtual RetVal removeChannelFromList(unsigned int uniqueID);
        virtual RetVal removeChannel(Channel *delChannel);

        virtual RetVal update(void) = 0; /*!> Calls apply () and updates all children*/

        //properties
        virtual void setToolbarVisible(bool visible);
        virtual bool getToolbarVisible() const;
        virtual void setContextMenuEnabled(bool show) = 0; 
        virtual bool getContextMenuEnabled() const = 0;

        QDockWidget *getPropertyDockWidget() const;
        QList<QMenu*> getMenus() const;
        QList<AbstractFigure::ToolBarItem> getToolbars() const;
        QList<AbstractFigure::ToolboxItem> getToolboxes() const; //the first toolbox is always the property dock widget

    protected:

        virtual RetVal init() { return retOk; } //this method is called from after construction and after that the api pointers have been transmitted

        virtual void importItomApi(void** apiPtr) = 0;      /*!< function to provide access to the itom API functions. this methods are implemented in the plugin itsself. Therefore put the macro DESIGNER_PLUGIN_ITOM_API before the public section in the designer plugin class. */
        virtual void importItomApiGraph(void** apiPtr) = 0; /*!< function to provide access to the itom API functions. this methods are implemented in the plugin itsself. Therefore put the macro DESIGNER_PLUGIN_ITOM_API before the public section in the designer plugin class. */

        void addToolBar(QToolBar *toolbar, const QString &key, Qt::ToolBarArea area = Qt::TopToolBarArea, int section = 1); /*!< Register any toolbar of the plot widget using this method. */
        void addToolBarBreak(const QString &key, Qt::ToolBarArea area = Qt::TopToolBarArea); /*!< Add a toolbar break, hence a new line for the following toolbars to the indicated area. */

        void addToolbox(QDockWidget *toolbox, const QString &key, Qt::DockWidgetArea area = Qt::RightDockWidgetArea); /*!< Every plot widget is automatically equipped with a property toolbox. If you want to add further toolboxes (dock widgets), register and append them using this method. */

        void showToolBar(const QString &key); /*!< show a toolbar with given key. This toolbar must first be registered using addToolBar. */
        void hideToolBar(const QString &key); /*!< hide a toolbar with given key. This toolbar must first be registered using addToolBar. */

        void addMenu(QMenu *menu); /*!< append a menu to the figure. AbstractFigure then takes care about the menu. Only use this method to add menus since the menu bar of figures is differently handled depending on the window mode of the figure. */

        void updatePropertyDock(); /*!< call this method if any property of the figure changed such that the property toolbox is synchronized and updated. */
        void setPropertyObservedObject(QObject* obj); /*!< registeres obj for a property observation of the property toolbox. All readable properties are then listed in the property toolbox. */

        RetVal initialize();

        WindowMode m_windowMode;
        QString m_itomSettingsFile;
        QWidget *m_mainParent; //the parent of this figure is only set to m_mainParent, if the stay-on-top behaviour is set to the right value

        void **m_apiFunctionsGraphBasePtr;
        void **m_apiFunctionsBasePtr;

        ito::uint8 m_lineCutType;
        ito::uint8 m_zSliceType;
        ito::uint8 m_zoomCutType;

    private:
        AbstractFigurePrivate *d;

    private slots:
        inline void mnuShowToolbar(bool /*checked*/) { setToolbarVisible(true); } /*!< shows all registered toolbars*/
        void mnuShowProperties(bool checked); /*!< set the visibility of the property toolbox */

    public slots:

        int getPlotID() 
        { 
            if(!ito::ITOM_API_FUNCS_GRAPH) return 0;
            ito::uint32 thisID = 0;
            ito::RetVal retval = apiGetFigureIDbyHandle(this, thisID);

            if(retval.containsError())
            {
                return 0;
            }
            return thisID; 
        }

        void refreshPlot() { update(); }
};

} // namespace ito

#endif //#if !defined(Q_MOC_RUN) || defined(ITOMCOMMONQT_MOC)

#endif // ABSTRACTFIGURE_H
