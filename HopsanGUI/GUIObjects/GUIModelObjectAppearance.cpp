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
//! @file   ModelObjectAppearance.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by ModelObjects and the LibraryWidget
//!
//$Id$

#include "global.h"
#include "GUIModelObjectAppearance.h"
#include "Utilities/GUIUtilities.h"
#include "LibraryHandler.h"
#include "Utilities/XMLUtilities.h"
#include "version_gui.h"

//These are only used in temporary save to file
#include <QFile>

#include <QIcon>

// ========== Defines for load/save common strings ==========
#define CAF_TYPENAME "typename"
#define CAF_SUBTYPENAME "subtypename"
#define CAF_TYPE "type"
#define CAF_DISPLAYNAME "displayname"
#define CAF_SOURCECODE "sourcecode"
#define CAF_LIBPATH "libpath"
#define CAF_RECOMPILABLE "recompilable"
#define CAF_NAME "name"

#define CAF_HMFFILE "hmffile"

#define CAF_ICON "icon"
#define CAF_ICONS "icons"
#define CAF_PATH "path"
#define CAF_ICONROTATION "iconrotation"
#define CAF_SCALE "scale"

#define CAF_HELP "help"
#define CAF_HELPTEXT "text"
#define CAF_HELPPICTURE "picture"
#define CAF_HELPLINK "link"

#define CAF_PORTS "ports"
#define CAF_PORT "port"
#define CAF_DESCRIPTION "description"
#define CAF_PORTPOSITIONS "portpositions"
#define CAF_PORTPOSE "portpose"

#define CAF_REPLACABLES "replacables"
#define CAF_REPLACABLE "replacable"

#define CAF_ANIMATION "animation"

// =============== Help Functions ===============
QDomElement appendOrGetCAFRootTag(QDomElement parentElement)
{
   QDomElement cafroot = getOrAppendNewDomElement(parentElement, CAF_ROOT);
   cafroot.setAttribute(CAF_VERSION, CAF_VERSIONNUM);
   return cafroot;
}

//! @brief Special purpose function for parsing a Hopsan specific XML tag containing PortPose information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rName The name of the port
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
//! @param[out] rTheta The orientaion (angle)
//! @deprecated Only use for loading old version files
void parsePortPoseTag(QDomElement domElement, QString &rName, double &rX, double &rY, double &rTheta)
{
    rName = domElement.attribute(CAF_NAME);
    bool dummy;
    parsePoseTag(domElement, rX, rY, rTheta, dummy);
}

//! @brief Help function that writes PortAppearance to DOM Element
//! @param [in,out] domElement The DOM element to append to
//! @param [in] portName QString containing the port name
//! @param [in] rPortAppearance Reference to PortAppearance object to be writen
void appendPortDomElement(QDomElement &rDomElement, const QString portName, const PortAppearance &rPortAppearance)
{
    QDomElement xmlPort = appendDomElement(rDomElement, CAF_PORT);
    xmlPort.setAttribute(CAF_NAME, portName);
    setQrealAttribute(xmlPort, "x", rPortAppearance.x, 10, 'g');
    setQrealAttribute(xmlPort, "y", rPortAppearance.y, 10, 'g');
    setQrealAttribute(xmlPort, "a", rPortAppearance.rot, 6, 'g');
    if(rPortAppearance.mAutoPlaced)
    {
        xmlPort.setAttribute("autoplaced", HMF_TRUETAG);
    }
    else
    {
        xmlPort.setAttribute("autoplaced", HMF_FALSETAG);
    }
    if(rPortAppearance.mEnabled)
    {
        xmlPort.setAttribute("enabled", HMF_TRUETAG);
    }
    else
    {
        xmlPort.setAttribute("enabled", HMF_FALSETAG);
    }

//    // Save visible or not, only write if hidden is set, as default is visible (to avoid clutter in xml file)
//    //! @todo maybe should always write, this is wrong, should look at if it is hide or not instead... Now that is not in appearance
//    if (!rPortAppearance.mVisible) //This is wrong
//    {
//        xmlPort.setAttribute("visible", HMF_FALSETAG);
//    }

    // Save description if any
    if (!rPortAppearance.mDescription.isEmpty())
    {
        appendDomTextNode(xmlPort, CAF_DESCRIPTION, rPortAppearance.mDescription);
    }
}

//! @brief Help function that parses a port DOM Element
//! @param [in] domElement The DOM element to parse
//! @param [out] rPortName Reference to QString that will contain port name
//! @param [out] rPortAppearance Reference to PortAppearance object that will contain parsed data
void parsePortDomElement(QDomElement domElement, QString &rPortName, PortAppearance &rPortAppearance)
{
    rPortName = domElement.attribute(CAF_NAME);
    rPortAppearance.x = parseAttributeQreal(domElement, "x", 0);
    rPortAppearance.y = parseAttributeQreal(domElement, "y", 0);
    rPortAppearance.rot = parseAttributeQreal(domElement, "a", 0);

    rPortAppearance.mAutoPlaced = parseAttributeBool(domElement, "autoplaced", true);
    rPortAppearance.mEnabled = parseAttributeBool(domElement, "enabled", parseAttributeBool(domElement, "visible", true));
    //! @todo should we use enabled or visible, should not be both

    //! @todo port descriptions have been moved into core, remove this load code later /Peter
    QDomElement xmlPortDescription = domElement.firstChildElement(CAF_DESCRIPTION);
    if (!xmlPortDescription.isNull())
    {
        rPortAppearance.mDescription = xmlPortDescription.text();
    }
}


// =============================================

