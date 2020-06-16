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
    double *mpM, *mpB, *mpK;
    int mSolverType;
    double mTolerance;
    double *mpXMin, *mpXMax;

    double *mpP1_f, *mpP1_x, *mpP1_v, *mpP1_c, *mpP1_Zx, *mpP1_me;
    double *mpP2_f, *mpP2_x, *mpP2_v, *mpP2_c, *mpP2_Zx, *mpP2_me;

    KinsolSolver *mpSolver;

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
        mpP1_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
        mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
        mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
        mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
        mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
        mpP1_me = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);

        mpP2_f = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
        mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
        mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
        mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
        mpP2_Zx = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);
        mpP2_me = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);

        mpSolver = new KinsolSolver(this, mTolerance, 2, KinsolSolver::SolverTypeEnum(mSolverType));

        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        double xmax = (*mpXMax);
        double xmin = (*mpXMin);
        double x1,v1,f1,c1,Zc1,x2,v2,f2,c2,Zc2;
        c1 = (*mpP1_c);
        Zc1 = (*mpP1_Zx);
        v2 = (*mpP2_v);
        x2 = (*mpP2_x);
        c2 = (*mpP2_c);
        Zc2 = (*mpP2_Zx);

        //Provide Kinsol with updated state variables
        mpSolver->setState(0,v2);
        mpSolver->setState(1,x2);

        //Solve algebraic equation system
        mpSolver->solve();

        //Obtain new state variables from Kinsol
        v2 = mpSolver->getState(0);
        x2 = mpSolver->getState(1);

        if(x2 > xmax) {
            x2 = xmax;
            if(v2 > 0) {
                v2 = 0;
            }
        }
        if(x2 < xmin) {
            x2 = xmin;
            if(v2 < 0) {
                v2 = 0;
            }
        }

        //Final algebraic expressions
        x1 = -x2;
        v1 = -v2;
        f1 = c1 + Zc1*v1;
        f2 = c2 + Zc2*v2;

        (*mpP1_x) = x1;
        (*mpP1_v) = v1;
        (*mpP1_f) = f1;
        (*mpP2_x) = x2;
        (*mpP2_v) = v2;
        (*mpP2_f) = f2;
    }

    void finalize()
    {
        delete mpSolver;
    }


    //! @brief Returns the residuals for speed and position
    //! @param [in] y Array of state variables from previous iteration
    //! @param [out] res Array of residuals or new state variables
    void getResiduals(double *y, double *res) {
        double v2 = y[0]; //y1 = v
        double x2 = y[1]; //y2 = x
        double oldV2 = (*mpP2_v);
        double oldX2 = (*mpP2_x);
        double M = (*mpM);
        double B = (*mpB);
        double K = (*mpK);
        double c1 = (*mpP1_c);
        double c2 = (*mpP2_c);
        double Zc1 = (*mpP1_Zx);
        double Zc2 = (*mpP2_Zx);

        // Compute new state variables
        double vnew = (M*oldV2/mTimestep - K*x2 + c1 - c2)/(M/mTimestep + B + Zc1 + Zc2);
        double xnew = oldX2 + mTimestep/2.0*(oldV2 + vnew);

        if(mSolverType == KinsolSolver::NewtonIteration) {
            //Newton iteration, return residuals (old variables minus new variables)
            res[0] = v2 - vnew;
            res[1] = x2 - xnew;
        }
        else if(mSolverType == KinsolSolver::FixedPointIteration) {
            //Fixed-point iteration, return new variables
            res[0] = vnew;
            res[1] = xnew;
        }
    }

    void getJacobian(double *y, double *f, double *J)
    {
        (void)y;
        (void)f;
        double M = (*mpM);
        double B = (*mpB);
        double K = (*mpK);
        double Zc1 = (*mpP1_Zx);
        double Zc2 = (*mpP2_Zx);

        J[0*2+0] = M/mTimestep/(M/mTimestep + B + Zc1 + Zc2);
        J[1*2+0] = -K/(M/mTimestep + B + Zc1 + Zc2);
        J[0*2+1] = mTimestep/2.0;
        J[1*2+1] = 1.0;
    }
};
}

#endif // MECHANICALSEPARATOR_HPP

