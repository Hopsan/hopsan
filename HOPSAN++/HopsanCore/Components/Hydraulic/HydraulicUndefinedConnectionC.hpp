//!
//! @file   HydraulicUndefinedConnectionC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-03-09
//!
//! @brief Contains a Hydraulic Undefined Connection Component of C-type
//!
//$Id: HydraulicUndefinedConnectionC.hpp 2596 2011-02-23 11:43:38Z robbr48 $

#ifndef HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED
#define HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

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
            return new HydraulicUndefinedConnectionC("UndefinedConnectionC");
        }

        HydraulicUndefinedConnectionC(const std::string name) : ComponentC(name)
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
