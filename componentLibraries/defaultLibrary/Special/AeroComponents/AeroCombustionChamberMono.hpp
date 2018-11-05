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

#ifndef AEROCOMBUSTIONCHAMBERMONO_HPP_INCLUDED
#define AEROCOMBUSTIONCHAMBERMONO_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file AeroCombustionChamberMono.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Fri 28 Nov 2014 12:54:04
//! @brief Hydraulic volume with two connection
//! @ingroup AeroComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Special, \
AeroComponents}/AeroCombustionChamberMono.nb*/

using namespace hopsan;

class AeroCombustionChamberMono : public ComponentC
{
private:
     double Vc;
     double R;
     double cv;
     double vfuel;
     double ethap;
     double rhofuel;
     double As;
     double Med;
     double alpha;
     Port *mpP1;
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
     int order[3];
     int mNstep;
     //Port P1 variable
     double p1;
     double q1;
     double T1;
     double dE1;
     double c1;
     double Zc1;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double pa;
     //outputVariables
     double thrust;
     double Tc;
     double rhogas;
     double mdot;
     double Ae;
     double pe;
     double pc;
     double Te;
     double ve;
     double Pin;
     double Pout;
     //InitialExpressions variables
     double c2;
     double c1f;
     double c2f;
     //LocalExpressions variables
     double gam;
     double c10;
     double c20;
     //Expressions variables
     //Port P1 pointer
     double *mpND_p1;
     double *mpND_q1;
     double *mpND_T1;
     double *mpND_dE1;
     double *mpND_c1;
     double *mpND_Zc1;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mppa;
     //inputParameters pointers
     double *mpVc;
     double *mpR;
     double *mpcv;
     double *mpvfuel;
     double *mpethap;
     double *mprhofuel;
     double *mpAs;
     double *mpMed;
     double *mpalpha;
     //outputVariables pointers
     double *mpthrust;
     double *mpTc;
     double *mprhogas;
     double *mpmdot;
     double *mpAe;
     double *mppe;
     double *mppc;
     double *mpTe;
     double *mpve;
     double *mpPin;
     double *mpPout;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new AeroCombustionChamberMono();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(3,3);
        systemEquations.create(3);
        delayedPart.create(4,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpP1=addPowerPort("P1","NodeHydraulic");
        //Add inputVariables to the component
            addInputVariable("pa","Free stream pressure"," ",100000.,&mppa);

        //Add inputParammeters to the component
            addInputVariable("Vc", "Chamber volume (for numerics)", " ", \
0.02,&mpVc);
            addInputVariable("R", "Gas constant", "", 396,&mpR);
            addInputVariable("cv", "Heat capacity", "", 1800,&mpcv);
            addInputVariable("vfuel", "Exhaust speed", "m/s", \
1571.,&mpvfuel);
            addInputVariable("ethap", "Efficiency factor (<1)", "", \
0.9,&mpethap);
            addInputVariable("rhofuel", "Exhaust speed", "kg/m3", \
1200.,&mprhofuel);
            addInputVariable("As", "min effective area", "m2", \
0.00196,&mpAs);
            addInputVariable("Med", "Design exit Mach", "", 2.5,&mpMed);
            addInputVariable("alpha", "Damp. factor", "Hz", 0.3,&mpalpha);
        //Add outputVariables to the component
            addOutputVariable("thrust","thrust","m3/s",0.,&mpthrust);
            addOutputVariable("Tc","cahmber temerature","K",273.,&mpTc);
            addOutputVariable("rhogas","density in \
chamber","kg/m3",0.,&mprhogas);
            addOutputVariable("mdot","Exit mass flow","kg/s",0.,&mpmdot);
            addOutputVariable("Ae","exit Area","m2",0.,&mpAe);
            addOutputVariable("pe","exit pressure","Pa",0.,&mppe);
            addOutputVariable("pc","chamber pressure","Pa",0.,&mppc);
            addOutputVariable("Te","exit temperature","K",273.,&mpTe);
            addOutputVariable("ve","exit velocity","K",0.,&mpve);
            addOutputVariable("Pin","Input power","W",0.,&mpPin);
            addOutputVariable("Pout","Output power","W",0.,&mpPout);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,3);
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

        //Read variables from nodes
        //Port P1
        p1 = (*mpND_p1);
        q1 = (*mpND_q1);
        T1 = (*mpND_T1);
        dE1 = (*mpND_dE1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);

        //Read inputVariables from nodes
        pa = (*mppa);

        //Read inputParameters from nodes
        Vc = (*mpVc);
        R = (*mpR);
        cv = (*mpcv);
        vfuel = (*mpvfuel);
        ethap = (*mpethap);
        rhofuel = (*mprhofuel);
        As = (*mpAs);
        Med = (*mpMed);
        alpha = (*mpalpha);

