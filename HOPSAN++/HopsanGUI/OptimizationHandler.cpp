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
//! @file   HcomHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a handler for the HCOM scripting language
//!
//HopsanGUI includes
#include "common.h"
#include "OptimizationHandler.h"
#include "HcomHandler.h"
#include "ModelHandler.h"
#include "MainWindow.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "GUIObjects/GUISystem.h"
#include "DesktopHandler.h"
#include "PlotHandler.h"
#include "Widgets/ProjectTabWidget.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "PlotCurve.h"


OptimizationHandler::OptimizationHandler(HcomHandler *pHandler)
{
    mpHcomHandler = pHandler;
    mpConsole = pHandler->mpConsole;

    mOptAlgorithm = Uninitialized;
    mOptPlotPoints = false;
    mOptPlotBestWorst = false;
}

double OptimizationHandler::getOptimizationObjectiveValue(int idx)
{
    if(idx<0 || idx > mOptObjectives.size()-1)
    {
        return 0;
    }
    return mOptObjectives[idx];
}

void OptimizationHandler::optComplexInit()
{
    gpMainWindow->mpModelHandler->setCurrentModel(mpOptModel);

    //Load default optimization functions
    QString oldPath = mpHcomHandler->getWorkingDirectory();
    mpHcomHandler->setWorkingDirectory(gDesktopHandler.getExecPath());
    //executeCommand("exec ../Scripts/HCOM/optDefaultFunctions.hcom");
    QFile testFile1(gDesktopHandler.getScriptsPath()+"/HCOM/optDefaultFunctions.hcom");
    QFile testFile2(gDesktopHandler.getExecPath()+"../Scripts/HCOM/optDefaultFunctions.hcom");
    if(testFile1.exists())
    {
        mpHcomHandler->executeCommand("exec "+testFile1.fileName());
    }
    else if(testFile2.exists())
    {
        mpHcomHandler->executeCommand("exec "+testFile2.fileName());
    }
    else
    {
        mpConsole->printErrorMessage("Cannot find optimization default functions script file.","",false);
        return;
    }
    mpHcomHandler->setWorkingDirectory(oldPath);

    for(int p=0; p<mOptNumPoints; ++p)
    {
        mOptParameters[p].resize(mOptNumParameters);
        for(int i=0; i<mOptNumParameters; ++i)
        {
            double r = (double)rand() / (double)RAND_MAX;
            mOptParameters[p][i] = mOptParMin[i] + r*(mOptParMax[i]-mOptParMin[i]);
            if(mOptParameterType == Int)
            {
                mOptParameters[p][i] = round(mOptParameters[p][i]);
            }
        }
    }
    mOptObjectives.resize(mOptNumPoints);

    mOptKf = 1.0-pow(mOptAlpha/2.0, mOptGamma/mOptNumPoints);

    if(!gpMainWindow->mpModelHandler->getCurrentModel()->isSaved())
    {
        mpConsole->printErrorMessage("Current model is not saved. Please save it before running an optimization.", "", false);
        return;
    }

    LogDataHandler *pHandler = gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasPlotData("WorstObjective"))
    {
        pHandler->deleteVariable("WorstObjective");
    }
    if (pHandler->hasPlotData("BestObjective"))
    {
        pHandler->deleteVariable("BestObjective");
    }

    // Close these plotwindows before optimization to make sure old data is removed
    //! @todo should have define or const for this name "parplot"
    gpPlotHandler->closeWindow("parplot");
}


