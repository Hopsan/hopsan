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

#ifndef HYDRAULICPRESSURECONTROLLEDPUMPG_HPP_INCLUDED
#define HYDRAULICPRESSURECONTROLLEDPUMPG_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicPressureControlledPumpG.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Mon 7 Sep 2015 14:45:16
//! @brief Pressure controlled pump
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Hydraulic, \
Special}/HydraulicPressureControlledPumpG.nb*/

using namespace hopsan;

class HydraulicPressureControlledPumpG : public ComponentQ
{
private:
     double rho;
     double qmin;
     double Dp;
     double epsmin;
     double Lp;
     double Rp;
     double wp1;
     double Kcp;
     double tauv;
     double Tp;
     double Tm;
     double pnom;
     double speednom;
     Port *mpP1;
     Port *mpP2;
     Port *mpP3;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[5];
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
     //Port P3 variable
     double p3;
     double q3;
     double T3;
     double dE3;
     double c3;
     double Zc3;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double pdif;
     double speed;
     //outputVariables
     double qp;
     double dqp;
     double eps;
     //LocalExpressions variables
     double qmine;
     double qmaxe;
     double dqmin;
     double dqmax;
     double Lpe;
     double Rpe;
     double wp1e;
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
     //Port P3 pointer
     double *mpND_p3;
     double *mpND_q3;
     double *mpND_T3;
     double *mpND_dE3;
     double *mpND_c3;
     double *mpND_Zc3;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mppdif;
     double *mpspeed;
     //inputParameters pointers
     double *mprho;
     double *mpqmin;
     double *mpDp;
     double *mpepsmin;
     double *mpLp;
     double *mpRp;
     double *mpwp1;
     double *mpKcp;
     double *mptauv;
     double *mpTp;
     double *mpTm;
     double *mppnom;
     double *mpspeednom;
     //outputVariables pointers
     double *mpqp;
     double *mpdqp;
     double *mpeps;
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
        return new HydraulicPressureControlledPumpG();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(5,5);
        systemEquations.create(5);
        delayedPart.create(6,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpP1=addPowerPort("P1","NodeHydraulic");
        mpP2=addPowerPort("P2","NodeHydraulic");
        mpP3=addPowerPort("P3","NodeHydraulic");
        //Add inputVariables to the component
            addInputVariable("pdif","Reference pressure","Pa",2.e7,&mppdif);
            addInputVariable("speed","Pump speed","rad/sec",157,&mpspeed);

        //Add inputParammeters to the component
            addInputVariable("rho", "Oil density", "kg/m3", 870, &mprho);
            addInputVariable("qmin", "Min flow at nom speed", "m3/s", \
0.,&mpqmin);
            addInputVariable("Dp", "Max pump displacement", "m3", \
0.00030000000000000003,&mpDp);
            addInputVariable("epsmin", "Relative min pump displacement", "", \
0.,&mpepsmin);
            addInputVariable("Lp", "Pump inductance", "", 1.83e10,&mpLp);
            addInputVariable("Rp", "Pump resistance", "", 1.68e10,&mpRp);
            addInputVariable("wp1", "Reg break freq", "rad/sec", \
13.3,&mpwp1);
            addInputVariable("Kcp", "Pump speed", "", 1.e-12,&mpKcp);
            addInputVariable("tauv", "Time c of control valve", "s", \
0.05,&mptauv);
            addInputVariable("Tp", "Time min-full disp", "s", 0.001,&mpTp);
            addInputVariable("Tm", "Time full-min disp", "s", 0.001,&mpTm);
            addInputVariable("pnom", "Nominal pressure", "pa", 7.e6,&mppnom);
            addInputVariable("speednom", "Nominal speed", "rad/s", \
157,&mpspeednom);
        //Add outputVariables to the component
            addOutputVariable("qp","Pump flow","m^3/s",0.,&mpqp);
            addOutputVariable("dqp","Pump flow rate of \
change","m^3/s^2",0.,&mpdqp);
            addOutputVariable("eps","Relative pump stroke","",0.,&mpeps);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,5);
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
        //Port P3
        mpND_p3=getSafeNodeDataPtr(mpP3, NodeHydraulic::Pressure);
        mpND_q3=getSafeNodeDataPtr(mpP3, NodeHydraulic::Flow);
        mpND_T3=getSafeNodeDataPtr(mpP3, NodeHydraulic::Temperature);
        mpND_dE3=getSafeNodeDataPtr(mpP3, NodeHydraulic::HeatFlow);
        mpND_c3=getSafeNodeDataPtr(mpP3, NodeHydraulic::WaveVariable);
        mpND_Zc3=getSafeNodeDataPtr(mpP3, NodeHydraulic::CharImpedance);

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
        //Port P3
        p3 = (*mpND_p3);
        q3 = (*mpND_q3);
        T3 = (*mpND_T3);
        dE3 = (*mpND_dE3);
        c3 = (*mpND_c3);
        Zc3 = (*mpND_Zc3);

