//!
//! @file   ProgressBarThread.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-11
//!
//! @brief Contains a class for displaying progress bar in a separate thread
//!
//$Id$

#include "ProgressBarThread.h"
#include "Configuration.h"
//#include "Widgets/ProjectTabWidget.h"

//! Constructor.
ProgressBarThread::ProgressBarThread(QObject *parent/*ProjectTab *parent*/)
    : QThread(parent)
{
    //mpParentProjectTab = parent; //!< @todo Why do we need project tab here it is never used, we could use QObject* as parent in constructor instead
}


//! Implements the task for the thread.
void ProgressBarThread::run()
{
    this->msleep(gConfig.getProgressBarStep());
}
