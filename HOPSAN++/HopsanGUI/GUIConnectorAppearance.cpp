#include "GUIConnectorAppearance.h"
#include "qdebug.h"

GUIConnectorAppearance::GUIConnectorAppearance(QString type, graphicsType gfxType)
{
    //! @todo Dont set these here should be set once when the program starts, should be possible to change appearance by config
    //Sets the hardcoded connector pen appearance
    mPrimaryPenPowerIso = QPen(QColor("black"),1, Qt::SolidLine, Qt::RoundCap);
    mActivePenPowerIso = QPen(QColor("red"), 2, Qt::SolidLine, Qt::RoundCap);
    mHoverPenPowerIso = QPen(QColor("darkRed"),2, Qt::SolidLine, Qt::RoundCap);

    mPrimaryPenSignalIso = QPen(QColor("blue"),1, Qt::DashLine);
    mActivePenSignalIso = QPen(QColor("red"), 2, Qt::DashLine);
    mHoverPenSignalIso = QPen(QColor("darkRed"),2, Qt::DashLine);

    mPrimaryPenPowerUser = QPen(QColor("black"),2, Qt::SolidLine, Qt::RoundCap);
    mActivePenPowerUser = QPen(QColor("red"), 3, Qt::SolidLine, Qt::RoundCap);
    mHoverPenPowerUser = QPen(QColor("darkRed"),3, Qt::SolidLine, Qt::RoundCap);

    mPrimaryPenSignalUser = QPen(QColor("blue"),1, Qt::DashLine);
    mActivePenSignalUser = QPen(QColor("red"), 2, Qt::DashLine);
    mHoverPenSignalUser = QPen(QColor("darkRed"),2, Qt::DashLine);

    mNonFinishedPen = QPen(QColor("lightslategray"),3,Qt::SolidLine, Qt::RoundCap);

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
