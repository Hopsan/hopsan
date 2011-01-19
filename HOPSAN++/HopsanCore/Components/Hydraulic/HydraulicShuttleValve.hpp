//!
//! @file   HydraulicShuttleValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-17
//!
//! @brief Contains a Shuttle Valve component
//!
//$Id$

#ifndef HYDRAULICSHUTTLEVALVE_HPP_INCLUDED
#define HYDRAULICSHUTTLEVALVE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicShuttleValve : public ComponentQ
    {

    private:
        double Kc;
        double p;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_p3, *mpND_q3, *mpND_c3, *mpND_Zc3;

        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicShuttleValve("ShuttleValve");
        }

        HydraulicShuttleValve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicShuttleValve";
            Kc = 1.0e-11;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeHydraulic", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);

            mpND_p3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::PRESSURE);
            mpND_q3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::FLOW);
        }


        void simulateOneTimestep()
        {
            double p1, p2, p3, q1, q2, q3, c1, c2;

            //Get variable values from nodes
            c1 = (*mpND_c1);
            c2 = (*mpND_c2);

            //Shuttle valve equations
            p1 = c1;
            p2 = c2;

            if(p1 < 0.0) { p1 = 0.0; }
            if(p2 < 0.0) { p2 = 0.0; }

            q1 = 0;
            q2 = 0;
            q3 = 0;

            if(p1>p2) { p3 = p1; }
            else { p3 = p2; }

            //Write new variables to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_p3) = p3;
            (*mpND_q3) = q3;
        }
    };
}

#endif // HydraulicGREATERTHAN_HPP_INCLUDED