        //Read inputVariables from nodes
        pdif = (*mppdif);
        speed = (*mpspeed);

        //Read inputParameters from nodes
        rho = (*mprho);
        qmin = (*mpqmin);
        Dp = (*mpDp);
        epsmin = (*mpepsmin);
        Lp = (*mpLp);
        Rp = (*mpRp);
        wp1 = (*mpwp1);
        Kcp = (*mpKcp);
        tauv = (*mptauv);
        Tp = (*mpTp);
        Tm = (*mpTm);
        pnom = (*mppnom);
        speednom = (*mpspeednom);

        //Read outputVariables from nodes
        qp = (*mpqp);
        dqp = (*mpdqp);
        eps = (*mpeps);

//==This code has been autogenerated using Compgen==

        //LocalExpressions
        qmine = 0.159155*Dp*epsmin*speed;
        qmaxe = 0.159155*Dp*speed;
        dqmin = -(((qmaxe - qmine)*sqrt(p2/pnom))/Tm);
        dqmax = ((qmaxe - qmine)*sqrt(p2/pnom))/Tp;
        Lpe = (Lp*speednom*sqrt(pnom/p2))/lowLimit(speed,0.1);
        Rpe = (Rp*speednom*sqrt(pnom/p2))/lowLimit(speed,0.1);
        wp1e = (wp1*lowLimit(speed,0.1))/speednom;

