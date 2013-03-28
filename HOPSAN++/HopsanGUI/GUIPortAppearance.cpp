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
#include "CoreAccess.h"

PortAppearance::PortAppearance()
{
    //Default values
    mEnabled = true;
    mAutoPlaced = true;
}

//! @brief Contains hardcoded appearance for different hopsancore ports
//! @todo maybe this should be placed in som more generic external .txt file in som way
void PortAppearance::selectPortIcon(QString CQSType, QString porttype, QString nodetype)
{
    mMainIconPath.clear();

    mMainIconPath = QString(PORTICONPATH);
    if (nodetype == "NodeSignal")
    {
        mMainIconPath.append("SignalPort");
        if ( porttype == "ReadPortType" || porttype == "ReadMultiportType")
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
        QString niceName = NodeInfo(nodetype).niceName;
        if(niceName.isEmpty())
        {
            mMainIconPath.append("SystemPort");
        }
        else
        {
            niceName[0] = niceName[0].toUpper();
            mMainIconPath.append(niceName+"Port");
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
        else if (CQSType == "UndefinedCQSType")
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
    if (porttype.contains("MultiportType"))
    {
        mMultiPortOverlayPath = (QString(PORTICONPATH) + "MultiPortOverlay.svg");
    }
    else
    {
        mMultiPortOverlayPath.clear();
    }
}

