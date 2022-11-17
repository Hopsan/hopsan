/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   ActivityDiagramForkN.hpp
//! @author Petter Krus 
//! @date   2018-12-31
//! based on ElectricCapacitanceMultiPort.hpp by Björn Eriksson <bjorn.eriksson@liu.se>
//! @brief Contains a multi in join Component
//!

#ifndef ACTIVITYDIAGRAMFORKN_HPP_INCLUDED
#define ACTIVITYDIAGRAMFORKN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <vector>
#include <sstream>
#include <iostream>

namespace hopsan {

    //!
    //! @brief A ActivityDiagram FORK multi port component
    //! @ingroup ActivityDiagram
    //!
    class ActivityDiagramForkN : public ComponentQ
    {

    private:
    double diffevent;
    double event;
    double state;
    double oldState;
    double oldEvent;

    std::vector<double*> mvpN_s, mvpN_q;
    std::vector<double> mvp_S0;
    size_t mNumPorts;
    Port *mpPpn1;  
    Port *mpPpn2;

    //Port Ppn2 variable
    double s2;  
    double q2;

    //Port Ppn2 pointer
    double *mpP_s2;  
    double *mpP_q2;
     
    //inputVariables pointers
    double *mpevent;
     
    //inputParameters pointers
    double *mpdiffevent;
     
    //outputVariables pointers
    double *mpstate;

    public:
        static Component *Creator()
        {
            return new ActivityDiagramForkN();
        }

        void configure()
        {
            mpPpn1 = addPowerMultiPort("Ppn1", "NodePetriNet");
	   mpPpn2=addPowerPort("Ppn2","NodePetriNet");
			
            //Add inputVariables to the component
            addInputVariable("event","event ","",1.,&mpevent);

            //Add inputParammeters to the component
            addInputVariable("diffevent", "Trigg on level (0) or flank (1)", "", 0.,&mpdiffevent);
            
            //Add outputVariables to the component
            addOutputVariable("state","state","",0.,&mpstate);

        }

        void initialize()
        {
			//Port Ppn1
            mNumPorts = mpPpn1->getNumPorts();
            mvpN_s.resize(mNumPorts);
            mvpN_q.resize(mNumPorts);
            mvp_S0.resize(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpN_s[i]  = getSafeMultiPortNodeDataPtr(mpPpn1, i, NodePetriNet::State, 0.0);
                mvpN_q[i]  = getSafeMultiPortNodeDataPtr(mpPpn1, i, NodePetriNet::Flow, 0.0);
				
//                *mvpN_q[i] = getDefaultStartValue(mpPpn1, NodePetriNet::Flow)/double(mNumPorts);
//                *mvpN_s[i] = getDefaultStartValue(mpPpn1, NodePetriNet::State);
            }
            
            //Port Ppn2
            mpP_s2=getSafeNodeDataPtr(mpPpn2, NodePetriNet::State);
            mpP_q2=getSafeNodeDataPtr(mpPpn2, NodePetriNet::Flow);

            //Port Ppn2
            q2 = (*mpP_q2);
			
            //Read inputVariables from nodes
            event = (*mpevent);

            //Read inputParameters from nodes
            diffevent = (*mpdiffevent);

            //Read outputVariables from nodes
            state = (*mpstate);
			
            oldEvent=0.;
        }

        void simulateOneTimestep()
        {
            double stot;
            double se=0;
            //Read inputVariables from nodes
            event = (*mpevent);

            //Read inputParameters from nodes
            diffevent = (*mpdiffevent);

            stot=0.;
            s2 = (*mpP_s2);
            state = onPositive(-0.5 + event - diffevent*oldEvent);  
            oldEvent = event;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                stot = stot+(*mvpN_s[i]);
                se=onPositive(stot-0.5);
            }
            for (size_t i=0; i<mNumPorts; ++i)
            {
                (*mvpN_q[i]) = state*onPositive(s2-se-0.5);
            }

            //Port Ppn2
            (*mpP_q2)=-state*onPositive(s2-se-0.5);
            //outputVariables
            (*mpstate)=state;
        }

        void finalize()
        {
        }
    };
}

#endif // ACTIVITYDIAGRAMFORKN_HPP_INCLUDED
