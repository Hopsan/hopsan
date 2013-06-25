/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   InitializationThread.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-09
//!
//! @brief Contains a class for Initializing in a separate thread
//!
//$Id$


#include "InitializationThread.h"
#include "Widgets/ModelWidget.h"
#include "MainWindow.h"
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
