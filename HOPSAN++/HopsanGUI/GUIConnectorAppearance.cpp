#include "GUIConnectorAppearance.h"
#include "qdebug.h"
#include "Configuration.h"
#include "common.h"

GUIConnectorAppearance::GUIConnectorAppearance(QString type, graphicsType gfxType)
{
        //Obtain pen styles from config object
    mPrimaryPenPowerIso = gConfig.getPen("Power", "Iso", "Primary");
    mActivePenPowerIso = gConfig.getPen("Power", "Iso", "Active");
    mHoverPenPowerIso = gConfig.getPen("Power", "Iso", "Hover");

    mPrimaryPenSignalIso = gConfig.getPen("Signal", "Iso", "Primary");
    mActivePenSignalIso = gConfig.getPen("Signal", "Iso", "Active");
    mHoverPenSignalIso = gConfig.getPen("Signal", "Iso", "Hover");

    mPrimaryPenPowerUser = gConfig.getPen("Power", "User", "Primary");
    mActivePenPowerUser = gConfig.getPen("Power", "User", "Active");
    mHoverPenPowerUser = gConfig.getPen("Power", "User", "Hover");

    mPrimaryPenSignalUser = gConfig.getPen("Signal", "User", "Primary");
    mActivePenSignalUser = gConfig.getPen("Signal", "User", "Active");
    mHoverPenSignalUser = gConfig.getPen("Signal", "User", "Hover");

    mNonFinishedPen = gConfig.getPen("NonFinished", "User", "Primary");

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
    return getPen(situation, mConnectorType, mGfxType);
}

//! Get function for primary pen style
//! @todo Hardcoded appearance stuff (should maybe be loaded from external file (not prio 1)
QPen GUIConnectorAppearance::getPen(QString situation, QString type, graphicsType gfxType)
{
    //! @todo store pens in some smarter way, maybe in an array where situation and type are enums or used to calculate index, will be necessary for larger variations as a mega if else code is madness
    if(situation == "Primary")
    {
        if(type == "Power")
        {
            if (gfxType == ISOGRAPHICS)
            {
                return mPrimaryPenPowerIso;
            }
            else
            {
                return mPrimaryPenPowerUser;
            }
        }
        if(type == "Signal")
        {
            if (gfxType == ISOGRAPHICS)
            {
                return mPrimaryPenSignalIso;
            }
            else
            {
                return mPrimaryPenSignalUser;
            }
        }
    }
    else if(situation == "Active")
    {
        if(type == "Power")
        {
            if (gfxType == ISOGRAPHICS)
            {
                return mActivePenPowerIso;
            }
            else
            {
                return mActivePenPowerUser;
            }
        }
        if(type == "Signal")
        {
            if (gfxType == ISOGRAPHICS)
            {
                return mActivePenSignalIso;
            }
            else
            {
                return mActivePenSignalUser;
            }
        }
    }
    else if(situation == "Hover")
    {
        if(type == "Power")
        {
            if (gfxType == ISOGRAPHICS)
            {
                return mHoverPenPowerIso;
            }
            else
            {
                return mHoverPenPowerUser;
            }
        }
        if(type == "Signal")
        {
            if (gfxType == ISOGRAPHICS)
            {
                return mHoverPenSignalIso;
            }
            else
            {
                return mHoverPenSignalUser;
            }
        }
    }
    return mNonFinishedPen;
}

void GUIConnectorAppearance::adjustToZoom(qreal zoomFactor)
{
    qDebug() << "Adjust to zoom, zoomFactor = " << zoomFactor;
    qreal zoomFactor2 = zoomFactor;
    qreal zoomFactor3 = zoomFactor;
    if(zoomFactor > 1.0)
    {
        zoomFactor = 1.0;
        zoomFactor2 = zoomFactor;
        zoomFactor3 = zoomFactor;
    }
    else if( zoomFactor < 0.5)
    {
        zoomFactor2 = zoomFactor*2.0;
        zoomFactor3 = zoomFactor*3.0;
    }

    mPrimaryPenPowerIso.setWidth(1.5/zoomFactor);
    mActivePenPowerIso.setWidth(2.5/zoomFactor2);
    mHoverPenPowerIso.setWidth(2.5/zoomFactor2);

    mPrimaryPenSignalIso.setWidth(1.5/zoomFactor);
    mActivePenSignalIso.setWidth(2.5/zoomFactor2);
    mHoverPenSignalIso.setWidth(2.5/zoomFactor2);

    mPrimaryPenPowerUser.setWidth(2.5/zoomFactor2);
    mActivePenPowerUser.setWidth(4.5/zoomFactor3);
    mHoverPenPowerUser.setWidth(4.5/zoomFactor3);

    mPrimaryPenSignalUser.setWidth(1.5/zoomFactor);
    mActivePenSignalUser.setWidth(2.5/zoomFactor2);
    mHoverPenSignalUser.setWidth(2.5/zoomFactor2);
qDebug() << 2.0/zoomFactor2*zoomFactor;
}
