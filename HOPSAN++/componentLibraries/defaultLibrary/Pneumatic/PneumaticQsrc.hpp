#ifndef PNEUMATICQSRC_HPP_INCLUDED
#define PNEUMATICQSRC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticQsrc.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Mon 11 Mar 2013 17:47:29
//! @brief Pneumatic massflow source
//! @ingroup PneumaticComponents
//!
//This code has been autogenerate using Compgen for Hopsan simulation
//from 
/*{, C:, Users, petkr14.IEI, Documents, CompgenNG}/PneumaticNG4.nb*/

using namespace hopsan;

class PneumaticQsrc : public ComponentQ
{
private:
     double mcv;
     double mqminput;
     double mTinput;
     Port *mpPp1;
     Port *mpPqminput;
     Port *mpPTinput;
     int mNstep;
     //Port Pp1 variable
     double pp1;
     double qmp1;
     double Tp1;
     double dEp1;
     double cp1;
     double Zcp1;
//This code has been autogenerate using Compgen for Hopsan simulation
     //inputVariables
     double qminput;
     double Tinput;
     //outputVariables

     //Expressions variables
     //Port Pp1 pointer
     double *mpND_pp1;
     double *mpND_qmp1;
     double *mpND_Tp1;
     double *mpND_dEp1;
     double *mpND_cp1;
     double *mpND_Zcp1;
     //Delay declarations
//This code has been autogenerate using Compgen for Hopsan simulation
     //inputVariables pointers
     double *mpND_qminput;
     double *mpND_Tinput;
     //outputVariables pointers
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new PneumaticQsrc();
     }

     void configure()
     {
        const double cv = 718;
        const double qminput = 0.;
        const double Tinput = 273;
//This code has been autogenerate using Compgen for Hopsan simulation

        mNstep=9;
        mcv = cv;
        mqminput = qminput;
        mTinput = Tinput;

        //Add ports to the component
        mpPp1=addPowerPort("Pp1","NodePneumatic");

        //Add inputVariables ports to the component
        mpPqminput=addReadPort("Pqminput","NodeSignal", Port::NOTREQUIRED);
        mpPTinput=addReadPort("PTinput","NodeSignal", Port::NOTREQUIRED);

        //Add outputVariables ports to the component

//This code has been autogenerate using Compgen for Hopsan simulation
        //Register changable parameters to the HOPSAN++ core
        registerParameter("cv", "heatcoeff)", "", mcv);
        registerParameter("qminput", "mass flow rate", "kg/s", mqminput);
        registerParameter("Tinput", "Temperature", "K", mTinput);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pp1
        mpND_pp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::PRESSURE);
        mpND_qmp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::MASSFLOW);
        mpND_Tp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::TEMPERATURE);
        mpND_dEp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::ENERGYFLOW);
        mpND_cp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::WAVEVARIABLE);
        mpND_Zcp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::CHARIMP);
        //Read inputVariables pointers from nodes
        mpND_qminput=getSafeNodeDataPtr(mpPqminput, \
NodeSignal::VALUE,mqminput);
        mpND_Tinput=getSafeNodeDataPtr(mpPTinput, NodeSignal::VALUE,mTinput);
        //Read outputVariable pointers from nodes

        //Read variables from nodes
        //Port Pp1
        pp1 = (*mpND_pp1);
        qmp1 = (*mpND_qmp1);
        Tp1 = (*mpND_Tp1);
        dEp1 = (*mpND_dEp1);
        cp1 = (*mpND_cp1);
        Zcp1 = (*mpND_Zcp1);

        //Read inputVariables from nodes
        qminput = (*mpND_qminput);
        Tinput = (*mpND_Tinput);

        //Read outputVariables from nodes

//This code has been autogenerate using Compgen for Hopsan simulation


        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Pp1
        Tp1 = (*mpND_Tp1);
        cp1 = (*mpND_cp1);
        Zcp1 = (*mpND_Zcp1);

        //Read inputVariables from nodes
        qminput = (*mpND_qminput);
        Tinput = (*mpND_Tinput);

        //LocalExpressions

          //Expressions
          qmp1 = qminput;
          dEp1 = mcv*qmp1*Tp1*onNegative(qmp1) + \
mcv*qmp1*Tinput*onPositive(qmp1);
          pp1 = cp1 + dEp1*Zcp1;

        //Calculate the delayed parts


        //Write new values to nodes
        //Port Pp1
        (*mpND_pp1)=pp1;
        (*mpND_qmp1)=qmp1;
        (*mpND_dEp1)=dEp1;
        //outputVariables

        //Update the delayed variabels

     }
};
#endif // PNEUMATICQSRC_HPP_INCLUDED
