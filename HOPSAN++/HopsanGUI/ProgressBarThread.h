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
#include <QProgressDialog>

class ProjectTab;

class ProgressBarThread : public QThread
{
public:
    ProgressBarThread(ProjectTab *parent);

protected:
    void run();

private:
    ProjectTab *mpParentProjectTab;
};

#endif // PROGRESSBARTHREAD_H
