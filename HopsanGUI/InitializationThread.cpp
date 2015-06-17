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
//! @file   InitializationThread.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-09
//!
//! @brief Contains a class for Initializing in a separate thread
//!
//$Id$

#include <QDebug>

#include "InitializationThread.h"
#include "Widgets/ModelWidget.h"
#include "CoreAccess.h"


//! @class InitializationThread
//! @brief The InitializationThread class implement a class to initialize a core model in a separate thread
//!
//! One reason to implement a initialization in a separate thread is to enable a progress bar.
//!


//! Constructor.
//! @param pComponentSystem is a pointer to the system to initialize.
//! @param startTime is the start time for the initialization.
//! @param finishTime is the finish time for the initialization.
//! @param parent is the parent of the thread, the a CentralTabWidget
InitializationThread::InitializationThread(CoreSystemAccess *pGUIRootSystem, double startTime, double finishTime, size_t nSamples, ModelWidget *parent)
{
    mpParentModelWidget = parent;

    mpGUIRootSystem = pGUIRootSystem;

    mStartTime = startTime;
    mFinishTime = finishTime;
    mSamples = nSamples;

}

//! @brief Check if initialize was successful
bool InitializationThread::wasInitSuccessful() const
{
    return mInitSuccessful;
}


//! Implements the task for the thread.
void InitializationThread::run()
{
    mInitSuccessful = mpGUIRootSystem->initialize(mStartTime, mFinishTime, mSamples);
    qDebug() << "Initialized!";
    //exec(); //Is used if one want to run an event loop in this thread.
}




//! Constructor.
MultipleInitializationThread::MultipleInitializationThread(QVector<CoreSystemAccess *> vGUIRootSystemPtrs, double startTime, double finishTime, size_t nSamples)
{
    mvGUIRootSystemPtrs = vGUIRootSystemPtrs;

    mStartTime = startTime;
    mFinishTime = finishTime;
    mSamples = nSamples;

}

//! @brief Check if initialize was successful
bool MultipleInitializationThread::wasInitSuccessful() const
{
    return mInitSuccessful;
}


//! Implements the task for the thread.
void MultipleInitializationThread::run()
{
    mInitSuccessful = true;
    for(int i=0; i<mvGUIRootSystemPtrs.size(); ++i)
    {
        if(!mvGUIRootSystemPtrs.at(i)->initialize(mStartTime, mFinishTime, mSamples))
            mInitSuccessful = false;
    }

    //exec(); //Is used if one want to run an event loop in this thread.
}
