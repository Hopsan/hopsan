//!
//! @file   InitializationThread.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-09
//!
//! @brief Contains a class for Initializing in a separate thread
//!
//$Id$


#ifndef INITIALIZATIONTHREAD_H
#define INITIALIZATIONTHREAD_H

#include <QThread>

class ProjectTab;
class CoreSystemAccess;

class InitializationThread : public QThread
{
public:
    InitializationThread(CoreSystemAccess *pGUIRootSystem, double startTime, double finishTime, size_t nSamples, ProjectTab *parent);

    ProjectTab *mpParentProjectTab;

    CoreSystemAccess *mpGUIRootSystem;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;
    size_t mSamples;

};

#endif // INITIALIZATIONTHREAD_H
