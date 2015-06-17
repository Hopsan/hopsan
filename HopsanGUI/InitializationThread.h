/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

class ModelWidget;
class CoreSystemAccess;

class InitializationThread : public QThread
{
public:
    InitializationThread(CoreSystemAccess *pGUIRootSystem, double startTime, double finishTime, size_t nSamples, ModelWidget *parent);
    bool wasInitSuccessful() const;

protected:
    void run();

private:
    double mStartTime;
    double mFinishTime;
    size_t mSamples;
    bool mInitSuccessful;

    ModelWidget *mpParentModelWidget;
    CoreSystemAccess *mpGUIRootSystem;
};


class MultipleInitializationThread : public QThread
{
public:
    MultipleInitializationThread(QVector<CoreSystemAccess *> vGUIRootSystemPtrs, double startTime, double finishTime, size_t nSamples);
    bool wasInitSuccessful() const;

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
