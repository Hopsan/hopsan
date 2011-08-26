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
//! @file   GUIModelObjectAppearance.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by ModelObjects and the LibraryWidget
//!
//$Id$

#include "qdebug.h"
#include "../MainWindow.h"
#include "GUIModelObjectAppearance.h"
#include "../Utilities/GUIUtilities.h"
#include "../version.h"

//These are only used in temporary save to file
#include <QFile>
#include "version.h"

// ========== Defines for load/save common strings ==========

#define CAF_VERSION "version"

#define CAF_TYPENAME "typename"
#define CAF_TYPE "type"
#define CAF_DISPLAYNAME "displayname"
#define CAF_NAME "name"

#define CAF_ICON "icon"
#define CAF_ICONS "icons"
#define CAF_PATH "path"
#define CAF_ICONROTATION "iconrotation"
#define CAF_SCALE "scale"

#define CAF_HELP "help"
#define CAF_HELPTEXT "text"
#define CAF_HELPPICTURE "picture"

#define CAF_PORTPOSITIONS "portpositions"
#define CAF_PORTPOSE "portpose"

// =============== Help Functions ===============

//! @brief Special purpose function for adding a Hopsan specific XML tag containing PortPose information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] name The port name
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
//! @param[in] th The orientaion (angle)
void appendPortPoseTag(QDomElement &rDomElement, QString name, qreal x, qreal y, qreal th)
{
    QDomElement pose = appendDomElement(rDomElement, CAF_PORTPOSE);
    pose.setAttribute(CAF_NAME,name);
    QString xString;
    xString.setNum(x,'f',20);
    pose.setAttribute("x",xString);
    QString yString;
    yString.setNum(y,'f',20);
    pose.setAttribute("y",yString);
    pose.setAttribute("a",th);
}

//! @brief Special purpose function for parsing a Hopsan specific XML tag containing PortPose information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rName The name of the port
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
//! @param[out] rTheta The orientaion (angle)
void parsePortPoseTag(QDomElement domElement, QString &rName, qreal &rX, qreal &rY, qreal &rTheta)
{
    rName = domElement.attribute(CAF_NAME);
    bool dummy;
    parsePoseTag(domElement, rX, rY, rTheta, dummy);
}


// =============================================

GUIModelObjectAppearance::GUIModelObjectAppearance()
{
    mIconUserRotationBehaviour = "ON";
    mIconIsoRotationBehaviour = "ON";
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
    return mDisplayName;
}

//! @brief This function returns the name or typename (if name is empty)
//! Useful if display name has not been specified, then we use the type name
//! @returns A non-empty name
QString GUIModelObjectAppearance::getNonEmptyName()
{
    if (mDisplayName.isEmpty())
    {
        return mTypeName;
    }
    else
    {
        return mDisplayName;
    }
}


QString GUIModelObjectAppearance::getHelpPicture()
{
    return mHelpPicture;
}


QString GUIModelObjectAppearance::getHelpText()
{
    return mHelpText;
}

//! @brief Get the full Icon path for specified graphics type
//! @param [in] gfxType The graphics type enum (ISO or USER)
//! If the specified type is missing, return the other type.
//! If that is also missing return a path to the missing graphics icon
QString GUIModelObjectAppearance::getFullAvailableIconPath(graphicsType gfxType)
{
    makeSurePathsAbsolute();

    QFileInfo iconUserFileInfo(QDir(mBasePath), mIconUserPath);
    QFileInfo iconISOFileInfo(QDir(mBasePath), mIconIsoPath);

    //qDebug() << "Type: " << mTypeName << " iconUser: " << iconUserFileInfo.absoluteFilePath();
    //qDebug() << "Type: " << mTypeName << " iconISO: " << iconISOFileInfo.absoluteFilePath();

    // We want USERICON and have USERICON
    if ( (gfxType == USERGRAPHICS) && iconUserFileInfo.isFile() )
    {
        //Use user icon
        return iconUserFileInfo.absoluteFilePath();
    }
    // We want ISOICON and have ISOICON
    else if ( (gfxType == ISOGRAPHICS) &&  iconISOFileInfo.isFile() )
    {
        //Use ISO icon
        return  iconISOFileInfo.absoluteFilePath();
    }
    // If what we requested does not exist but USER graphics do exist
    else if (iconUserFileInfo.isFile() && ! iconISOFileInfo.isFile() )
    {
        //Use user icon
        return iconUserFileInfo.absoluteFilePath();
    }
    // If what we requested does not exist but ISO graphics do exist
    else if ( !iconUserFileInfo.isFile() && iconISOFileInfo.isFile() )
    {
        //Use ISO icon
        return iconISOFileInfo.absoluteFilePath();
    }
    else
    {
        //No icon available use the missing graphics icon
        return QString(OBJECTICONPATH) + QString("missingcomponenticon.svg");
    }
}

