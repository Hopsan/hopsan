//!
//! @file   AppearanceData.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by guiobjects and library widget
//! @todo Also contains some other appearance stuff that maybe should not be in this fil
//!
//$Id$

#include "qdebug.h"
#include "GUIModelObjectAppearance.h"
#include "../Utilities/GUIUtilities.h"
#include "../version.h"

GUIModelObjectAppearance::GUIModelObjectAppearance()
{
    //Assume all strings default to ""
    mPortAppearanceMap.clear();
}

//! @brief get the type-name
//! @returns The type-name
QString GUIModelObjectAppearance::getTypeName()
{
    return mTypeName;
}

//! @brief get the display name, even if it is empty
//! @returns The display name
QString GUIModelObjectAppearance::getName()
{
    return mName;
}

//! @brief This function returns the name or typename (if name is empty)
//! Useful if display name has not been specified, then we use the type name
//! @returns A non-empty name
QString GUIModelObjectAppearance::getNonEmptyName()
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

QString GUIModelObjectAppearance::getFullIconPath(graphicsType gfxType)
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
        return COMPONENTPATH + QString("missingcomponenticon.svg");
    }
}

QString GUIModelObjectAppearance::getIconPathUser()
{
    return mIconPathUser;
}

QString GUIModelObjectAppearance::getIconPathISO()
{
    return mIconPathISO;
}

QString GUIModelObjectAppearance::getIconRotationBehaviour()
{
    return mIconRotationBehaviour;
}

QPointF GUIModelObjectAppearance::getNameTextPos()
{
    return mNameTextPos;
}


PortAppearanceMapT &GUIModelObjectAppearance::getPortAppearanceMap()
{
    return mPortAppearanceMap;
}


QString GUIModelObjectAppearance::getBasePath()
{
    return mBasePath;
}

void GUIModelObjectAppearance::readFromTextStream(QTextStream &rIs)
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

            GUIPortAppearance portapp;

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

//            if( (portapp.rot == 0) || (portapp.rot == 180) )
//            {
//                portapp.direction = LEFTRIGHT;
//            }
//            else
//            {
//                portapp.direction = TOPBOTTOM;
//            }

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

    this->saveToXML("caf"); //only test function to test savetoxml
}


void GUIModelObjectAppearance::readFromDomElement(QDomElement domElement)
{
//    mTypeName       = rDomElement.firstChildElement(HMF_TYPETAG).text();
//    mName           = rDomElement.firstChildElement(HMF_DISPLAYNAMETAG).text();
//    mIconPathISO    = rDomElement.firstChildElement(HMF_ISOICONTAG).text();
//    mIconPathUser   = rDomElement.firstChildElement(HMF_USERICONTAG).text();
//    mIconRotationBehaviour = rDomElement.firstChildElement(HMF_ICONROTATIONTAG).text();

//    QString portname;
//    QDomElement xmlPortPose = rDomElement.firstChildElement(HMF_PORTPOSETAG);
//    while (!xmlPortPose.isNull())
//    {
//        GUIPortAppearance portApp;
////        parseDomValueNode3(xmlPort.firstChildElement(HMF_POSETAG), portApp.x, portApp.y, portApp.rot);
////        mPortAppearanceMap.insert(xmlPort.firstChildElement(HMF_NAMETAG).text(), portApp);
////        QString portname = xmlPort.attribute(HMF_NAMETAG);
////        parsePoseTag(xmlPort.firstChildElement(HMF_POSETAG), portApp.x, portApp.y, portApp.rot);

//        parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
//        mPortAppearanceMap.insert(portname, portApp);
//        xmlPortPose = xmlPortPose.nextSiblingElement(HMF_PORTPOSETAG);
//    }

    //! @todo we should not overwrite existing data if xml file is missing data, that is dont overwrite with null
    mTypeName       = domElement.attribute(HMF_TYPETAG);
    mName           = domElement.attribute(HMF_DISPLAYNAMETAG);

    QDomElement xmlIcon = domElement.firstChildElement("icon");
    mIconPathISO    = xmlIcon.attribute("isopath");
    mIconPathUser   = xmlIcon.attribute("userpath");
    mIconRotationBehaviour = xmlIcon.attribute("iconrotation");

    QString portname;
    QDomElement xmlPortPose = domElement.firstChildElement(HMF_PORTPOSETAG);
    while (!xmlPortPose.isNull())
    {
        GUIPortAppearance portApp;
        parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
        mPortAppearanceMap.insert(portname, portApp);
        xmlPortPose = xmlPortPose.nextSiblingElement(HMF_PORTPOSETAG);
    }

     this->mIsReadOK = true; //Assume read will be ok
     //! @todo maybe remove this in xml load
}


