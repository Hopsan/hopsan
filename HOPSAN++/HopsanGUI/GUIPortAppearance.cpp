//!
//! @file   GUIPortAppearance.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPortAppearance class
//!
//$Id$

#include "MainWindow.h"
#include "GUIPortAppearance.h"
#include "common.h"
#include <iostream>

//! @brief Contains hardcoded appearance for different hopsancore ports
//! @todo maybe this should be placed in som more generic external .txt file in som way
void GUIPortAppearance::selectPortIcon(QString CQSType, QString porttype, QString nodetype)
{
    mIconPath.clear();
    mIconOverlayPaths.clear();

    mIconPath = QString(PORTICONPATH);
    if (nodetype == "NodeSignal")
    {
        mIconPath.append("SignalPort");
        if ( porttype == "READPORT")
        {
            mIconPath.append("_read");
        }
        else
        {
            mIconPath.append("_write");
        }
    }
    else
    {
        if (nodetype == "NodeMechanic")
        {
            mIconPath.append("MechanicPort");
        }
        else if (nodetype == "NodeMechanicRotational")
        {
            mIconPath.append("RotationalMechanicPort");
        }
        else if (nodetype == "NodeHydraulic")
        {
            mIconPath.append("HydraulicPort");
        }
        else
        {
            //SystemPort is a blank port (that is why we use it here)
            mIconPath.append("SystemPort");
        }

        //Select overlay icon depending on cqs type and multiport
        //! @todo maybe should be able to select bassed on other things than cqs type
        mIconOverlayPaths.clear();
        if (CQSType == "C")
        {
            mIconOverlayPaths.append(QString(PORTICONPATH) + "PortOverlayC.svg");
        }
        else if (CQSType == "Q")
        {
            mIconOverlayPaths.append(QString(PORTICONPATH) + "PortOverlayQ.svg");
        }
    }
    if (porttype.contains("MULTIPORT"))
    {
        mIconOverlayPaths.append(QString(PORTICONPATH) + "MultiPortOverlay.svg");
    }
    mIconPath.append(".svg");
}

