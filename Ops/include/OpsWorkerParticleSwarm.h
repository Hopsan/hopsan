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
//! @file   Ops.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2015-08-31
//!
//! @brief Contains the optimization worker class for the PSO algorithm
//!
//$Id$

#ifndef OPSWORKERPARTICLESWARM_H
#define OPSWORKERPARTICLESWARM_H
#include "OpsWorker.h"

namespace Ops {

enum OpsInertiaStrategy {InertiaConstant, InertiaLinearDecreasing};

class WorkerParticleSwarm : public Worker
{
    Q_OBJECT
public:
    WorkerParticleSwarm(Evaluator *pEvaluator);

    AlgorithmT getAlgorithm();

    void initialize();
    void run();

    void setNumberOfPoints(int value);

    void moveParticles();

    void setOptVar(const QString &var, const QString &value);
    double getOptVar(const QString &var, bool &ok);

    void setOmega1(double value);
    void setOmega2(double value);
    void setC1(double value);
    void setC2(double value);
    void setVmax(double value);
    void setInertiaStrategy(OpsInertiaStrategy strategy);
    void setNumberOfParameters(int value);
protected:

    OpsInertiaStrategy mInertiaStrategy;
    double mOmega, mOmega1, mOmega2, mC1, mC2;
    bool mPrintLogOutput;
    QStringList mLogOutput;
    QVector< QVector<double> > mVelocities, mLocalBestPoints;
    QVector<double> mLocalBestObjectives, mBestPoint;
    double mBestObjective;
    double mVmax;

private:
    void moveParticle(int p);
protected:
    double mRandomFactor;

    QVector<double> mCentroidPoint;
};

}




#endif // OPSWORKERPARTICLESWARM_H
