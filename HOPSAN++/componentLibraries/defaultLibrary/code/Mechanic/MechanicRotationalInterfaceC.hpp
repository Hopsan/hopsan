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
//! @file   MechanicRotationalInterfaceC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-30
//!
//! @brief Contains a rotational mechanic interface component of C-type
//!
//$Id$

#ifndef MECHANICROTATIONALINTERFACEC_HPP_INCLUDED
#define MECHANICROTATIONALINTERFACEC_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInterfaceC : public ComponentC
    {

    private:
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInterfaceC();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        }

        void initialize()
        {
            //Interfacing is handled through readnode/writenode from the RT wrapper file
        }

        void simulateOneTimestep()
        {
            //Interfacing is handled through readnode/writenode from the RT wrapper file
        }
    };
}
#endif // MECHANICROTATIONALINTERFACEC_HPP_INCLUDED