ModelObjectIconAppearance::ModelObjectIconAppearance()
{
    mScale = 1.0;
    mRotationBehaviour = "ON";
    mIsValid = false;
}


//! @brief Loads animation data from XML element
//! @param [in] element Element to read from
//! @param [in] basePath Absolute path for the CAF (xml) file location
void ModelObjectAnimationData::readFromDomElement(QDomElement &rDomElement, QString basePath)
{
    if(!rDomElement.isNull())
    {
        QDomElement iconElement = rDomElement.firstChildElement("icon");
        if(!iconElement.isNull())
        {
            baseIconPath = iconElement.attribute("userpath");
        }

        QFileInfo baseIconFileInfo(baseIconPath);
        if (baseIconFileInfo.isRelative() && !baseIconPath.isEmpty())
        {
            baseIconFileInfo.setFile(QDir(basePath), baseIconPath);
            baseIconPath = baseIconFileInfo.absoluteFilePath();
        }

        if(rDomElement.hasAttribute("flowspeed"))
        {
            flowSpeed = rDomElement.attribute("flowspeed").toDouble();
        }
        else
        {
            flowSpeed = 100;
        }

        if(rDomElement.hasAttribute("hydraulicminpressure"))
        {
            hydraulicMinPressure = rDomElement.attribute("hydraulicminpressure").toDouble();
        }
        else
        {
            hydraulicMinPressure = 0;
        }

        if(rDomElement.hasAttribute("hydraulicmaxpressure"))
        {
            hydraulicMaxPressure = rDomElement.attribute("hydraulicmaxpressure").toDouble();
        }
        else
        {
            hydraulicMaxPressure = 2e7;
        }

        QDomElement xmlMovable = rDomElement.firstChildElement("movable");
        int idx=0;
        while(!xmlMovable.isNull())
        {
            {
                while(movables.size() < idx+1)
                    movables.append(ModelObjectAnimationMovableData());

                ModelObjectAnimationMovableData &m = movables[idx];

                if(xmlMovable.hasAttribute("idx"))
                {
                    m.idx = xmlMovable.attribute("idx").toInt();
                }

                if(!xmlMovable.firstChildElement("icon").isNull() && !xmlMovable.firstChildElement("icon").attribute("userpath").isEmpty())
                {
                    m.iconPath = xmlMovable.firstChildElement("icon").attribute("userpath");
                }

                QDomElement dataElement = xmlMovable.firstChildElement("data");
                if(!dataElement.isNull())
                {
                    m.dataPorts.clear();
                    m.dataNames.clear();
                }
                while(!dataElement.isNull())
                {
                    int idx = dataElement.attribute("idx").toInt();
                    while(m.dataPorts.size() < idx+1)
                    {
                        m.dataPorts.append("");
                        m.dataNames.append(""); //Assume names and ports always have same size
                    }
                    m.dataPorts[idx] = dataElement.attribute("port");
                    m.dataNames[idx] = dataElement.attribute("dataname");
                    dataElement = dataElement.nextSiblingElement("data");
                }

                QDomElement multiplierElement = xmlMovable.firstChildElement("multiplier");
                if(!multiplierElement.isNull())
                {
                    m.multipliers.clear();
                }
                while(!multiplierElement.isNull())
                {
                    m.multipliers.append(multiplierElement.attribute("name"));
                    multiplierElement = multiplierElement.nextSiblingElement("multiplier");
                }

                QDomElement divisorElement = xmlMovable.firstChildElement("divisor");
                if(!divisorElement.isNull())
                {
                    m.divisors.clear();
                }
                while(!divisorElement.isNull())
                {
                    m.divisors.append(divisorElement.attribute("name"));
                    divisorElement = divisorElement.nextSiblingElement("divisor");
                }

                if(!xmlMovable.firstChildElement("movement").isNull())
                {
                    m.movementDataIdx = xmlMovable.firstChildElement("movement").attribute("idx").toDouble();
                    m.movementX = xmlMovable.firstChildElement("movement").attribute("x").toDouble();
                    m.movementY = xmlMovable.firstChildElement("movement").attribute("y").toDouble();
                    m.movementTheta = xmlMovable.firstChildElement("movement").attribute("a").toDouble();
                }
                if(!xmlMovable.firstChildElement("start").isNull())
                {
                    m.startX = xmlMovable.firstChildElement("start").attribute("x").toDouble();
                    m.startY = xmlMovable.firstChildElement("start").attribute("y").toDouble();
                    m.startTheta = xmlMovable.firstChildElement("start").attribute("a").toDouble();
                }

                if(!xmlMovable.firstChildElement("initscale").isNull())
                {
                    m.initScaleX = xmlMovable.firstChildElement("initscale").attribute("x").toDouble();
                    m.initScaleY = xmlMovable.firstChildElement("initscale").attribute("y").toDouble();
                }
                if(!xmlMovable.firstChildElement("resize").isNull())
                {
                    m.resizeX = xmlMovable.firstChildElement("resize").attribute("x").toDouble();
                    m.resizeY = xmlMovable.firstChildElement("resize").attribute("y").toDouble();
                    m.scaleDataIdx1 = xmlMovable.firstChildElement("resize").attribute("idx1").toInt();
                    m.scaleDataIdx2 = xmlMovable.firstChildElement("resize").attribute("idx2").toInt();
                }

                if(!xmlMovable.firstChildElement("color").isNull())
                {
                    m.colorDataIdx = xmlMovable.firstChildElement("color").attribute("idx").toInt();
                    m.colorR = xmlMovable.firstChildElement("color").attribute("r").toDouble();
                    m.colorG = xmlMovable.firstChildElement("color").attribute("g").toDouble();
                    m.colorB = xmlMovable.firstChildElement("color").attribute("b").toDouble();
                    m.colorA = xmlMovable.firstChildElement("color").attribute("a").toDouble();
                }
                if(!xmlMovable.firstChildElement("initcolor").isNull())
                {
                    m.initColorR = xmlMovable.firstChildElement("initcolor").attribute("r").toDouble();
                    m.initColorG = xmlMovable.firstChildElement("initcolor").attribute("g").toDouble();
                    m.initColorB = xmlMovable.firstChildElement("initcolor").attribute("b").toDouble();
                    m.initColorA = xmlMovable.firstChildElement("initcolor").attribute("a").toDouble();
                }

                if(!xmlMovable.firstChildElement("transformorigin").isNull())
                {
                    m.transformOriginX = xmlMovable.firstChildElement("transformorigin").attribute("x").toDouble();
                    m.transformOriginY = xmlMovable.firstChildElement("transformorigin").attribute("y").toDouble();
                }

                QFileInfo movableIconFileInfo(movables.last().iconPath);
                if (movableIconFileInfo.isRelative() && !movables.last().iconPath.isEmpty())
                {
                    movableIconFileInfo.setFile(QDir(basePath), movables.last().iconPath);
                    m.iconPath = movableIconFileInfo.absoluteFilePath();
                }
                QDomElement xmlAdjustable = xmlMovable.firstChildElement("adjustable");
                if(!xmlAdjustable.isNull())
                {
                    m.isAdjustable = true;
                    m.adjustableMinX = xmlAdjustable.attribute("xmin").toDouble();
                    m.adjustableMaxX = xmlAdjustable.attribute("xmax").toDouble();
                    m.adjustableMinY = xmlAdjustable.attribute("ymin").toDouble();
                    m.adjustableMaxY = xmlAdjustable.attribute("ymax").toDouble();
                    m.adjustablePort = xmlAdjustable.attribute("port");
                    m.adjustableDataName = xmlAdjustable.attribute("dataname");
                    m.adjustableGainX = xmlAdjustable.attribute("xgain").toDouble();
                    m.adjustableGainY = xmlAdjustable.attribute("ygain").toDouble();
                }
                else
                {
                    m.isAdjustable = false;
                }

                QDomElement xmlSwitchable = xmlMovable.firstChildElement("switchable");
                if(!xmlSwitchable.isNull())
                {
                    m.isSwitchable = true;
                    m.switchableOffValue = xmlSwitchable.attribute("offvalue").toDouble();
                    m.switchableOnValue = xmlSwitchable.attribute("onvalue").toDouble();
                    m.switchablePort = xmlSwitchable.attribute("port");
                    m.switchableDataName = xmlSwitchable.attribute("dataname");
                }
                else
                {
                    m.isSwitchable = false;
                    m.switchableOffValue = 0;
                    m.switchableOnValue = 0;
                    m.switchablePort = QString();
                    m.switchableDataName = QString();
                }

                QDomElement xmlIndicaor = xmlMovable.firstChildElement("indicator");
                if(!xmlIndicaor.isNull())
                {
                    m.isIndicator = true;
                    m.indicatorPort = xmlIndicaor.attribute("port");
                    m.indicatorDataName = xmlIndicaor.attribute("dataname");
                }
                else
                {
                    m.isIndicator = false;
                    m.indicatorPort = QString();
                    m.indicatorDataName = QString();
                }


                QDomElement xmlMovingPorts = xmlMovable.firstChildElement("movingport");
                if(!xmlMovingPorts.isNull())
                {
                    m.movablePortNames.clear();
                    m.movablePortStartX.clear();
                    m.movablePortStartY.clear();
                }
                while(!xmlMovingPorts.isNull())
                {
                    m.movablePortNames.append(xmlMovingPorts.attribute("portname"));
                    m.movablePortStartX.append(xmlMovingPorts.attribute("startx").toDouble());
                    m.movablePortStartY.append(xmlMovingPorts.attribute("starty").toDouble());
                    xmlMovingPorts = xmlMovingPorts.nextSiblingElement("movingport");
                }

                QDomElement xmlRelative = xmlMovable.firstChildElement("relative");
                if(!xmlRelative.isNull())
                {
                    m.movableRelative = xmlRelative.attribute("idx").toInt();
                }
                else
                {
                    m.movableRelative = -1;
                }
            }
            ++idx;
            xmlMovable = xmlMovable.nextSiblingElement("movable");
        }

        //Sort movables by indexes (no index = leave at bottom)
        QList<ModelObjectAnimationMovableData> tempList = movables;
        movables.clear();
        for(int i=0; i<tempList.size(); ++i)
        {
            int m=0;
            for(m=0; m<movables.size(); ++m)
            {
                if(movables[m].idx >= tempList[i].idx && m == 0)
                    break;
                if(movables[m].idx >= tempList[i].idx && m>0 && movables[m-1].idx <= tempList[i].idx)
                    break;
            }
            movables.insert(m, tempList[i]);
        }
    }
}


