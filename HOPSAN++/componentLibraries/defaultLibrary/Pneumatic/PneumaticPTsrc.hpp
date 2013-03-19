#ifndef PNEUMATICPTSRC_HPP_INCLUDED
#define PNEUMATICPTSRC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticPTsrc.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Mon 11 Mar 2013 17:47:29
//! @brief Pneumatic pressure and temperature source
//! @ingroup PneumaticComponents
//!
//This code has been autogenerate using Compgen for Hopsan simulation
//from 
/*{, C:, Users, petkr14.IEI, Documents, CompgenNG}/PneumaticNG4.nb*/

using namespace hopsan;

class PneumaticPTsrc : public ComponentC
{
private:
     double mpinput;
     double mTinput;
     Port *mpPp1;
     Port *mpPpinput;
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
     double pinput;
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
     double *mpND_pinput;
     double *mpND_Tinput;
     //outputVariables pointers
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new PneumaticPTsrc();
     }

     void configure()
     {
        const double pinput = 100000;
        const double Tinput = 273.;
//This code has been autogenerate using Compgen for Hopsan simulation

        mNstep=9;
        mpinput = pinput;
        mTinput = Tinput;

        //Add ports to the component
        mpPp1=addPowerPort("Pp1","NodePneumatic");

        //Add inputVariables ports to the component
        mpPpinput=addReadPort("Ppinput","NodeSignal", Port::NOTREQUIRED);
        mpPTinput=addReadPort("PTinput","NodeSignal", Port::NOTREQUIRED);

        //Add outputVariables ports to the component

//This code has been autogenerate using Compgen for Hopsan simulation
        //Register changable parameters to the HOPSAN++ core
        registerParameter("pinput", "Input Pressure", "Pa", mpinput);
        registerParameter("Tinput", "Input Temperature", "K", mTinput);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pp1
        mpND_pp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::Pressure);
        mpND_qmp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::MassFlow);
        mpND_Tp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::Temperature);
        mpND_dEp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::EnergyFlow);
        mpND_cp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::WaveVariable);
        mpND_Zcp1=getSafeNodeDataPtr(mpPp1, NodePneumatic::CharImpedance);
        //Read inputVariables pointers from nodes
        mpND_pinput=getSafeNodeDataPtr(mpPpinput, NodeSignal::Value,mpinput);
        mpND_Tinput=getSafeNodeDataPtr(mpPTinput, NodeSignal::Value,mTinput);
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
        pinput = (*mpND_pinput);
        Tinput = (*mpND_Tinput);

        //Read outputVariables from nodes

//This code has been autogenerate using Compgen for Hopsan simulation


        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Pp1
        pp1 = (*mpND_pp1);
        qmp1 = (*mpND_qmp1);
        Tp1 = (*mpND_Tp1);
        dEp1 = (*mpND_dEp1);
        cp1 = (*mpND_cp1);
        Zcp1 = (*mpND_Zcp1);

        //Read inputVariables from nodes
        pinput = (*mpND_pinput);
        Tinput = (*mpND_Tinput);

        //LocalExpressions

          //Expressions
          cp1 = pinput;
          Tp1 = Tinput;
          Zcp1 = 0.;

        //Calculate the delayed parts


        //Write new values to nodes
        //Port Pp1
        (*mpND_Tp1)=Tp1;
        (*mpND_cp1)=cp1;
        (*mpND_Zcp1)=Zcp1;
        //outputVariables

        //Update the delayed variabels

     }
};
#endif // PNEUMATICPTSRC_HPP_INCLUDED
