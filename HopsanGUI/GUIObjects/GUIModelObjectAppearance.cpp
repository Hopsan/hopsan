/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
#define CAF_IDX "idx"

#define CAF_HMFFILE "hmffile"

#define CAF_ICON "icon"
#define CAF_ICONS "icons"
#define CAF_PATH "path"
#define CAF_SCALE "scale"
#define CAF_ICONROTATION "iconrotation"
#define CAF_USERPATH "userpath"
#define CAF_ISOPATH "isopath"
#define CAF_USERSCALE "userscale"
#define CAF_ISOSCALE "isoscale"

#define CAF_HELP "help"
#define CAF_HELPTEXT "text"
#define CAF_HELPPICTURE "picture"
#define CAF_HELPLINK "link"
#define CAF_HELPHTML "html"


#define CAF_PARAMETERS "defaultparameters"
#define CAF_PARAMETER "parameter"
#define CAF_NAME "name"
#define CAF_HIDDEN "hidden"

#define CAF_PORTS "ports"
#define CAF_PORT "port"
#define CAF_DESCRIPTION "description"
#define CAF_PORTPOSITIONS "portpositions"
#define CAF_PORTPOSE "portpose"

#define CAF_REPLACABLES "replacables"
#define CAF_REPLACABLE "replacable"

#define CAF_AUTOPLACED "autoplaced"
#define CAF_ENABLED "enabled"

#define CAF_ANIMATION "animation"

#define CAF_DATANAME "dataname"
#define CAF_ONVALUE "onvalue"
#define CAF_OFFVALUE "offvalue"
#define CAF_XMIN "xmin"
#define CAF_XMAX "xmax"
#define CAF_YMIN "ymin"
#define CAF_YMAX "ymax"

#define CAF_MULTIPLIER "multiplier"
#define CAF_DIVISOR "divisor"
#define CAF_START "start"
#define CAF_MOVEMENT "movement"
#define CAF_INITSCALE "initscale"
#define CAF_RESIZE "resize"
#define CAF_INITCOLOR "initcolor"
#define CAF_COLOR "color"
#define CAF_TRANSFORMORIGIN "transformorigin"
#define CAF_FLOWSPEED "flowspeed"
#define CAF_HYDRAULICMINPRESSURE "hydraulicminpressure"
#define CAF_HYDRAULICMAXPRESSURE "hydraulicmaxpressure"

