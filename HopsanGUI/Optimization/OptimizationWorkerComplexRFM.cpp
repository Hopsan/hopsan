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

//Hopsan includes
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "HcomHandler.h"
#include "LogDataHandler.h"
#include "MainWindow.h"
#include "OptimizationHandler.h"
#include "OptimizationWorkerComplex.h"
#include "OptimizationWorkerComplexRFM.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "ComponentUtilities/matrix.h"
#include "ComponentUtilities/EquationSystemSolver.h"

//C++ includes
#include <math.h>

OptimizationWorkerComplexRFM::OptimizationWorkerComplexRFM(OptimizationHandler *pHandler)
    : OptimizationWorkerComplex(pHandler)
{

}


//! @brief Initializes a Complex-RF optimization
void OptimizationWorkerComplexRFM::init()
{
    OptimizationWorkerComplex::init();

    mLastWorstId = -1;
    mWorstCounter = 0;

    for(int p=0; p<mNumPoints; ++p)
    {
        mParameters[p].resize(mNumParameters);
        for(int i=0; i<mNumParameters; ++i)
        {
            double r = (double)rand() / (double)RAND_MAX;
            mParameters[p][i] = mParMin[i] + r*(mParMax[i]-mParMin[i]);
            if(mpHandler->mParameterType == OptimizationHandler::Integer)
            {
                mParameters[p][i] = round(mParameters[p][i]);
            }
        }
    }
    mObjectives.resize(mNumPoints);

    mKf = 1.0-pow(mAlpha/2.0, mGamma/mNumPoints);

    LogDataHandler *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasVariable("WorstObjective"))
    {
        pHandler->deleteVariable("WorstObjective");
    }
    if (pHandler->hasVariable("BestObjective"))
    {
        pHandler->deleteVariable("BestObjective");
    }

    // Close these plotwindows before optimization to make sure old data is removed
    //! @todo should have define or const for this name "parplot"
    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow("parplot");
    if(pPlotWindow)
    {
        PlotTab *pPlotTab = pPlotWindow->getCurrentPlotTab();
        if(pPlotTab)
        {
            while(!pPlotTab->getCurves().isEmpty())
            {
                pPlotTab->removeCurve(pPlotTab->getCurves().first());
            }
        }
    }

    //Calculate how many iterations to store for meta model
    mStorageSize = mNumParameters*mNumParameters/2.0+1.5*mNumParameters+1.0;
    mMetaModelCoefficients.create(mStorageSize);
    mStorageSize = int(mStorageSize*1.5);
    mStoredObjectives.create(mStorageSize);
}



