//!
//! @file   ProgressBarThread.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-11
//!
//! @brief Contains a class for displaying progress bar in a separate thread
//!
//$Id$


#include "ProgressBarThread.h"
#include "Widgets/ProjectTabWidget.h"
#include "CoreAccess.h"
#include "MainWindow.h"
#include <QThread>
#include "Configuration.h"


//! Constructor.
ProgressBarThread::ProgressBarThread(ProjectTab *parent)
{
    mpParentProjectTab = parent;
}


//! Implements the task for the thread.
void ProgressBarThread::run()
{
    this->msleep(gConfig.getProgressBarStep());
}
