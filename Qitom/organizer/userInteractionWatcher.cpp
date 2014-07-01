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

#include "userInteractionWatcher.h"
#include "common/sharedStructuresPrimitives.h"
#include <qwidget.h>

namespace ito
{

UserInteractionWatcher::UserInteractionWatcher(QWidget *plotWidget, int geomtriecType, int maxNrOfPoints, QSharedPointer<ito::DataObject> coords, ItomSharedSemaphore *semaphore, QObject *parent) :
    QObject(parent), 
    m_pPlotWidget(plotWidget), 
    m_pSemaphore(semaphore), 
    m_maxNrOfPoints(maxNrOfPoints), 
    m_coords(coords),
    m_waiting(true)
{
    connect(m_pPlotWidget, SIGNAL(destroyed(QObject*)), this, SLOT(plotWidgetDestroyed(QObject*)));

    if (coords.data() == NULL)
    {
        if (m_pSemaphore)
        {
            m_pSemaphore->returnValue += ito::RetVal(ito::retError,0,"The given data object is NULL.");
            m_pSemaphore->release();
            m_pSemaphore->deleteSemaphore();
            m_pSemaphore = NULL;
        }
        emit finished();
        return;
    }
        
    if (!connect(m_pPlotWidget, SIGNAL(userInteractionDone(int,bool,QPolygonF)), this, SLOT(userInteractionDone(int,bool,QPolygonF))) )
    {
        if (m_pSemaphore)
        {
            m_pSemaphore->returnValue += ito::RetVal(ito::retError,0,"The given widget does not have the necessary signals and slots for a user interaction.");
            m_pSemaphore->release();
            m_pSemaphore->deleteSemaphore();
            m_pSemaphore = NULL;
        }
        emit finished();
    }
    else if (!connect(this, SIGNAL(userInteractionStart(int,bool,int)), m_pPlotWidget, SLOT(userInteractionStart(int,bool,int))) )
    {
        if (m_pSemaphore)
        {
            m_pSemaphore->returnValue += ito::RetVal(ito::retError,0,"The given widget does not have the necessary signals and slots for a user interaction.");
            m_pSemaphore->release();
            m_pSemaphore->deleteSemaphore();
            m_pSemaphore = NULL;
        }
        emit finished();
    }
    else
    {
        emit userInteractionStart(geomtriecType, true, m_maxNrOfPoints);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
UserInteractionWatcher::~UserInteractionWatcher()
{
    if (m_pSemaphore)
    {
        m_pSemaphore->release();
        m_pSemaphore->deleteSemaphore();
        m_pSemaphore = NULL;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
//! shortdesc
/*! longdesc

    \param obj
*/
void UserInteractionWatcher::plotWidgetDestroyed(QObject *obj)
{
    if (m_pSemaphore)
    {
        if (m_waiting)
        {
            m_pSemaphore->returnValue += ito::RetVal(ito::retError,0,"User interaction terminated due to deletion of plot.");
        }
        m_pSemaphore->release();
        m_pSemaphore->deleteSemaphore();
        m_pSemaphore = NULL;
    }

    if (m_waiting)
    {
        emit finished();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
//! shortdesc
/*! longdesc

    \param type
    \param aborted
    \param points
*/
void UserInteractionWatcher::userInteractionDone(int type, bool aborted, QPolygonF points)
{
    int dims = 2; //m_dObjPtr ? m_dObjPtr->getDims() : 2;
    
    switch(type)
    {
        case ito::PrimitiveContainer::tSquare:
        case ito::PrimitiveContainer::tCircle:
        case ito::PrimitiveContainer::tPolygon:
        default:
        {
            *m_coords = ito::DataObject();
            break;
        }
        case ito::PrimitiveContainer::tMultiPointPick:
        case ito::PrimitiveContainer::tPoint:
        {
            if (aborted)
            {
                points.clear();
                *m_coords = ito::DataObject();
                break;
            }
            m_waiting = false;

            if (aborted) points.clear();

            ito::DataObject output(dims, points.size(), ito::tFloat64);

            ito::float64 *ptr = (ito::float64*)output.rowPtr(0,0);
            int stride = points.size();

            for (int i = 0; i < points.size(); ++i)
            {
                ptr[i] = points[i].rx();
                ptr[i + stride] = points[i].ry();
            }

            *m_coords = output;
            break;
        }
        case ito::PrimitiveContainer::tLine:
        case ito::PrimitiveContainer::tRectangle:
        case ito::PrimitiveContainer::tEllipse:
        {
            if (aborted)
            {
                points.clear();
                *m_coords = ito::DataObject();
                break;
            }
            m_waiting = false;

            dims = 8;
            int elementCount = (points.size() * 2)/ dims;

            ito::DataObject output(dims, elementCount, ito::tFloat64);

#if 0
            ito::float64 *ptr = (ito::float64*)output.rowPtr(0, 0);
            for (int i = 0; i < elementCount; i++)
            {
                int n = i;
                ptr[i]                    = points[i].rx();      //idx
                
                n += elementCount;
                ptr[n]     = points[i].ry();      // type
                
                n += elementCount;
                ptr[n] = points[i + 1].rx();  // x1
                
                n += elementCount;
                ptr[n] = points[i + 1].ry();  // y1
                
                n += elementCount;
                ptr[n] = points[i + 2].rx();  // x2
                
                n += elementCount;
                ptr[n] = points[i + 2].ry();  // y2
                
                n += elementCount;
                ptr[n] = 0.0;  // to be announced
                
                n += elementCount;
                ptr[n] = 0.0;  // alpha
            }
#else
            cv::Mat* dst = (cv::Mat*)(output.get_mdata()[0]);
            for (int i = 0; i < elementCount; i++)
            {
                dst->at<ito::float64>(0, i) = points[4 * i].rx();      //idx
                dst->at<ito::float64>(1, i) = points[4 * i].ry();      //type
                dst->at<ito::float64>(2, i) = points[4 * i + 1].rx();      //x1
                dst->at<ito::float64>(3, i) = points[4 * i + 1].ry();      //y1
                dst->at<ito::float64>(4, i) = points[4 * i + 2].rx();      //x2
                dst->at<ito::float64>(5, i) = points[4 * i + 2].ry();      //y2
                dst->at<ito::float64>(6, i) = 0.0; //points[i + 3].rx();      //???
                dst->at<ito::float64>(7, i) = 0.0; //points[i + 3].ry();      //???
            }
#endif
            *m_coords = output;
            break;
        }
        break;
    }

        

    if (m_pSemaphore)
    {
        if (aborted)
        {
            m_pSemaphore->returnValue += ito::RetVal(ito::retError,0,"User interaction aborted.");
        }
        m_pSemaphore->release();
        m_pSemaphore->deleteSemaphore();
        m_pSemaphore = NULL;
    }

    emit finished();
}




} //end namespace ito