        //Read outputVariables from nodes
        thrust = (*mpthrust);
        Tc = (*mpTc);
        rhogas = (*mprhogas);
        mdot = (*mpmdot);
        Ae = (*mpAe);
        pe = (*mppe);
        pc = (*mppc);
        Te = (*mpTe);
        ve = (*mpve);
        Pin = (*mpPin);
        Pout = (*mpPout);

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        c1 = p1;
        c2 = p1;
        pc = p1;
        c1f = p1;
        c2f = p1;
        Tc = T1;
        rhogas = pc/(R*(1 + Tc));

        //LocalExpressions
        gam = (cv + R)/cv;
        c10 = c2 - (2*mdot*mTimestep*(cv + R)*Tc)/(gam*Vc);
        c20 = c1 + (ethap*mTimestep*q1*rhofuel*Power(vfuel,2))/(gam*Vc);

        //Initialize delays
        delayParts1[1] = (mdot*mTimestep - mTimestep*q1*rhofuel - \
2*rhogas*Vc)/(2.*Vc);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(3);
        Vec stateVark(3);
        Vec deltaStateVar(3);

        //Read variables from nodes
        //Port P1
        p1 = (*mpND_p1);
        q1 = (*mpND_q1);
        dE1 = (*mpND_dE1);

        //Read inputVariables from nodes
        pa = (*mppa);

        //LocalExpressions
        gam = (cv + R)/cv;
        c10 = c2 - (2*mdot*mTimestep*(cv + R)*Tc)/(gam*Vc);
        c20 = c1 + (ethap*mTimestep*q1*rhofuel*Power(vfuel,2))/(gam*Vc);

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = rhogas;
        stateVark[1] = Tc;
        stateVark[2] = mdot;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //CombustionChamberMono
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =rhogas + (mTimestep*(mdot - \
q1*rhofuel))/(2.*Vc) + delayedPart[1][1];
          systemEquations[1] =-(pc/(R*rhogas)) + Tc;
          systemEquations[2] =mdot - limit(Power(2,(1 + gam)/(2.*(-1 + \
gam)))*As*Power(1 + gam,(1 + gam)/(2 - 2*gam))*pc*Sqrt(gam/(R + \
R*Abs(Tc))),0.,1.e9);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = mTimestep/(2.*Vc);
          jacobianMatrix[1][0] = pc/(R*Power(rhogas,2));
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = (Power(2,-1 + (1 + gam)/(2.*(-1 + \
gam)))*As*gam*Power(1 + gam,(1 + gam)/(2 - \
2*gam))*pc*R*dxAbs(Tc)*dxLimit(Power(2,(1 + gam)/(2.*(-1 + gam)))*As*Power(1 \
+ gam,(1 + gam)/(2 - 2*gam))*pc*Sqrt(gam/(R + \
R*Abs(Tc))),0.,1.e9))/(Sqrt(gam/(R + R*Abs(Tc)))*Power(R + R*Abs(Tc),2));
          jacobianMatrix[2][2] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          rhogas=stateVark[0];
          Tc=stateVark[1];
          mdot=stateVark[2];
          //Expressions
          c1 = alpha*c1 + (1 - alpha)*c10;
          c2 = alpha*c2 + (1 - alpha)*c20;
          Zc1 = (ethap*mTimestep*rhofuel*Power(vfuel,2))/(2.*gam*Vc);
          pc = c2 - (mdot*mTimestep*(cv + R)*Tc)/(gam*Vc);
          Ae = (Power(2,(1 + gam)/(2.*(-1 + gam)))*As*Power(1 + ((-1 + \
gam)*Power(Med,2))/2.,(1 + gam)/(2.*(-1 + gam))))/(Power(1 + gam,(1 + \
gam)/(2.*(-1 + gam)))*Med);
          pe = pc/Power(1 + ((-1 + gam)*Power(Med,2))/2.,gam/(-1 + gam));
          Te = Tc/(1 + ((-1 + gam)*Power(Med,2))/2.);
          ve = Med*Sqrt(gam*R*Te);
          thrust = lowLimit(Ae*(-pa + pe) + mdot*ve,0);
          Pin = (ethap*q1*rhofuel*Power(vfuel,2))/2.;
          Pout = (mdot*Power(ve,2))/2.;
        }

        //Calculate the delayed parts
        delayParts1[1] = (mdot*mTimestep - mTimestep*q1*rhofuel - \
2*rhogas*Vc)/(2.*Vc);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];

        //Write new values to nodes
        //Port P1
        (*mpND_T1)=T1;
        (*mpND_c1)=c1;
        (*mpND_Zc1)=Zc1;
        //outputVariables
        (*mpthrust)=thrust;
        (*mpTc)=Tc;
        (*mprhogas)=rhogas;
        (*mpmdot)=mdot;
        (*mpAe)=Ae;
        (*mppe)=pe;
        (*mppc)=pc;
        (*mpTe)=Te;
        (*mpve)=ve;
        (*mpPin)=Pin;
        (*mpPout)=Pout;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // AEROCOMBUSTIONCHAMBERMONO_HPP_INCLUDED
