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
//! @file   ProgressBarThread.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-11
//!
//! @brief Contains a class for displaying a progress bar in a separate class
//!
//$Id$


#ifndef PROGRESSBARTHREAD_H
#define PROGRESSBARTHREAD_H

#include <QThread>
//#include <QProgressDialog>

//class ProjectTab;

class ProgressBarThread : public QThread
{
public:
    ProgressBarThread(QObject *parent/*ProjectTab *parent*/);

protected:
    void run();

private:
    //ProjectTab *mpParentProjectTab;
};

#endif // PROGRESSBARTHREAD_H
