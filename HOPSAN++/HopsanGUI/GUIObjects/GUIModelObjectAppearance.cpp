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

#include "GUIModelObjectAppearance.h"
#include "MainWindow.h"
#include "Utilities/GUIUtilities.h"
#include "version_gui.h"

//These are only used in temporary save to file
#include <QFile>

// ========== Defines for load/save common strings ==========
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

ModelObjectIconAppearance::ModelObjectIconAppearance()
{
    mScale = 1.0;
    mRotationBehaviour = "ON";
}

GUIModelObjectAppearance::GUIModelObjectAppearance()
{
    mPortAppearanceMap.clear();
    mDefaultMissingIconPath = "missingcomponenticon.svg";
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
    QFileInfo iconUserFileInfo(mUserIconAppearance.mAbsolutePath);
    QFileInfo iconISOFileInfo(mIsoIconAppearance.mAbsolutePath);

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
        //No icon available use the default missing icon
        return QString(OBJECTICONPATH) + mDefaultMissingIconPath;
    }
}

//! @brief Returns the path to the graphics icon of requested type, regardles of wheter it is valid or not
QString GUIModelObjectAppearance::getIconPath(const graphicsType gfxType, const AbsoluteRelativeT absrel)
{
    if (gfxType == USERGRAPHICS)
    {
        if (absrel == ABSOLUTE)
        {
            return mUserIconAppearance.mAbsolutePath;
        }
        else
        {
            return mUserIconAppearance.mRelativePath;
        }
    }
    else if (gfxType == ISOGRAPHICS)
    {
        if (absrel == ABSOLUTE)
        {
            return mIsoIconAppearance.mAbsolutePath;
        }
        else
        {
            return mIsoIconAppearance.mRelativePath;
        }
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
        return mUserIconAppearance.mScale;
    }
    else if (gfxType == ISOGRAPHICS)
    {
        return mIsoIconAppearance.mScale;
    }
    else
    {
        return 1.0; //Invalid type
    }
}

void GUIModelObjectAppearance::setRelativePathFromAbsolute()
{
    QFileInfo absUserPath(mUserIconAppearance.mAbsolutePath);
    QFileInfo absIsoPath(mIsoIconAppearance.mAbsolutePath);

    //Check if given filepath is absolute or relative, if absolute assume we want relative to basepath
    if (absUserPath.isAbsolute())
    {
        mUserIconAppearance.mRelativePath = relativePath(absUserPath, QDir(mBasePath));
    }

    if (absIsoPath.isAbsolute())
    {
        mIsoIconAppearance.mRelativePath = relativePath(absIsoPath, QDir(mBasePath));
    }
}

void GUIModelObjectAppearance::setAbsoultePathFromRelative()
{
    QFileInfo relUserPath(mUserIconAppearance.mRelativePath);
    QFileInfo relIsoPath(mIsoIconAppearance.mRelativePath);

    //Check if given filepath is absolute or relative, if relative assume relative to basepath, but don do anything if path empty (no icon specified)
    if (!mUserIconAppearance.mRelativePath.isEmpty())
    {
        relUserPath.setFile(QDir(mBasePath), mUserIconAppearance.mRelativePath);
        mUserIconAppearance.mAbsolutePath = relUserPath.absoluteFilePath();
    }

    if (!mIsoIconAppearance.mRelativePath.isEmpty())
    {
        relIsoPath.setFile(QDir(mBasePath), mIsoIconAppearance.mRelativePath);
        mIsoIconAppearance.mAbsolutePath = relIsoPath.absoluteFilePath();
    }

}


