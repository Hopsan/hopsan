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
#include "global.h"

ConnectorAppearance::ConnectorAppearance(QString type, GraphicsTypeEnumT gfxType)
{
    //Set the connector type and style
    setTypeAndIsoStyle(type, gfxType);     //Need to use set type instead of setting directly as setType narrows types down to power or signal
}

//! @brief Set the Connector type
void ConnectorAppearance::setStyle(const ConnectorStyleEnumT style)
{
    mConnectorStyle = style;
}

ConnectorStyleEnumT ConnectorAppearance::getStyle() const
{
    return mConnectorStyle;
}

void ConnectorAppearance::setIsoStyle(GraphicsTypeEnumT gfxType)
{
    mGfxType = gfxType;
}

void ConnectorAppearance::setTypeAndIsoStyle(QString porttype, GraphicsTypeEnumT gfxType)
{
    if(porttype=="PowerPortType")
    {
        setStyle(PowerConnectorStyle);
    }
    else if(porttype=="ReadPortType" || "WritePortType")
    {
        setStyle(SignalConnectorStyle);
    }
    else
    {
        setStyle(UndefinedConnectorStyle);
    }
    setIsoStyle(gfxType);
}

QPen ConnectorAppearance::getPen(const QString &rSituation) const
{
    return getPen(mConnectorStyle, mGfxType, rSituation);
}

void ConnectorAppearance::setCustomColor(const QColor &rColor)
{
    mCustomColor = rColor;
}

QColor ConnectorAppearance::getCustomColor() const
{
    return mCustomColor;
}


//! @brief Get function for primary pen style
QPen ConnectorAppearance::getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, const QString &rSituation) const
{
    if(rSituation == "Primary" && mCustomColor != QColor())
    {
        QPen retval = gpConfig->getPen(style, gfxType, rSituation);
        retval.setColor(mCustomColor);
        return retval;
    }
    return gpConfig->getPen(style, gfxType, rSituation);
}
