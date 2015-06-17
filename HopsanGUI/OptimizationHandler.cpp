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

﻿/*-----------------------------------------------------------------------------
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
#include "Optimization/OptimizationWorkerSimplex.h"
#include "Optimization/OptimizationWorkerComplexRF.h"
#include "Optimization/OptimizationWorkerComplexRFM.h"
#include "Optimization/OptimizationWorkerComplexRFP.h"
#include "Optimization/OptimizationWorker.h"
#include "Optimization/OptimizationWorkerParticleSwarm.h"
#include "Optimization/OptimizationWorkerParameterSweep.h"
#include "MessageHandler.h"
#include "ModelHandler.h"
#include "Widgets/ModelWidget.h"


//! @brief Constructor for optimization  handler class
OptimizationHandler::OptimizationHandler(HcomHandler *pHandler)
{
    mpHcomHandler = pHandler;
    mpMessageHandler = new GUIMessageHandler(this);
    mpMessageHandler->startPublish();
    mpConfig = new Configuration(); //!< @todo memory leak, never deleted
    mpConfig->loadFromXml();        //This should work, since changes are always saved to file immediately from gpConfig

    mpWorker = 0;
    mAlgorithm = Uninitialized;
    mIsRunning = false;
}

void OptimizationHandler::startOptimization(ModelWidget *pModel, QString &modelPath)
{
    if(mpWorker)
    {
        mpWorker->init(pModel, modelPath);
        mpWorker->run();
    }
    else
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
    }
}

void OptimizationHandler::setOptimizationObjectiveValue(int idx, double value)
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setOptimizationObjectiveValue(idx, value);
}

void OptimizationHandler::setParMin(int idx, double value)
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setParMin(idx, value);
}


void OptimizationHandler::setParMax(int idx, double value)
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setParMax(idx, value);
}


//! @brief Returns objective value with specified index
double OptimizationHandler::getOptimizationObjectiveValue(int idx)
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
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
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
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
        if(value == "simplex")
        {
            mAlgorithm = OptimizationHandler::Simplex;
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new OptimizationWorkerSimplex(this);
        }
        else if(value == "complexrf")
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
        else if(value == "parametersweep")
        {
            mAlgorithm = OptimizationHandler::ParameterSweep;
            if(mpWorker)
            {
                delete mpWorker;
            }
            mpWorker = new OptimizationWorkerParameterSweep(this);
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
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return;
    }

    mpWorker->setOptVar(var, value);
}

double OptimizationHandler::getParameter(const int pointIdx, const int parIdx) const
{
    if(!mpWorker)
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
        mpMessageHandler->addInfoMessage("Hint: use \"opt set algorithm <type>\" to selected algorithm.");
        return 0;
    }

    return mpWorker->getParameter(pointIdx, parIdx);
}

void OptimizationHandler::setIsRunning(bool value)
{
    mIsRunning = value;
}

bool OptimizationHandler::isRunning()
{
    return mIsRunning;
}

QStringList *OptimizationHandler::getOptParNamesPtr()
{
    return mpWorker->getParNamesPtr();
}


const QVector<ModelWidget *> *OptimizationHandler::getModelPtrs() const
{
    if(mpWorker)
    {
        return (&mpWorker->mModelPtrs);
    }

    //! @todo this is a memory leak
    return new QVector<ModelWidget *>();
}

void OptimizationHandler::clearModels()
{
    if (mpWorker)
    {
        mpWorker->clearModels();
    }
}

void OptimizationHandler::addModel(ModelWidget *pModel)
{
    if (mpWorker)
    {
        mpWorker->mModelPtrs.append(pModel);
        pModel->setMessageHandler(mpMessageHandler);
    }
    else
    {
        mpMessageHandler->addErrorMessage("No optimization algorithm selected.");
    }
}

int OptimizationHandler::getAlgorithm() const
{
    return (int)mAlgorithm;
}

GUIMessageHandler *OptimizationHandler::getMessageHandler()
{
    return mpMessageHandler;
}

