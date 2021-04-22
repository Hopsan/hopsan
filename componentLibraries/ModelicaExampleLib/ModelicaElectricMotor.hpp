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

#ifndef MODELICAELECTRICMOTOR_HPP_INCLUDED
#define MODELICAELECTRICMOTOR_HPP_INCLUDED

#include <math.h>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <sstream>
#include <cstring>
#include <vector>
#include <string>

using namespace std;

namespace hopsan {

    class ModelicaElectricMotor : public ComponentQ
    {
    private:                         // Private section
        //Declare local variables
        double mTolerance;
        KinsolSolver* mpSolver;
        Delay mDelay0;
        double Ke;
        double Kt;
        double L;
        double R;
        double U5;
        double I5;
        double c5;
        double Zc5;
        double U6;
        double I6;
        double c6;
        double Zc6;
        double w7;
        double T7;
        double a7;
        double c7;
        double Zc7;

        //Declare data pointer variables
        double *mpKe, *mpKt, *mpL, *mpR, *mpP1_U, *mpP1_I, *mpP1_c, *mpP1_Zc, *mpP2_U, *mpP2_I, *mpP2_c, *mpP2_Zc, *mpP3_w, *mpP3_T, *mpP3_a, *mpP3_c, *mpP3_Zc;

        //Declare ports
                Port *mpP1, *mpP2, *mpP3;

    public:                              //Public section
        static Component *Creator()
        {
            return new ModelicaElectricMotor();
        }
        
        //Configure
        void configure()
        {
            //Register constants
            addConstant("tolerance", "Solver tolerance", "", 1e-5, mTolerance);

            //Add ports
            addInputVariable("Ke", "", "", 0.05, &mpKe);
            addInputVariable("Kt", "", "", 0.05, &mpKt);
            addInputVariable("L", "", "", 1, &mpL);
            addInputVariable("R", "", "", 1, &mpR);
            mpP1 = addPowerPort("P1", "NodeElectric");
            mpP2 = addPowerPort("P2", "NodeElectric");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");
            

            //Configuration code
            
        }
        