//! @brief Returns the path to the graphics icon of requested type, regardles of wheter it is valid or not
QString GUIModelObjectAppearance::getIconPath(graphicsType gfxType)
{
    if (gfxType == USERGRAPHICS)
    {
        return mIconUserPath;
    }
    else if (gfxType == ISOGRAPHICS)
    {
        return mIconIsoPath;
    }
    else
    {
        return ""; //Invalid type
    }
}

qreal GUIModelObjectAppearance::getIconScale(const graphicsType gfxType)
{
    if (gfxType == USERGRAPHICS)
    {
        return mIconUserScale;
    }
    else if (gfxType == ISOGRAPHICS)
    {
        return mIconIsoScale;
    }
    else
    {
        return 1.0; //Invalid type
    }
}

void GUIModelObjectAppearance::makeSurePathsAbsolute()
{
    QFileInfo iconUserFileInfo(mIconUserPath);
    QFileInfo iconIsoFileInfo(mIconIsoPath);

    //Check if given filepath is absolute or relative, if relative assume relative to basepath, but don do anything if path empty (no icon specified)
    if (iconUserFileInfo.isRelative() && !mIconUserPath.isEmpty())
    {
        iconUserFileInfo.setFile(QDir(mBasePath), mIconUserPath);
        mIconUserPath = iconUserFileInfo.absoluteFilePath();
    }

    if (iconIsoFileInfo.isRelative() && !mIconIsoPath.isEmpty())
    {
        iconIsoFileInfo.setFile(QDir(mBasePath), mIconIsoPath);
        mIconIsoPath = iconIsoFileInfo.absoluteFilePath();
    }

    //!< @todo maybe more paths should be manipulated


}

void GUIModelObjectAppearance::makeSurePathsRelative()
{
    QFileInfo iconUserFileInfo(mIconUserPath);
    QFileInfo iconIsoFileInfo(mIconIsoPath);

    //Check if given filepath is absolute or relative, if absolute assume we want relative to basepath
    if (iconUserFileInfo.isAbsolute())
    {
        mIconUserPath = relativePath(iconUserFileInfo, QDir(mBasePath));
    }

    if (iconIsoFileInfo.isAbsolute())
    {
        mIconIsoPath = relativePath(iconIsoFileInfo, QDir(mBasePath));
    }

    //!< @todo maybe more paths should be manipulated

}


QString GUIModelObjectAppearance::getIconRotationBehaviour(const graphicsType gfxType)
{
    if (gfxType == USERGRAPHICS)
    {
        return mIconUserRotationBehaviour;
    }
    else if (gfxType == ISOGRAPHICS)
    {
        return mIconIsoRotationBehaviour;
    }
    else
    {
        //Incorrect type, return something, maybe give error instead
        return "ON";
    }
}

QPointF GUIModelObjectAppearance::getNameTextPos()
{
    return mNameTextPos;
}

//! @brief Returns a reference to the map containing port appearance
//! @returns Reference to mPortAppearanceMap
PortAppearanceMapT &GUIModelObjectAppearance::getPortAppearanceMap()
{
    return mPortAppearanceMap;
}

//! @brief Removes a port appearance post for a specified portname
//! @param[in] portName The port name for the port Appearance to be erased
void GUIModelObjectAppearance::erasePortAppearance(const QString portName)
{
    PortAppearanceMapT::iterator pait = mPortAppearanceMap.find(portName);
    if (pait != mPortAppearanceMap.end())
    {
        mPortAppearanceMap.erase(pait);
    }
    else
    {
        qDebug() << "GUIModelObjectAppearance, ERROR: specified portappearance could not be found in the map: " << portName;
    }
}

