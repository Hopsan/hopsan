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
//! @file   GUIConnectorAppearance.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIConnectorAppearance class
//!
//$Id$

#include "GUIConnectorAppearance.h"
#include "qdebug.h"
#include "Configuration.h"
#include "common.h"

GUIConnectorAppearance::GUIConnectorAppearance(QString type, graphicsType gfxType)
{
    //Set the connector type and style
    setTypeAndIsoStyle(type, gfxType);     //Need to use set type instead of setting directly as setType narrows types down to power or signal
}

//! @brief Set the Connector type
void GUIConnectorAppearance::setStyle(const connectorStyle style)
{
    mConnectorStyle = style;
}

connectorStyle GUIConnectorAppearance::getStyle()
{
    return mConnectorStyle;
}

void GUIConnectorAppearance::setIsoStyle(graphicsType gfxType)
{
    mGfxType = gfxType;
}

void GUIConnectorAppearance::setTypeAndIsoStyle(QString porttype, graphicsType gfxType)
{
    if(porttype=="POWERPORT")
    {
        setStyle(POWERCONNECTOR);
    }
    else if(porttype=="READPORT" || "WRITEPORT")
    {
        setStyle(SIGNALCONNECTOR);
    }
    else
    {
        setStyle(UNDEFINEDCONNECTOR);
    }
    setIsoStyle(gfxType);
}

QPen GUIConnectorAppearance::getPen(QString situation)
{
    return getPen(mConnectorStyle, mGfxType, situation);
}

//! Get function for primary pen style
QPen GUIConnectorAppearance::getPen(connectorStyle style, graphicsType gfxType, QString situation)
{
    return gConfig.getPen(style, gfxType, situation);
}
