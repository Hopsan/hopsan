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

#ifndef ELECTRICICONTROLLER_HPP_INCLUDED
#define ELECTRICICONTROLLER_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file ElectricIcontroller.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Mon 7 Apr 2014 13:07:01
//! @brief This is an imaginary analog component that represents a PWM current controller.
//! @ingroup ElectricComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, ComponentLibraries, defaultLibrary, \
Electric}/ElectricIcontroller.nb*/

using namespace hopsan;

class ElectricIcontroller : public ComponentQ
{
private:
     double resist;
     double wf;
     double umin;
     double imax;
     double imin;
     double umax;
     Port *mpPel1;
     Port *mpPel2;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[4];
     int mNstep;
     //Port Pel1 variable
     double uel1;
     double iel1;
     double cel1;
     double Zcel1;
     //Port Pel2 variable
     double uel2;
     double iel2;
     double cel2;
     double Zcel2;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double iref;
     //outputVariables
     //Port Pel1 pointer
     double *mpND_uel1;
     double *mpND_iel1;
     double *mpND_cel1;
     double *mpND_Zcel1;
     //Port Pel2 pointer
     double *mpND_uel2;
     double *mpND_iel2;
     double *mpND_cel2;
     double *mpND_Zcel2;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpiref;
     //inputParameters pointers
     double *mpresist;
     double *mpwf;
     double *mpumin;
     double *mpimax;
     double *mpimin;
     double *mpumax;
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     EquationSystemSolver *mpSolver = nullptr;

public:
     static Component *Creator()
     {
        return new ElectricIcontroller();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(4,4);
        systemEquations.create(4);
        delayedPart.create(5,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPel1=addPowerPort("Pel1","NodeElectric");
        mpPel2=addPowerPort("Pel2","NodeElectric");
        //Add inputVariables to the component
            addInputVariable("iref","Conductivity \
(1/resistance)","A/V",0.,&mpiref);

        //Add inputParammeters to the component
            addInputVariable("resist", "loss resistans (at 1)", "Resistance", \
0.01,&mpresist);
            addInputVariable("wf", "controller break frequency", "rad/s", \
10.,&mpwf);
            addInputVariable("umin", "minimum voltage difference", "V", \
1,&mpumin);
            addInputVariable("imax", "i max limit", "A", 10000.,&mpimax);
            addInputVariable("imin", "i min limit", "A", 0.01,&mpimin);
            addInputVariable("umax", "u max limit", "V", 1000.,&mpumax);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,4);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pel1
        mpND_uel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Voltage);
        mpND_iel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Current);
        mpND_cel1=getSafeNodeDataPtr(mpPel1, NodeElectric::WaveVariable);
        mpND_Zcel1=getSafeNodeDataPtr(mpPel1, NodeElectric::CharImpedance);
        //Port Pel2
        mpND_uel2=getSafeNodeDataPtr(mpPel2, NodeElectric::Voltage);
        mpND_iel2=getSafeNodeDataPtr(mpPel2, NodeElectric::Current);
        mpND_cel2=getSafeNodeDataPtr(mpPel2, NodeElectric::WaveVariable);
        mpND_Zcel2=getSafeNodeDataPtr(mpPel2, NodeElectric::CharImpedance);

        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpND_uel1);
        iel1 = (*mpND_iel1);
        cel1 = (*mpND_cel1);
        Zcel1 = (*mpND_Zcel1);
        //Port Pel2
        uel2 = (*mpND_uel2);
        iel2 = (*mpND_iel2);
        cel2 = (*mpND_cel2);
        Zcel2 = (*mpND_Zcel2);

        //Read inputVariables from nodes
        iref = (*mpiref);

        //Read inputParameters from nodes
        resist = (*mpresist);
        wf = (*mpwf);
        umin = (*mpumin);
        imax = (*mpimax);
        imin = (*mpimin);
        umax = (*mpumax);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (iref*mTimestep*uel1*wf - iref*mTimestep*uel2*wf - \
2*iel1*limit(uel1 - uel2,umin,umax) + iel1*mTimestep*wf*limit(uel1 - \
uel2,umin,umax))/(2*limit(uel1 - uel2,umin,umax) + mTimestep*wf*limit(uel1 - \
uel2,umin,umax));
        mDelayedPart11.initialize(mNstep,delayParts1[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(4);
        Vec stateVark(4);
        Vec deltaStateVar(4);

        //Read variables from nodes
        //Port Pel1
        cel1 = (*mpND_cel1);
        Zcel1 = (*mpND_Zcel1);
        //Port Pel2
        cel2 = (*mpND_cel2);
        Zcel2 = (*mpND_Zcel2);

        //Read inputVariables from nodes
        iref = (*mpiref);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = iel1;
        stateVark[1] = iel2;
        stateVark[2] = uel1;
        stateVark[3] = uel2;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //Icontroller
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =iel1 + (iref*mTimestep*(uel1 - uel2)*wf)/((2 + \
mTimestep*wf)*limit(uel1 - uel2,umin,umax)) + delayedPart[1][1];
          systemEquations[1] =iel2 + (iel1*(iel1*resist + \
limit(uel1,umin,umax)))/limit(uel2,umin,umax);
          systemEquations[2] =-cel1 + uel1 - iel1*Zcel1;
          systemEquations[3] =-cel2 + uel2 - iel2*Zcel2;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = -((iref*mTimestep*(uel1 - \
uel2)*wf*dxLimit(uel1 - uel2,umin,umax))/((2 + mTimestep*wf)*Power(limit(uel1 \
- uel2,umin,umax),2))) + (iref*mTimestep*wf)/((2 + mTimestep*wf)*limit(uel1 - \
uel2,umin,umax));
          jacobianMatrix[0][3] = (iref*mTimestep*(uel1 - \
uel2)*wf*dxLimit(uel1 - uel2,umin,umax))/((2 + mTimestep*wf)*Power(limit(uel1 \
- uel2,umin,umax),2)) - (iref*mTimestep*wf)/((2 + mTimestep*wf)*limit(uel1 - \
uel2,umin,umax));
          jacobianMatrix[1][0] = (iel1*resist)/limit(uel2,umin,umax) + \
(iel1*resist + limit(uel1,umin,umax))/limit(uel2,umin,umax);
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = \
(iel1*dxLimit(uel1,umin,umax))/limit(uel2,umin,umax);
          jacobianMatrix[1][3] = -((iel1*dxLimit(uel2,umin,umax)*(iel1*resist \
+ limit(uel1,umin,umax)))/Power(limit(uel2,umin,umax),2));
          jacobianMatrix[2][0] = -Zcel1;
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = -Zcel2;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          iel1=stateVark[0];
          iel2=stateVark[1];
          uel1=stateVark[2];
          uel2=stateVark[3];
        }

        //Calculate the delayed parts
        delayParts1[1] = (iref*mTimestep*uel1*wf - iref*mTimestep*uel2*wf - \
2*iel1*limit(uel1 - uel2,umin,umax) + iel1*mTimestep*wf*limit(uel1 - \
uel2,umin,umax))/(2*limit(uel1 - uel2,umin,umax) + mTimestep*wf*limit(uel1 - \
uel2,umin,umax));

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        //Write new values to nodes
        //Port Pel1
        (*mpND_uel1)=uel1;
        (*mpND_iel1)=iel1;
        //Port Pel2
        (*mpND_uel2)=uel2;
        (*mpND_iel2)=iel2;
        //outputVariables

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // ELECTRICICONTROLLER_HPP_INCLUDED
