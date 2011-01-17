//!
//! @file   MechanicTorsionalSpring.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a torsional spring
//!
//$Id$

#ifndef MECHANICTORSIONALSPRING_HPP_INCLUDED
#define MECHANICTORSIONALSPRING_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTorsionalSpring : public ComponentC
    {

    private:
        double k;
        double w1, c1, lastc1, w2, c2, lastc2, Zx;
        double *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_w2, *mpND_c2, *mpND_Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTorsionalSpring("TorsionalSpring");
        }

        MechanicTorsionalSpring(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "MechanicTorsionalSpring";
            k   = 100.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("k", "Spring Coefficient", "[N/rad]",  k);
        }


        void initialize()
        {
            mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CHARIMP);
            mpND_w2 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            w1 = (*mpND_w1);
            lastc1 = (*mpND_c1);
            w2 = (*mpND_w2);
            lastc2 = (*mpND_c2);

            //Spring equations
            Zx = k*mTimestep;
            c1 = lastc2 + 2.0*Zx*w2;
            c2 = lastc1 + 2.0*Zx*w1;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zx1) = Zx;
            (*mpND_c2) = c2;
            (*mpND_Zx2) = Zx;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


