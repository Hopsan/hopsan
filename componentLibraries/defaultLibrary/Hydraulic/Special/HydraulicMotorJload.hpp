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

#ifndef HYDRAULICMOTORJLOAD_HPP_INCLUDED
#define HYDRAULICMOTORJLOAD_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicMotorJload.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Tue 10 Jun 2014 16:03:36
//! @brief This is a motor with an inertia load
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, ComponentLibraries, defaultLibrary, Hydraulic, \
Special}/HydraulicMotorJload.nb*/

using namespace hopsan;

class HydraulicMotorJload : public ComponentQ
{
private:
     double Dm;
     double Cim;
     double Bm;
     double Jm;
     Port *mpP1;
     Port *mpP2;
     Port *mpPm1;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     double delayParts6[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[6];
     int mNstep;
     //Port P1 variable
     double p1;
     double q1;
     double T1;
     double dE1;
     double c1;
     double Zc1;
     //Port P2 variable
     double p2;
     double q2;
     double T2;
     double dE2;
     double c2;
     double Zc2;
     //Port Pm1 variable
     double torm1;
     double thetam1;
     double wm1;
     double cm1;
     double Zcm1;
     double eqInertiam1;
//==This code has been autogenerated using Compgen==
     //inputVariables
     //outputVariables
     //Expressions variables
     //Port P1 pointer
     double *mpND_p1;
     double *mpND_q1;
     double *mpND_T1;
     double *mpND_dE1;
     double *mpND_c1;
     double *mpND_Zc1;
     //Port P2 pointer
     double *mpND_p2;
     double *mpND_q2;
     double *mpND_T2;
     double *mpND_dE2;
     double *mpND_c2;
     double *mpND_Zc2;
     //Port Pm1 pointer
     double *mpND_torm1;
     double *mpND_thetam1;
     double *mpND_wm1;
     double *mpND_cm1;
     double *mpND_Zcm1;
     double *mpND_eqInertiam1;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     //inputParameters pointers
     double *mpDm;
     double *mpCim;
     double *mpBm;
     double *mpJm;
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart12;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart30;
     EquationSystemSolver *mpSolver = nullptr;

public:
     static Component *Creator()
     {
        return new HydraulicMotorJload();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(6,6);
        systemEquations.create(6);
        delayedPart.create(7,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpP1=addPowerPort("P1","NodeHydraulic");
        mpP2=addPowerPort("P2","NodeHydraulic");
        mpPm1=addPowerPort("Pm1","NodeMechanicRotational");
        //Add inputVariables to the component

        //Add inputParammeters to the component
            addInputVariable("Dm", "Displacement", "m3", \
0.000049999999999999996,&mpDm);
            addInputVariable("Cim", "Leak coeff.", "m3/(s Pa)", 0.,&mpCim);
            addInputVariable("Bm", "Visc. friction coeff.", "N/m/s", \
0.,&mpBm);
            addInputVariable("Jm", "Moment of inertia", "kg", 0.1,&mpJm);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,6);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port P1
        mpND_p1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
        mpND_q1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
        mpND_T1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Temperature);
        mpND_dE1=getSafeNodeDataPtr(mpP1, NodeHydraulic::HeatFlow);
        mpND_c1=getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
        mpND_Zc1=getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
        //Port P2
        mpND_p2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
        mpND_q2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
        mpND_T2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Temperature);
        mpND_dE2=getSafeNodeDataPtr(mpP2, NodeHydraulic::HeatFlow);
        mpND_c2=getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
        mpND_Zc2=getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        //Port Pm1
        mpND_torm1=getSafeNodeDataPtr(mpPm1, NodeMechanicRotational::Torque);
        mpND_thetam1=getSafeNodeDataPtr(mpPm1, \
NodeMechanicRotational::Angle);
        mpND_wm1=getSafeNodeDataPtr(mpPm1, \
NodeMechanicRotational::AngularVelocity);
        mpND_cm1=getSafeNodeDataPtr(mpPm1, \
NodeMechanicRotational::WaveVariable);
        mpND_Zcm1=getSafeNodeDataPtr(mpPm1, \
NodeMechanicRotational::CharImpedance);
        mpND_eqInertiam1=getSafeNodeDataPtr(mpPm1, \
NodeMechanicRotational::EquivalentInertia);

