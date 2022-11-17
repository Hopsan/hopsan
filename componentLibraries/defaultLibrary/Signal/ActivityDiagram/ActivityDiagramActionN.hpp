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
//! @file   ActivityDiagramActionN.hpp
//! @author Petter Krus, Robert Braun, Emilia Villani
//! @date   2019-08-15
//! based on ElectricCapacitanceMultiPort.hpp and Volume component by Björn Eriksson <bjorn.eriksson@liu.se>
//! @brief Contains a Electric Volume Component
//!

#ifndef ACTIVITYDIAGRAMACTIONN_HPP_INCLUDED
#define ACTIVITYDIAGRAMACTIONN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <mutex>

namespace hopsan {

    //!
    //! @brief A StateMachine state component
    //! @ingroup StateMachine
    //!
    class ActivityDiagramActionN : public ComponentC
    {

    private:
        double state;
        double *mpAlpha;
        double *mpstate;
//        double capacitance;

        std::vector<double*> mvpN_s, mvpN_q;
        std::vector<double> mvp_S0;
        size_t mNumPorts;
        Port *mpPpn1;
        
        std::mutex mTokenMutex;
        bool mTokenLocked;

    public:
        static Component *Creator()
        {
            return new ActivityDiagramActionN();
        }

        void configure()
        {
            mpPpn1 = addPowerMultiPort("Ppn1", "NodePetriNet");
//            addConstant("Capacitance", "Capacitance", "Fa", 0.0001, capacitance);
            addOutputVariable("state","State activated","",0.,&mpstate);
        }


        void initialize()
        {
            mNumPorts = mpPpn1->getNumPorts();
            mvpN_s.resize(mNumPorts);
            mvpN_q.resize(mNumPorts);
            mvp_S0.resize(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpN_s[i]  = getSafeMultiPortNodeDataPtr(mpPpn1, i, NodePetriNet::State, 0.0);
                mvpN_q[i]  = getSafeMultiPortNodeDataPtr(mpPpn1, i, NodePetriNet::Flow, 0.0);
            
                *mvpN_s[i] = getDefaultStartValue(mpPpn1, NodePetriNet::State);
                *mvpN_q[i] = getDefaultStartValue(mpPpn1, NodePetriNet::Flow)/double(mNumPorts);
            }
            
            mTokenLocked=false;
        }


        void simulateOneTimestep()
        {
            double state;

            double sTot = 0.0;
  
            for (size_t i=0; i<mNumPorts; ++i)
            {
                sTot += onPositive((*mvpN_s[i])-0.5) + 2.0*double(mNumPorts)*(*mvpN_q[i]);
            }
            sTot=onPositive(sTot-0.5);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                 mvp_S0[i] = onPositive(sTot-0.5);
                (*mvpN_s[i]) =  mvp_S0[i];
            }  
            state=onPositive(-0.5+sTot);
          (*mpstate)=state;
          
          mTokenLocked=false;
        }


        void finalize()
        {
        }
        
        bool tryAndLockToken() 
        {
            mTokenMutex.lock();
            if(mTokenLocked) {
                mTokenMutex.unlock();
                addDebugMessage("Failed to lock token!");
                return false;
            }
            mTokenLocked=true;
            mTokenMutex.unlock();
            addDebugMessage("Success to lock token!");
            return true;
        }
    };
}

#endif // ACTIVITYDIAGRAMACTIONN_HPP_INCLUDED