QString GUIModelObjectAppearance::getIconRotationBehaviour(const graphicsType gfxType)
{
    if (gfxType == USERGRAPHICS)
    {
        return mUserIconAppearance.mRotationBehaviour;
    }
    else if (gfxType == ISOGRAPHICS)
    {
        return mIsoIconAppearance.mRotationBehaviour;
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
void GUIModelObjectAppearance::addPortAppearance(const QString portName, PortAppearance *pPortAppearance)
{
    if (pPortAppearance == 0)
    {
        mPortAppearanceMap.insert(portName, PortAppearance());
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
    mTypeName       = domElement.attribute(CAF_TYPENAME);
    mDisplayName    = domElement.attribute(CAF_DISPLAYNAME, mTypeName);

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
            mIsoIconAppearance.mRelativePath = xmlIcon.attribute(CAF_PATH);
            mIsoIconAppearance.mScale = parseAttributeQreal(xmlIcon, CAF_SCALE, 1.0);
            mIsoIconAppearance.mRotationBehaviour = xmlIcon.attribute(CAF_ICONROTATION);
        }
        else if (type == "user")
        {
            mUserIconAppearance.mRelativePath = xmlIcon.attribute(CAF_PATH);
            mUserIconAppearance.mScale = parseAttributeQreal(xmlIcon, CAF_SCALE, 1.0);
            mUserIconAppearance.mRotationBehaviour = xmlIcon.attribute(CAF_ICONROTATION);
        }
        else if (type == "defaultmissing")
        {
            //! @todo maye have a DefaultIconAppearance object to, an load all data
            mDefaultMissingIconPath = xmlIcon.attribute(CAF_PATH);
        }
        //else ignore, maybe should give warning

        xmlIcon = xmlIcon.nextSiblingElement(CAF_ICON);
    }

    QString portname;
    QDomElement xmlPorts = domElement.firstChildElement(CAF_PORTPOSITIONS);
    while (!xmlPorts.isNull())
    {
        QDomElement xmlPortPose = xmlPorts.firstChildElement(CAF_PORTPOSE);
        while (!xmlPortPose.isNull())
        {
            PortAppearance portApp;
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
        mIsoIconAppearance.mRelativePath = xmlIcon2.attribute("isopath");
        mUserIconAppearance.mRelativePath = xmlIcon2.attribute("userpath");
        mIsoIconAppearance.mRotationBehaviour = xmlIcon2.attribute("iconrotation");
        mUserIconAppearance.mRotationBehaviour = xmlIcon2.attribute("iconrotation");
        mUserIconAppearance.mScale = parseAttributeQreal(xmlIcon2, "userscale", 1.0);
        mIsoIconAppearance.mScale = parseAttributeQreal(xmlIcon2, "isoscale", 1.0);
    }

    // Read old style portposes, where portposes were not contained inside a common "ports" element
    QDomElement xmlPortPose = domElement.firstChildElement(CAF_PORTPOSE);
    while (!xmlPortPose.isNull())
    {
        PortAppearance portApp;
        parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
        mPortAppearanceMap.insert(portname, portApp);
        xmlPortPose = xmlPortPose.nextSiblingElement(CAF_PORTPOSE);
    }

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    //Now set absolute paths from relative and basepath
    setAbsoultePathFromRelative();
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
    QDomElement xmlIcons = appendDomElement(xmlObject, CAF_ICONS);
    if (hasIcon(USERGRAPHICS))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, CAF_ICON);
        xmlIcon.setAttribute(CAF_TYPE, "user");
        xmlIcon.setAttribute(CAF_PATH, mUserIconAppearance.mRelativePath);
        xmlIcon.setAttribute(CAF_SCALE, mUserIconAppearance.mScale);
        xmlIcon.setAttribute(CAF_ICONROTATION, mUserIconAppearance.mRotationBehaviour);
    }
    if (hasIcon(ISOGRAPHICS))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, CAF_ICON);
        xmlIcon.setAttribute(CAF_TYPE, "iso");
        xmlIcon.setAttribute(CAF_PATH, mIsoIconAppearance.mRelativePath);
        setQrealAttribute(xmlIcon, CAF_SCALE, mIsoIconAppearance.mScale);
        xmlIcon.setAttribute(CAF_ICONROTATION, mIsoIconAppearance.mRotationBehaviour);
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
}

//! @brief Save Appearancedata to XML file, currently used as a test function
void GUIModelObjectAppearance::saveToXMLFile(QString filename)
{
    //Save to file
    QDomDocument doc;
    QDomElement cafroot = doc.createElement(CAF_ROOT);
    doc.appendChild(cafroot);
    cafroot.setAttribute(CAF_VERSION, CAF_VERSIONNUM);
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
    mBasePath = path;
    setRelativePathFromAbsolute(); //Reset relative path to new basepath
}

//! @brief Access method to manually set the BasePath relative UserIconPath
void GUIModelObjectAppearance::setIconPath(const QString path, const graphicsType gfxType, const AbsoluteRelativeT absrel)
{

    if (absrel == ABSOLUTE)
    {
        if (gfxType == USERGRAPHICS)
        {
            mUserIconAppearance.mAbsolutePath = path;
        }
        else if (gfxType == ISOGRAPHICS)
        {
            mIsoIconAppearance.mAbsolutePath = path;
        }
        setRelativePathFromAbsolute();
    }
    else
    {
        if (gfxType == USERGRAPHICS)
        {
            mUserIconAppearance.mRelativePath = path;
        }
        else if (gfxType == ISOGRAPHICS)
        {
            mIsoIconAppearance.mRelativePath = path;
        }
        setAbsoultePathFromRelative();
    }
    //else dont do anything
}

void GUIModelObjectAppearance::setIconScale(const qreal scale, const graphicsType gfxType)
{
    if (gfxType == USERGRAPHICS)
    {
        mUserIconAppearance.mScale = scale;
    }
    else if (gfxType == ISOGRAPHICS)
    {
        mIsoIconAppearance.mScale = scale;
    }
    //else dont do anything
}


//! @brief Check if specified Icon path is availiable and icon exists
bool GUIModelObjectAppearance::hasIcon(const graphicsType gfxType)
{
    if (gfxType == ISOGRAPHICS)
    {
        QFileInfo iso(mIsoIconAppearance.mAbsolutePath);
        return iso.isFile();
    }
    else if (gfxType == USERGRAPHICS)
    {
        QFileInfo user(mUserIconAppearance.mAbsolutePath);
        return user.isFile();
    }
    else
    {
        return false;
    }
}