        //Read variables from nodes
        //Port P1
        p1 = (*mpND_p1);
        q1 = (*mpND_q1);
        T1 = (*mpND_T1);
        dE1 = (*mpND_dE1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);
        //Port P2
        p2 = (*mpND_p2);
        q2 = (*mpND_q2);
        T2 = (*mpND_T2);
        dE2 = (*mpND_dE2);
        c2 = (*mpND_c2);
        Zc2 = (*mpND_Zc2);
        //Port Pm1
        torm1 = (*mpND_torm1);
        thetam1 = (*mpND_thetam1);
        wm1 = (*mpND_wm1);
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        eqInertiam1 = (*mpND_eqInertiam1);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        Dm = (*mpDm);
        Cim = (*mpCim);
        Bm = (*mpBm);
        Jm = (*mpJm);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (-0.31831*Dm*Power(mTimestep,2)*p1 + \
0.31831*Dm*Power(mTimestep,2)*p2 - 8.*Jm*thetam1 + \
2.*Power(mTimestep,2)*torm1)/(4.*Jm + 2.*Bm*mTimestep);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts1[2] = (-0.159155*Dm*Power(mTimestep,2)*p1 + \
0.159155*Dm*Power(mTimestep,2)*p2 + 4.*Jm*thetam1 - 2.*Bm*mTimestep*thetam1 + \
Power(mTimestep,2)*torm1)/(4.*Jm + 2.*Bm*mTimestep);
        mDelayedPart12.initialize(mNstep,delayParts1[2]);
        delayParts2[1] = (-0.159155*Dm*mTimestep*p1 + \
0.159155*Dm*mTimestep*p2 + mTimestep*torm1 - 2.*Jm*wm1 + \
Bm*mTimestep*wm1)/(2.*Jm + Bm*mTimestep);
        mDelayedPart21.initialize(mNstep,delayParts2[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(1);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(6);
        Vec stateVark(6);
        Vec deltaStateVar(6);

        //Read variables from nodes
        //Port P1
        T1 = (*mpND_T1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);
        //Port P2
        T2 = (*mpND_T2);
        c2 = (*mpND_c2);
        Zc2 = (*mpND_Zc2);
        //Port Pm1
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);

        //Read inputVariables from nodes

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = thetam1;
        stateVark[1] = wm1;
        stateVark[2] = q2;
        stateVark[3] = p1;
        stateVark[4] = p2;
        stateVark[5] = torm1;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //MotorJload
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =thetam1 + (Power(mTimestep,2)*(-0.159155*Dm*p1 \
+ 0.159155*Dm*p2 + torm1))/(4.*Jm + 2.*Bm*mTimestep) + delayedPart[1][1] + \
delayedPart[1][2];
          systemEquations[1] =(mTimestep*(-0.159155*Dm*p1 + 0.159155*Dm*p2 + \
torm1))/(2.*Jm + Bm*mTimestep) + wm1 + delayedPart[2][1];
          systemEquations[2] =Cim*(-1.*p1 + p2) + q2 - 0.159155*Dm*wm1;
          systemEquations[3] =p1 - lowLimit(c1 - q2*Zc1,0);
          systemEquations[4] =p2 - lowLimit(c2 + q2*Zc2,0);
          systemEquations[5] =-cm1 + torm1 - wm1*Zcm1;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = (-0.159155*Dm*Power(mTimestep,2))/(4.*Jm + \
2.*Bm*mTimestep);
          jacobianMatrix[0][4] = (0.159155*Dm*Power(mTimestep,2))/(4.*Jm + \
2.*Bm*mTimestep);
          jacobianMatrix[0][5] = Power(mTimestep,2)/(4.*Jm + \
2.*Bm*mTimestep);
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = (-0.159155*Dm*mTimestep)/(2.*Jm + \
Bm*mTimestep);
          jacobianMatrix[1][4] = (0.159155*Dm*mTimestep)/(2.*Jm + \
Bm*mTimestep);
          jacobianMatrix[1][5] = mTimestep/(2.*Jm + Bm*mTimestep);
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = -0.159155*Dm;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = -1.*Cim;
          jacobianMatrix[2][4] = Cim;
          jacobianMatrix[2][5] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = Zc1*dxLowLimit(c1 - q2*Zc1,0);
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = 0;
          jacobianMatrix[3][5] = 0;
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = 0;
          jacobianMatrix[4][2] = -(Zc2*dxLowLimit(c2 + q2*Zc2,0));
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[5][0] = 0;
          jacobianMatrix[5][1] = -Zcm1;
          jacobianMatrix[5][2] = 0;
          jacobianMatrix[5][3] = 0;
          jacobianMatrix[5][4] = 0;
          jacobianMatrix[5][5] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          thetam1=stateVark[0];
          wm1=stateVark[1];
          q2=stateVark[2];
          p1=stateVark[3];
          p2=stateVark[4];
          torm1=stateVark[5];
          //Expressions
          q1 = -q2;
        }

        //Calculate the delayed parts
        delayParts1[1] = (-0.31831*Dm*Power(mTimestep,2)*p1 + \
0.31831*Dm*Power(mTimestep,2)*p2 - 8.*Jm*thetam1 + \
2.*Power(mTimestep,2)*torm1)/(4.*Jm + 2.*Bm*mTimestep);
        delayParts1[2] = (-0.159155*Dm*Power(mTimestep,2)*p1 + \
0.159155*Dm*Power(mTimestep,2)*p2 + 4.*Jm*thetam1 - 2.*Bm*mTimestep*thetam1 + \
Power(mTimestep,2)*torm1)/(4.*Jm + 2.*Bm*mTimestep);
        delayParts2[1] = (-0.159155*Dm*mTimestep*p1 + \
0.159155*Dm*mTimestep*p2 + mTimestep*torm1 - 2.*Jm*wm1 + \
Bm*mTimestep*wm1)/(2.*Jm + Bm*mTimestep);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(0);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];

        //Write new values to nodes
        //Port P1
        (*mpND_p1)=p1;
        (*mpND_q1)=q1;
        (*mpND_dE1)=dE1;
        //Port P2
        (*mpND_p2)=p2;
        (*mpND_q2)=q2;
        (*mpND_dE2)=dE2;
        //Port Pm1
        (*mpND_torm1)=torm1;
        (*mpND_thetam1)=thetam1;
        (*mpND_wm1)=wm1;
        (*mpND_eqInertiam1)=eqInertiam1;
        //outputVariables

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart12.update(delayParts1[2]);
        mDelayedPart21.update(delayParts2[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // HYDRAULICMOTORJLOAD_HPP_INCLUDED
