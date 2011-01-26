//!
//! @file   HydraulicPressureSource.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCE_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSource : public ComponentC
    {
    private:
        double mZc;
        double mPressure;
        Port *mpIn, *mpP1;

        double *mpND_in, *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSource("PressureSource");
        }

        HydraulicPressureSource(const std::string name) : ComponentC(name)
        {
            mTypeName = "HydraulicPressureSource";
            mPressure       = 1.0e5;
            mZc             = 0.0;

            mpIn = addReadPort("In", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("P", "Default pressure", "Pa", mPressure);

            setStartValue(mpP1, NodeHydraulic::PRESSURE, mPressure);
            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
        }


        void initialize()
        {

            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, mPressure);
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            (*mpND_p) = mPressure; //Override the startvalue for the pressure
            setStartValue(mpP1, NodeHydraulic::PRESSURE, mPressure); //This is here to show the user that the start value is hard coded!
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpND_in);
            (*mpND_Zc) = mZc;
        }
    };
}

#endif // HYDRAULICPRESSURESOURCE_HPP_INCLUDED
