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
        double *w1_ptr, *c1_ptr, *Zx1_ptr, *w2_ptr, *c2_ptr, *Zx2_ptr;
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
            w1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::ANGULARVELOCITY);
            c1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::WAVEVARIABLE);
            Zx1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::CHARIMP);
            w2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::ANGULARVELOCITY);
            c2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::WAVEVARIABLE);
            Zx2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            w1 = (*w1_ptr);
            lastc1 = (*c1_ptr);
            w2 = (*w2_ptr);
            lastc2 = (*c2_ptr);

            //Spring equations
            Zx = k*mTimestep;
            c1 = lastc2 + 2.0*Zx*w2;
            c2 = lastc1 + 2.0*Zx*w1;

            //Write new values to nodes
            (*c1_ptr) = c1;
            (*Zx1_ptr) = Zx;
            (*c2_ptr) = c2;
            (*Zx2_ptr) = Zx;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


