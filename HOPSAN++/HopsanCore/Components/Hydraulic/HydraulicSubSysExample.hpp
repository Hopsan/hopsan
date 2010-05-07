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
#include "../Components.h"

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

public:
    static Component *Creator()
    {
        return new HydraulicSubSysExample("SubSysExample");
    }

    HydraulicSubSysExample(const string name) : ComponentSystem(name)
    {
        volumeL = new HydraulicVolume("volumeL");
        orificeC = new HydraulicLaminarOrifice("orificeC");
        volumeR = new HydraulicVolume("volumeR");

        orificeC->setParameter("Kc", 1e-12);

        addComponent(volumeL);
        addComponent(orificeC);
        addComponent(volumeR);

        addSystemPort("subP1");
        addSystemPort("subP2");

        //Connect components in subModel2
        connect(this, "subP1" , volumeL, "P1");
        connect(volumeL, "P2", orificeC, "P1");
        connect(orificeC, "P2", volumeR, "P1");
        connect(volumeR, "P2" , this, "subP2");

        //Decide submodel type
        setTypeCQS("C");
    }

};

#endif // HYDRAULICSUBSYSEXAMPLE_HPP_INCLUDED
