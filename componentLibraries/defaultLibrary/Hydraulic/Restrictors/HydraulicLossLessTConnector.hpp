/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicLosslessTConnector.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-04
//!
//! @brief Contains a Hydraulic Lossless Connector with 3 ports
//!

#ifndef HYDRAULICLOSSLESSTCONNECTOR_HPP_INCLUDED
#define HYDRAULICLOSSLESSTCONNECTOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic lossless T-connector component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicLosslessTConnector : public ComponentQ
    {
    private:
        double Kc;
        double p;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_p3, *mpND_q3, *mpND_c3, *mpND_Zc3;
        double q1, q2, q3, c1, Zc1, c2, Zc2, c3, Zc3;

        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicLosslessTConnector();
        }

        void configure()
        {
            Kc = 1.0e-11;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeHydraulic");
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            mpND_p3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::Pressure);
            mpND_q3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::Flow);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::WaveVariable);
            mpND_Zc3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            c3 = (*mpND_c3);
            Zc3 = (*mpND_Zc3);

            //T-Connector equations
            p = (c1/Zc1 + c2/Zc2 + c3/Zc3) / ( 1/Zc1 + 1/Zc2 + 1/Zc3);
            q1 = (p-c1)/Zc1;
            q2 = (p-c2)/Zc2;
            q3 = (p-c3)/Zc3;

            //Cavitation check
            if(p < 0.0)
            {
                p = 0.0;
            }

            //Write new variables to nodes
            (*mpND_p1) = p;
            (*mpND_q1) = q1;
            (*mpND_p2) = p;
            (*mpND_q2) = q2;
            (*mpND_p3) = p;
            (*mpND_q3) = q3;
        }
    };
}

#endif // HYDRAULICLOSSLESSTCONNECTOR_HPP_INCLUDED

