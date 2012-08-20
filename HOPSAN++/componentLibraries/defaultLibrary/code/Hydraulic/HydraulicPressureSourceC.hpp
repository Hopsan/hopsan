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
//! @file   HydraulicPressureSourceC.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCEC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSourceC : public ComponentC
    {
    private:
        double Zc;
        double p;
        Port *mpIn, *mpP1;

        double *mpND_in, *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSourceC();
        }

        void configure()
        {
            p         = 1.0e5;
            Zc        = 0.0;

            mpIn = addReadPort("In", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("p", "Default pressure", "[Pa]", p);

            disableStartValue(mpP1, NodeHydraulic::PRESSURE);
            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
        }


        void initialize()
        {

            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, p);
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            (*mpND_p) = p; //Override the startvalue for the pressure

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpND_in);
            (*mpND_Zc) = Zc;

//            if(mpIn->isConnected())
//                    (*mpND_c) = (*mpND_in);
//            else
//                    (*mpND_c) = p;
//            (*mpND_Zc) = Zc;
        }
    };
}

#endif // HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
