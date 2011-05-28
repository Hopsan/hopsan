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
//! @file   HydraulicPressureSourceQ.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of Q-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSourceQ : public ComponentQ
    {
    private:
        double p;

        double *mpND_in, *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;

        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSourceQ("PressureSourceQ");
        }

        HydraulicPressureSourceQ(const std::string name) : ComponentQ(name)
        {
            p = 1e5;

            mpIn = addReadPort("in", "NodeSignal",  Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("p", "Default pressure", "[Pa]", p);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, p);
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double in, q, p, c, Zc;

            //Get variable values from nodes
            in = (*mpND_in);
            c = (*mpND_c);
            Zc = (*mpND_Zc);

            //Equations
            q = (in - c)/Zc;
            p = in;

            //Write variables to nodes
            (*mpND_p) = p;
            (*mpND_q) = q;
        }
    };
}

#endif // HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