//! @brief Adds or updates a port appearance post for a specified portname
//! @param[in] portName The port name for the port Appearance to be added
//! @param[in] pPortAppearance A pointer to the port Appearance to add, if 0 then a new undefined appearance will be created
void GUIModelObjectAppearance::addPortAppearance(const QString portName, GUIPortAppearance *pPortAppearance)
{
    if (pPortAppearance == 0)
    {
        mPortAppearanceMap.insert(portName, GUIPortAppearance());
    }
    else
    {
        mPortAppearanceMap.insert(portName, *pPortAppearance);
    }
}

//! @brief Get the base path that all icon paths are relative
QString GUIModelObjectAppearance::getBasePath()
{
    return mBasePath;
}

//! @brief Read the ModelObjectAppearance contents from an XML DOM Element
void GUIModelObjectAppearance::readFromDomElement(QDomElement domElement)
{
    //! @todo we should not overwrite existing data if xml file is missing data, that is dont overwrite with null
    mTypeName       = domElement.attribute(HMF_TYPETAG);
    mDisplayName    = domElement.attribute(CAF_DISPLAYNAME);

    QDomElement xmlHelp = domElement.firstChildElement(CAF_HELP);
    if(!xmlHelp.isNull())
    {
        mHelpPicture    = xmlHelp.attribute(CAF_HELPPICTURE);
        mHelpText       = xmlHelp.attribute(CAF_HELPTEXT);
    }

    //We assume only one icons element
    QDomElement xmlIcons = domElement.firstChildElement(CAF_ICONS);
    QDomElement xmlIcon = xmlIcons.firstChildElement(CAF_ICON);
    while (!xmlIcon.isNull())
    {
        QString type = xmlIcon.attribute(CAF_TYPE);
        if (type == "iso")
        {
            mIconIsoPath = xmlIcon.attribute(CAF_PATH);
            mIconIsoScale = parseAttributeQreal(xmlIcon, CAF_SCALE, 1.0);
            mIconIsoRotationBehaviour = xmlIcon.attribute(CAF_ICONROTATION);
        }
        else //!< @todo maybe elseif user and somehow give error message, for now assume user if not iso
        {
            mIconUserPath = xmlIcon.attribute(CAF_PATH);
            mIconUserScale = parseAttributeQreal(xmlIcon, CAF_SCALE, 1.0);
            mIconUserRotationBehaviour = xmlIcon.attribute(CAF_ICONROTATION);
        }

        xmlIcon = xmlIcon.nextSiblingElement("icon");
    }


    this->makeSurePathsAbsolute();

    QString portname;
    QDomElement xmlPorts = domElement.firstChildElement(CAF_PORTPOSITIONS);
    while (!xmlPorts.isNull())
    {
        QDomElement xmlPortPose = xmlPorts.firstChildElement(CAF_PORTPOSE);
        while (!xmlPortPose.isNull())
        {
            GUIPortAppearance portApp;
            parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
            mPortAppearanceMap.insert(portname, portApp);
            xmlPortPose = xmlPortPose.nextSiblingElement(CAF_PORTPOSE);
        }
        // There should only be one <ports>, but lets check for more just in case
        xmlPorts = xmlPorts.nextSiblingElement(CAF_PORTPOSITIONS);
    }

    // vvvvvvvvvvvvvvvvvvvvv=== Bellow Reads old Format Tags ===vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

    // Read old style icons
    QDomElement xmlIcon2 = domElement.firstChildElement("icon");
    if (!xmlIcon2.isNull())
    {
        mIconIsoPath = xmlIcon2.attribute("isopath");
        mIconUserPath = xmlIcon2.attribute("userpath");
        mIconIsoRotationBehaviour = xmlIcon2.attribute("iconrotation");
        mIconUserRotationBehaviour = xmlIcon2.attribute("iconrotation");
        mIconUserScale = parseAttributeQreal(xmlIcon2, "userscale", 1.0);
        mIconIsoScale = parseAttributeQreal(xmlIcon2, "isoscale", 1.0);
    }

    // Read old style portposes, where portposes were not contained inside a common "ports" element
    QDomElement xmlPortPose = domElement.firstChildElement(CAF_PORTPOSE);
    while (!xmlPortPose.isNull())
    {
        GUIPortAppearance portApp;
        parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
        mPortAppearanceMap.insert(portname, portApp);
        xmlPortPose = xmlPortPose.nextSiblingElement(CAF_PORTPOSE);
    }
}