void ModelObjectAnimationData::saveToDomElement(QDomElement &rDomElement)
{
    rDomElement.setAttribute("flowspeed", flowSpeed);
    rDomElement.setAttribute("hydraulicminpressure", hydraulicMinPressure);
    rDomElement.setAttribute("hydraulicmaxpressure", hydraulicMaxPressure);
    foreach(const ModelObjectAnimationMovableData &m, movables)
    {
        QDomElement movableElement = appendDomElement(rDomElement, "movable");

        //! @note Saving icons is disabled, because it probably makes no sense (paths will not work if moving hmf to other location)
        //QDomElement iconElement = appendDomElement(movableElement, "icon");
        //iconElement.setAttribute("userpath", m.iconPath);

        if(m.idx >= 0)
        {
            movableElement.setAttribute("idx", m.idx);
        }

        for(int i=0; i<m.dataNames.size(); ++i)
        {
            if(!m.dataNames[i].isEmpty())
            {
                QDomElement dataElement = appendDomElement(movableElement, "data");
                dataElement.setAttribute("port", m.dataPorts[i]);
                dataElement.setAttribute("dataname", m.dataNames[i]);
                dataElement.setAttribute("idx", i);
            }
        }

        for(int i=0; i<m.multipliers.size(); ++i)
        {
            QDomElement multiplierElement = appendDomElement(movableElement, "multiplier");
            multiplierElement.setAttribute("name", m.multipliers[i]);
        }

        for(int i=0; i<m.divisors.size(); ++i)
        {
            QDomElement divisorElement = appendDomElement(movableElement, "divisor");
            divisorElement.setAttribute("name", m.divisors[i]);
        }

        QDomElement startElement = appendDomElement(movableElement, "start");
        setQrealAttribute(startElement, "x", m.startX);
        setQrealAttribute(startElement, "y", m.startY);
        setQrealAttribute(startElement, "a", m.startTheta);

        QDomElement movementElement = appendDomElement(movableElement, "movement");
        setQrealAttribute(movementElement, "x", m.movementX);
        setQrealAttribute(movementElement, "y", m.movementY);
        setQrealAttribute(movementElement, "a", m.movementTheta);
        movementElement.setAttribute("idx", m.movementDataIdx);

        QDomElement initScaleElement = appendDomElement(movableElement, "initscale");
        setQrealAttribute(initScaleElement, "x", m.initScaleX);
        setQrealAttribute(initScaleElement, "y", m.initScaleY);

        QDomElement resizeElement = appendDomElement(movableElement, "resize");
        setQrealAttribute(resizeElement, "x", m.resizeX);
        setQrealAttribute(resizeElement, "y", m.resizeY);
        movementElement.setAttribute("idx1", m.scaleDataIdx1);
        movementElement.setAttribute("idx2", m.scaleDataIdx2);

        QDomElement initColorElement = appendDomElement(movableElement, "initcolor");
        setQrealAttribute(initColorElement, "r", m.initColorR);
        setQrealAttribute(initColorElement, "g", m.initColorG);
        setQrealAttribute(initColorElement, "b", m.initColorB);
        setQrealAttribute(initColorElement, "a", m.initColorA);

        QDomElement colorElement = appendDomElement(movableElement, "color");
        setQrealAttribute(colorElement, "r", m.colorR);
        setQrealAttribute(colorElement, "g", m.colorG);
        setQrealAttribute(colorElement, "b", m.colorB);
        setQrealAttribute(colorElement, "a", m.colorA);
        colorElement.setAttribute("idx", m.colorDataIdx);

        QDomElement transformOriginElement = appendDomElement(movableElement, "transformorigin");
        setQrealAttribute(transformOriginElement, "x", m.transformOriginX);
        setQrealAttribute(transformOriginElement, "y", m.transformOriginY);

        for(int i=0; i<m.movablePortNames.size(); ++i)
        {
            QDomElement movingPortElement = appendDomElement(movableElement, "movingport");
            movingPortElement.setAttribute("portname", m.movablePortNames[i]);
            setQrealAttribute(movingPortElement, "startx", m.movablePortStartX[i]);
            setQrealAttribute(movingPortElement, "starty", m.movablePortStartY[i]);
        }

        QDomElement relativeElement = appendDomElement(movableElement, "relative");
        relativeElement.setAttribute("idx", m.movableRelative);

        if(m.isAdjustable)
        {
            QDomElement adjustableElement = appendDomElement(movableElement, "adjustable");
            adjustableElement.setAttribute("xmin", m.adjustableMinX);
            adjustableElement.setAttribute("xmax", m.adjustableMaxX);
            adjustableElement.setAttribute("ymin", m.adjustableMinY);
            adjustableElement.setAttribute("ymax", m.adjustableMaxY);
            adjustableElement.setAttribute("dataname", m.adjustableDataName);
            adjustableElement.setAttribute("port", m.adjustablePort);
            adjustableElement.setAttribute("xgain", m.adjustableGainX);
            adjustableElement.setAttribute("ygain", m.adjustableGainY);
        }

        if(m.isSwitchable)
        {
            QDomElement switchableElement = appendDomElement(movableElement, "switchable");
            switchableElement.setAttribute("dataname", m.switchableDataName);
            switchableElement.setAttribute("port", m.switchablePort);
            switchableElement.setAttribute("onvalue", m.switchableOnValue);
            switchableElement.setAttribute("offvalue", m.switchableOffValue);
        }

        if(m.isIndicator)
        {
            QDomElement indicatorElement = appendDomElement(movableElement, "indicator");
            indicatorElement.setAttribute("dataname", m.indicatorDataName);
            indicatorElement.setAttribute("port", m.indicatorPort);
        }
    }
}


