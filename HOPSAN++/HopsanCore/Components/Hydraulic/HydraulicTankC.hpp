//!
//! @file   HydraulicPressureSource.hpp
//! @author Robert Braun
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Tank Component of C-type
//!
//$Id$

#ifndef HYDRAULICTANKC_HPP_INCLUDED
#define HYDRAULICTANKC_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

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
            return new HydraulicTankC("PressureSource");
        }

        HydraulicTankC(const std::string name) : ComponentC(name)
        {
            mTypeName = "HydraulicTankC";
            mPressure       = 1.0e5;
            mZc             = 0.0;

            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("P", "Default pressure", "Pa", mPressure);

            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP1, NodeHydraulic::PRESSURE, mPressure);
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
            setStartValue(mpP1, NodeHydraulic::PRESSURE, mPressure); //Change the startvalue to notify the user
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
