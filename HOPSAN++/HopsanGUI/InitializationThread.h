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


class ComponentSystem;
class ProjectTabWidget;

class InitializationThread : public QThread
{
public:
    InitializationThread(ComponentSystem *pComponentSystem, double startTime, double finishTime, ProjectTabWidget *parent);

    ProjectTabWidget *mpParentProjectTabWidget;
    ComponentSystem *mpComponentSystem;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;

};

#endif // INITIALIZATIONTHREAD_H