//! @brief Executes a Complex-RF optimization. optComplexInit() must be called before this one.
void OptimizationWorkerComplexRFM::run()
{
    //Plot optimization points
    plotPoints();

    mpHandler->mpConsole->mpTerminal->setEnabledAbortButton(true);

    //Reset convergence reason variable (0 = failed to converge)
    mConvergenceReason=0;

    //Verify that everything is ok
    if(!mpHandler->mpHcomHandler->hasFunction("evalall"))
    {
        printError("Function \"evalall\" not defined.","",false);
        return;
    }
    if(!mpHandler->mpHcomHandler->hasFunction("evalworst"))
    {
        printError("Function \"evalworst\" not defined.","",false);
        return;
    }

    print("Running optimization...");

    //Turn of terminal output during optimization
    execute("echo off");

    //Evaluate initial objective values
    execute("call evalall");

    mEvaluations = mNumPoints;

    //Calculate best and worst id, and initialize last worst id
    calculateBestAndWorstId();
    mLastWorstId = mWorstId;

    //Store parameters for undo
    mParameters = mParameters;

    //Run optimization loop
    int i=0;
    bool metaModelExist = false;
    int metaModelCounter = 0;
    double percDiff = 0.002;
    int countMax = 2;
    int timesWeHaveNotRunTheLastCode=10;
    for(; i<mMaxEvals && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        if(!metaModelExist && mStoredParameters.size() == mStorageSize)
        {
            createMetaModel();
            metaModelExist = true;
        }

        //Plot optimization points
        plotPoints();

        //Process UI events (required so that we don't lock up the program)
        qApp->processEvents();

        //Stop if user pressed abort button
        if(mpHandler->mpHcomHandler->isAborted())
        {
            print("Optimization aborted.");
            finalize();
            return;
        }

        //Print progress as percentage of maximum number of evaluations
        updateProgressBar(i);

        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        forget();

        //Calculate best and worst point
        calculateBestAndWorstId();
        int wid = mWorstId;

        //Plot best and worst objective values
        plotObjectiveFunctionValues();

        //Find geometrical center
        findCenter();

        //Reflect worst point
        QVector<double> newPoint;
        newPoint.resize(mNumParameters);
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[wid][j];
            mParameters[wid][j] = mCenter[j] + (mCenter[j]-worst)*(mAlpha);

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mParameters[wid][j] = mParameters[wid][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mParameters[wid][j] = min(mParameters[wid][j], mParMax[j]);
            mParameters[wid][j] = max(mParameters[wid][j], mParMin[j]);
        }
        newPoint = mParameters[wid]; //Remember the new point, in case we need to iterate below

        gpMainWindow->mpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

        //Evaluate new point
        if(metaModelExist)
        {
            evaluateWithMetaModel();
            metaModelCounter++;
            ++mMetaModelEvaluations;
        }
        else
        {
            execute("call evalworst");
            ++mEvaluations;
            if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                return;
            }
            metaModelCounter=0;
            storeValuesForMetaModel(mWorstId);
        }

        //Calculate best and worst points
        mLastWorstId=wid;
        calculateBestAndWorstId();
        wid = mWorstId;

        //Iterate until worst point is no longer the same
        mWorstCounter=0;
        while(mLastWorstId == wid)
        {
            plotPoints();

            qApp->processEvents();
            if(mpHandler->mpHcomHandler->isAborted())
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                mpHandler->mpHcomHandler->abortHCOM();
                return;
            }

            if(i>mMaxEvals) break;

            double a1 = 1.0-exp(-double(mWorstCounter)/5.0);

            //Reflect worst point
            for(int j=0; j<mNumParameters; ++j)
            {
                double best = mParameters[mBestId][j];
                double maxDiff = getMaxParDiff();
                double r = (double)rand() / (double)RAND_MAX;
                mParameters[wid][j] = (mCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
                mParameters[wid][j] = min(mParameters[wid][j], mParMax[j]);
                mParameters[wid][j] = max(mParameters[wid][j], mParMin[j]);
            }
            newPoint = mParameters[wid];
            gpMainWindow->mpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

            //Evaluate new point
            if(metaModelExist)
            {
                evaluateWithMetaModel();
                metaModelCounter++;
                ++mMetaModelEvaluations;
            }
            else
            {
                execute("call evalworst");
                ++mEvaluations;
                if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
                {
                    execute("echo on");
                    print("Optimization aborted.");
                    finalize();
                    return;
                }
                metaModelCounter = 0;
                storeValuesForMetaModel(mWorstId);
            }

            if(!metaModelExist && mStoredParameters.size() == mStorageSize)
            {
                createMetaModel();
                metaModelExist = true;
            }

            //Calculate best and worst points
            mLastWorstId=wid;
            calculateBestAndWorstId();
            wid = mWorstId;

            ++mWorstCounter;
            ++i;
            execute("echo off");

            if(mWorstCounter >= 10)
            {
                break;
            }
        }

        plotParameters();

        if(metaModelCounter > 0 && timesWeHaveNotRunTheLastCode >= countMax)
        {
            metaModelCounter = 0;

            mWorstId = mLastWorstId;
            execute("call evalworst");
            ++mEvaluations;
            storeValuesForMetaModel(mWorstId);
            calculateBestAndWorstId();
            wid=mWorstId;
            if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                return;
            }

            hopsan::Vec oldMetaModelCoefficients = mMetaModelCoefficients;
            createMetaModel();

            double maxDiff = 0;
            for(int j=0; j<mMetaModelCoefficients.length(); ++j)
            {
                double diff = fabs((mMetaModelCoefficients[j]-oldMetaModelCoefficients[j])/mMetaModelCoefficients[j]);
                if(diff > maxDiff)
                {
                    maxDiff = diff;
                }
            }

            if(maxDiff < percDiff)
            {
                timesWeHaveNotRunTheLastCode=0;
            }
            else
            {
                timesWeHaveNotRunTheLastCode++;
            }
        }
        else
        {
            timesWeHaveNotRunTheLastCode++;
        }
    }

    execute("echo on");

    switch(mConvergenceReason)
    {
    case 0:
        print("Optimization failed to converge after "+QString::number(i)+" iterations.");
        break;
    case 1:
        print("Optimization converged in function values after "+QString::number(i)+" iterations.");
        break;
    case 2:
        print("Optimization converged in parameter values after "+QString::number(i)+" iterations.");
        break;
    }

    print("\nBest point:");
    for(int i=0; i<mNumParameters; ++i)
    {
        print("par("+QString::number(i)+"): "+QString::number(mParameters[mBestId][i]));
    }

    mIterations = mMetaModelEvaluations + mEvaluations;

    // Clean up
    finalize();

    return;
}

