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

//! @brief Contains hardcoded appearance for different hopsancore ports
//! @todo maybe this should be placed in som more generic external .txt file in som way
void PortAppearance::selectPortIcon(QString CQSType, QString porttype, QString nodetype)
{
    mMainIconPath.clear();

    mMainIconPath = QString(PORTICONPATH);
    if (nodetype == "NodeSignal")
    {
        mMainIconPath.append("SignalPort");
        if ( porttype == "READPORT" || porttype == "READMULTIPORT")
        {
            mMainIconPath.append("_read");
        }
        else
        {
            mMainIconPath.append("_write");
        }
        mCQSOverlayPath.clear();
    }
    else
    {
        if (nodetype == "NodeMechanic")
        {
            mMainIconPath.append("MechanicPort");
        }
        else if (nodetype == "NodeMechanicRotational")
        {
            mMainIconPath.append("RotationalMechanicPort");
        }
        else if (nodetype == "NodeHydraulic")
        {
            mMainIconPath.append("HydraulicPort");
        }
        else if (nodetype == "NodeElectric")
        {
            mMainIconPath.append("ElectricPort");
        }
        else
        {
            //SystemPort is a blank port (that is why we use it here)
            mMainIconPath.append("SystemPort");
        }

        //Select cqs overlay icon path depending on cqs type
        if (CQSType == "C")
        {
            mCQSOverlayPath = (QString(PORTICONPATH) + "PortOverlayC.svg");
        }
        else if (CQSType == "Q")
        {
            mCQSOverlayPath = (QString(PORTICONPATH) + "PortOverlayQ.svg");
        }
        else if (CQSType == "UNDEFINEDCQSTYPE")
        {
            mCQSOverlayPath = (QString(PORTICONPATH) + "PortOverlayUnknown.svg");
        }
        else
        {
            mCQSOverlayPath.clear();
        }
    }
    mMainIconPath.append(".svg");

    //Check if we need to add multiport overlay
    if (porttype.contains("MULTIPORT"))
    {
        mMultiPortOverlayPath = (QString(PORTICONPATH) + "MultiPortOverlay.svg");
    }
    else
    {
        mMultiPortOverlayPath.clear();
    }
}

