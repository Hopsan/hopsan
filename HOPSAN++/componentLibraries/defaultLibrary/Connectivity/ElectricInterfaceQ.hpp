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
//! @file   ElectricInterfaceQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-03-01
//!
//! @brief Contains a electric interface component of Q-type
//!
//$Id$

#ifndef ELECTRICINTERFACEQ_HPP_INCLUDED
#define ELECTRICINTERFACEQ_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class ElectricInterfaceQ : public ComponentQ
    {

    private:
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new ElectricInterfaceQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeElectric");
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

#endif // ELECTRICINTERFACEQ_HPP_INCLUDED
