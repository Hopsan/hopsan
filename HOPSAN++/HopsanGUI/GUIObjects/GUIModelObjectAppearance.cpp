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

GUIModelObjectAppearance::GUIModelObjectAppearance()
{
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

    qDebug() << "Type: " << mTypeName << " iconUser: " << iconUserFileInfo.absoluteFilePath();
    qDebug() << "Type: " << mTypeName << " iconISO: " << iconISOFileInfo.absoluteFilePath();

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
    mDisplayName    = domElement.attribute(HMF_DISPLAYNAMETAG);

    QDomElement xmlHelp = domElement.firstChildElement("help");
    if(!xmlHelp.isNull())
    {
        mHelpPicture    = xmlHelp.attribute("picture");
        mHelpText       = xmlHelp.attribute("text");
    }

    QDomElement xmlIcon = domElement.firstChildElement("icon");
    mIconIsoPath = xmlIcon.attribute("isopath");
    mIconUserPath = xmlIcon.attribute("userpath");
    mIconRotationBehaviour = xmlIcon.attribute("iconrotation");

    this->makeSurePathsAbsolute();

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
    xmlObject.setAttribute(HMF_DISPLAYNAMETAG, mDisplayName);
    QDomElement xmlIcon = appendDomElement(xmlObject, "icon");
    this->makeSurePathsRelative(); //We want to save paths relative the basepath, to avoid incompatibility with absolute paths between systems
    xmlIcon.setAttribute("isopath", mIconIsoPath);
    xmlIcon.setAttribute("userpath", mIconUserPath);
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
    this->makeSurePathsAbsolute(); //Switch back to absolute paths
}

//! @brief Save Appearancedata to XML file, currently used as a test function
void GUIModelObjectAppearance::saveToXMLFile(QString filename)
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