void GUIModelObjectAppearance::saveToDomElement(QDomElement &rDomElement)
{
//    appendDomTextNode(rDomElement, HMF_TYPETAG, mTypeName);
//    appendDomTextNode(rDomElement, HMF_DISPLAYNAMETAG, mName);
//    appendDomTextNode(rDomElement, HMF_ISOICONTAG, mIconPathISO);
//    appendDomTextNode(rDomElement, HMF_USERICONTAG, mIconPathUser);
//    appendDomTextNode(rDomElement, HMF_ICONROTATIONTAG, mIconRotationBehaviour);

//    PortAppearanceMapT::iterator pit;
//    for (pit=mPortAppearanceMap.begin(); pit!=mPortAppearanceMap.end(); ++pit)
//    {
////        QDomElement xmlPort = appendDomElement(rDomElement,HMF_PORTPOSETAG);
////        appendDomTextNode(xmlPort, HMF_NAMETAG, pit.key());
////        appendDomValueNode3(xmlPort, HMF_POSETAG, pit.value().x, pit.value().y, pit.value().rot);
////        xmlPort.setAttribute(HMF_NAMETAG, pit.key());
//        appendPortPoseTag(rDomElement, pit.key(), pit.value().x, pit.value().y, pit.value().rot);
//    }

    //! @todo not use hardcoded strings here
    QDomElement xmlObject = appendDomElement(rDomElement, "modelobject");
    xmlObject.setAttribute(HMF_TYPETAG, mTypeName);
    xmlObject.setAttribute(HMF_DISPLAYNAMETAG, mName);
    QDomElement xmlIcon = appendDomElement(xmlObject, "icon");
    xmlIcon.setAttribute("isopath", mIconPathISO);
    xmlIcon.setAttribute("userpath", mIconPathUser);
    xmlIcon.setAttribute("iconrotation", mIconRotationBehaviour);

    PortAppearanceMapT::iterator pit;
    for (pit=mPortAppearanceMap.begin(); pit!=mPortAppearanceMap.end(); ++pit)
    {
        appendPortPoseTag(xmlObject, pit.key(), pit.value().x, pit.value().y, pit.value().rot);
    }
}

//! @brief Temporary hack to test xml appearancedata
void GUIModelObjectAppearance::saveToXML(QString filename)
{
    //Save to file
    #include <QFile>
    #include "version.h"
    QDomDocument doc;
    QDomElement cafroot = doc.createElement("componentappearancefile");
    doc.appendChild(cafroot);
    cafroot.setAttribute("version", CAFVERSION);
    this->saveToDomElement(cafroot);
    const int IndentSize = 4;
    QFile xml(filename);
    if (!xml.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << xml.fileName();
        return;
    }
    QTextStream out(&xml);
    appendRootXMLProcessingInstruction(doc); //The xml "comment" on the first line
    doc.save(out, IndentSize);
    xml.close();
}



void GUIModelObjectAppearance::setTypeName(QString name)
{
    mTypeName = name;
}

void GUIModelObjectAppearance::setName(QString name)
{
    mName = name;
}

void GUIModelObjectAppearance::setBasePath(QString path)
{
    mBasePath = path;
}

void GUIModelObjectAppearance::setIconPathUser(QString path)
{
    mIconPathUser = path;
}

void GUIModelObjectAppearance::setIconPathISO(QString path)
{
    mIconPathISO = path;
}

bool GUIModelObjectAppearance::haveIsoIcon()
{
    return !mIconPathISO.isEmpty();
}

bool GUIModelObjectAppearance::haveUserIcon()
{
    return !mIconPathUser.isEmpty();
}
