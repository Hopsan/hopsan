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
//! @file   HydraulicTankQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-12
//!
//! @brief Contains a Hydraulic Tank Component of Q-type
//!
//$Id$

#ifndef HYDRAULICTANKQ_HPP_INCLUDED
#define HYDRAULICTANKQ_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTankQ : public ComponentQ
    {
    private:
        double p;

        double *mpND_in, *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;

        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicTankQ("TankQ");
        }

        HydraulicTankQ(const std::string name) : ComponentQ(name)
        {
            p = 1e5;

            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("p", "Default Pressure", "[Pa]", p);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double q, c, Zc;

            //Get variable values from nodes
            c = (*mpND_c);
            Zc = (*mpND_Zc);

            //Equations
            q = (p - c)/Zc;

            //Write variables to nodes
            (*mpND_p) = p;
            (*mpND_q) = q;
        }
    };
}

#endif // HYDRAULICTANKQ_HPP_INCLUDED