ModelObjectAppearance::ModelObjectAppearance()
{
    mPortAppearanceMap.clear();
    mDefaultMissingIconPath = "missingcomponenticon.svg";

    //Defaults for animation
    //! @todo This should maybe be somewhere else...
    mAnimationData.flowSpeed = 100;
    mAnimationData.hydraulicMinPressure = 0;
    mAnimationData.hydraulicMaxPressure = 2e7;
}

void ModelObjectAppearance::cacheIcons()
{
    //Pre-cache icons for faster updating of library
    QString iconPath = getFullAvailableIconPath(UserGraphics);
    mUserIcon.addFile(iconPath,QSize(55,55));
    iconPath = getFullAvailableIconPath(ISOGraphics);
    mIsoIcon.addFile(iconPath,QSize(55,55));
}

//! @brief get the type-name
//! @returns The type-name
QString ModelObjectAppearance::getTypeName() const
{
    return mTypeName;
}

QString ModelObjectAppearance::getSubTypeName() const
{
    return mSubTypeName;
}

//! @brief get the display name, even if it is empty
//! @returns The display name
QString ModelObjectAppearance::getDisplayName() const
{
    return mDisplayName;
}

//! @brief This function returns the name or typename (if name is empty)
//! Useful if display name has not been specified, then we use the type name
//! @returns A non-empty name
QString ModelObjectAppearance::getNonEmptyName() const
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