void OptimizationWorkerComplexRFM::finalize()
{
    OptimizationWorkerComplex::finalize();
}


void OptimizationWorkerComplexRFM::storeValuesForMetaModel(int idx)
{
    mStoredParameters.append(QVector<double>());
    for(int p=0; p<mParameters[idx].size(); ++p)
    {
        mStoredParameters.last().append(mParameters[idx][p]);
    }

    //Shift vector to the left
    for(int i=0; i<mStoredObjectives.length()-1; ++i)
    {
        mStoredObjectives[i] = mStoredObjectives[i+1];
    }
    mStoredObjectives[mStoredObjectives.length()-1] = mObjectives[idx];

    //Remove first element if stored vectors are too long
    if(mStoredParameters.size() > mStorageSize)
    {
        mStoredParameters.remove(0);        //Assume both vectors always has same length
    }
}


void OptimizationWorkerComplexRFM::createMetaModel()
{
    //Skapa metamodell
    int n=mStorageSize;
    int m=mMetaModelCoefficients.length();

    mMatrix.create(n, m);

    qDebug() << "Number of columns: " << m;

    for(int i=0; i<n; ++i)
    {
        mMatrix[i][0] = 1;
    }

    for(int i=0; i<n; ++i)
    {
        for(int j=0; j<mNumParameters; ++j)
        {
            mMatrix[i][j+1] = mStoredParameters[i][j];
        }
    }

    for(int i=0; i<n; ++i)
    {
        for(int j=0; j<mNumParameters; ++j)
        {
            for(int k=j; k<mNumParameters; ++k)
            {
                mMatrix[i][1+mNumParameters+k+j*mNumParameters-j] = mStoredParameters[i][j]*mStoredParameters[i][k];
            }
        }
    }

    mBVec.create(n);
    for(int i=0; i<n; ++i)
    {
        mBVec[i] = mStoredObjectives[i];
    }


    //Solve system using L and U matrices
    hopsan::Matrix matrixT = mMatrix.transpose();       //Multiply matrix and vector with transpose of matrix, because we need a square matrix
    hopsan::Matrix tempMatrix = matrixT*mMatrix;
    hopsan::Vec tempVec = matrixT*mStoredObjectives;
    int* order = new int[m];
    hopsan::ludcmp(tempMatrix, order);
    hopsan::solvlu(tempMatrix,tempVec,mMetaModelCoefficients,order);
}


void OptimizationWorkerComplexRFM::printMatrix(hopsan::Matrix &matrix)
{
    execute("echo on");
    print("Matrix:");
    for(int i=0; i<matrix.rows(); ++i)
    {
        QString line;
        for(int j=0; j<matrix.cols(); ++j)
        {
            line.append(QString::number(matrix[i][j])+"   ");
        }
        print(line, "", false);
    }
    execute("echo off");
}


void OptimizationWorkerComplexRFM::evaluateWithMetaModel()
{
    double obj=1*mMetaModelCoefficients[0];

    for(int i=0; i<mNumParameters; ++i)
    {
        obj += mMetaModelCoefficients[i+1]*mParameters[mWorstId][i];
    }

    for(int i=0; i<mNumParameters; ++i)
    {
        for(int j=i; j<mNumParameters; ++j)
        {
            obj += mMetaModelCoefficients[1+i*mNumParameters+j+mNumParameters]*mParameters[mWorstId][i]*mParameters[mWorstId][j];
        }
    }

    mObjectives[mWorstId] = obj;
}
