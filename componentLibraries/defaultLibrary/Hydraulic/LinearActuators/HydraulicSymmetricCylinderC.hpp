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
//! @file   HydraulicSymmetricCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-09-02
//!
//! @brief A symmetric piston component of C-type. Inherits HydraulicCylinderC.
//!
//$Id$

#ifndef HYDRAULICSYMMETRICCYLINDERC_H
#define HYDRAULICSYMMETRICCYLINDERC_H


#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HydraulicCylinderC.hpp"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicSymmetricCylinderC : public HydraulicCylinderC
{

    private:
        double *mpA;

    public:
        static Component *Creator()
        {
            return new HydraulicSymmetricCylinderC();
        }

        void configure()
        {
            HydraulicCylinderC::configure();

            removePort("A_1");
            removePort("A_2");

            addInputVariable("A", "Piston Area", "m^2", 0.001, &mpA);
        }

        void initialize()
        {
            mpA1 = mpA;
            mpA2 = mpA;
            HydraulicCylinderC::initialize();
        }

        void simulateOneTimestep()
        {
            HydraulicCylinderC::simulateOneTimestep();
        }
    };
}

#endif // HYDRAULICSYMMETRICCYLINDERC_H
