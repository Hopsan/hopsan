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
#include <QVector>

class ProjectTab;
class CoreSystemAccess;

class InitializationThread : public QThread
{
public:
    InitializationThread(CoreSystemAccess *pGUIRootSystem, double startTime, double finishTime, size_t nSamples, ProjectTab *parent);
    const bool wasInitSuccessful() const;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;
    size_t mSamples;
    bool mInitSuccessful;

    ProjectTab *mpParentProjectTab;
    CoreSystemAccess *mpGUIRootSystem;
};


class MultipleInitializationThread : public QThread
{
public:
    MultipleInitializationThread(QVector<CoreSystemAccess *> vGUIRootSystemPtrs, double startTime, double finishTime, size_t nSamples);
    const bool wasInitSuccessful() const;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;
    size_t mSamples;
    bool mInitSuccessful;

    QVector<CoreSystemAccess *> mvGUIRootSystemPtrs;
};

#endif // INITIALIZATIONTHREAD_H
