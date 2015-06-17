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
//! @file   OptimizationWorkerParameterSweep.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-19
//! @version $Id$
//!
//! @brief Contains an optimization worker object for parameter sweeps
//!


#ifndef OPTIMIZATIONWORKERPARAMETERSWEEP_H
#define OPTIMIZATIONWORKERPARAMETERSWEEP_H

#include "OptimizationWorker.h"

class OptimizationWorkerParameterSweep : public OptimizationWorker
{
public:
    OptimizationWorkerParameterSweep(OptimizationHandler *pHandler);

    virtual void init(const ModelWidget *pModel, const QString &modelPath);
    virtual void run();
    virtual void finalize();

    virtual void setOptVar(const QString &var, const QString &value);
    virtual double getOptVar(const QString &var, bool &ok);

    void evaluateAllPoints();
    void printLogFile();

private:
    QVector< QVector<double> > mAllPoints;
    QVector<double> mAllObjectives;
    int mNumModels;
    int mLength;
};

#endif // OPTIMIZATIONWORKERPARAMETERSWEEP_H
