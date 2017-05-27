/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