void OptimizationHandler::optComplexRun()
{
    optPlotPoints();

    mOptConvergenceReason=0;

    if(mOptAlgorithm == Uninitialized)
    {
        mpConsole->printErrorMessage("Optimization not initialized.", "", false);
        return;
    }
    if(!mpHcomHandler->hasFunction("evalall"))
    {
        mpConsole->printErrorMessage("Function \"evalall\" not defined.","",false);
        return;
    }
    if(!mpHcomHandler->hasFunction("evalworst"))
    {
        mpConsole->printErrorMessage("Function \"evalworst\" not defined.","",false);
        return;
    }

    mpConsole->print("Running optimization...");
    mpHcomHandler->executeCommand("echo off");

    //Evaluate initial objevtive values
    mpHcomHandler->executeCommand("call evalall");

    //Store parameters for undo
    mOptOldParameters = mOptParameters;

    int i=0;
    int percent=-1;
    for(; i<mOptMaxEvals && !mpHcomHandler->isAborted(); ++i)
    {
        optPlotPoints();

        qApp->processEvents();
        if(mpHcomHandler->isAborted())
        {
            mpConsole->print("Optimization aborted.");
            return;
        }

        //Print progress %
        int dummy=int(100.0*double(i)/mOptMaxEvals);
        if(dummy != percent)
        {
            mpConsole->setDontPrint(false);
            mpConsole->print(QString::number(dummy)+"%");
            mpConsole->setDontPrint(true);
            percent = dummy;
        }

        //Check convergence
        if(optComlexCheckconvergence()) break;

        //Increase all objective values (forgetting principle)
        optComplexForget();

        //Calculate best and worst point
        optComplexCalculatebestandworstid();
        mpConsole->print("WORST: "+QString::number(mOptWorstId));
        int wid = mOptWorstId;

        optPlotBestWorstObj();

        //Find geometrical center
        optComplexFindcenter();

        //Reflect worst point
        QVector<double> newPoint;
        newPoint.resize(mOptNumParameters);
        for(int j=0; j<mOptNumParameters; ++j)
        {
            //Reflect
            double worst = mOptParameters[wid][j];
            mOptParameters[wid][j] = mOptCenter[j] + (mOptCenter[j]-worst)*mOptAlpha;

            //Add some random noise
            double maxDiff = optComplexMaxpardiff();
            double r = (double)rand() / (double)RAND_MAX;
            mOptParameters[wid][j] = mOptParameters[wid][j] + mOptRfak*(mOptParMax[j]-mOptParMin[j])*maxDiff*(r-0.5);
            mOptParameters[wid][j] = min(mOptParameters[wid][j], mOptParMax[j]);
            mOptParameters[wid][j] = max(mOptParameters[wid][j], mOptParMin[j]);
        }
        newPoint = mOptParameters[wid];

        //Evaluate new point
        mpHcomHandler->executeCommand("call evalworst");
        if(mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
        {
            mpHcomHandler->executeCommand("echo on");
            mpConsole->print("Optimization aborted.");
            return;
        }

        //Calculate best and worst points
        mOptLastWorstId=wid;
        optComplexCalculatebestandworstid();
        wid = mOptWorstId;

        //Iterate until worst point is no longer the same
        mOptWorstCounter=0;
        while(mOptLastWorstId == wid)
        {
            optPlotPoints();

            qApp->processEvents();
            if(mpHcomHandler->isAborted())
            {
                mpHcomHandler->executeCommand("echo on");
                mpConsole->print("Optimization aborted.");
                mpHcomHandler->abortHCOM();
                return;
            }

            if(i>mOptMaxEvals) break;

            double a1 = 1.0-exp(-double(mOptWorstCounter)/5.0);

            //Reflect worst point
            for(int j=0; j<mOptNumParameters; ++j)
            {
                double best = mOptParameters[mOptBestId][j];
                double maxDiff = optComplexMaxpardiff();
                double r = (double)rand() / (double)RAND_MAX;
                mOptParameters[wid][j] = (mOptCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mOptRfak*(mOptParMax[j]-mOptParMin[j])*maxDiff*(r-0.5);
                mOptParameters[wid][j] = min(mOptParameters[wid][j], mOptParMax[j]);
                mOptParameters[wid][j] = max(mOptParameters[wid][j], mOptParMin[j]);
            }
            newPoint = mOptParameters[wid];

            //Evaluate new point
            mpHcomHandler->executeCommand("call evalworst");
            if(mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
            {
                mpHcomHandler->executeCommand("echo on");
                mpConsole->print("Optimization aborted.");
                return;
            }

            //Calculate best and worst points
            mOptLastWorstId=wid;
            optComplexCalculatebestandworstid();
            wid = mOptWorstId;

            ++mOptWorstCounter;
            ++i;
        }
    }

    mpHcomHandler->executeCommand("echo on");

    switch(mOptConvergenceReason)
    {
    case 0:
        mpConsole->print("Optimization failed to converge after "+QString::number(i)+" iterations.");
        break;
    case 1:
        mpConsole->print("Optimization converged in function values after "+QString::number(i)+" iterations.");
        break;
    case 2:
        mpConsole->print("Optimization converged in parameter values after "+QString::number(i)+" iterations.");
        break;
    }

    mpConsole->print("\nBest point:");
    for(int i=0; i<mOptNumParameters; ++i)
    {
        mpConsole->print("par("+QString::number(i)+"): "+QString::number(mOptParameters[mOptBestId][i]));
    }

    //! @todo currentWidget MAY hav changed, solve in better way
    ModelWidget *pOrgModel = qobject_cast<ModelWidget*>(gpMainWindow->mpCentralTabs->currentWidget());
    pOrgModel->getTopLevelSystemContainer()->getLogDataHandler()->takeOwnershipOfData(mpOptModel->getTopLevelSystemContainer()->getLogDataHandler(), -2);
    gpMainWindow->mpModelHandler->setCurrentModel(pOrgModel);

    // Close the obsolete optimisation model
    gpMainWindow->mpModelHandler->closeModel(mpOptModel);

    return;
}


void OptimizationHandler::optComplexForget()
{
    double maxObj = mOptObjectives[0];
    double minObj = mOptObjectives[0];
    for(int i=0; i<mOptNumPoints; ++i)
    {
        double obj = mOptObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    for(int i=0; i<mOptObjectives[0]; ++i)
    {
        mOptObjectives[0] = mOptObjectives[0]+(maxObj-minObj)*mOptKf;
    }
}

void OptimizationHandler::optComplexCalculatebestandworstid()
{
    double maxObj = mOptObjectives[0];
    double minObj = mOptObjectives[0];
    mOptWorstId=0;
    mOptBestId=0;
    for(int i=1; i<mOptNumPoints; ++i)
    {
        double obj = mOptObjectives[i];
        if(obj > maxObj)
        {
            maxObj = obj;
            mOptWorstId = i;
        }
        if(obj < minObj)
        {
            minObj = obj;
            mOptBestId = i;
        }
    }
}

void OptimizationHandler::optComplexFindcenter()
{
    mOptCenter.resize(mOptNumParameters);
    for(int i=0; i<mOptCenter.size(); ++i)
    {
        mOptCenter[i] = 0;
    }
    for(int p=0; p<mOptNumPoints; ++p)
    {
        for(int i=0; i<mOptNumParameters; ++i)
        {
            mOptCenter[i] = mOptCenter[i]+mOptParameters[p][i];
        }
    }
    for(int i=0; i<mOptCenter.size(); ++i)
    {
        mOptCenter[i] = mOptCenter[i]/double(mOptNumPoints);
    }
}

bool OptimizationHandler::optComlexCheckconvergence()
{
    //Check objective function convergence
    double maxObj = mOptObjectives[0];
    double minObj = mOptObjectives[0];
    for(int i=0; i<mOptNumPoints; ++i)
    {
        double obj = mOptObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    if(fabs(maxObj-minObj) <= mOptFuncTol)
    {
        mOptConvergenceReason=1;
        return true;
    }
    else if(minObj != 0.0 && fabs(maxObj-minObj)/fabs(minObj) <= mOptFuncTol)
    {
        mOptConvergenceReason=1;
        return true;
    }

    //Check parameter value convergence
    double maxDiff=optComplexMaxpardiff();
    if(fabs(maxDiff) < mOptParTol)
    {
        mOptConvergenceReason=2;
        return true;
    }
    return false;
}


double OptimizationHandler::optComplexMaxpardiff()
{
    double maxDiff = -1e100;
    for(int i=0; i<mOptNumParameters; ++i)
    {
        double maxPar = -1e100;
        double minPar = 1e100;
        for(int p=0; p<mOptNumPoints; ++p)
        {
            if(mOptParameters[p][i] > maxPar) maxPar = mOptParameters[p][i];
            if(mOptParameters[p][i] < minPar) minPar = mOptParameters[p][i];
        }
        if((maxPar-minPar)/(mOptParMax[i]-mOptParMin[i]) > maxDiff)
        {
            maxDiff = (maxPar-minPar)/(mOptParMax[i]-mOptParMin[i]);
        }
    }
    return maxDiff;
}


void OptimizationHandler::optParticleInit()
{
    gpMainWindow->mpModelHandler->setCurrentModel(mpOptModel);

    //Load default optimization functions
    QString oldPath = mpHcomHandler->getWorkingDirectory();
    mpHcomHandler->setWorkingDirectory(gDesktopHandler.getExecPath());
    mpHcomHandler->executeCommand("exec ../Scripts/HCOM/optDefaultFunctions.hcom");
    mpHcomHandler->setWorkingDirectory(oldPath);

    if(mOptMulticore)
    {
        QString modelPath = gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getModelFileInfo().filePath();
        gpMainWindow->mpModelHandler->getCurrentModel()->save();
        gpMainWindow->mpModelHandler->closeAllModels();
        for(int i=0; i<mOptNumPoints; ++i)
        {
            gpMainWindow->mpModelHandler->loadModel(modelPath, true);
        }
    }

    for(int p=0; p<mOptNumPoints; ++p)
    {
        mOptParameters[p].resize(mOptNumParameters);
        mOptVelocities[p].resize(mOptNumParameters);
        for(int i=0; i<mOptNumParameters; ++i)
        {
            //Initialize points
            double r = double(rand()) / double(RAND_MAX);
            mOptParameters[p][i] = mOptParMin[i] + r*(mOptParMax[i]-mOptParMin[i]);
            if(mOptParameterType == Int)
            {
                mOptParameters[p][i] = round(mOptParameters[p][i]);
            }

            //Initialize velocities
            double minVel = -fabs(mOptParMax[i]-mOptParMin[i]);
            double maxVel = fabs(mOptParMax[i]-mOptParMin[i]);
            r = double(rand()) / double(RAND_MAX);
            mOptVelocities[p][i] = minVel + r*(maxVel-minVel);
        }
    }
    mOptObjectives.resize(mOptNumPoints);

    LogDataHandler *pHandler = gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasPlotData("WorstObjective"))
    {
        pHandler->deleteVariable("WorstObjective");
    }
    if (pHandler->hasPlotData("BestObjective"))
    {
        pHandler->deleteVariable("BestObjective");
    }

    // Close these plotwindows before optimization to make sure old data is removed
    //! @todo should have define or const for this name "parplot"
    gpPlotHandler->closeWindow("parplot");
}


void OptimizationHandler::optParticleRun()
{
    optPlotPoints();

    //connect(gpMainWindow->mpModelHandler->getCurrentModel()->mpSimulationThreadHandler, SIGNAL(done(bool)), this, SLOT(optPlotPoints(bool)));

    mOptConvergenceReason=0;

    if(mOptAlgorithm == Uninitialized)
    {
        mpConsole->printErrorMessage("Optimization not initialized.", "", false);
        return;
    }
    if(!mpHcomHandler->hasFunction("evalall"))
    {
        mpConsole->printErrorMessage("Function \"evalall\" not defined.","",false);
        return;
    }

    mpConsole->print("Running optimization...");
    mpHcomHandler->executeCommand("echo off");

    //Evaluate initial objevtive values
    mpHcomHandler->executeCommand("call evalall");
    if(mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
    {
        mpHcomHandler->executeCommand("echo on");
        mpConsole->print("Optimization aborted.");
        return;
    }

    //Initialize best known point for each point
    for(int i=0; i<mOptNumPoints; ++i)
    {
        mOptBestKnowns[i] = mOptParameters[i];
        mOptBestObjectives[i] = mOptObjectives[i];
    }

    //Calculate best known global position
    optComplexCalculatebestandworstid();
    mOptBestObj = mOptObjectives[mOptBestId];
    mOptBestPoint = mOptParameters[mOptBestId];

    int i=0;
    int percent=-1;
    for(; i<mOptMaxEvals && !mpHcomHandler->isAborted(); ++i)
    {
        qApp->processEvents();
        if(mpHcomHandler->isAborted())
        {
            mpConsole->print("Optimization aborted.");
            return;
        }

        //Print progress %
        int dummy=int(100.0*double(i)/mOptMaxEvals);
        if(dummy != percent)
        {
            mpConsole->setDontPrint(false);
            mpConsole->print(QString::number(dummy)+"%");
            mpConsole->setDontPrint(true);
            percent = dummy;
        }

        //Move particles
        for (int p=0; p<mOptNumPoints; ++p)
        {
          double r1 = double(rand())/double(RAND_MAX);
          double r2 = double(rand())/double(RAND_MAX);
          for(int j=0; j<mOptNumParameters; ++j)
          {
              mOptVelocities[p][j] = mOptOmega*mOptVelocities[p][j] + mOptC1*r1*(mOptBestKnowns[p][j]-mOptParameters[p][j]) + mOptC2*r2*(mOptBestPoint[j]-mOptParameters[p][j]);
              mOptParameters[p][j] = mOptParameters[p][j]+mOptVelocities[p][j];
              if(mOptParameters[p][j] <= mOptParMin[j])
              {
                  mOptParameters[p][j] = mOptParMin[j];
                  mOptVelocities[p][j] = 0.0;
              }
              if(mOptParameters[p][j] >= mOptParMax[j])
              {
                  mOptParameters[p][j] = mOptParMax[j];
                  mOptVelocities[p][j] = 0.0;
              }
          }
        }

        //Evaluate objevtive values
        mpHcomHandler->executeCommand("call evalall");
        if(mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
        {
            mpHcomHandler->executeCommand("echo on");
            mpConsole->print("Optimization aborted.");
            return;
        }

        //Calculate best known positions
        for(int p=0; p<mOptNumPoints; ++p)
        {
            if(mOptObjectives[p] < mOptBestObjectives[p])
            {
                mOptBestKnowns[p] = mOptParameters[p];
                mOptBestObjectives[p] = mOptObjectives[p];
            }
        }

        //Calculate best known global position
        optComplexCalculatebestandworstid();
        if(mOptObjectives[mOptBestId] < mOptBestObj)
        {
            mOptBestObj = mOptObjectives[mOptBestId];
            mOptBestPoint = mOptParameters[mOptBestId];
        }

        optPlotPoints();
        optPlotBestWorstObj();

        //Check convergence
        if(optComlexCheckconvergence()) break;      //Use complex method, it's the same principle

        //optPlotPoints();
    }

    mpHcomHandler->executeCommand("echo on");

    switch(mOptConvergenceReason)
    {
    case 0:
        mpConsole->print("Optimization failed to converge after "+QString::number(i)+" iterations.");
        break;
    case 1:
        mpConsole->print("Optimization converged in function values after "+QString::number(i)+" iterations.");
        break;
    case 2:
        mpConsole->print("Optimization converged in parameter values after "+QString::number(i)+" iterations.");
        break;
    }


    ModelWidget *pOrgModel = qobject_cast<ModelWidget*>(gpMainWindow->mpCentralTabs->currentWidget());
    pOrgModel->getTopLevelSystemContainer()->getLogDataHandler()->takeOwnershipOfData(mpOptModel->getTopLevelSystemContainer()->getLogDataHandler(), -2);
    gpMainWindow->mpModelHandler->setCurrentModel(pOrgModel);

    // Close the obsolete optimisation model
    gpMainWindow->mpModelHandler->closeModel(mpOptModel);
}

void OptimizationHandler::optPlotPoints()
{
    if(!mOptPlotPoints) { return; }

    if(mOptNumParameters != 2)
    {
        mpConsole->printErrorMessage("Points can only be plotted with two parameters.");
        return;
    }

    LogDataHandler *pHandler = gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getLogDataHandler();
    for(int p=0; p<mOptNumPoints; ++p)
    {
        QString namex = "par"+QString::number(p)+"x";
        QString namey = "par"+QString::number(p)+"y";
        double x = mOptParameters[p][0];
        double y = mOptParameters[p][1];
        SharedLogVariableDataPtrT parVar_x = pHandler->getPlotData(namex, -1);
        SharedLogVariableDataPtrT parVar_y = pHandler->getPlotData(namey, -1);
        if(!parVar_x)
        {
            //! @todo we should set name and unit and maybe description (in define variable)
            parVar_x = pHandler->defineNewVariable(namex);
            parVar_y = pHandler->defineNewVariable(namey);
            parVar_x->preventAutoRemoval();
            parVar_y->preventAutoRemoval();

            parVar_x->assignFrom(x);
            parVar_y->assignFrom(y);

            gpPlotHandler->plotDataToWindow("parplot", parVar_x, parVar_y, 0);
        }
        else
        {
            //! @todo need to turn of auto refresh on plot and trygger it manually to avoid multiple redraws here
            parVar_x->assignFrom(x);
            parVar_y->assignFrom(y);
        }
    }

    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow("parplot");
    if(pPlotWindow)
    {
        PlotTab *pTab = pPlotWindow->getCurrentPlotTab();
        for(int c=0; c<pTab->getCurves(FirstPlot).size(); ++c)
        {
            if(c==mOptBestId)
            {
                pTab->getCurves(FirstPlot).at(c)->setLineSymbol("Star 1");
            }
            else
            {
                pTab->getCurves(FirstPlot).at(c)->setLineSymbol("XCross");
            }
        }
        pTab->getPlot(FirstPlot)->setAxisScale(QwtPlot::xBottom, mOptParMin[0], mOptParMax[0]);
        pTab->getPlot(FirstPlot)->setAxisScale(QwtPlot::yLeft, mOptParMin[1], mOptParMax[1]);
        pTab->update();
    }
}


void OptimizationHandler::optPlotBestWorstObj()
{
    if(!mOptPlotBestWorst) { return; }

    LogDataHandler *pHandler = gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getLogDataHandler();
    SharedLogVariableDataPtrT bestVar = pHandler->getPlotData("BestObjective", -1);
    if(bestVar.isNull())
    {
        //! @todo unit and description
        bestVar = pHandler->defineNewVariable("BestObjective");
        bestVar->preventAutoRemoval();
        bestVar->assignFrom(mOptObjectives[mOptBestId]);
        bestVar->setCacheDataToDisk(false);
    }
    else
    {
        bestVar->append(mOptObjectives[mOptBestId]);
    }
    SharedLogVariableDataPtrT worstVar = pHandler->getPlotData("WorstObjective", -1);
    if(worstVar.isNull())
    {
        worstVar = pHandler->defineNewVariable("WorstObjective");
        worstVar->preventAutoRemoval();
        worstVar->assignFrom(mOptObjectives[mOptWorstId]);
        worstVar->setCacheDataToDisk(false);
    }
    else
    {
        worstVar->append(mOptObjectives[mOptWorstId]);
    }

    // If this is the first time, then recreate the plotwindows
    // Note! plots will autoupdate when new data is appended, so there is no need to call plotab->update()
    if(bestVar.data()->getDataSize() == 1)
    {
        PlotWindow *pPW = gpPlotHandler->createNewOrReplacePlotwindow("ObjectiveFunction");
        gpPlotHandler->plotDataToWindow(pPW, bestVar, 0, QColor("Green"));
        gpPlotHandler->plotDataToWindow(pPW, worstVar, 0, QColor("Red"));
    }
}





