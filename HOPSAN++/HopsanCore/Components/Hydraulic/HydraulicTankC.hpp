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
//! @file   HydraulicPressureSource.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Tank Component of C-type
//!
//$Id$

#ifndef HYDRAULICTANKC_HPP_INCLUDED
#define HYDRAULICTANKC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTankC : public ComponentC
    {
    private:
        double mZc;
        double mPressure;

        double *mpND_p, *mpND_c, *mpND_Zc;

        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicTankC("TankC");
        }

        HydraulicTankC(const std::string name) : ComponentC(name)
        {
            mPressure       = 1.0e5;
            mZc             = 0.0;

            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("p", "Default Pressure", "[Pa]", mPressure);

            disableStartValue(mpP1, NodeHydraulic::PRESSURE);
            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            //Override the start value
            (*mpND_p) = mPressure;
            (*mpND_c) = mPressure;
            (*mpND_Zc) = mZc;
        }


        void simulateOneTimestep()
        {
            //Nothing will change
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICTANKC_HPP_INCLUDED