const QString &ModelObjectAppearance::getHelpPicture() const
{
    return mHelpPicture;
}


const QString &ModelObjectAppearance::getHelpText() const
{
    return mHelpText;
}

const QString &ModelObjectAppearance::getHelpLink() const
{
    return mHelpLink;
}

//! @brief Get the full Icon path for specified graphics type
//! @param [in] gfxType The graphics type enum (ISO or USER)
//! If the specified type is missing, return the other type.
//! If that is also missing return a path to the missing graphics icon
QString ModelObjectAppearance::getFullAvailableIconPath(GraphicsTypeEnumT gfxType)
{
    // Determine which type to use based on aviliablity of icons
    gfxType = selectAvailableGraphicsType(gfxType);

    // Return user graphics path
    if ( gfxType == UserGraphics )
    {
        return mUserIconAppearance.mAbsolutePath;
    }
    // Return ISO graphics path
    else if (gfxType == ISOGraphics)
    {
        return  mIsoIconAppearance.mAbsolutePath;
    }
    else
    {
        //No icon available use the default missing icon
        return QString(OBJECTICONPATH) + mDefaultMissingIconPath;
    }
}

//! @brief Returns the path to the graphics icon of requested type, regardles of wheter it is valid or not
QString ModelObjectAppearance::getIconPath(const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrel)
{
    if (gfxType == UserGraphics)
    {
        if (absrel == Absolute)
        {
            return mUserIconAppearance.mAbsolutePath;
        }
        else
        {
            return mUserIconAppearance.mRelativePath;
        }
    }
    else if (gfxType == ISOGraphics)
    {
        if (absrel == Absolute)
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

QIcon &ModelObjectAppearance::getIcon(const GraphicsTypeEnumT gfxType)
{
    if(gfxType == UserGraphics)
    {
        return mUserIcon;
    }
    return mIsoIcon;
}

QString ModelObjectAppearance::getDefaultMissingIconPath() const
{
    return QString(OBJECTICONPATH) + mDefaultMissingIconPath;
}

//! @todo This is a bit wrong, it will return the scale for the availiable type not necessarily the requested
double ModelObjectAppearance::getIconScale(const GraphicsTypeEnumT gfxType)
{
    // Determine which type to use based on aviliablity of icons
    GraphicsTypeEnumT gfxTypeI = selectAvailableGraphicsType(gfxType);

    if (gfxTypeI == UserGraphics)
    {
        return mUserIconAppearance.mScale;
    }
    else if (gfxTypeI == ISOGraphics)
    {
        return mIsoIconAppearance.mScale;
    }
    else
    {
        return 1.0; //Invalid type
    }
}

void ModelObjectAppearance::setRelativePathFromAbsolute()
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

void ModelObjectAppearance::setAbsoultePathFromRelative()
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

//! @todo This is a bit wrong, it will return the scale for the availiable type not necessarily the requested
QString ModelObjectAppearance::getIconRotationBehaviour(const GraphicsTypeEnumT gfxType)
{
    // Determine which type to use based on aviliablity of icons
    GraphicsTypeEnumT gfxTypeI = selectAvailableGraphicsType(gfxType);

    if (gfxTypeI == UserGraphics)
    {
        return mUserIconAppearance.mRotationBehaviour;
    }
    else if (gfxTypeI == ISOGraphics)
    {
        return mIsoIconAppearance.mRotationBehaviour;
    }
    else
    {
        //Incorrect type, return something, maybe give error instead
        return "ON";
    }
}

QPointF ModelObjectAppearance::getNameTextPos() const
{
    return mNameTextPos;
}


QString ModelObjectAppearance::getSourceCodeFile() const
{
    return mSourceCode;
}


QString ModelObjectAppearance::getLibPath() const
{
    return mLibPath;
}


bool ModelObjectAppearance::isRecompilable() const
{
    return mIsRecompilable;
}


ModelObjectAnimationData *ModelObjectAppearance::getAnimationDataPtr()
{
    return &mAnimationData;
}



//! @brief Returns a reference to the map containing port appearance
//! @returns Reference to mPortAppearanceMap
PortAppearanceMapT &ModelObjectAppearance::getPortAppearanceMap()
{
    return mPortAppearanceMap;
}

const PortAppearance *ModelObjectAppearance::getPortAppearance(const QString &rPortName) const
{
    PortAppearanceMapT::const_iterator it = mPortAppearanceMap.find(rPortName);
    if (it != mPortAppearanceMap.end())
    {
        return &it.value();
    }
    return 0;
}

PortAppearance *ModelObjectAppearance::getPortAppearance(const QString &rPortName)
{
    PortAppearanceMapT::iterator it = mPortAppearanceMap.find(rPortName);
    if (it != mPortAppearanceMap.end())
    {
        return &it.value();
    }
    return 0;
}



//! @brief Removes a port appearance post for a specified portname
//! @param[in] portName The port name for the port Appearance to be erased
void ModelObjectAppearance::erasePortAppearance(const QString portName)
{
    PortAppearanceMapT::iterator pait = mPortAppearanceMap.find(portName);
    if (pait != mPortAppearanceMap.end())
    {
        mPortAppearanceMap.erase(pait);
    }
    else
    {
        qDebug() << "ModelObjectAppearance, ERROR: specified portappearance could not be found in the map: " << portName;
    }
}

//! @brief Adds or updates a port appearance post for a specified portname
//! @param[in] portName The port name for the port Appearance to be added
//! @param[in] pPortAppearance A pointer to the port Appearance to add, if 0 then a new undefined appearance will be created
void ModelObjectAppearance::addPortAppearance(const QString portName, PortAppearance *pPortAppearance)
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
QString ModelObjectAppearance::getBasePath() const
{
    return mBasePath;
}

//! @brief Read the ModelObjectAppearance contents from an XML DOM Element
void ModelObjectAppearance::readFromDomElement(QDomElement domElement)
{
    mHmfFile        = domElement.attribute(CAF_HMFFILE, mHmfFile);
    mTypeName       = domElement.attribute(CAF_TYPENAME, mTypeName);
    mSubTypeName    = domElement.attribute(CAF_SUBTYPENAME, "");
    mDisplayName    = domElement.attribute(CAF_DISPLAYNAME, mDisplayName);
    QString newSourceCode     = domElement.attribute(CAF_SOURCECODE, "");
    if(!newSourceCode.isEmpty())
        mSourceCode = newSourceCode;
    QString newLibPath        = domElement.attribute(CAF_LIBPATH, "");
    if(!newLibPath.isEmpty())
        mLibPath = newLibPath;
    mIsRecompilable   = parseAttributeBool(domElement, CAF_RECOMPILABLE, false);

    //Use typename if displayname not set
    if (mDisplayName.isEmpty())
    {
        mDisplayName = mTypeName;
    }

    QDomElement xmlHelp = domElement.firstChildElement(CAF_HELP);
    if(!xmlHelp.isNull())
    {
        QDomElement xmlHelpText = xmlHelp.firstChildElement(CAF_HELPTEXT);
        if (!xmlHelpText.isNull())
        {
            mHelpText = xmlHelpText.text();
        }

        QDomElement xmlHelpPicture = xmlHelp.firstChildElement(CAF_HELPPICTURE);
        if (!xmlHelpPicture.isNull())
        {
            mHelpPicture = xmlHelpPicture.text();
        }

        QDomElement xmlHelpLink = xmlHelp.firstChildElement(CAF_HELPLINK);
        if (!xmlHelpLink.isNull())
        {
            mHelpLink = xmlHelpLink.text();
        }
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
            mIsoIconAppearance.mRotationBehaviour = xmlIcon.attribute(CAF_ICONROTATION,mIsoIconAppearance.mRotationBehaviour);
        }
        else if (type == "user")
        {
            mUserIconAppearance.mRelativePath = xmlIcon.attribute(CAF_PATH);
            mUserIconAppearance.mScale = parseAttributeQreal(xmlIcon, CAF_SCALE, 1.0);
            mUserIconAppearance.mRotationBehaviour = xmlIcon.attribute(CAF_ICONROTATION,mUserIconAppearance.mRotationBehaviour);
        }
        else if (type == "defaultmissing")
        {
            //! @todo maye have a DefaultIconAppearance object to, an load all data
            mDefaultMissingIconPath = xmlIcon.attribute(CAF_PATH);
        }
        //else ignore, maybe should give warning

        xmlIcon = xmlIcon.nextSiblingElement(CAF_ICON);
    }

    QDomElement xmlPorts = domElement.firstChildElement(CAF_PORTS);
    while (!xmlPorts.isNull())
    {
        QDomElement xmlPort = xmlPorts.firstChildElement(CAF_PORT);
        while (!xmlPort.isNull())
        {
            QString portname;
            PortAppearance portApp;
            parsePortDomElement(xmlPort, portname, portApp);
            if (mPortAppearanceMap.contains(portname))
            {
                // We need to copy data, not replace as there may be pointers to data (wich is kind of unsafe)
                mPortAppearanceMap[portname] = portApp;
            }
            else
            {
                mPortAppearanceMap.insert(portname, portApp);
            }
            xmlPort = xmlPort.nextSiblingElement(CAF_PORT);
        }
        // There should only be one <ports>, but lets check for more just in case
        xmlPorts = xmlPorts.nextSiblingElement(CAF_PORTS);
    }

    QDomElement xmlReplacables = domElement.firstChildElement(CAF_REPLACABLES);
    while (!xmlReplacables.isNull())
    {
        QDomElement xmlReplacable = xmlReplacables.firstChildElement(CAF_REPLACABLE);
        while (!xmlReplacable.isNull())
        {
            QString typeName = xmlReplacable.attribute(CAF_TYPENAME);
            gpLibraryHandler->addReplacement(mTypeName, typeName); //!< @todo mainwindow and library should not be called in this file it is suposed to be a leaf class
            xmlReplacable = xmlReplacable.nextSiblingElement(CAF_REPLACABLE);
        }

        xmlReplacables = xmlReplacables.nextSiblingElement(CAF_REPLACABLES);
    }


    // vvvvvvvvvvvvvvvvvvvvv=== Bellow Reads old Format 0.2 Tags ===vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

    QDomElement xmlHelp_02 = domElement.firstChildElement(CAF_HELP);
    if(!xmlHelp_02.isNull())
    {
        if (xmlHelp_02.hasAttribute(CAF_HELPPICTURE))
        {
            mHelpPicture = xmlHelp_02.attribute(CAF_HELPPICTURE);
        }

        if (xmlHelp_02.hasAttribute(CAF_HELPTEXT))
        {
            mHelpText = xmlHelp_02.attribute(CAF_HELPTEXT);
        }
    }

    QString portname;
    QDomElement xmlPorts_02 = domElement.firstChildElement(CAF_PORTPOSITIONS);
    while (!xmlPorts_02.isNull())
    {
        QDomElement xmlPortPose = xmlPorts_02.firstChildElement(CAF_PORTPOSE);
        while (!xmlPortPose.isNull())
        {
            PortAppearance portApp;
            parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
            mPortAppearanceMap.insert(portname, portApp);
            xmlPortPose = xmlPortPose.nextSiblingElement(CAF_PORTPOSE);
        }
        // There should only be one <ports>, but lets check for more just in case
        xmlPorts_02 = xmlPorts_02.nextSiblingElement(CAF_PORTPOSITIONS);
    }
    
    QDomElement xmlAnimation = domElement.firstChildElement(CAF_ANIMATION);
    mAnimationData.readFromDomElement(xmlAnimation, mBasePath);
    

    // vvvvvvvvvvvvvvvvvvvvv=== Bellow Reads old Format 0.1 Tags ===vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

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
    // Check if icons exist and refresh isValid bool
    refreshIconValid();
}

//! @brief Adds the model object root dom element, or returns one that already exist
QDomElement ModelObjectAppearance::addModelObjectRootElement(QDomElement parentDomElement)
{
    QDomElement more = getOrAppendNewDomElement(parentDomElement, CAF_MODELOBJECT);
    more.setAttribute(CAF_TYPENAME, mTypeName);
    if(mTypeName.startsWith("CppComponent"))
    {
        more.setAttribute(CAF_TYPENAME, "CppComponent");
    }
    more.setAttribute(CAF_DISPLAYNAME, mDisplayName);
    if (!mSubTypeName.isEmpty())
    {
        more.setAttribute(CAF_SUBTYPENAME, mSubTypeName);
    }
    return more;
}

//! @brief Writes the ModelObjectAppearance contents to an XML DOM Element
//! @param rDomElement The DOM element to write to
void ModelObjectAppearance::saveToDomElement(QDomElement &rDomElement)
{
    // Save type and name data
    QDomElement xmlObject = addModelObjectRootElement(rDomElement);

    //  Save icon data
    QDomElement xmlIcons = appendDomElement(xmlObject, CAF_ICONS);
    if (hasIcon(UserGraphics))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, CAF_ICON);
        xmlIcon.setAttribute(CAF_TYPE, "user");
        xmlIcon.setAttribute(CAF_PATH, mUserIconAppearance.mRelativePath);
        setQrealAttribute(xmlIcon, CAF_SCALE, mUserIconAppearance.mScale, 6, 'g');
        xmlIcon.setAttribute(CAF_ICONROTATION, mUserIconAppearance.mRotationBehaviour);
    }
    if (hasIcon(ISOGraphics))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, CAF_ICON);
        xmlIcon.setAttribute(CAF_TYPE, "iso");
        xmlIcon.setAttribute(CAF_PATH, mIsoIconAppearance.mRelativePath);
        setQrealAttribute(xmlIcon, CAF_SCALE, mIsoIconAppearance.mScale, 6, 'g');
        xmlIcon.setAttribute(CAF_ICONROTATION, mIsoIconAppearance.mRotationBehaviour);
    }

    // If default missing have changed, then save that data as well
    //! @todo not hardcoded should be defined
    if (mDefaultMissingIconPath != "missingcomponenticon.svg")
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, CAF_ICON);
        xmlIcon.setAttribute(CAF_TYPE, "defaultmissing");
        xmlIcon.setAttribute(CAF_PATH, mDefaultMissingIconPath);
    }

    // Save help text and picture data
    if(!mHelpText.isEmpty() || !mHelpPicture.isEmpty())
    {
        QDomElement xmlHelp = appendDomElement(xmlObject, CAF_HELP);
        if( !mHelpText.isEmpty() )
        {
            appendDomTextNode(xmlHelp, CAF_HELPTEXT, mHelpText);
        }

        if( !mHelpPicture.isEmpty() )
        {
            appendDomTextNode(xmlHelp, CAF_HELPPICTURE, mHelpPicture);
        }

        if( !mHelpPicture.isEmpty() )
        {
            appendDomTextNode(xmlHelp, CAF_HELPPICTURE, mHelpPicture);
        }
    }

    // Save port data
    //! @todo maybe make the port appearance  class capable of saving itself to DOM
    QDomElement xmlPorts = appendDomElement(xmlObject, CAF_PORTS);
    PortAppearanceMapT::iterator pit;
    for (pit=mPortAppearanceMap.begin(); pit!=mPortAppearanceMap.end(); ++pit)
    {
        appendPortDomElement(xmlPorts, pit.key(), pit.value());
    }

    QDomElement xmlAnimation;
    if(xmlObject.firstChildElement(CAF_ANIMATION).isNull())
    {
        xmlAnimation = appendDomElement(xmlObject, CAF_ANIMATION);
    }
    else
    {
        xmlAnimation = xmlObject.firstChildElement(CAF_ANIMATION);
    }
    mAnimationData.saveToDomElement(xmlAnimation);
}

