//!
//! @file   GUIModelObjectAppearance.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by ModelObjects and the LibraryWidget
//!
//$Id$

#include "qdebug.h"
#include "GUIModelObjectAppearance.h"
#include "../Utilities/GUIUtilities.h"
#include "../version.h"

//These are only used in temporary save to file
#include <QFile>
#include "version.h"

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
//! If that is also missing retunr a path to the missing graphics icon
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
        return OBJECTICONPATH + QString("missingcomponenticon.svg");
    }
}

//! @brief Returns the (not full) path to the user graphics icon
QString GUIModelObjectAppearance::getIconPathUser()
{
    return mIconPathUser;
}

//! @brief Returns the (not full) path to the ISO graphics icon
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

//! @brief Returns a reference to the map containing port appearance
//! @returns Reference to mPortAppearanceMap
PortAppearanceMapT &GUIModelObjectAppearance::getPortAppearanceMap()
{
    return mPortAppearanceMap;
}

//! @brief Get the base path that all icon paths are relative
QString GUIModelObjectAppearance::getBaseIconPath()
{
    return mBasePath;
}

//! @brief Read the ModelObjectAppearance contents from an XML DOM Element
void GUIModelObjectAppearance::readFromDomElement(QDomElement domElement)
{
    //! @todo we should not overwrite existing data if xml file is missing data, that is dont overwrite with null
    mTypeName       = domElement.attribute(HMF_TYPETAG);
    mName           = domElement.attribute(HMF_DISPLAYNAMETAG);

    QDomElement xmlHelp = domElement.firstChildElement("help");
    if(!xmlHelp.isNull())
    {
        mHelpPicture    = xmlHelp.attribute("picture");
        mHelpText       = xmlHelp.attribute("text");
    }

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
}

//! @brief Writes the ModelObjectAppearance contents to an XML DOM Element
//! @param rDomElement The DOM element to write to
void GUIModelObjectAppearance::saveToDomElement(QDomElement &rDomElement)
{
    //! @todo not use hardcoded strings here
    QDomElement xmlObject = appendDomElement(rDomElement, "modelobject");
    xmlObject.setAttribute(HMF_TYPETAG, mTypeName);
    xmlObject.setAttribute(HMF_DISPLAYNAMETAG, mName);
    QDomElement xmlIcon = appendDomElement(xmlObject, "icon");
    xmlIcon.setAttribute("isopath", mIconPathISO);
    xmlIcon.setAttribute("userpath", mIconPathUser);
    xmlIcon.setAttribute("iconrotation", mIconRotationBehaviour);
    if(!mHelpText.isNull() || !mHelpPicture.isNull())
    {
        QDomElement xmlHelp = appendDomElement(xmlObject, "help");
        if(!mHelpText.isNull())
            xmlHelp.setAttribute("text", mHelpText);
        if(!mHelpPicture.isNull())
            xmlHelp.setAttribute("picture", mHelpPicture);
    }

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
    QDomDocument doc;
    QDomElement cafroot = doc.createElement(CAF_ROOTTAG);
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


//! @brief Access method to manually set the TypeName
void GUIModelObjectAppearance::setTypeName(QString name)
{
    mTypeName = name;
}

//! @brief Access method to manually set the Name
void GUIModelObjectAppearance::setName(QString name)
{
    mName = name;
}

//! @brief Access method to manually set the HelpText
void GUIModelObjectAppearance::setHelpText(QString text)
{
    mHelpText = text;
}

//! @brief Access method to manually set the BaseIconPath
void GUIModelObjectAppearance::setBaseIconPath(QString path)
{
    mBasePath = path;
}

//! @brief Access method to manually set the BasePath relative UserIconPath
void GUIModelObjectAppearance::setIconPathUser(QString path)
{
    mIconPathUser = path;
}

//! @brief Access method to manually set the BasePath relative ISOIconPath
void GUIModelObjectAppearance::setIconPathISO(QString path)
{
    mIconPathISO = path;
}

//! @brief Check if a IsoIcon path is available, (does not check if icon actaully exists)
bool GUIModelObjectAppearance::haveIsoIcon()
{
    return !mIconPathISO.isEmpty();
}

//! @brief Check if a UserIcon path is available, (does not check if icon actaully exists)
bool GUIModelObjectAppearance::haveUserIcon()
{
    return !mIconPathUser.isEmpty();
}
