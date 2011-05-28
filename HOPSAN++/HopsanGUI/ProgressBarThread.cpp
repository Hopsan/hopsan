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
