/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   AppearanceData.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by guiobjects and library widget
//! @todo Also contains some other appearance stuff that maybe should not be in this fil
//!
//$Id$

#include "AppearanceData.h"
#include "qdebug.h"
#include "GUIUtilities.h"

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




//! @brief Contains hardcoded appearance for different hopsancore ports
//! @todo maybe this should be placed in som more generic external .txt file in som way
void PortAppearance::selectPortIcon(QString CQSType, QString porttype, QString nodetype)
{
    mIconPath.clear();
    mIconPath.append(QString(PORTICONPATH));
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
    else if (nodetype == "NodeMechanic")
    {
        mIconPath.append("MechanicPort");
        if (CQSType == "C")
        {
            mIconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            mIconPath.append("Q");
        }
    }
    else if (nodetype == "NodeMechanicRotational")
    {
        mIconPath.append("RotationalMechanicPort");
        if (CQSType == "C")
        {
            mIconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            mIconPath.append("Q");
        }
    }
    else if (nodetype == "NodeHydraulic")
    {
        mIconPath.append("HydraulicPort");
        if (CQSType == "C")
        {
            mIconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            mIconPath.append("Q");
        }
    }
    else
    {
        //SystemPort is a blank port (that is why we use it here)
        mIconPath.append("SystemPort");
    }
    mIconPath.append(".svg");
}

AppearanceData::AppearanceData()
{
    //Assume all strings default to ""
    mPortAppearanceMap.clear();
}


//QTextStream& operator >>(QTextStream &is, AppearanceData &rData)
//{
//    //! @todo handle returned error indication
//    bool sucess = rData.readFromTextStream(is);
//    rData.mIsReadOK = sucess;
//    return is;
//}

//QTextStream& operator <<(QTextStream &os, AppearanceData &rData)
//{
//    //! @todo maybe write header here (probaly not a good place, better somewhere else)
//    //! @todo find out how to make newline in qt instead of "\n"
//    os << "TYPENAME " << addQuotes(rData.mTypeName) << "\n";
//    os << "DISPLAYNAME " << addQuotes(rData.mName) << "\n";
//    //os << "BASEPATH " << rData.getBasePath() << "\n"; //Base path is computer dependant
//    if (!rData.mQString(ICONPATH)ISO.isEmpty())
//    {
//        os << "ISOICON " << addQuotes(rData.mQString(ICONPATH)ISO) << "\n";
//    }
//    if (!rData.mQString(ICONPATH)User.isEmpty())
//    {
//        os << "USERICON " << addQuotes(rData.mQString(ICONPATH)User) << "\n";
//    }
//    if (!rData.mIconRotationBehaviour.isEmpty())
//    {
//        os << "ICONROTATION " << rData.mIconRotationBehaviour << "\n";
//    }

//    PortAppearanceMapT::iterator it;
//    for (it = rData.mPortAppearanceMap.begin(); it != rData.mPortAppearanceMap.end(); ++it)
//    {
//        os << "PORT " << " "
//           << addQuotes(it.key()) << " "
//           << it.value().x << " "
//           << it.value().y << " "
//           << it.value().rot << "\n";
//    }
//    return os;
//}

//! @brief get the type-name
//! @returns The type-name
QString AppearanceData::getTypeName()
{
    return mTypeName;
}

//! @brief get the display name, even if it is empty
//! @returns The display name
QString AppearanceData::getName()
{
    return mName;
}

//! @brief This function returns the name or typename (if name is empty)
//! Useful if display name has not been specified, then we use the type name
//! @returns A non-empty name
QString AppearanceData::getNonEmptyName()
{
    if (mName.isEmpty())
    {
        return mTypeName;
    }
    else
    {
        return mName;
    }
}

