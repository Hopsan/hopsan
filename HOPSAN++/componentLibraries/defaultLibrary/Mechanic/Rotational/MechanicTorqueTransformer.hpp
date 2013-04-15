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
//! @file   MechanicTorqueTransformer.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a mechanic prescribed torque component
//!
//$Id$

#ifndef MECHANICTORQUETRANSFORMER_HPP_INCLUDED
#define MECHANICTORQUETRANSFORMER_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTorqueTransformer : public ComponentC
    {

    private:
        double *mpTref,*mpND_t, *mpND_c, *mpND_Zx;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicTorqueTransformer();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            addInputVariable("T", "", "", 0.0, &mpTref);
            disableStartValue(mpP1, NodeMechanicRotational::Torque);
        }


        void initialize()
        {
            mpND_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            (*mpND_t) = (*mpTref);
            (*mpND_Zx) = 0.0;
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpTref);
            (*mpND_Zx) = 0.0;
        }
    };
}
#endif // MECHANICTORQUETRANSFORMER_HPP_INCLUDED
