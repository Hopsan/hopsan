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
//!
//$Id$

#include "AppearanceData.h"
#include "qdebug.h"
#include "GUIUtilities.h"

GUIConnectorAppearance::GUIConnectorAppearance(QString type, bool useISO)
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
    setTypeAndIsoStyle(type, useISO);     //Need to use set type instead of setting directly as setType narrows types down to power or signal
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

void GUIConnectorAppearance::setIsoStyle(bool useISO)
{
    mUseISOStyle = useISO;
}

void GUIConnectorAppearance::setTypeAndIsoStyle(QString porttype, bool useISO)
{
    setType(porttype);
    setIsoStyle(useISO);
}

QPen GUIConnectorAppearance::getPen(QString situation)
{
    return getPen(situation, mConnectorType, mUseISOStyle);
}

//! Get function for primary pen style
//! @todo Hardcoded appearance stuff (should maybe be loaded from external file (not prio 1)
QPen GUIConnectorAppearance::getPen(QString situation, QString type, bool useISO)
{
    //! @todo store pens in some smarter way, maybe in an array where situation and type are enums or used to calculate index, will be necessary for larger variations as a mega if else code is madness
    if(situation == "Primary")
    {
        if(type == "Power")
        {
            if (useISO)
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
            if (useISO)
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
            if (useISO)
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
            if (useISO)
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
            if (useISO)
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
            if (useISO)
            {
                return mHoverPenSignalIso;
            }
            else
            {
                return mHoverPenSignalUser;
            }
        }
    }
    else if(situation == "NonFinished")
    {
        return mNonFinishedPen;
    }

    qDebug() << "ERROR no such connector appearance: " << situation << " " <<  type << " ISOstyle: " << useISO << "   DONT WORRY ABOUT THIS ERROR WILL FIX LATER, /Peter";

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
    iconPath.clear();
    iconPath.append("../../HopsanGUI/porticons/"); //!< @todo Not very goood to have this hardcoded everywhere (should be decidet on runtime or something or at least be global or defined)
    if (nodetype == "NodeSignal")
    {
        iconPath.append("SignalPort");
        if ( porttype == "READPORT")
        {
            iconPath.append("_read");
        }
        else
        {
            iconPath.append("_write");
        }
    }
    else if (nodetype == "NodeMechanic")
    {
        iconPath.append("MechanicPort");
        if (CQSType == "C")
        {
            iconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            iconPath.append("Q");
        }
    }
    else if (nodetype == "NodeHydraulic")
    {
        iconPath.append("HydraulicPort");
        if (CQSType == "C")
        {
            iconPath.append("C");
        }
        else if (CQSType == "Q")
        {
            iconPath.append("Q");
        }
    }
    else
    {
        //SystemPort is a blank port (that is why we use it here)
        iconPath.append("SystemPort");
    }
    iconPath.append(".svg");
}

AppearanceData::AppearanceData()
{
    //Assume all strings default to ""
    mPortAppearanceMap.clear();
}


QTextStream& operator >>(QTextStream &is, AppearanceData &rData)
{
    //! @todo handle returned error indication
    bool sucess = rData.setAppearanceData(is);
    sucess = sucess;
    return is;
}

QTextStream& operator <<(QTextStream &os, AppearanceData &rData)
{
    //! @todo maybe write header here (probaly not a good place, better somewhere else)
    //! @todo find out how to make newline in qt instead of "\n"
    os << "TYPENAME " << addQuotes(rData.mTypeName) << "\n";
    os << "DISPLAYNAME " << addQuotes(rData.mName) << "\n";
    //os << "BASEPATH " << rData.getBasePath() << "\n"; //Base path is computer dependant
    if (!rData.mIconPathISO.isEmpty())
    {
        os << "ISOICON " << rData.mIconPathISO << "\n";
    }
    if (!rData.mIconPathUser.isEmpty())
    {
        os << "USERICON " << rData.mIconPathUser << "\n";
    }
    if (!rData.mIconRotationBehaviour.isEmpty())
    {
        os << "ICONROTATION " << rData.mIconRotationBehaviour << "\n";
    }

    PortAppearanceMapT::iterator it;
    for (it = rData.mPortAppearanceMap.begin(); it != rData.mPortAppearanceMap.end(); ++it)
    {
        os << "PORT " << " "
           << addQuotes(it.key()) << " "
           << it.value().x << " "
           << it.value().y << " "
           << it.value().rot << "\n";
    }
    return os;
}

QString AppearanceData::getTypeName()
{
    return mTypeName;
}

QString AppearanceData::getName()
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

QString AppearanceData::getFullIconPath(bool useIso)
{
    if ( !mIconPathUser.isEmpty() && !useIso )
    {
        //Use user icon
        return mBasePath + mIconPathUser;
    }
    else if ( !mIconPathISO.isEmpty() && useIso )
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

bool AppearanceData::setAppearanceData(QTextStream &is)
{
    QString command;
    QString lineStr;
    bool sucess = true;

    while (!is.atEnd())
    {
        //! @todo make sure we dont read any file hader if that exist # in the begining
        //! @todo need som error handling here if file stream has incorect data
        is >> command; //Read the command word

        if (command == "TYPENAME")
        {
            mTypeName = readName(is.readLine().trimmed());
        }
        else if (command == "DISPLAYNAME")
        {
            mName = readName(is.readLine().trimmed());
        }
        else if (command == "ISOICON")
        {
            mIconPathISO = is.readLine().trimmed();
        }
        else if (command == "USERICON")
        {
            mIconPathUser = is.readLine().trimmed();
        }
        else if (command == "ICONROTATION")
        {
            mIconRotationBehaviour = is.readLine().trimmed();
        }
        else if (command == "PORT")
        {
            lineStr = is.readLine();
            QTextStream portStream(&lineStr);
            QString portName=readName(portStream);
            if(portName == "")
            {
                qDebug() << "FEL I PORTNAMN";
                return false;
            }

            PortAppearance portapp;

            if(portStream.atEnd())
            {
                qDebug() << "SAKNAS DATA";
                return false;
            }
            portStream >> portapp.x;
            if(portStream.atEnd())
            {
                qDebug() << "SAKNAS DATA";
                return false;
            }
            portStream >> portapp.y;
            if(portStream.atEnd())
            {
                qDebug() << "SAKNAS DATA";
                return false;
            }
            portStream >> portapp.rot;

            if( (portapp.rot == 0) || (portapp.rot == 180) )
            {
                portapp.direction = PortAppearance::HORIZONTAL;
            }
            else
            {
                portapp.direction = PortAppearance::VERTICAL;
            }

            mPortAppearanceMap.insert(portName, portapp);
        }
        else if (command == "BASEPATH")
        {
            mBasePath = is.readLine().trimmed();
        }
        else
        {
            //Ignore empty lines
            if (!command.isEmpty())
            {
                qDebug() << "appearanceData: Incorrect command: " + command;
                sucess = false;
            }
        }
    }
    return sucess;
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