//! @brief Writes the ModelObjectAppearance contents to an XML DOM Element
//! @param rDomElement The DOM element to write to
void GUIModelObjectAppearance::saveToDomElement(QDomElement &rDomElement)
{
    // Save type and name data
    QDomElement xmlObject = appendDomElement(rDomElement, CAF_MODELOBJECT);
    xmlObject.setAttribute(CAF_TYPENAME, mTypeName);
    xmlObject.setAttribute(CAF_DISPLAYNAME, mDisplayName);

    //  Save icon data
    this->makeSurePathsRelative(); //We want to save paths relative the basepath, to avoid incompatibility with absolute paths between systems
    QDomElement xmlIcons = appendDomElement(xmlObject, CAF_ICONS);
    if (hasIcon(USERGRAPHICS))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, CAF_ICON);
        xmlIcon.setAttribute(CAF_TYPE, "user");
        xmlIcon.setAttribute(CAF_PATH, mIconUserPath);
        xmlIcon.setAttribute(CAF_SCALE, mIconUserScale);
        xmlIcon.setAttribute(CAF_ICONROTATION, mIconUserRotationBehaviour);
    }
    if (hasIcon(ISOGRAPHICS))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, CAF_ICON);
        xmlIcon.setAttribute(CAF_TYPE, "iso");
        xmlIcon.setAttribute(CAF_PATH, mIconIsoPath);
        xmlIcon.setAttribute(CAF_SCALE, mIconIsoScale);
        xmlIcon.setAttribute(CAF_ICONROTATION, mIconIsoRotationBehaviour);
    }

    // Save help text and picture data
    if(!mHelpText.isNull() || !mHelpPicture.isNull())
    {
        QDomElement xmlHelp = appendDomElement(xmlObject, CAF_HELP);
        if(!mHelpText.isNull())
            xmlHelp.setAttribute(CAF_HELPTEXT, mHelpText);
        if(!mHelpPicture.isNull())
            xmlHelp.setAttribute(CAF_HELPPICTURE, mHelpPicture);
    }

    // Save port psoition data
    QDomElement xmlPortPositions = appendDomElement(rDomElement, CAF_PORTPOSITIONS);
    PortAppearanceMapT::iterator pit;
    for (pit=mPortAppearanceMap.begin(); pit!=mPortAppearanceMap.end(); ++pit)
    {
        appendPortPoseTag(xmlPortPositions, pit.key(), pit.value().x, pit.value().y, pit.value().rot);
    }

    this->makeSurePathsAbsolute(); //Switch back to absolute paths
}

//! @brief Save Appearancedata to XML file, currently used as a test function
void GUIModelObjectAppearance::saveToXMLFile(QString filename)
{
    //Save to file
    QDomDocument doc;
    QDomElement cafroot = doc.createElement(CAF_ROOT);
    doc.appendChild(cafroot);
    cafroot.setAttribute(CAF_VERSION, CAFVERSION);
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


//! @brief Access method to manually set the TypeName
void GUIModelObjectAppearance::setTypeName(QString name)
{
    mTypeName = name;
}

//! @brief Access method to manually set the Name
void GUIModelObjectAppearance::setName(QString name)
{
    mDisplayName = name;
}

//! @brief Access method to manually set the HelpText
void GUIModelObjectAppearance::setHelpText(QString text)
{
    mHelpText = text;
}

//! @brief Access method to manually set the BaseIconPath
void GUIModelObjectAppearance::setBasePath(QString path)
{
    //Set new base path,
    //We need to make sure that paths are absolute before changing this or else ther relative path will be wrong
    this->makeSurePathsAbsolute();
    mBasePath = path;
}

//! @brief Access method to manually set the BasePath relative UserIconPath
void GUIModelObjectAppearance::setIconPath(QString path, graphicsType gfxType)
{
    if (gfxType == USERGRAPHICS)
    {
        mIconUserPath = path;
    }
    else if (gfxType == ISOGRAPHICS)
    {
        mIconIsoPath = path;
    }
    //else dont do anything
    this->makeSurePathsAbsolute();
}


//! @brief Check if specified Icon path is availiable and icon exists
bool GUIModelObjectAppearance::hasIcon(const graphicsType gfxType)
{
    this->makeSurePathsAbsolute();

    if (gfxType == ISOGRAPHICS)
    {
        QFileInfo iso(mIconIsoPath);
        return iso.isFile();
    }
    else if (gfxType == USERGRAPHICS)
    {
        QFileInfo user(mIconUserPath);
        return user.isFile();
    }
    else
    {
        return false;
    }
}
