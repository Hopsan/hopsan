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
//! @todo Maybe should use enum or some other non (can be whatever the programer want) type, or make it possible to "register" string keys with predetermined appearce through a ini file or similar
void GUIConnectorAppearance::setType(const QString type)
{
    if (type == "POWERPORT")
    {
        mConnectorType = "Power";
    }
    else if(type == "SIGNALPORT")
    {
        mConnectorType = "Signal";
    }
    else
    {
        mConnectorType = "Undefined";
    }
}

QString GUIConnectorAppearance::getType()
{
    return mConnectorType;
}

void GUIConnectorAppearance::setIsoStyle(graphicsType gfxType)
{
    mGfxType = gfxType;
}

void GUIConnectorAppearance::setTypeAndIsoStyle(QString porttype, graphicsType gfxType)
{
    setType(porttype);
    setIsoStyle(gfxType);
}

QPen GUIConnectorAppearance::getPen(QString situation)
{
    return getPen(mConnectorType, mGfxType, situation);
}

//! Get function for primary pen style
QPen GUIConnectorAppearance::getPen(QString type, graphicsType gfxType, QString situation)
{
    return gConfig.getPen(type, gfxType, situation);
}
