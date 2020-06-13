/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   LinearInertia.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2020-05-27
//!
//! @brief Contains a linear inertia component using the Kinsol solver
//!
//$Id$

#ifndef LINEARINERTIA_HPP
#define LINEARINERTIA_HPP

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "ComponentUtilities/EquationSystemSolver.h"
#include <vector>
#include <iostream>

namespace hopsan {

class LinearInertia : public ComponentQ
{

private:
    Port *mpP1;
    Port *mpP2;
    MechanicNodeDataPointerStructT mP1, mP2;
    double *mpM, *mpB, *mpK;
    KinsolSolver *mpSolver;
    int mSolverType;
    double mTolerance;
    double *mpXMin, *mpXMax;

public:
    static Component *Creator()
    {
        return new LinearInertia();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeMechanic");
        mpP2 = addPowerPort("P2", "NodeMechanic");

        std::vector<HString> solverTypes;
        solverTypes.push_back("Newton Iteration");
        solverTypes.push_back("Fixed Point Iteration");
        addConditionalConstant("solverType", "Solver type", solverTypes, KinsolSolver::NewtonIteration, mSolverType);

        addConstant("tolerance", "Solver tolerance", "-", 1e-5, mTolerance);
        addInputVariable("M", "Inertia", "kg", 100, &mpM);
        addInputVariable("B", "Viscous Friction", "Ns/m", 10, &mpB);
        addInputVariable("K", "Spring Constant", "N/m", 1, &mpK);
        addInputVariable("x_min", "Minimum Position of Port P2", "m", -1.0e+300, &mpXMin);
        addInputVariable("x_max", "Maximum Position of Port P2", "m", 1.0e+300, &mpXMax);
    }


    void initialize()
    {
        mpSolver = new KinsolSolver(this, mTolerance, 2, KinsolSolver::SolverTypeEnum(mSolverType));

        //Assign node data pointers
        getMechanicPortNodeDataPointers(mpP1, mP1);
        getMechanicPortNodeDataPointers(mpP2, mP2);

        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        //Provide Kinsol with updated state variables
        mpSolver->setState(0,mP2.v());
        mpSolver->setState(1,mP2.x());

        //Solve algebraic equation system
        mpSolver->solve();

        //Obtain new state variables from Kinsol
        mP2.rv() = mpSolver->getState(0);
        mP2.rx() = mpSolver->getState(1);

        //Final algebraic expressions
        mP1.rx() = -mP2.x();
        mP1.rv() = -mP2.v();
        mP1.rf() = mP1.c() + mP1.Zc()*mP1.v();
        mP2.rf() = mP2.c() + mP2.Zc()*mP2.v();
    }


    //! @brief Returns the residuals for speed and position
    //! @param [in] y Array of state variables from previous iteration
    //! @param [out] res Array of residuals or new state variables
    void getResiduals(double *y, double *res) {
        double v = y[0]; //y1 = v
        double x = y[1]; //y2 = x
        double oldV = mP2.v();
        double oldX = mP2.x();
        double M = (*mpM);
        double B = (*mpB);
        double K = (*mpK);
        double xmin = (*mpXMin);
        double xmax = (*mpXMax);

        // Compute new state variables
        double vnew = (M*oldV/mTimestep - K*x + mP1.c() - mP2.c())/((M/mTimestep + B + mP1.Zc() + mP2.Zc()));
        double xnew = oldX + mTimestep/2.0*(oldV + vnew);

        // Apply variable limitations
        vnew = dxLimit2(limit(xnew, xmin,xmax),vnew,xmin,xmax)*vnew;
        xnew = limit(xnew,xmin,xmax);

        if(mSolverType == KinsolSolver::NewtonIteration) {
            //Newton iteration, return residuals (old variables minus new variables)
            res[0] = v - dxLimit2(limit(xnew,xmin,xmax),vnew,xmin,xmax) * vnew;
            res[1] = x - limit(xnew,xmin,xmax);
        }
        else if(mSolverType == KinsolSolver::FixedPointIteration) {
            //Fixed-point iteration, return new variables
            res[0] = dxLimit2(limit(xnew, xmin,xmax),vnew,xmin,xmax)*vnew;
            res[1] = limit(xnew,xmin,xmax);
        }
    }
};
}

#endif // MECHANICALSEPARATOR_HPP

