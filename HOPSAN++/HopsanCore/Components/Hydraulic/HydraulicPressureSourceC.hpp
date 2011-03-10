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
#include "../../ComponentEssentials.h"

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
            return new HydraulicPressureSourceC("PressureSourceC");
        }

        HydraulicPressureSourceC(const std::string name) : ComponentC(name)
        {
            mTypeName = "HydraulicPressureSourceC";
            p         = 1.0e5;
            Zc        = 0.0;

            mpIn = addReadPort("In", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("P", "Default pressure", "[Pa]", p);

            setStartValue(mpP1, NodeHydraulic::PRESSURE, p);
            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
        }


        void initialize()
        {

            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, p);
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            (*mpND_p) = p; //Override the startvalue for the pressure
            setStartValue(mpP1, NodeHydraulic::PRESSURE, p); //This is here to show the user that the start value is hard coded!

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpND_in);
            (*mpND_Zc) = Zc;
        }
    };
}

#endif // HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
