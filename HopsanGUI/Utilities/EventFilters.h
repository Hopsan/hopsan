/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   EventFilters.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @brief Contains event filters that can be used to gobble events
//!
//$Id$

#ifndef EVENTFILTERS_H
#define EVENTFILTERS_H

#include <QObject>
#include <QWidget>
#include <QEvent>

class MouseWheelEventEater : public QObject
{
    Q_OBJECT
public:
    MouseWheelEventEater(QWidget *pParent) : QObject(pParent)
    {
        // Nothing in particular
    }
protected:
    bool eventFilter(QObject *pObj, QEvent *pEvent)
    {
        if (pEvent->type() == QEvent::Wheel)
        {
            //QWheelEvent *pWheelEvent = static_cast<QKeyEvent *>(pEvent);
            return true;
        }
        else
        {
            // standard event processing
            return QObject::eventFilter(pObj, pEvent);
        }
    }
};

#endif // EVENTFILTERS_H
