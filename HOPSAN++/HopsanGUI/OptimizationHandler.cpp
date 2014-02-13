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
//! @file   OptimizationHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-08-02
//! @version $Id$
//!
//! @brief Contains a handler for optimizations
//!

//HopsanGUI includes
#include "Configuration.h"
#include "HcomHandler.h"
#include "OptimizationHandler.h"
#include "Optimization/OptimizationWorkerComplexRF.h"
#include "Optimization/OptimizationWorkerComplexRFM.h"
#include "Optimization/OptimizationWorkerComplexRFP.h"
#include "Optimization/OptimizationWorker.h"
#include "Optimization/OptimizationWorkerParticleSwarm.h"
#include "Widgets/HcomWidget.h"


//! @brief Constructor for optimization  handler class
OptimizationHandler::OptimizationHandler(HcomHandler *pHandler)
{
    mpHcomHandler = pHandler;
    mpConsole = pHandler->mpConsole;
    mpConfig = new Configuration();
    mpConfig->loadFromXml();        //This should work, since changes are always saved to file immideately from gpConfig

    mpWorker = 0;

    mAlgorithm = Uninitialized;
}

void OptimizationHandler::startOptimization()
{
    if(mpWorker)
    {
        mpWorker->init();
        mpWorker->run();
    }
    else
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
    }
}

void OptimizationHandler::setOptimizationObjectiveValue(int idx, double value)
{
    if(!mpWorker)
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setOptimizationObjectiveValue(idx, value);
}

void OptimizationHandler::setParMin(int idx, double value)
{
    if(!mpWorker)
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setParMin(idx, value);
}


void OptimizationHandler::setParMax(int idx, double value)
{
    if(!mpWorker)
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setParMax(idx, value);
}


//! @brief Returns objective value with specified index
double OptimizationHandler::getOptimizationObjectiveValue(int idx)
{
    if(!mpWorker)
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return mpWorker->getOptimizationObjectiveValue(idx);
}

double OptimizationHandler::getOptVar(const QString &var)
{
    bool ok;
    return getOptVar(var, ok);
}

double OptimizationHandler::getOptVar(const QString &var, bool &ok) const
{
    if(var == "algorithm")
    {
        return mAlgorithm;
    }
    else if(var == "datatype")
    {
        return mParameterType;
    }

    if(!mpWorker)
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return mpWorker->getOptVar(var, ok);
}


//! @todo "ok" parameter should be used
void OptimizationHandler::setOptVar(const QString &var, const QString &value, bool &ok)
{
    Q_UNUSED(ok);

    if(var == "algorithm")
    {
        if(value == "complexrf")
        {
            mAlgorithm = OptimizationHandler::ComplexRF;
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new OptimizationWorkerComplexRF(this);
        }
        else if(value == "complexrfm")
        {
            mAlgorithm = OptimizationHandler::ComplexRFM;
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new OptimizationWorkerComplexRFM(this);
        }
        else if(value == "complexrfp")
        {
            mAlgorithm = OptimizationHandler::ComplexRFP;
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new OptimizationWorkerComplexRFP(this);
        }
        else if(value == "particleswarm")
        {
            mAlgorithm = OptimizationHandler::ParticleSwarm;
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new OptimizationWorkerParticleSwarm(this);
        }
        return;
    }
    else if(var == "datatype")
    {
        if(value == "double")
            mParameterType = OptimizationHandler::Double;
        else if(value == "int")
            mParameterType = OptimizationHandler::Integer;
    }

    if(!mpWorker)
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setOptVar(var, value);
}

double OptimizationHandler::getParameter(const int pointIdx, const int parIdx) const
{
    if(!mpWorker)
    {
        mpConsole->printErrorMessage("No optimization algorithm selected.");
        mpConsole->printInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return mpWorker->getParameter(pointIdx, parIdx);
}


QVector<ModelWidget *> *OptimizationHandler::getModelPtrs() const
{
    if(mpWorker)
    {
        return (&mpWorker->mModelPtrs);
    }

    return new QVector<ModelWidget *>();
}

