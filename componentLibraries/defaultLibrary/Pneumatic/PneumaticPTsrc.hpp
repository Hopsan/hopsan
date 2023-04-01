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

#ifndef PNEUMATICPTSRC_HPP_INCLUDED
#define PNEUMATICPTSRC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticPTsrc.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Fri 28 Jun 2013 13:05:33
//! @brief Pneumatic pressure and temperature source
//! @ingroup PneumaticComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/PneumaticComponents.nb*/

using namespace hopsan;

class PneumaticPTsrc : public ComponentC
{
private:
     Port *mpPp1;
     int mNstep;
     //Port Pp1 variable
     double pp1;
     double qmp1;
     double Tp1;
     double dEp1;
     double cp1;
     double Zcp1;
//==This code has been autogenerated using Compgen==
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
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mppinput;
     double *mpTinput;
     //inputParameters pointers
     //outputVariables pointers
     EquationSystemSolver *mpSolver = nullptr;

public:
     static Component *Creator()
     {
        return new PneumaticPTsrc();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        mpPp1=addPowerPort("Pp1","NodePneumatic");
        //Add inputVariables to the component
            addInputVariable("pinput","Input \
Pressure","Pa",100000,&mppinput);
            addInputVariable("Tinput","Input \
Temperature","K",273.,&mpTinput);

        //Add inputParammeters to the component
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
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

        //Read variables from nodes
        //Port Pp1
        pp1 = (*mpND_pp1);
        qmp1 = (*mpND_qmp1);
        Tp1 = (*mpND_Tp1);
        dEp1 = (*mpND_dEp1);
        cp1 = (*mpND_cp1);
        Zcp1 = (*mpND_Zcp1);

        //Read inputVariables from nodes
        pinput = (*mppinput);
        Tinput = (*mpTinput);

        //Read inputParameters from nodes

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==


        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Pp1
        pp1 = (*mpND_pp1);
        qmp1 = (*mpND_qmp1);
        dEp1 = (*mpND_dEp1);

        //Read inputVariables from nodes
        pinput = (*mppinput);
        Tinput = (*mpTinput);

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
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // PNEUMATICPTSRC_HPP_INCLUDED
