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
//! @file   ActivityDiagramMergeN.hpp
//! @author Petter Krus 
//! @date   2018-12-31
//! based on ElectricCapacitanceMultiPort.hpp by Björn Eriksson <bjorn.eriksson@liu.se>
//! @brief Contains a multi in join Component
//!

#ifndef ACTIVITYDIAGRAMMERGEN_HPP_INCLUDED
#define ACTIVITYDIAGRAMMERGEN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <vector>
#include <sstream>
#include <iostream>

namespace hopsan {

    //!
    //! @brief A StateMchine AND multi port component
    //! @ingroup StateMachine
    //!
    class ActivityDiagramMergeN : public ComponentQ
   {

    private:
        std::vector<double*> mvpN_q, mvpN_s;
        size_t mNumPorts;
        Port *mpPpn1;
        Port *mpPpn2;

		//Port Ppn2 variable
		double q2;
		double s2;

		//Port Ppn2 pointer
		double *mpP_q2;
		double *mpP_s2;

    public:
        static Component *Creator()
        {
            return new ActivityDiagramMergeN();
        }

        void configure()
        {    
            mpPpn1 = addPowerMultiPort("Ppn1", "NodePetriNet");
            mpPpn2=addPowerPort("Ppn2","NodePetriNet");
        }


        void initialize()
        {
			//Port Ppn1
            mNumPorts = mpPpn1->getNumPorts();
            mvpN_q.resize(mNumPorts);
            mvpN_s.resize(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpN_q[i]  = getSafeMultiPortNodeDataPtr(mpPpn1, i, NodePetriNet::Flow, 0.0);
                mvpN_s[i]  = getSafeMultiPortNodeDataPtr(mpPpn1, i, NodePetriNet::State, 0.0);

 //               *mvpN_q[i] = getDefaultStartValue(mpPpn1, NodePetriNet::Flow)/double(mNumPorts);
 //               *mvpN_s[i] = getDefaultStartValue(mpPpn1, NodePetriNet::State);
            }
			//Port Ppn2
			mpP_q2=getSafeNodeDataPtr(mpPpn2, NodePetriNet::Flow);
			mpP_s2=getSafeNodeDataPtr(mpPpn2, NodePetriNet::State);
			//Read variables from nodes

			//Port Ppn2
			q2 = (*mpP_q2);
			s2 = (*mpP_s2);
        }

        void simulateOneTimestep()
		{
            double q2 = 0;
			double q2d;
			q2d=0.;
			s2 = (*mpP_s2);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                q2 = q2+onPositive(q2d+onPositive(*mvpN_s[i])-s2-0.5);
				q2d=q2;
            }
            q2= onPositive(((*mvpN_s[0])-s2)-0.5);
            (*mvpN_q[0])=-q2;
            for (size_t i=1; i<mNumPorts; ++i)
            {
                (*mvpN_q[i]) = -onPositive((1-q2)*((*mvpN_s[i])-s2)-0.5);
				q2=onPositive(q2-(*mvpN_q[i])-0.5);
            }
			
			//Port Ppn2
			(*mpP_q2)=q2;
        }

    };
}

#endif // ACTIVITYDIAGRAMMERGEN_HPP_INCLUDED