//! @brief Convenience function to save only specific ports to dom element, used to save dynamic parameter ports
void ModelObjectAppearance::saveSpecificPortsToDomElement(QDomElement &rDomElement, const QStringList &rParametNames)
{
    //! @todo maybe make the port appearance  class capable of saving itself to DOM
    QDomElement xmlModelObject = addModelObjectRootElement(rDomElement);

    // First check if ports already exist, else add the element
    QDomElement xmlPorts = getOrAppendNewDomElement(xmlModelObject, CAF_PORTS);
    QStringList::const_iterator pnit; // PortName iterator
    for (pnit=rParametNames.begin(); pnit!=rParametNames.end(); ++pnit)
    {
        appendPortDomElement(xmlPorts, *pnit, mPortAppearanceMap.value(*pnit));
    }
}

//! @brief Save Appearancedata to XML file
void ModelObjectAppearance::saveToXMLFile(QString filename)
{
    //Save to file
    QDomDocument doc;
    QDomElement cafroot = doc.createElement(CAF_ROOT);
    doc.appendChild(cafroot);
    cafroot.setAttribute(CAF_VERSION, CAF_VERSIONNUM);
    this->saveToDomElement(cafroot);
    QFile xml(filename);
    if (!xml.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << xml.fileName();
        return;
    }
    QTextStream out(&xml);
    appendRootXMLProcessingInstruction(doc); //The xml "comment" on the first line
    doc.save(out, XMLINDENTATION);
    xml.close();
}