        //Initialize delays
        delayParts1[1] = (-8*p2 + 8*p3 + 8*pdif + \
2*dqp*Power(mTimestep,2)*Rpe*wp1e - \
8*dqp*Lpe*tauv*wp1e)/(2*Lpe*mTimestep*wp1e + Power(mTimestep,2)*Rpe*wp1e + \
4*Lpe*tauv*wp1e + 2*mTimestep*Rpe*tauv*wp1e);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts1[2] = (4*p2 - 4*p3 - 4*pdif - 2*dqp*Lpe*mTimestep*wp1e - \
2*mTimestep*p2*wp1e + 2*mTimestep*p3*wp1e + 2*mTimestep*pdif*wp1e + \
dqp*Power(mTimestep,2)*Rpe*wp1e + 4*dqp*Lpe*tauv*wp1e - \
2*dqp*mTimestep*Rpe*tauv*wp1e)/(2*Lpe*mTimestep*wp1e + \
Power(mTimestep,2)*Rpe*wp1e + 4*Lpe*tauv*wp1e + 2*mTimestep*Rpe*tauv*wp1e);
        mDelayedPart12.initialize(mNstep,delayParts1[2]);
        delayParts2[1] = (-(dqp*mTimestep) - 2*qp)/2.;
        mDelayedPart21.initialize(mNstep,delayParts2[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(1);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];

        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        Vec stateVar(5);
        Vec stateVark(5);
        Vec deltaStateVar(5);

        //Read variables from nodes
        //Port P1
        T1 = (*mpND_T1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);
        //Port P2
        T2 = (*mpND_T2);
        c2 = (*mpND_c2);
        Zc2 = (*mpND_Zc2);
        //Port P3
        T3 = (*mpND_T3);
        c3 = (*mpND_c3);
        Zc3 = (*mpND_Zc3);

        //Read inputVariables from nodes
        pdif = (*mppdif);
        speed = (*mpspeed);

        //Read inputParameters from nodes
        rho = (*mprho);
        qmin = (*mpqmin);
        Dp = (*mpDp);
        epsmin = (*mpepsmin);
        Lp = (*mpLp);
        Rp = (*mpRp);
        wp1 = (*mpwp1);
        Kcp = (*mpKcp);
        tauv = (*mptauv);
        Tp = (*mpTp);
        Tm = (*mpTm);
        pnom = (*mppnom);
        speednom = (*mpspeednom);

        //LocalExpressions
        qmine = 0.159155*Dp*epsmin*speed;
        qmaxe = 0.159155*Dp*speed;
        dqmin = -(((qmaxe - qmine)*sqrt(p2/pnom))/Tm);
        dqmax = ((qmaxe - qmine)*sqrt(p2/pnom))/Tp;
        Lpe = (Lp*speednom*sqrt(pnom/p2))/lowLimit(speed,0.1);
        Rpe = (Rp*speednom*sqrt(pnom/p2))/lowLimit(speed,0.1);
        wp1e = (wp1*lowLimit(speed,0.1))/speednom;

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = dqp;
        stateVark[1] = qp;
        stateVark[2] = q2;
        stateVark[3] = p1;
        stateVark[4] = p2;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //PressureControlledPumpG
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =dqp - limit((-2*(p2 - p3 - pdif)*(2 + \
mTimestep*wp1e))/((2*Lpe + mTimestep*Rpe)*(mTimestep + 2*tauv)*wp1e) - \
delayedPart[1][1] - delayedPart[1][2],dqmin,dqmax);
          systemEquations[1] =qp - limit((dqp*mTimestep)/2. - \
delayedPart[2][1],qmine,qmaxe);
          systemEquations[2] =Kcp*(-p1 + p2) + q2 - qp;
          systemEquations[3] =p1 - lowLimit(c1 + q1*Zc1,0);
          systemEquations[4] =p2 - lowLimit(c2 + q2*Zc2,0);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[0][4] = (2*(2 + mTimestep*wp1e)*dxLimit((-2*(p2 - p3 \
- pdif)*(2 + mTimestep*wp1e))/((2*Lpe + mTimestep*Rpe)*(mTimestep + \
2*tauv)*wp1e) - delayedPart[1][1] - delayedPart[1][2],dqmin,dqmax))/((2*Lpe + \
mTimestep*Rpe)*(mTimestep + 2*tauv)*wp1e);
          jacobianMatrix[1][0] = -(mTimestep*dxLimit((dqp*mTimestep)/2. - \
delayedPart[2][1],qmine,qmaxe))/2.;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[1][4] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = -1;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = -Kcp;
          jacobianMatrix[2][4] = Kcp;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = 0;
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = 0;
          jacobianMatrix[4][2] = -(Zc2*dxLowLimit(c2 + q2*Zc2,0));
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          dqp=stateVark[0];
          qp=stateVark[1];
          q2=stateVark[2];
          p1=stateVark[3];
          p2=stateVark[4];
          //Expressions
          q1 = -q2;
          p3 = c3;
          q3 = 0.;
          eps = qp/qmaxe;
        }

        //Calculate the delayed parts
        delayParts1[1] = (-8*p2 + 8*p3 + 8*pdif + \
2*dqp*Power(mTimestep,2)*Rpe*wp1e - \
8*dqp*Lpe*tauv*wp1e)/(2*Lpe*mTimestep*wp1e + Power(mTimestep,2)*Rpe*wp1e + \
4*Lpe*tauv*wp1e + 2*mTimestep*Rpe*tauv*wp1e);
        delayParts1[2] = (4*p2 - 4*p3 - 4*pdif - 2*dqp*Lpe*mTimestep*wp1e - \
2*mTimestep*p2*wp1e + 2*mTimestep*p3*wp1e + 2*mTimestep*pdif*wp1e + \
dqp*Power(mTimestep,2)*Rpe*wp1e + 4*dqp*Lpe*tauv*wp1e - \
2*dqp*mTimestep*Rpe*tauv*wp1e)/(2*Lpe*mTimestep*wp1e + \
Power(mTimestep,2)*Rpe*wp1e + 4*Lpe*tauv*wp1e + 2*mTimestep*Rpe*tauv*wp1e);
        delayParts2[1] = (-(dqp*mTimestep) - 2*qp)/2.;

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(0);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];

        //Write new values to nodes
        //Port P1
        (*mpND_p1)=p1;
        (*mpND_q1)=q1;
        (*mpND_dE1)=dE1;
        //Port P2
        (*mpND_p2)=p2;
        (*mpND_q2)=q2;
        (*mpND_dE2)=dE2;
        //Port P3
        (*mpND_p3)=p3;
        (*mpND_q3)=q3;
        (*mpND_dE3)=dE3;
        //outputVariables
        (*mpqp)=qp;
        (*mpdqp)=dqp;
        (*mpeps)=eps;

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
#endif // HYDRAULICPRESSURECONTROLLEDPUMPG_HPP_INCLUDED
