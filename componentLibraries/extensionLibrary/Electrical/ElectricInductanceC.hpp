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

#ifndef ELECTRICINDUCTANCEC_HPP_INCLUDED
#define ELECTRICINDUCTANCEC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file ElectricInductanceC.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Wed 11 May 2016 21:28:30
//! @brief Inductance modelled as a c-component
//! @ingroup ElectricComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, \
Electric}/ElectricInductanceC.nb*/

using namespace hopsan;

class ElectricInductanceC : public ComponentC
{
private:
     double Induct;
     double alpha;
     Port *mpPel1;
     Port *mpPel2;
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
     //outputVariables
     double Cp;
     //InitialExpressions variables
     double cel1r;
     double cel2r;
     double cel1rf;
     double cel2rf;
     //LocalExpressions variables
     //Expressions variables
     //Port Pel1 pointer
     double *mpP_uel1;
     double *mpP_iel1;
     double *mpP_cel1;
     double *mpP_Zcel1;
     //Port Pel2 pointer
     double *mpP_uel2;
     double *mpP_iel2;
     double *mpP_cel2;
     double *mpP_Zcel2;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     //inputParameters pointers
     double *mpInduct;
     double *mpalpha;
     //outputVariables pointers
     double *mpCp;
     EquationSystemSolver *mpSolver = nullptr;

public:
     static Component *Creator()
     {
        return new ElectricInductanceC();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        mpPel1=addPowerPort("Pel1","NodeElectric");
        mpPel2=addPowerPort("Pel2","NodeElectric");
        //Add inputVariables to the component

        //Add inputParammeters to the component
            addInputVariable("Induct", "Inductance", "A/(Vs)", \
0.1,&mpInduct);
            addInputVariable("alpha", "numerical damping", "", 0.1,&mpalpha);
        //Add outputVariables to the component
            addOutputVariable("Cp","Parasitic Capacitance due to TLM","(A \
s)/V",0.,&mpCp);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pel1
        mpP_uel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Voltage);
        mpP_iel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Current);
        mpP_cel1=getSafeNodeDataPtr(mpPel1, NodeElectric::WaveVariable);
        mpP_Zcel1=getSafeNodeDataPtr(mpPel1, NodeElectric::CharImpedance);
        //Port Pel2
        mpP_uel2=getSafeNodeDataPtr(mpPel2, NodeElectric::Voltage);
        mpP_iel2=getSafeNodeDataPtr(mpPel2, NodeElectric::Current);
        mpP_cel2=getSafeNodeDataPtr(mpPel2, NodeElectric::WaveVariable);
        mpP_Zcel2=getSafeNodeDataPtr(mpPel2, NodeElectric::CharImpedance);

        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpP_uel1);
        iel1 = (*mpP_iel1);
        cel1 = (*mpP_cel1);
        Zcel1 = (*mpP_Zcel1);
        //Port Pel2
        uel2 = (*mpP_uel2);
        iel2 = (*mpP_iel2);
        cel2 = (*mpP_cel2);
        Zcel2 = (*mpP_Zcel2);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        Induct = (*mpInduct);
        alpha = (*mpalpha);

        //Read outputVariables from nodes
        Cp = (*mpCp);

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        cel1r = uel1 + 2*iel1*Zcel1;
        cel2r = uel2 + 2*iel2*Zcel2;
        cel1rf = uel1;
        cel2rf = uel2;

        //LocalExpressions
        cel2r = cel2 + 2*iel2*Zcel2;
        cel1r = cel1 + 2*iel1*Zcel1;
        cel1rf = (1 - alpha)*cel1r + alpha*cel2;
        cel2rf = alpha*cel1 + (1 - alpha)*cel2r;

        //Initialize delays


        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Pel1
        uel1 = (*mpP_uel1);
        iel1 = (*mpP_iel1);
        //Port Pel2
        uel2 = (*mpP_uel2);
        iel2 = (*mpP_iel2);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        Induct = (*mpInduct);
        alpha = (*mpalpha);

        //LocalExpressions
        cel2r = cel2 + 2*iel2*Zcel2;
        cel1r = cel1 + 2*iel1*Zcel1;
        cel1rf = (1 - alpha)*cel1r + alpha*cel2;
        cel2rf = alpha*cel1 + (1 - alpha)*cel2r;

        //Expressions
        Zcel1 = ((1 - alpha)*Induct)/mTimestep;
        Zcel2 = ((1 - alpha)*Induct)/mTimestep;
        cel1 = cel2rf;
        cel2 = cel1rf;
        Cp = Power(mTimestep,2)/Induct;

        //Calculate the delayed parts


        //Write new values to nodes
        //Port Pel1
        (*mpP_cel1)=cel1;
        (*mpP_Zcel1)=Zcel1;
        //Port Pel2
        (*mpP_cel2)=cel2;
        (*mpP_Zcel2)=Zcel2;
        //outputVariables
        (*mpCp)=Cp;

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // ELECTRICINDUCTANCEC_HPP_INCLUDED
