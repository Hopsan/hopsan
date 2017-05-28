/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include "global.h"

//! Constructor.
ProgressBarThread::ProgressBarThread(QObject *parent/*ModelWidget *parent*/)
    : QThread(parent)
{
    //mpParentModelWidget = parent; //!< @todo Why do we need project tab here it is never used, we could use QObject* as parent in constructor instead
}


//! Implements the task for the thread.
void ProgressBarThread::run()
{
    this->msleep(gpConfig->getProgressBarStep());
}
