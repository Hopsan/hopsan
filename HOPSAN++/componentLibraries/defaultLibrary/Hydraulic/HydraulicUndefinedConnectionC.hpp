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
//! @file   HydraulicUndefinedConnectionC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-03-09
//!
//! @brief Contains a Hydraulic Undefined Connection Component of C-type
//!
//$Id$

#ifndef HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED
#define HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicUndefinedConnectionC : public ComponentC
    {
    private:
        Port *mpP1;
        double C, Z;
        double *mpND_c, *mpND_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicUndefinedConnectionC();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            C = 1e5;
            Z = 1;

            registerParameter("C",  "Wave Variable",     "[-]",  C);
            registerParameter("Z",  "Impedance",         "[-]",  Z);
        }


        void initialize()
        {
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            (*mpND_c) = C;
            (*mpND_Zc) = Z;
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = C;
            (*mpND_Zc) = Z;
        }
    };
}

#endif // HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED
