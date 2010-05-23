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
#include "GUIRootSystem.h"


class ComponentSystem;
class ProjectTabWidget;

class InitializationThread : public QThread
{
public:
    InitializationThread(GUIRootSystem *pGUIRootSystem, double startTime, double finishTime, ProjectTabWidget *parent);

    ProjectTabWidget *mpParentProjectTabWidget;

    GUIRootSystem *mpGUIRootSystem;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;

};

#endif // INITIALIZATIONTHREAD_H
