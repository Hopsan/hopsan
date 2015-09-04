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
//! @file   OptimizationWorkerSimplex.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2015-05-25
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Simplex algorithm
//!


//Hopsan includes
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "HcomHandler.h"
#include "OptimizationHandler.h"
#include "OptimizationWorkerSimplex.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"

//C++ includes
#include <math.h>

OptimizationWorkerNelderMead::OptimizationWorkerNelderMead(OptimizationHandler *pHandler)
    : Ops::WorkerNelderMead(0)
{
    mpHandler = pHandler;
}

//! @brief Initializes a Complex-RF optimization
void OptimizationWorkerNelderMead::initialize(const ModelWidget *pModel, const QString &modelPath)
{
    Ops::WorkerNelderMead::initialize();
}

void OptimizationWorkerNelderMead::evaluateCandidate(int idx)
{
    mpHandler->evaluateCandidate(idx);
}