//! @brief Access method to manually set the TypeName
void ModelObjectAppearance::setTypeName(const QString type)
{
    mTypeName = type;
}

//! @brief Access method to manually set the SubTypeName
void ModelObjectAppearance::setSubTypeName(const QString subtype)
{
    mSubTypeName = subtype;
}

//! @brief Access method to manually set the Name
void ModelObjectAppearance::setDisplayName(const QString name)
{
    mDisplayName = name;
}

//! @brief Access method to manually set the HelpText
void ModelObjectAppearance::setHelpText(const QString text)
{
    mHelpText = text;
}

//! @brief Access method to manually set the BaseIconPath
void ModelObjectAppearance::setBasePath(const QString path)
{
    mBasePath = path;
    setRelativePathFromAbsolute(); //Reset relative path to new basepath
}

//! @brief Access method to manually set the BasePath relative UserIconPath
void ModelObjectAppearance::setIconPath(const QString path, const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrel)
{
    if (absrel == Absolute)
    {
        if (gfxType == UserGraphics)
        {
            mUserIconAppearance.mAbsolutePath = path;
        }
        else if (gfxType == ISOGraphics)
        {
            mIsoIconAppearance.mAbsolutePath = path;
        }
        setRelativePathFromAbsolute();
    }
    else
    {
        if (gfxType == UserGraphics)
        {
            mUserIconAppearance.mRelativePath = path;
        }
        else if (gfxType == ISOGraphics)
        {
            mIsoIconAppearance.mRelativePath = path;
        }
        setAbsoultePathFromRelative();
    }

    //Now check if icons are valid
    refreshIconValid();
}