#define CAF_ADJUSTABLE "adjustable"
#define CAF_XGAIN "xgain"
#define CAF_YGAIN "ygain"
#define CAF_SWITCHABLE "switchable"
#define CAF_INDICATOR "indicator"
#define CAF_PORTNAME "portname"
#define CAF_STARTX "startx"
#define CAF_STARTY "starty"
#define CAF_MOVINGPORT "movingport"
#define CAF_RELATIVE "relative"
#define CAF_MOVABLE "movable"
#define CAF_HIDEICON "hideicon"


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
//! @param[out] rTheta The orientation (angle)
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
//! @param [in] rPortAppearance Reference to PortAppearance object to be written
void appendPortDomElement(QDomElement &rDomElement, const QString portName, const PortAppearance &rPortAppearance)
{
    QDomElement xmlPort = appendDomElement(rDomElement, CAF_PORT);
    xmlPort.setAttribute(CAF_NAME, portName);
    setQrealAttribute(xmlPort, "x", rPortAppearance.x, 10, 'g');
    setQrealAttribute(xmlPort, "y", rPortAppearance.y, 10, 'g');
    setQrealAttribute(xmlPort, "a", rPortAppearance.rot, 6, 'g');
    if(rPortAppearance.mAutoPlaced)
    {
        xmlPort.setAttribute(CAF_AUTOPLACED, HMF_TRUETAG);
    }
    else
    {
        xmlPort.setAttribute(CAF_AUTOPLACED, HMF_FALSETAG);
    }
    if(rPortAppearance.mEnabled)
    {
        xmlPort.setAttribute(CAF_ENABLED, HMF_TRUETAG);
    }
    else
    {
        xmlPort.setAttribute(CAF_ENABLED, HMF_FALSETAG);
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

    rPortAppearance.mAutoPlaced = parseAttributeBool(domElement, CAF_AUTOPLACED, true);
    rPortAppearance.mEnabled = parseAttributeBool(domElement, CAF_ENABLED, true);
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
        QDomElement iconElement = rDomElement.firstChildElement(CAF_ICON);
        if(!iconElement.isNull())
        {
            baseIconPath = iconElement.attribute(CAF_USERPATH);
        }

        QFileInfo baseIconFileInfo(baseIconPath);
        if (baseIconFileInfo.isRelative() && !baseIconPath.isEmpty())
        {
            baseIconFileInfo.setFile(QDir(basePath), baseIconPath);
            baseIconPath = baseIconFileInfo.absoluteFilePath();
        }

        if(rDomElement.hasAttribute(CAF_FLOWSPEED))
        {
            flowSpeed = rDomElement.attribute(CAF_FLOWSPEED).toDouble();
        }
        else
        {
            flowSpeed = 100;
        }

        if(rDomElement.hasAttribute(CAF_HYDRAULICMINPRESSURE))
        {
            hydraulicMinPressure = rDomElement.attribute(CAF_HYDRAULICMINPRESSURE).toDouble();
        }
        else
        {
            hydraulicMinPressure = 0;
        }

        if(rDomElement.hasAttribute(CAF_HYDRAULICMAXPRESSURE))
        {
            hydraulicMaxPressure = rDomElement.attribute(CAF_HYDRAULICMAXPRESSURE).toDouble();
        }
        else
        {
            hydraulicMaxPressure = 2e7;
        }

        QDomElement xmlMovable = rDomElement.firstChildElement(CAF_MOVABLE);
        int idx=0;
        while(!xmlMovable.isNull())
        {
            {
                while(movables.size() < idx+1)
                    movables.append(ModelObjectAnimationMovableData());

                ModelObjectAnimationMovableData &m = movables[idx];
                m.readFromDomElement(xmlMovable, basePath);


            }
            ++idx;
            xmlMovable = xmlMovable.nextSiblingElement(CAF_MOVABLE);
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
    rDomElement.setAttribute(CAF_FLOWSPEED, flowSpeed);
    rDomElement.setAttribute(CAF_HYDRAULICMINPRESSURE, hydraulicMinPressure);
    rDomElement.setAttribute(CAF_HYDRAULICMAXPRESSURE, hydraulicMaxPressure);
    foreach(const ModelObjectAnimationMovableData &m, movables)
    {
        QDomElement movableElement = appendDomElement(rDomElement, CAF_MOVABLE);

        //! @note Saving icons is disabled, because it probably makes no sense (paths will not work if moving hmf to other location)
        //QDomElement iconElement = appendDomElement(movableElement, CAF_ICON);
        //iconElement.setAttribute(CAF_USERPATH, m.iconPath);

        if(m.idx >= 0)
        {
            movableElement.setAttribute(CAF_IDX, m.idx);
        }

        for(int i=0; i<m.dataNames.size(); ++i)
        {
            if(!m.dataNames[i].isEmpty())
            {
                QDomElement dataElement = appendDomElement(movableElement, "data");
                dataElement.setAttribute(CAF_PORT, m.dataPorts[i]);
                dataElement.setAttribute(CAF_DATANAME, m.dataNames[i]);
                dataElement.setAttribute(CAF_IDX, i);
            }
        }

        for(int i=0; i<m.multipliers.size(); ++i)
        {
            QDomElement multiplierElement = appendDomElement(movableElement, CAF_MULTIPLIER);
            multiplierElement.setAttribute(CAF_NAME, m.multipliers[i]);
        }

        for(int i=0; i<m.divisors.size(); ++i)
        {
            QDomElement divisorElement = appendDomElement(movableElement, CAF_DIVISOR);
            divisorElement.setAttribute(CAF_NAME, m.divisors[i]);
        }

        QDomElement startElement = appendDomElement(movableElement, CAF_START);
        setQrealAttribute(startElement, "x", m.startX);
        setQrealAttribute(startElement, "y", m.startY);
        setQrealAttribute(startElement, "a", m.startTheta);

        for(int i=0; i<m.movementData.size(); ++i)
        {
            QDomElement movementElement = appendDomElement(movableElement, CAF_MOVEMENT);
            m.movementData[i].saveToDomElement(movementElement);
        }

        QDomElement initScaleElement = appendDomElement(movableElement, CAF_INITSCALE);
        setQrealAttribute(initScaleElement, "x", m.initScaleX);
        setQrealAttribute(initScaleElement, "y", m.initScaleY);

        for(int i=0; i<m.resizeData.size(); ++i)
        {
            QDomElement resizeElement = appendDomElement(movableElement, CAF_RESIZE);
            m.resizeData[i].saveToDomElement(resizeElement);
        }

        QDomElement initColorElement = appendDomElement(movableElement, CAF_INITCOLOR);
        QDomElement colorElement = appendDomElement(movableElement, CAF_COLOR);
        m.colorData.saveToDomElements(initColorElement, colorElement);

        QDomElement transformOriginElement = appendDomElement(movableElement, CAF_TRANSFORMORIGIN);
        setQrealAttribute(transformOriginElement, "x", m.transformOriginX);
        setQrealAttribute(transformOriginElement, "y", m.transformOriginY);

        for(int i=0; i<m.movablePortNames.size(); ++i)
        {
            QDomElement movingPortElement = appendDomElement(movableElement, CAF_MOVINGPORT);
            movingPortElement.setAttribute(CAF_PORTNAME, m.movablePortNames[i]);
            setQrealAttribute(movingPortElement, CAF_STARTX, m.movablePortStartX[i]);
            setQrealAttribute(movingPortElement, CAF_STARTY, m.movablePortStartY[i]);
        }

        QDomElement relativeElement = appendDomElement(movableElement, CAF_RELATIVE);
        relativeElement.setAttribute(CAF_IDX, m.movableRelative);

        if(m.isAdjustable)
        {
            QDomElement adjustableElement = appendDomElement(movableElement, CAF_ADJUSTABLE);
            setQrealAttribute(adjustableElement, CAF_XMIN, m.adjustableMinX);
            setQrealAttribute(adjustableElement, CAF_XMAX, m.adjustableMaxX);
            setQrealAttribute(adjustableElement, CAF_YMIN, m.adjustableMinY);
            setQrealAttribute(adjustableElement, CAF_YMAX, m.adjustableMaxY);
            adjustableElement.setAttribute(CAF_DATANAME, m.adjustableDataName);
            adjustableElement.setAttribute(CAF_PORT, m.adjustablePort);
            setQrealAttribute(adjustableElement, CAF_XGAIN, m.adjustableGainX);
            setQrealAttribute(adjustableElement, CAF_YGAIN, m.adjustableGainY);
        }

        if(m.isSwitchable)
        {
            QDomElement switchableElement = appendDomElement(movableElement, CAF_SWITCHABLE);
            switchableElement.setAttribute(CAF_DATANAME, m.switchableDataName);
            switchableElement.setAttribute(CAF_PORT, m.switchablePort);
            setQrealAttribute(switchableElement, CAF_ONVALUE, m.switchableOnValue);
            setQrealAttribute(switchableElement, CAF_OFFVALUE, m.switchableOffValue);

            if(m.hideIconOnSwitch)
            {
                switchableElement.setAttribute(CAF_HIDEICON, HMF_TRUETAG);
            }
            else
            {
                switchableElement.setAttribute(CAF_HIDEICON, HMF_FALSETAG);
            }
        }

        if(m.isIndicator)
        {
            QDomElement indicatorElement = appendDomElement(movableElement, CAF_INDICATOR);
            indicatorElement.setAttribute(CAF_DATANAME, m.indicatorDataName);
            indicatorElement.setAttribute(CAF_PORT, m.indicatorPort);
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

QString ModelObjectAppearance::getFullTypeName(const QString sep) const
{
    if (mSubTypeName.isEmpty())
    {
        return getTypeName();
    }
    else
    {
        return getTypeName()+sep+getSubTypeName();
    }
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

const QStringList &ModelObjectAppearance::getHelpLinks() const
{
    return mHelpLinks;
}

QString ModelObjectAppearance::getHelpHtmlPath() const
{
    return mHelpHtmlPath;
}

const QMap<QString, QString> &ModelObjectAppearance::getOverridedDefaultParameters() const
{
    return mOverridedDefaultParameters;
}

bool ModelObjectAppearance::isParameterHidden(const QString &name) const
{
    return mHiddenParameters.contains(name);
}

//! @brief Get the full Icon path for specified graphics type
//! @param [in] gfxType The graphics type enum (ISO or USER)
//! If the specified type is missing, return the other type.
//! If that is also missing return a path to the missing graphics icon
QString ModelObjectAppearance::getFullAvailableIconPath(GraphicsTypeEnumT gfxType)
{
    // Determine which type to use based on availability of icons
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

//! @brief Returns the path to the graphics icon of requested type, regardless of whether it is valid or not
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

//! @todo This is a bit wrong, it will return the scale for the available type not necessarily the requested
double ModelObjectAppearance::getIconScale(const GraphicsTypeEnumT gfxType)
{
    // Determine which type to use based on availability of icons
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
    //Check if given filepath is absolute or relative, if relative assume relative to basepath, but don do anything if path empty (no icon specified)
    if (!mUserIconAppearance.mRelativePath.isEmpty())
    {
        QFileInfo relUserPath(mUserIconAppearance.mRelativePath);
        relUserPath.setFile(QDir(mBasePath), mUserIconAppearance.mRelativePath);
        mUserIconAppearance.mAbsolutePath = relUserPath.absoluteFilePath();
    }

    if (!mIsoIconAppearance.mRelativePath.isEmpty())
    {
        QFileInfo relIsoPath(mIsoIconAppearance.mRelativePath);
        relIsoPath.setFile(QDir(mBasePath), mIsoIconAppearance.mRelativePath);
        mIsoIconAppearance.mAbsolutePath = relIsoPath.absoluteFilePath();
    }

}

//! @todo This is a bit wrong, it will return the scale for the available type not necessarily the requested
QString ModelObjectAppearance::getIconRotationBehaviour(const GraphicsTypeEnumT gfxType)
{
    // Determine which type to use based on availability of icons
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
            mHelpText.clear();
            // We read the help text line by line and remove whitespaces from the front of each line
            QString text = xmlHelpText.text();
            QTextStream ts(&text);
            while (!ts.atEnd())
            {
                mHelpText.append(ts.readLine().trimmed()).append("\n");
            }
        }

        QDomElement xmlHelpPicture = xmlHelp.firstChildElement(CAF_HELPPICTURE);
        if (!xmlHelpPicture.isNull())
        {
            mHelpPicture = xmlHelpPicture.text();
        }

        QDomElement xmlHelpLink = xmlHelp.firstChildElement(CAF_HELPLINK);
        while (!xmlHelpLink.isNull())
        {
            mHelpLinks.append(xmlHelpLink.text());
            xmlHelpLink = xmlHelpLink.nextSiblingElement(CAF_HELPLINK);
        }

        QDomElement xmlHelpHtml = xmlHelp.firstChildElement(CAF_HELPHTML);
        if (!xmlHelpHtml.isNull())
        {
            mHelpHtmlPath = xmlHelpHtml.text();
        }
    }

    QDomElement xmlParameters = domElement.firstChildElement(CAF_PARAMETERS);
    if(!xmlParameters.isNull())
    {
        QDomElement xmlParameter = xmlParameters.firstChildElement(CAF_PARAMETER);
        while(!xmlParameter.isNull())
        {
            QString name = xmlParameter.attribute(CAF_NAME);
            QString value = xmlParameter.text();
            mOverridedDefaultParameters.insert(name, value);
            if(parseAttributeBool(xmlParameter, CAF_HIDDEN, false))
            {
                mHiddenParameters.append(name);
            }
            xmlParameter = xmlParameter.nextSiblingElement(CAF_PARAMETER);
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
            //! @todo maybe have a DefaultIconAppearance object to, an load all data
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
                // We need to copy data, not replace as there may be pointers to data (which is kind of unsafe)
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
            gpLibraryHandler->addReplacement(mTypeName, typeName); //!< @todo mainwindow and library should not be called in this file it is supposed to be a leaf class
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
    QDomElement xmlIcon2 = domElement.firstChildElement(CAF_ICON);
    if (!xmlIcon2.isNull())
    {
        mIsoIconAppearance.mRelativePath = xmlIcon2.attribute(CAF_ISOPATH);
        mUserIconAppearance.mRelativePath = xmlIcon2.attribute(CAF_USERPATH);
        mIsoIconAppearance.mRotationBehaviour = xmlIcon2.attribute(CAF_RELATIVE);
        mUserIconAppearance.mRotationBehaviour = xmlIcon2.attribute(CAF_RELATIVE);
        mUserIconAppearance.mScale = parseAttributeQreal(xmlIcon2, CAF_USERSCALE, 1.0);
        mIsoIconAppearance.mScale = parseAttributeQreal(xmlIcon2, CAF_ISOSCALE, 1.0);
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
    if(!mHelpText.isEmpty() || !mHelpPicture.isEmpty() || !mHelpLinks.isEmpty() || !mHelpHtmlPath.isEmpty() )
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

        for( QString &link : mHelpLinks )
        {
            appendDomTextNode(xmlHelp, CAF_HELPLINK, link);
        }

        if( !mHelpHtmlPath.isEmpty() )
        {
            appendDomTextNode(xmlHelp, CAF_HELPHTML, mHelpHtmlPath);
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
void ModelObjectAppearance::saveSpecificPortsToDomElement(QDomElement &rDomElement, const QStringList &rPortNames)
{
    //! @todo maybe make the port appearance  class capable of saving itself to DOM
    QDomElement xmlModelObject = addModelObjectRootElement(rDomElement);

    // First check if the ports element already exist, else add the element
    QDomElement xmlPorts = getOrAppendNewDomElement(xmlModelObject, CAF_PORTS);
    for (const QString &portName : rPortNames)
    {
        appendPortDomElement(xmlPorts, portName, mPortAppearanceMap.value(portName));
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
    //else don't do anything
}

QString ModelObjectAppearance::getHmfFile() const
{
    return mHmfFile;
}


//! @brief Check if specified Icon path is available and icon exists
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

bool ModelObjectAppearance::iconValid(const GraphicsTypeEnumT gfxType) const
{
    if (gfxType == UserGraphics)
    {
        return mUserIconAppearance.mIsValid;
    }
    else if (gfxType == ISOGraphics)
    {
        return mIsoIconAppearance.mIsValid;
    }
    return false;
}

void ModelObjectAppearance::refreshIconValid()
{
    mUserIconAppearance.mIsValid = hasIcon(UserGraphics);
    mIsoIconAppearance.mIsValid = hasIcon(ISOGraphics);
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


void ModelObjectAnimationMovableData::readFromDomElement(QDomElement &rDomElement, QString basePath)
{
    if(rDomElement.hasAttribute(CAF_IDX))
    {
        idx = rDomElement.attribute(CAF_IDX).toInt();
    }

    if(iconPath.isEmpty() && !rDomElement.firstChildElement(CAF_ICON).isNull() && !rDomElement.firstChildElement(CAF_ICON).attribute(CAF_USERPATH).isEmpty())
    {
        iconPath = rDomElement.firstChildElement(CAF_ICON).attribute(CAF_USERPATH);
    }

    QDomElement dataElement = rDomElement.firstChildElement("data");
    if(!dataElement.isNull())
    {
        dataPorts.clear();
        dataNames.clear();
    }
    while(!dataElement.isNull())
    {
        int idx = dataElement.attribute(CAF_IDX).toInt();
        while(dataPorts.size() < idx+1)
        {
            dataPorts.append("");
            dataNames.append(""); //Assume names and ports always have same size
        }
        dataPorts[idx] = dataElement.attribute(CAF_PORT);
        dataNames[idx] = dataElement.attribute(CAF_DATANAME);
        dataElement = dataElement.nextSiblingElement("data");
    }

    QDomElement multiplierElement = rDomElement.firstChildElement(CAF_MULTIPLIER);
    if(!multiplierElement.isNull())
    {
        multipliers.clear();
    }
    while(!multiplierElement.isNull())
    {
        multipliers.append(multiplierElement.attribute(CAF_NAME));
        multiplierElement = multiplierElement.nextSiblingElement(CAF_MULTIPLIER);
    }

    QDomElement divisorElement = rDomElement.firstChildElement(CAF_DIVISOR);
    if(!divisorElement.isNull())
    {
        divisors.clear();
    }
    while(!divisorElement.isNull())
    {
        divisors.append(divisorElement.attribute(CAF_NAME));
        divisorElement = divisorElement.nextSiblingElement(CAF_DIVISOR);
    }

    QDomElement xmlMovement = rDomElement.firstChildElement(CAF_MOVEMENT);
    if(!xmlMovement.isNull())
    {
        movementData.clear();
    }
    while(!xmlMovement.isNull())
    {
        ModelObjectAnimationMovementData movement;
        movement.readFromDomElement(xmlMovement);
        movementData.append(movement);
        xmlMovement = xmlMovement.nextSiblingElement(CAF_MOVEMENT);
    }
    if(!rDomElement.firstChildElement(CAF_START).isNull())
    {
        startX = rDomElement.firstChildElement(CAF_START).attribute("x").toDouble();
        startY = rDomElement.firstChildElement(CAF_START).attribute("y").toDouble();
        startTheta = rDomElement.firstChildElement(CAF_START).attribute("a").toDouble();
    }

    if(!rDomElement.firstChildElement(CAF_INITSCALE).isNull())
    {
        initScaleX = rDomElement.firstChildElement(CAF_INITSCALE).attribute("x").toDouble();
        initScaleY = rDomElement.firstChildElement(CAF_INITSCALE).attribute("y").toDouble();
    }
    else
    {
        initScaleX = 1;
        initScaleY = 1;
    }

    QDomElement xmlResize = rDomElement.firstChildElement(CAF_RESIZE);

    resizeData.clear();

    while(!xmlResize.isNull())
    {
        ModelObjectAnimationResizeData resize;
        resize.readFromDomElement(xmlResize);
        resizeData.append(resize);
        xmlResize = xmlResize.nextSiblingElement(CAF_RESIZE);
    }


    QDomElement xmlInitColor = rDomElement.firstChildElement(CAF_INITCOLOR);
    QDomElement xmlColor = rDomElement.firstChildElement(CAF_COLOR);
    if(!xmlInitColor.isNull() && !xmlColor.isNull())
    {
        colorData.readFromDomElements(xmlInitColor, xmlColor);
    }

    if(!rDomElement.firstChildElement(CAF_TRANSFORMORIGIN).isNull())
    {
        transformOriginX = rDomElement.firstChildElement(CAF_TRANSFORMORIGIN).attribute("x").toDouble();
        transformOriginY = rDomElement.firstChildElement(CAF_TRANSFORMORIGIN).attribute("y").toDouble();
    }

    QFileInfo movableIconFileInfo(iconPath);
    if (movableIconFileInfo.isRelative() && !iconPath.isEmpty())
    {
        movableIconFileInfo.setFile(QDir(basePath), iconPath);
        iconPath = movableIconFileInfo.absoluteFilePath();
    }
    QDomElement xmlAdjustable = rDomElement.firstChildElement(CAF_ADJUSTABLE);
    if(!xmlAdjustable.isNull())
    {
        isAdjustable = true;
        adjustableMinX = xmlAdjustable.attribute(CAF_XMIN).toDouble();
        adjustableMaxX = xmlAdjustable.attribute(CAF_XMAX).toDouble();
        adjustableMinY = xmlAdjustable.attribute(CAF_YMIN).toDouble();
        adjustableMaxY = xmlAdjustable.attribute(CAF_YMAX).toDouble();
        adjustablePort = xmlAdjustable.attribute(CAF_PORT);
        adjustableDataName = xmlAdjustable.attribute(CAF_DATANAME);
        adjustableGainX = xmlAdjustable.attribute(CAF_XGAIN).toDouble();
        adjustableGainY = xmlAdjustable.attribute(CAF_YGAIN).toDouble();
    }
    else
    {
        isAdjustable = false;
    }

    QDomElement xmlSwitchable = rDomElement.firstChildElement(CAF_SWITCHABLE);
    if(!xmlSwitchable.isNull())
    {
        isSwitchable = true;
        switchableOffValue = xmlSwitchable.attribute(CAF_OFFVALUE).toDouble();
        switchableOnValue = xmlSwitchable.attribute(CAF_ONVALUE).toDouble();
        switchablePort = xmlSwitchable.attribute(CAF_PORT);
        switchableDataName = xmlSwitchable.attribute(CAF_DATANAME);
        hideIconOnSwitch = parseAttributeBool(xmlSwitchable, CAF_HIDEICON, false);
    }
    else
    {
        isSwitchable = false;
        switchableOffValue = 0;
        switchableOnValue = 0;
        switchablePort = QString();
        switchableDataName = QString();
        hideIconOnSwitch = false;
    }

    QDomElement xmlIndicaor = rDomElement.firstChildElement(CAF_INDICATOR);
    if(!xmlIndicaor.isNull())
    {
        isIndicator = true;
        indicatorPort = xmlIndicaor.attribute(CAF_PORT);
        indicatorDataName = xmlIndicaor.attribute(CAF_DATANAME);
    }
    else
    {
        isIndicator = false;
        indicatorPort = QString();
        indicatorDataName = QString();
    }


    QDomElement xmlMovingPorts = rDomElement.firstChildElement(CAF_MOVINGPORT);
    if(!xmlMovingPorts.isNull())
    {
        movablePortNames.clear();
        movablePortStartX.clear();
        movablePortStartY.clear();
    }
    while(!xmlMovingPorts.isNull())
    {
        QString portName = xmlMovingPorts.attribute(CAF_PORTNAME);
        if(!portName.isEmpty() && !movablePortNames.contains(portName))   //Don't load tags without a port name, and tags that has already been loaded
        {
            movablePortNames.append(xmlMovingPorts.attribute(CAF_PORTNAME));
            movablePortStartX.append(xmlMovingPorts.attribute(CAF_STARTX).toDouble());
            movablePortStartY.append(xmlMovingPorts.attribute(CAF_STARTY).toDouble());
        }
        xmlMovingPorts = xmlMovingPorts.nextSiblingElement(CAF_MOVINGPORT);
    }

    QDomElement xmlRelative = rDomElement.firstChildElement(CAF_RELATIVE);
    if(!xmlRelative.isNull())
    {
        movableRelative = xmlRelative.attribute(CAF_IDX).toInt();
    }
    else
    {
        movableRelative = -1;
    }
}


//! @brief Loads animation movement data from dom element
void ModelObjectAnimationMovementData::readFromDomElement(QDomElement &rDomElement)
{
    dataIdx = rDomElement.attribute(CAF_IDX).toDouble();
    x = rDomElement.attribute("x").toDouble();
    y = rDomElement.attribute("y").toDouble();
    theta = rDomElement.attribute("a").toDouble();
    divisor = rDomElement.attribute(CAF_DIVISOR);
    multiplier = rDomElement.attribute(CAF_MULTIPLIER);
}


//! @brief Saves animation movement data to dom element
void ModelObjectAnimationMovementData::saveToDomElement(QDomElement &rDomElement) const
{
    setQrealAttribute(rDomElement, "x", x);
    setQrealAttribute(rDomElement, "y", y);
    setQrealAttribute(rDomElement, "a", theta);
    rDomElement.setAttribute(CAF_IDX, dataIdx);
    rDomElement.setAttribute(CAF_DIVISOR, divisor);
    rDomElement.setAttribute(CAF_MULTIPLIER, multiplier);
}


//! @brief Loads animation resize data from dom element
void ModelObjectAnimationResizeData::readFromDomElement(QDomElement &rDomElement)
{
    x = rDomElement.attribute("x").toDouble();
    y = rDomElement.attribute("y").toDouble();
    if(rDomElement.hasAttribute("idx1"))
        dataIdx1 = parseAttributeInt(rDomElement, "idx1", -1);
    else
        dataIdx1 = parseAttributeInt(rDomElement, "idx", 0);   //For backwards compatibility
    dataIdx2 = parseAttributeInt(rDomElement, "idx2", -1);
    divisor = rDomElement.attribute(CAF_DIVISOR);
    multiplier = rDomElement.attribute(CAF_MULTIPLIER);
}

void ModelObjectAnimationResizeData::saveToDomElement(QDomElement &rDomElement) const
{
    setQrealAttribute(rDomElement, "x", x);
    setQrealAttribute(rDomElement, "y", y);
    rDomElement.setAttribute("idx1", dataIdx1);
    rDomElement.setAttribute("idx2", dataIdx2);
    rDomElement.setAttribute(CAF_DIVISOR, divisor);
    rDomElement.setAttribute(CAF_MULTIPLIER, multiplier);
}


void ModelObjectAnimationColorData::readFromDomElements(QDomElement &rInitDomElement, QDomElement &rDomElement)
{
    initR = rInitDomElement.attribute("r").toDouble();
    initG = rInitDomElement.attribute("g").toDouble();
    initB = rInitDomElement.attribute("b").toDouble();
    initA = rInitDomElement.attribute("a").toDouble();

    dataIdx = rDomElement.attribute(CAF_IDX).toInt();
    r = rDomElement.attribute("r").toDouble();
    g = rDomElement.attribute("g").toDouble();
    b = rDomElement.attribute("b").toDouble();
    a = rDomElement.attribute("a").toDouble();
    divisor = rDomElement.attribute(CAF_DIVISOR);
    multiplier = rDomElement.attribute(CAF_MULTIPLIER);
}

void ModelObjectAnimationColorData::saveToDomElements(QDomElement &rInitDomElement, QDomElement &rDomElement) const
{
    setQrealAttribute(rInitDomElement, "r", initR);
    setQrealAttribute(rInitDomElement, "g", initG);
    setQrealAttribute(rInitDomElement, "b", initB);
    setQrealAttribute(rInitDomElement, "a", initA);

    setQrealAttribute(rDomElement, "r", r);
    setQrealAttribute(rDomElement, "g", g);
    setQrealAttribute(rDomElement, "b", b);
    setQrealAttribute(rDomElement, "a", a);
    rDomElement.setAttribute(CAF_IDX, dataIdx);
    rDomElement.setAttribute(CAF_DIVISOR, divisor);
    rDomElement.setAttribute(CAF_MULTIPLIER, multiplier);
}
