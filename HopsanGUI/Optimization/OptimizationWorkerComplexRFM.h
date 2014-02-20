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
//! @file   OptimizationWorkerComplexRFM.h
//! @author Johan Persson <johan.persson@liu.se>
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id: OptimizationHandler.cpp 6525 2014-01-30 15:58:59Z petno25 $
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

    void init();
    void run();
    void finalize();

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
};

#endif // OPTIMIZATIONWORKERCOMPLEXRFM_H
