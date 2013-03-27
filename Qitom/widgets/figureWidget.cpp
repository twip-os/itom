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

#include "../global.h"

#include "figureWidget.h"
#include "../AppManagement.h"
#include "../organizer/designerWidgetOrganizer.h"
#include "../organizer/uiOrganizer.h"
#include "plot/AbstractDObjFigure.h"

#include <qlayoutitem.h>


namespace ito {

//----------------------------------------------------------------------------------------------------------------------------------

FigureWidget::FigureWidget(const QString &title, bool docked, bool isDockAvailable, int rows, int cols, QWidget *parent, Qt::WindowFlags /*flags*/)
    : AbstractDockWidget(docked, isDockAvailable, floatingWindow, movingEnabled, title, parent),
    m_pGrid(NULL),
    m_pCenterWidget(NULL),
    m_menuWindow(NULL),
    m_menuSubplot(NULL),
    m_pSubplotActions(NULL),
    m_rows(rows),
    m_cols(cols)
{

    AbstractDockWidget::init();

    QWidget *temp;

    m_pCenterWidget = new QWidget(this);
    m_pGrid = new QGridLayout(m_pCenterWidget);
    m_pGrid->setSpacing(0);
    m_pGrid->setContentsMargins(0,0,0,0);
    m_pCenterWidget->setLayout(m_pGrid);

    for(int r = 0 ; r < rows ; r++)
    {
        for (int c = 0 ; c < cols ; c++)
        {
            temp = new QWidget(m_pCenterWidget);
            temp->setObjectName( QString("emptyWidget%1").arg(m_cols * r + c) );
            m_pGrid->addWidget(new QWidget(m_pCenterWidget), r, c);
        }
    }

    changeCurrentSubplot(0);

    resizeDockWidget(700,400);

    setContentWidget(m_pCenterWidget);
    m_pCenterWidget->setContentsMargins(0,0,0,0);
    //m_pCenterWidget->setStyleSheet( "background-color:#ffccee" );

    setFocusPolicy(Qt::StrongFocus);
//    setAcceptDrops(true);
    
}

//----------------------------------------------------------------------------------------------------------------------------------
//! destructor
/*!
    cancels connections and closes every tab.
*/
FigureWidget::~FigureWidget()
{


}

//----------------------------------------------------------------------------------------------------------------------------------------
void FigureWidget::closeEvent(QCloseEvent *event) // { event->accept(); };
{
    //usually any figure-instances (in python) keep references (QSharedPointer<unsigned int>) to this figure. If the last python-instance is closed,
    //the deleter-method of their guarded figure handle deletes this figure, if this figure does not keep its own reference. This is only the case,
    //if a figure is directly created by the plot of liveImage command from the module 'itom'. Then no corresponding figure-instance is created and
    //the figure is only closed if the user closes it or the static close-method of class 'figure'.
    m_guardedFigHandle.clear();

    event->accept();
}

//----------------------------------------------------------------------------------------------------------------------------------------
void FigureWidget::createActions()
{
    QAction *temp = NULL;
    if(m_rows > 1 || m_cols > 1)
    {
        m_pSubplotActions = new QActionGroup(this);
        m_pSubplotActions->setExclusive(true);
        connect(m_pSubplotActions, SIGNAL(triggered(QAction*)), this, SLOT(mnu_subplotActionsTriggered(QAction*)));

        for(int r = 0 ; r < m_rows ; r++)
        {
            for(int c = 0 ; c < m_cols ; c++)
            {
                temp = new QAction(tr("subplot %1 (empty)").arg( c + r * m_cols ),this);
                temp->setData( c + r * m_cols );
                temp->setCheckable(true);
                m_pSubplotActions->addAction(temp);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------
void FigureWidget::createMenus()
{
    if(m_pSubplotActions)
    {
        m_menuSubplot = new QMenu(tr("&Subplots"), this);
        m_menuSubplot->addActions(m_pSubplotActions->actions());
        getMenuBar()->addMenu(m_menuSubplot);
    }


	//create main menus
    m_menuWindow = new QMenu(tr("&Windows"), this);
    if (m_actStayOnTop)
    {
        m_menuWindow->addAction(m_actStayOnTop);
    }
    if (m_actStayOnTopOfApp)
    {
        m_menuWindow->addAction(m_actStayOnTopOfApp);
    }
	
    getMenuBar()->addMenu(m_menuWindow);
}

//----------------------------------------------------------------------------------------------------------------------------------------
void FigureWidget::createToolBars()
{
}

//----------------------------------------------------------------------------------------------------------------------------------------
void FigureWidget::createStatusBar()
{
}

//----------------------------------------------------------------------------------------------------------------------------------------
void FigureWidget::updateActions()
{
}

//----------------------------------------------------------------------------------------------------------------------------------------
RetVal FigureWidget::plot(QSharedPointer<ito::DataObject> dataObj, int areaRow, int areaCol, const QString &className)
{
    DesignerWidgetOrganizer *dwo = qobject_cast<DesignerWidgetOrganizer*>(AppManagement::getDesignerWidgetOrganizer());
    UiOrganizer *uiOrg = qobject_cast<UiOrganizer*>(AppManagement::getUiOrganizer());
    RetVal retval;
    QString plotClassName;
    bool exists = false;

    if(areaRow < 0 || areaRow >= m_rows)
    {
        retval += ito::RetVal::format(ito::retError,0,"areaRow out of range [0,%i]", m_rows-1);
    }

    if(areaCol < 0 || areaCol >= m_cols)
    {
        retval += ito::RetVal::format(ito::retError,0,"arealCol out of range [0,%i]", m_cols-1);
    }

    if(!retval.containsError())
    {

        if(dwo && uiOrg)
        {
            int dims = dataObj->getDims();
            int sizex = dataObj->getSize(dims - 1);
            int sizey = dataObj->getSize(dims - 2);
            if ((dims == 1) || ((dims > 1) && ((sizex == 1) || (sizey == 1))))
            {
                plotClassName = dwo->getFigureClass("DObjStaticLine", className, retval);
            
            }
            else
            {
                plotClassName = dwo->getFigureClass("DObjStaticImage", className, retval);
                //not 1D so try 2D ;-) new 2dknoten()
            }

            QLayoutItem *currentItem = m_pGrid->itemAtPosition(areaRow,areaCol);
            QWidget *currentItemWidget = currentItem ? currentItem->widget() : NULL;
            if(currentItemWidget)
            {
                const QMetaObject* meta = currentItemWidget->metaObject();
                if(QString::compare(plotClassName, meta->className(), Qt::CaseInsensitive) == 0)
                {
                    exists = true;
                }
            }

            if(exists == false)
            {
                QWidget* newWidget = uiOrg->loadDesignerPluginWidget(plotClassName, retval, m_pCenterWidget);
                if(newWidget)
                {
                    newWidget->setObjectName( QString("plot%1x%2").arg(areaRow).arg(areaCol) );

                    if(m_pSubplotActions)
                    {
                        int idx = areaCol + areaRow * m_cols;
                        m_pSubplotActions->actions()[ idx ]->setText( tr("subplot %1 (plot)").arg( idx ) );
                    }

                    QWidget *oldWidget = currentItem ? currentItemWidget : NULL;
                    m_pGrid->addWidget(newWidget, areaRow, areaCol, 1, 1);
                    currentItemWidget = newWidget;
                    if(oldWidget) oldWidget->deleteLater();
                }
                else
                {
                    retval += RetVal::format(retError,0,"could not create designer widget of class '%s'", plotClassName.toAscii().data());
                }
            }

            if(!retval.containsError() && currentItemWidget)
            {
                ito::AbstractDObjFigure *dObjFigure = NULL;
                if (currentItemWidget->inherits("ito::AbstractDObjFigure"))
                {
                    dObjFigure = (ito::AbstractDObjFigure*)(currentItemWidget);
                    dObjFigure->setSource(dataObj);
                }
                else
                {
                    retval += RetVal::format(retError,0,"designer widget of class '%s' cannot plot objects of type dataObject", plotClassName.toAscii().data());
                }
            }
        }
        else
        {
            retval += RetVal(retError,0,"designerWidgetOrganizer or uiOrganizer is not available");
        }
    }

    return retval;
}

//----------------------------------------------------------------------------------------------------------------------------------------
RetVal FigureWidget::liveImage(QPointer<AddInDataIO> cam, int areaRow, int areaCol, const QString &className)
{
    DesignerWidgetOrganizer *dwo = qobject_cast<DesignerWidgetOrganizer*>(AppManagement::getDesignerWidgetOrganizer());
    UiOrganizer *uiOrg = qobject_cast<UiOrganizer*>(AppManagement::getUiOrganizer());
    RetVal retval;
    QString plotClassName;
    bool exists = false;

    if(areaRow < 0 || areaRow >= m_rows)
    {
        retval += ito::RetVal::format(ito::retError,0,"areaRow out of range [0,%i]", m_rows-1);
    }

    if(areaCol < 0 || areaCol >= m_cols)
    {
        retval += ito::RetVal::format(ito::retError,0,"arealCol out of range [0,%i]", m_cols-1);
    }

    if(!retval.containsError())
    {

        if(cam.isNull())
        {
            retval += RetVal(retError,0,"camera is not available any more");
        }
        else if(dwo && uiOrg)
        {
            //get size of camera image
            QSharedPointer<ito::Param> sizex = getParamByInvoke(cam.data(), "sizex", retval);
            QSharedPointer<ito::Param> sizey = getParamByInvoke(cam.data(), "sizey", retval);
        
            if(!retval.containsError())
            {
                if(sizex->getVal<int>() == 1 || sizey->getVal<int>() == 1)
                {
                    plotClassName = dwo->getFigureClass("DObjLiveLine", className, retval);
                }
                else
                {
                    plotClassName = dwo->getFigureClass("DObjLiveImage", className, retval);
                }
            }

            QLayoutItem *currentItem = m_pGrid->itemAtPosition(areaRow,areaCol);
            QWidget *currentItemWidget = currentItem ? currentItem->widget() : NULL;
            if(currentItemWidget)
            {
                const QMetaObject* meta = currentItemWidget->metaObject();
                if(QString::compare(plotClassName, meta->className(), Qt::CaseInsensitive) == 0)
                {
                    exists = true;
                }
            }

            if(exists == false)
            {
                QWidget* newWidget = uiOrg->loadDesignerPluginWidget(plotClassName, retval, m_pCenterWidget);
                if(newWidget)
                {
                    newWidget->setObjectName( QString("plot%1x%2").arg(areaRow).arg(areaCol) );

                    if(m_pSubplotActions)
                    {
                        int idx = areaCol + areaRow * m_cols;
                        m_pSubplotActions->actions()[ idx ]->setText( tr("subplot %1 (liveImage)").arg( idx ) );
                    }

                    QWidget *oldWidget = currentItem ? currentItemWidget : NULL;
                    m_pGrid->addWidget(newWidget, areaRow, areaCol, 1, 1);
                    currentItemWidget = newWidget;
                    if(oldWidget) oldWidget->deleteLater();
                }
                else
                {
                    retval += RetVal::format(retError,0,"could not create designer widget of class '%s'", plotClassName.toAscii().data());
                }
            }

            if(!retval.containsError() && currentItemWidget)
            {
                ito::AbstractDObjFigure *dObjFigure = NULL;
                if (currentItemWidget->inherits("ito::AbstractDObjFigure"))
                {
                    dObjFigure = (ito::AbstractDObjFigure*)(currentItemWidget);
                    dObjFigure->setCamera(cam);
                }
                else
                {
                    retval += RetVal::format(retError,0,"designer widget of class '%s' cannot plot objects of type dataObject", plotClassName.toAscii().data());
                }
            }
        }
        else
        {
            retval += RetVal(retError,0,"designerWidgetOrganizer or uiOrganizer is not available");
        }
    }

    return retval;
}

//----------------------------------------------------------------------------------------------------------------------------------------
QWidget *FigureWidget::getSubplot(int index) const
{
    if(m_pGrid)
    {
        int column = index % m_cols;
        int row = (index - column) / m_rows;
        QLayoutItem *item = m_pGrid->itemAtPosition(row,column); //(index);
        if(item)
        {
            return item->widget();
        }
    }
    return NULL;
}


//----------------------------------------------------------------------------------------------------------------------------------------
QSharedPointer<ito::Param> FigureWidget::getParamByInvoke(ito::AddInBase* addIn, const QString &paramName, ito::RetVal &retval)
{
    QSharedPointer<ito::Param> result;

    if(addIn == NULL)
    {
        retval += RetVal(retError,0,"addInBase pointer is NULL");
    }
    else
    {
        ItomSharedSemaphoreLocker locker(new ItomSharedSemaphore());
        ito::Param param = addIn->getParamRec(paramName);
        if ( param.getName() != NULL)   // Parameter is defined
        {
            result = QSharedPointer<ito::Param>(new ito::Param(param));
            QMetaObject::invokeMethod(addIn, "getParam", Q_ARG(QSharedPointer<ito::Param>, result), Q_ARG(ItomSharedSemaphore *, locker.getSemaphore()));
            if (!locker.getSemaphore()->wait(5000) )
            {
                retval += RetVal::format(retError,0,"timeout while getting parameter '%s' from plugin", paramName.toAscii().data());
            }
            else
            {
                retval += locker.getSemaphore()->returnValue;
            }
        }
        else
        {
            retval += RetVal::format(retError,0,"parameter '%s' is not defined in plugin", paramName.toAscii().data());
        }
    }

    return result;
}

//bool FigureWidget::eventFilter(QObject *obj, QEvent *event)
//{
//    switch( event->type() )
//    {
//    case QEvent::KeyPress:
//    case QEvent::KeyRelease:
//    case QEvent::MouseButtonDblClick:
//    case QEvent::MouseButtonPress:
//    case QEvent::MouseButtonRelease:
//    case QEvent::MouseMove:
//        return true; //don't forward event to plot widgets
//    }
//
//    return QObject::eventFilter(obj,event);
//}

RetVal FigureWidget::changeCurrentSubplot(int newIndex)
{
    qDebug() << "new current action " << newIndex;
    QLayoutItem *item = NULL;
    QWidget *widget = NULL;
    int curIdx;

    for(int r = 0 ; r < m_rows ; r++)
    {
        for(int c = 0 ; c < m_cols ; c++)
        {
            curIdx = m_cols * r + c;
            item = m_pGrid->itemAtPosition(r,c);
            if(item)
            {
                widget = item->widget();
                if(widget && curIdx == newIndex)
                {
                    widget->setStyleSheet( QString("QWidget#%1 { border: 2px solid blue } ").arg( widget->objectName() ) );
                    if(m_pSubplotActions) m_pSubplotActions->actions()[ curIdx ]->setChecked(true);
                }
                else if(widget)
                {
                    widget->setStyleSheet( QString("QWidget#%1 { border: 2px none } ").arg( widget->objectName() ) );
                }
            }
        }
    }

    return retOk;
}


void FigureWidget::mnu_subplotActionsTriggered(QAction *action)
{
    if(action)
    {
        changeCurrentSubplot( action->data().toInt() );
        
    }
}

} //end namespace ito
