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
//! @file   OptimizationWorkerComplexRFM.h
//! @author Johan Persson <johan.persson@liu.se>
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Complex-RFM algorithm
//!

#ifndef OPTIMIZATIONWORKERCOMPLEXRFM_H
#define OPTIMIZATIONWORKERCOMPLEXRFM_H

#include <QVector>

#include "OptimizationWorkerComplex.h"
#include "ComponentUtilities/matrix.h"

class OptimizationHandler;

class OptimizationWorkerComplexRFM : public OptimizationWorkerComplex
{
public:
    OptimizationWorkerComplexRFM(OptimizationHandler *pHandler);

    void init(const ModelWidget *pModel, const QString &modelPath);
    void run();
    void finalize();

    bool checkForConvergence();

    void setOptVar(const QString &var, const QString &value);
    double getOptVar(const QString &var, bool &ok);

private:
    void storeValuesForMetaModel(int idx);
    void createMetaModel();
    void printMatrix(hopsan::Matrix &matrix);
    void evaluateWithMetaModel();

    int mStorageSize;
    QVector< QVector<double> > mStoredParameters;

    hopsan::Vec mStoredObjectives;
    hopsan::Vec mMetaModelCoefficients;
    hopsan::Matrix mMatrix;
    hopsan::Vec mAVec;
    hopsan::Vec mBVec;

    double mPercDiff;
    int mCountMax;

    bool mUseMetaModel;
    bool mMetaModelExist;
};

#endif // OPTIMIZATIONWORKERCOMPLEXRFM_H