QString AppearanceData::getFullIconPath(graphicsType gfxType)
{
    if ( !mIconPathUser.isEmpty() && (gfxType == USERGRAPHICS) )
    {
        //Use user icon
        return mBasePath + mIconPathUser;
    }
    else if ( !mIconPathISO.isEmpty() && (gfxType == ISOGRAPHICS) )
    {
        //use iso icon
        return mBasePath + mIconPathISO;
    }
    else if ( mIconPathUser.isEmpty() && !mIconPathISO.isEmpty() )
    {
        //Want user icon but not available, use iso icon
        return mBasePath + mIconPathISO;
    }
    else if ( !mIconPathUser.isEmpty() && mIconPathISO.isEmpty() )
    {
        //Want ISO icon but not available, Use user icon
        return mBasePath + mIconPathUser;
    }
    else
    {
        //No icon available use som noname icon
        return "som noname file"; //!< @todo Fix this, Empty icon
    }
}

QString AppearanceData::getIconPathUser()
{
    return mIconPathUser;
}

QString AppearanceData::getIconPathISO()
{
    return mIconPathISO;
}

QString AppearanceData::getIconRotationBehaviour()
{
    return mIconRotationBehaviour;
}

QPointF AppearanceData::getNameTextPos()
{
    return mNameTextPos;
}


PortAppearanceMapT &AppearanceData::getPortAppearanceMap()
{
    return mPortAppearanceMap;
}


QString AppearanceData::getBasePath()
{
    return mBasePath;
}

void AppearanceData::readFromTextStream(QTextStream &rIs)
{
    QString command;
    QString lineStr;
    this->mIsReadOK = true; //Assume read will be ok, set to false if fail bellow

    while (!rIs.atEnd())
    {
        //! @todo need som error handling here if file stream has incorect data
        rIs >> command; //Read the command word

        if (command == "TYPENAME")
        {
            mTypeName = readName(rIs.readLine().trimmed());
        }
        else if (command == "DISPLAYNAME")
        {
            mName = readName(rIs.readLine().trimmed());
        }
        else if (command == "ISOICON")
        {
            mIconPathISO = readName(rIs.readLine().trimmed());
        }
        else if (command == "USERICON")
        {
            mIconPathUser = readName(rIs.readLine().trimmed());
        }
        else if (command == "ICONROTATION")
        {
            mIconRotationBehaviour = rIs.readLine().trimmed();
        }
        else if (command == "PORT")
        {
            lineStr = rIs.readLine();
            QTextStream portStream(&lineStr);
            QString portName=readName(portStream);
            if(portName == "")
            {
                qDebug() << "FEL I PORTNAMN";
                mIsReadOK = false;
            }

            PortAppearance portapp;

            if(portStream.atEnd())
            {
                qDebug() << "SAKNAS DATA";
                mIsReadOK = false;
            }
            portStream >> portapp.x;
            if(portStream.atEnd())
            {
                qDebug() << "SAKNAS DATA";
                mIsReadOK = false;
            }
            portStream >> portapp.y;
            if(portStream.atEnd())
            {
                qDebug() << "SAKNAS DATA";
                mIsReadOK = false;
            }
            portStream >> portapp.rot;

            if( (portapp.rot == 0) || (portapp.rot == 180) )
            {
                portapp.direction = LEFTRIGHT;
            }
            else
            {
                portapp.direction = TOPBOTTOM;
            }

            mPortAppearanceMap.insert(portName, portapp);
        }
        else if (command == "BASEPATH")
        {
            mBasePath = rIs.readLine().trimmed();
        }
        else
        {
            //If incorrect command discard rest of line, ignoring empty lines
            if (!command.isEmpty())
            {
                rIs.readLine().trimmed();
                //qDebug() << "appearanceData: Incorrect command: " + command;
            }
        }
    }

    //Check if read OK!
    //We must have at least a type name
    if (mTypeName.isEmpty())
    {
        mIsReadOK = false;
    }
}

void AppearanceData::setTypeName(QString name)
{
    mTypeName = name;
}

void AppearanceData::setName(QString name)
{
    mName = name;
}

void AppearanceData::setBasePath(QString path)
{
    mBasePath = path;
}

void AppearanceData::setIconPathUser(QString path)
{
    mIconPathUser = path;
}

void AppearanceData::setIconPathISO(QString path)
{
    mIconPathISO = path;
}

bool AppearanceData::haveIsoIcon()
{
    return !mIconPathISO.isEmpty();
}

bool AppearanceData::haveUserIcon()
{
    return !mIconPathUser.isEmpty();
}
