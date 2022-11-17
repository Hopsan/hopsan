#ifndef ACTIVITYDIAGRAMFORK_HPP_INCLUDED
#define ACTIVITYDIAGRAMFORK_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"
#include "ActivityDiagramActionN.hpp"

//!
//! @file ActivityDiagramFork.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Wed 13 Nov 2019 16:14:51
//! @brief Fork for a state machine
//! @ingroup ActivityDiagramComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, Users, petkr14, Dropbox, HopsanComponents, StateMachine0, \
ActivityDiagram, ActivityDiagram}/ActivityDiagramFork.nb*/

using namespace hopsan;

class ActivityDiagramFork : public ComponentQ
{
private:
     double diffEvent;
     Port *mpPpn1;
     Port *mpPpn2;
     Port *mpPpn3;
     int mNstep;
     //Port Ppn1 variable
     double spn1;
     double qpn1;
	 double qpn10;
     //Port Ppn2 variable
     double spn2;
     double qpn2;
     //Port Ppn3 variable
     double spn3;
     double qpn3;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double event;
     //outputVariables
     //Expressions variables
     double dEvent;
     double oldEvent;
     //Port Ppn1 pointer
     double *mpP_spn1;
     double *mpP_qpn1;
     //Port Ppn2 pointer
     double *mpP_spn2;
     double *mpP_qpn2;
     //Port Ppn3 pointer
     double *mpP_spn3;
     double *mpP_qpn3;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpevent;
     //inputParameters pointers
     double *mpdiffEvent;
     //outputVariables pointers
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new ActivityDiagramFork();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        mpPpn1=addPowerPort("Ppn1","NodePetriNet");
        mpPpn2=addPowerPort("Ppn2","NodePetriNet");
        mpPpn3=addPowerPort("Ppn3","NodePetriNet");
        //Add inputVariables to the component
            addInputVariable("event","event (trigg on positive \
flank)","",1.,&mpevent);

        //Add inputParammeters to the component
            addInputVariable("diffEvent", "Trigg on level (0) or flank (1)", \
"", 0.,&mpdiffEvent);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Ppn1
        mpP_spn1=getSafeNodeDataPtr(mpPpn1, NodePetriNet::State);
        mpP_qpn1=getSafeNodeDataPtr(mpPpn1, NodePetriNet::Flow);
        //Port Ppn2
        mpP_spn2=getSafeNodeDataPtr(mpPpn2, NodePetriNet::State);
        mpP_qpn2=getSafeNodeDataPtr(mpPpn2, NodePetriNet::Flow);
        //Port Ppn3
        mpP_spn3=getSafeNodeDataPtr(mpPpn3, NodePetriNet::State);
        mpP_qpn3=getSafeNodeDataPtr(mpPpn3, NodePetriNet::Flow);

        //Read variables from nodes
        //Port Ppn1
        spn1 = (*mpP_spn1);
        qpn1 = (*mpP_qpn1);
        //Port Ppn2
        spn2 = (*mpP_spn2);
        qpn2 = (*mpP_qpn2);
        //Port Ppn3
        spn3 = (*mpP_spn3);
        qpn3 = (*mpP_qpn3);

        //Read inputVariables from nodes
        event = (*mpevent);

        //Read inputParameters from nodes
        diffEvent = (*mpdiffEvent);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==


        //Initialize delays


        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Ppn1
        spn1 = (*mpP_spn1);
        //Port Ppn2
        spn2 = (*mpP_spn2);
        //Port Ppn3
        spn3 = (*mpP_spn3);

        //Read inputVariables from nodes
        event = (*mpevent);

        //Read inputParameters from nodes
        diffEvent = (*mpdiffEvent);

        //LocalExpressions

        //Expressions
        dEvent = onPositive(-0.5 + event - diffEvent*oldEvent);
		dEvent = onPositive(-0.5 + event - diffEvent*oldEvent);
		//Preventing creation of extra token
        //double drop = onPositive(-0.5 + spn1 - spn2 - spn3);
		qpn2 = 0;
        qpn3 = 0;
        qpn1 = 0;
        qpn10 = -dEvent*onPositive(-0.5 + spn1 - spn2 - spn3);
        //if(dEvent > 0 && drop > 0) {
        if(qpn10 < 0) {
            bool successUpstream = true;
            
            //Try to lock upstream component
            for(auto pPort : mpPpn1->getConnectedPorts()) {
                if(pPort->getComponent()->getTypeName() == "ActivityDiagramActionN") {
                    auto component = dynamic_cast<ActivityDiagramActionN*>(pPort->getComponent());
                    successUpstream = component->tryAndLockToken();
                }
            }
            
            //Try to lock downstream component
            bool successDownstream = true;
            for(auto pPort : mpPpn2->getConnectedPorts()) {
                if(pPort->getComponent()->getTypeName() == "ActivityDiagramActionN") {
                    auto component = dynamic_cast<ActivityDiagramActionN*>(pPort->getComponent());
                    successDownstream = component->tryAndLockToken();
                }
            }    
			
            //Try to lock downstream component
            for(auto pPort : mpPpn3->getConnectedPorts()) {
                if(pPort->getComponent()->getTypeName() == "ActivityDiagramActionN") {
                    auto component = dynamic_cast<ActivityDiagramActionN*>(pPort->getComponent());
                    successDownstream = successDownstream && component->tryAndLockToken();
                }
            }    
            
            //Locked both upstream and downstream (or not ActionN-components), compute flow
            if(successUpstream && successDownstream) {
                addDebugMessage("Locked tokens!");
                qpn2 = -qpn10;
                qpn3 = -qpn10;
                qpn1 = qpn10;
            }
        }
		oldEvent = event;
        /*qpn2 = dEvent*onPositive(spn1 - spn2 - spn3);
        qpn3 = dEvent*onPositive(spn1 - spn2 - spn3);
        qpn1 = -qpn2;
        oldEvent = event;
		*/    
        //Calculate the delayed parts


        //Write new values to nodes
        //Port Ppn1
        (*mpP_qpn1)=qpn1;
        //Port Ppn2
        (*mpP_qpn2)=qpn2;
        //Port Ppn3
        (*mpP_qpn3)=qpn3;
        //outputVariables

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // ACTIVITYDIAGRAMFORK_HPP_INCLUDED
