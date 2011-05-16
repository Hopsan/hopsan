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
//! @file   HydraulicSubSysExample.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-30
//!
//! @brief Contains a Hydraulic Subsystem Example
//!
//$Id$

#ifndef HYDRAULICSUBSYSEXAMPLE_HPP_INCLUDED
#define HYDRAULICSUBSYSEXAMPLE_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentSystem.h"
#include "../Components.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicSubSysExample : public ComponentSystem
    {

    private:
        HydraulicVolume *volumeL;
        HydraulicLaminarOrifice *orificeC;
        HydraulicVolume *volumeR;
        Port *mpSubP1, *mpSubP2;

    public:
        static Component *Creator()
        {
            return new HydraulicSubSysExample("SubSysExample");
        }

        HydraulicSubSysExample(const std::string name) : ComponentSystem(name)
        {
            volumeL = new HydraulicVolume("volumeL");
            orificeC = new HydraulicLaminarOrifice("orificeC");
            volumeR = new HydraulicVolume("volumeR");

            orificeC->setParameterValue("Kc", 1e-12);

            addComponent(volumeL);
            addComponent(orificeC);
            addComponent(volumeR);

            mpSubP1 = addSystemPort("subP1");
            mpSubP2 = addSystemPort("subP2");

            //Connect components in subModel2
            connect(mpSubP1, volumeL->getPort("P1"));
            connect(volumeL->getPort("P2"), orificeC->getPort("P1"));
            connect(orificeC->getPort("P2"), volumeR->getPort("P1"));
            connect(volumeR->getPort("P2"), mpSubP2);

            //Decide submodel type
            setTypeCQS(Component::C);
        }

    };
}

#endif // HYDRAULICSUBSYSEXAMPLE_HPP_INCLUDED