        //Initialize
        void initialize()
        {
            //Initialize variables
            

            //Get data pointers
            mpP1_U = getSafeNodeDataPtr(mpP1, NodeElectric::Voltage);
            mpP1_I = getSafeNodeDataPtr(mpP1, NodeElectric::Current);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeElectric::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeElectric::CharImpedance);
            mpP2_U = getSafeNodeDataPtr(mpP2, NodeElectric::Voltage);
            mpP2_I = getSafeNodeDataPtr(mpP2, NodeElectric::Current);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeElectric::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeElectric::CharImpedance);
            mpP3_w = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::AngularVelocity);
            mpP3_T = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Torque);
            mpP3_a = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Angle);
            mpP3_c = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::WaveVariable);
            mpP3_Zc = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::CharImpedance);

            //Read input variables
            Ke = (*mpKe);
            Kt = (*mpKt);
            L = (*mpL);
            R = (*mpR);
            U5 = (*mpP1_U);
            I5 = (*mpP1_I);
            c5 = (*mpP1_c);
            Zc5 = (*mpP1_Zc);
            U6 = (*mpP2_U);
            I6 = (*mpP2_I);
            c6 = (*mpP2_c);
            Zc6 = (*mpP2_Zc);
            w7 = (*mpP3_w);
            T7 = (*mpP3_T);
            a7 = (*mpP3_a);
            c7 = (*mpP3_c);
            Zc7 = (*mpP3_Zc);

            //Initialization code
            mpSolver = new KinsolSolver(this, mTolerance, 6, KinsolSolver::NewtonIteration);
            mDelay0.initialize(1, I6*L*2.0+mTimestep*U5-mTimestep*U6-mTimestep*R*I6-mTimestep*Ke*w7);
            //Initial algorithm section

            //Write output variables
            (*mpP1_U) = U5;
            (*mpP1_I) = I5;
            (*mpP2_U) = U6;
            (*mpP2_I) = I6;
            (*mpP3_w) = w7;
            (*mpP3_T) = T7;
            (*mpP3_a) = a7;
        }

        //Simulate one time step
        void simulateOneTimestep()
        {
            //Read input variables
            Ke = (*mpKe);
            Kt = (*mpKt);
            L = (*mpL);
            R = (*mpR);
            U5 = (*mpP1_U);
            I5 = (*mpP1_I);
            c5 = (*mpP1_c);
            Zc5 = (*mpP1_Zc);
            U6 = (*mpP2_U);
            I6 = (*mpP2_I);
            c6 = (*mpP2_c);
            Zc6 = (*mpP2_Zc);
            w7 = (*mpP3_w);
            T7 = (*mpP3_T);
            a7 = (*mpP3_a);
            c7 = (*mpP3_c);
            Zc7 = (*mpP3_Zc);

            //Simulation code
            //Pre-algorithm section
            
            //Provide Kinsol with updated state variables
            mpSolver->setState(0,U5);
            mpSolver->setState(1,U6);
            mpSolver->setState(2,I6);
            mpSolver->setState(3,w7);
            mpSolver->setState(4,I5);
            mpSolver->setState(5,T7);
            
            //Solve algebraic equation system
            mpSolver->solve();
            
            //Obtain new state variables from Kinsol
            U5 = mpSolver->getState(0);
            U6 = mpSolver->getState(1);
            I6 = mpSolver->getState(2);
            w7 = mpSolver->getState(3);
            I5 = mpSolver->getState(4);
            T7 = mpSolver->getState(5);
            
            //Final algorithm section
            
            bool reachedLimit = false;
            if(reachedLimit) {
                mDelay0.initialize(1, I6*L*2.0+mTimestep*U5-mTimestep*U6-mTimestep*R*I6-mTimestep*Ke*w7);
            }
            
            mDelay0.update(I6*L*2.0+mTimestep*U5-mTimestep*U6-mTimestep*R*I6-mTimestep*Ke*w7);

            //Write output variables
            (*mpP1_U) = U5;
            (*mpP1_I) = I5;
            (*mpP2_U) = U6;
            (*mpP2_I) = I6;
            (*mpP3_w) = w7;
            (*mpP3_T) = T7;
            (*mpP3_a) = a7;
        }

        //Finalize
        void finalize()
        {
            //Finalize code
            
        }

        //Finalize
        void deconfigure()
        {
            //Deconfigure code
            
        }

        //Auxiliary functions
                //! @brief Returns the residuals for speed and position
                //! @param [in] y Array of state variables from previous iteration
                //! @param [out] res Array of residuals or new state variables
                void getResiduals(double *y, double *res)
                {
                    double U5 = y[0];
                    double U6 = y[1];
                    double I6 = y[2];
                    double w7 = y[3];
                    double I5 = y[4];
                    double T7 = y[5];
                    
                    res[0] = -mTimestep*Ke*w7-I6*L*2.0-mTimestep*R*I6-mTimestep*U6+mDelay0.getOldest()+U5*mTimestep;
                    res[1] = I5+I6;
                    res[2] = T7-Kt*I6;
                    res[3] = -Zc5*I5+U5-c5;
                    res[4] = -Zc6*I6+U6-c6;
                    res[5] = -Zc7*w7+T7-c7;
                }
                
                //! @brief Returns the residuals for speed and position
                //! @param [in] y Array of state variables from previous iteration
                //! @param [in] f Array of function values (f(y))
                //! @param [out] J Array of Jacobian elements, stored column-wise
                void getJacobian(double *y, double *f, double *J)
                {
                    double U5 = y[0];
                    double U6 = y[1];
                    double I6 = y[2];
                    double w7 = y[3];
                    double I5 = y[4];
                    double T7 = y[5];
                    
                    J[0*6+0] = mTimestep;
                    J[1*6+0] = -mTimestep;
                    J[2*6+0] = -L*2.0-mTimestep*R;
                    J[3*6+0] = -mTimestep*Ke;
                    J[2*6+1] = 1.0;
                    J[4*6+1] = 1.0;
                    J[2*6+2] = -Kt;
                    J[5*6+2] = 1.0;
                    J[0*6+3] = 1.0;
                    J[4*6+3] = -Zc5;
                    J[1*6+4] = 1.0;
                    J[2*6+4] = -Zc6;
                    J[3*6+5] = -Zc7;
                    J[5*6+5] = 1.0;
                }
    };
}

#endif // MODELICAELECTRICMOTOR_HPP_INCLUDED