void ModelObjectAppearance::setIconScale(const double scale, const GraphicsTypeEnumT gfxType)
{
    if (gfxType == UserGraphics)
    {
        mUserIconAppearance.mScale = scale;
    }
    else if (gfxType == ISOGraphics)
    {
        mIsoIconAppearance.mScale = scale;
    }
    //else dont do anything
}

QString ModelObjectAppearance::getHmfFile() const
{
    return mHmfFile;
}


//! @brief Check if specified Icon path is availiable and icon exists
bool ModelObjectAppearance::hasIcon(const GraphicsTypeEnumT gfxType)
{
    if (gfxType == ISOGraphics)
    {
        QFileInfo iso(mIsoIconAppearance.mAbsolutePath);
        return iso.isFile();
    }
    else if (gfxType == UserGraphics)
    {
        QFileInfo user(mUserIconAppearance.mAbsolutePath);
        return user.isFile();
    }
    else
    {
        return false;
    }
}

void ModelObjectAppearance::refreshIconValid()
{
    if (hasIcon(UserGraphics))
    {
        mUserIconAppearance.mIsValid = true;
    }
    else
    {
        mUserIconAppearance.mIsValid = false;
    }
    if (hasIcon(ISOGraphics))
    {
        mIsoIconAppearance.mIsValid = true;
    }
    else
    {
        mIsoIconAppearance.mIsValid = false;
    }
}

GraphicsTypeEnumT ModelObjectAppearance::selectAvailableGraphicsType(const GraphicsTypeEnumT type)
{
    // We want USERICON and have USERICON
    if ( (type == UserGraphics) && mUserIconAppearance.mIsValid )
    {
        //Use user icon
        return UserGraphics;
    }
    // We want ISOICON and have ISOICON
    else if ( (type == ISOGraphics) &&  mIsoIconAppearance.mIsValid )
    {
        //Use ISO icon
        return  ISOGraphics;
    }
    // If what we requested does not exist but USER graphics do exist
    else if ( mUserIconAppearance.mIsValid && !mIsoIconAppearance.mIsValid )
    {
        //Use user icon
        return UserGraphics;
    }
    // If what we requested does not exist but ISO graphics do exist
    else if ( !mUserIconAppearance.mIsValid && mIsoIconAppearance.mIsValid )
    {
        //Use ISO icon
        return ISOGraphics;
    }
    else
    {
        //No icon available return nothing type
        return NoGraphics;
    }
}
