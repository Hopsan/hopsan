/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

namespace caf {
    constexpr auto name = "name";
    constexpr auto path = "path";
    constexpr auto parametersets = "parametersets";
    constexpr auto parameterset = "parameterset";
    constexpr auto typenametag = "typename";
    constexpr auto subtypename = "subtypename";
    constexpr auto type = "type";
    constexpr auto displayname = "displayname";
    constexpr auto sourcecode = "sourcecode";
    constexpr auto libpath = "libpath";
    constexpr auto recompilable = "recompilable";
    constexpr auto idx = "idx";
    constexpr auto hmffile = "hmffile";
    constexpr auto icon = "icon";
    constexpr auto icons = "icons";
    constexpr auto scale = "scale";
    constexpr auto iconrotation = "iconrotation";
    constexpr auto userpath = "userpath";
    constexpr auto isopath = "isopath";
    constexpr auto userscasle = "userscale";
    constexpr auto isoscale = "isoscale";
    constexpr auto parameters = "defaultparameters";
    constexpr auto parameter = "parameter";
    constexpr auto hidden = "hidden";
    constexpr auto ports = "ports";
    constexpr auto port = "port";
    constexpr auto portpositions = "portpositions";
    constexpr auto portpose = "portpose";
    constexpr auto replacables = "replacables";
    constexpr auto replacable = "replacable";
    constexpr auto autoplaced = "autoplaced";
    constexpr auto enabled = "enabled";
    namespace help {
        constexpr auto root = "help";
        constexpr auto text = "text";
        constexpr auto picture = "picture";
        constexpr auto link = "link";
        constexpr auto html = "html";
        constexpr auto markdown = "md";
    }
    namespace animation {
        constexpr auto root = "animation";
        constexpr auto adjustable = "adjustable";
        constexpr auto xgain = "xgain";
        constexpr auto ygain = "ygain";
        constexpr auto switchable = "switchable";
        constexpr auto momentary = "momentary";
        constexpr auto indicator = "indicator";
        constexpr auto portname = "portname";
        constexpr auto startx = "startx";
        constexpr auto starty = "starty";
        constexpr auto movingport = "movingport";
        constexpr auto relative = "relative";
        constexpr auto movable = "movable";
        constexpr auto hideicon = "hideicon";
        constexpr auto dataname = "dataname";
        constexpr auto onvalue = "onvalue";
        constexpr auto offvalue = "offvalue";
        constexpr auto xmin = "xmin";
        constexpr auto xmax = "xmax";
        constexpr auto ymin = "ymin";
        constexpr auto ymax = "ymax"     ;
        constexpr auto multiplier = "multiplier";
        constexpr auto divisor = "divisor";
        constexpr auto start = "start";
        constexpr auto movement = "movement";
        constexpr auto initscale = "initscale";
        constexpr auto resize = "resize";
        constexpr auto initcolor = "initcolor";
        constexpr auto color = "color";
        constexpr auto transformorigin = "transformorigin";
        constexpr auto flowspeed = "flowspeed";
        constexpr auto hydraulicminpressure = "hydraulicminpressure";
        constexpr auto hydraulicmaxpressure = "hydraulicmaxpressure";
    }
}

// =============== Help Functions ===============
QDomElement appendOrGetCAFRootTag(QDomElement parentElement)
{
   QDomElement cafroot = getOrAppendNewDomElement(parentElement, caf::root);
   cafroot.setAttribute(caf::version, CAF_VERSIONNUM);
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
    rName = domElement.attribute(caf::name);
    bool dummy;
    parsePoseTag(domElement, rX, rY, rTheta, dummy);
}

//! @brief Help function that writes PortAppearance to DOM Element
//! @param [in,out] domElement The DOM element to append to
//! @param [in] portName QString containing the port name
//! @param [in] rPortAppearance Reference to PortAppearance object to be written
void appendPortDomElement(QDomElement &rDomElement, const QString portName, const PortAppearance &rPortAppearance)
{
    QDomElement xmlPort = appendDomElement(rDomElement, caf::port);
    xmlPort.setAttribute(caf::name, portName);
    setQrealAttribute(xmlPort, "x", rPortAppearance.x, 10, 'g');
    setQrealAttribute(xmlPort, "y", rPortAppearance.y, 10, 'g');
    setQrealAttribute(xmlPort, "a", rPortAppearance.rot, 6, 'g');
    if(rPortAppearance.mAutoPlaced)
    {
        xmlPort.setAttribute(caf::autoplaced, hmf::truetag);
    }
    else
    {
        xmlPort.setAttribute(caf::autoplaced, hmf::falsetag);
    }
    if(rPortAppearance.mEnabled)
    {
        xmlPort.setAttribute(caf::enabled, hmf::truetag);
    }
    else
    {
        xmlPort.setAttribute(caf::enabled, hmf::falsetag);
    }
}

//! @brief Help function that parses a port DOM Element
//! @param [in] domElement The DOM element to parse
//! @param [out] rPortName Reference to QString that will contain port name
//! @param [out] rPortAppearance Reference to PortAppearance object that will contain parsed data
void parsePortDomElement(QDomElement domElement, QString &rPortName, PortAppearance &rPortAppearance)
{
    rPortName = domElement.attribute(caf::name);
    rPortAppearance.x = parseAttributeQreal(domElement, "x", 0);
    rPortAppearance.y = parseAttributeQreal(domElement, "y", 0);
    rPortAppearance.rot = parseAttributeQreal(domElement, "a", 0);

    rPortAppearance.mAutoPlaced = parseAttributeBool(domElement, caf::autoplaced, true);
    rPortAppearance.mEnabled = parseAttributeBool(domElement, caf::enabled, true);
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
        QDomElement iconElement = rDomElement.firstChildElement(caf::icon);
        if(!iconElement.isNull())
        {
            baseIconPath = iconElement.attribute(caf::userpath);
        }

        QFileInfo baseIconFileInfo(baseIconPath);
        if (baseIconFileInfo.isRelative() && !baseIconPath.isEmpty())
        {
            baseIconFileInfo.setFile(QDir(basePath), baseIconPath);
            baseIconPath = baseIconFileInfo.absoluteFilePath();
        }

        if(rDomElement.hasAttribute(caf::animation::flowspeed))
        {
            flowSpeed = rDomElement.attribute(caf::animation::flowspeed).toDouble();
        }
        else
        {
            flowSpeed = 100;
        }

        if(rDomElement.hasAttribute(caf::animation::hydraulicminpressure))
        {
            hydraulicMinPressure = rDomElement.attribute(caf::animation::hydraulicminpressure).toDouble();
        }
        else
        {
            hydraulicMinPressure = 0;
        }

        if(rDomElement.hasAttribute(caf::animation::hydraulicmaxpressure))
        {
            hydraulicMaxPressure = rDomElement.attribute(caf::animation::hydraulicmaxpressure).toDouble();
        }
        else
        {
            hydraulicMaxPressure = 2e7;
        }

        QDomElement xmlMovable = rDomElement.firstChildElement(caf::animation::movable);
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
            xmlMovable = xmlMovable.nextSiblingElement(caf::animation::movable);
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
    rDomElement.setAttribute(caf::animation::flowspeed, flowSpeed);
    rDomElement.setAttribute(caf::animation::hydraulicminpressure, hydraulicMinPressure);
    rDomElement.setAttribute(caf::animation::hydraulicmaxpressure, hydraulicMaxPressure);
    for(const ModelObjectAnimationMovableData &m : movables) {
        QDomElement movableElement = appendDomElement(rDomElement, caf::animation::movable);

        //! @note Saving icons is disabled, because it probably makes no sense (paths will not work if moving hmf to other location)
        //QDomElement iconElement = appendDomElement(movableElement, CAF_ICON);
        //iconElement.setAttribute(CAF_USERPATH, m.iconPath);

        if(m.idx >= 0)
        {
            movableElement.setAttribute(caf::idx, m.idx);
        }

        for(int i=0; i<m.dataNames.size(); ++i)
        {
            if(!m.dataNames[i].isEmpty())
            {
                QDomElement dataElement = appendDomElement(movableElement, "data");
                dataElement.setAttribute(caf::port, m.dataPorts[i]);
                dataElement.setAttribute(caf::animation::dataname, m.dataNames[i]);
                dataElement.setAttribute(caf::idx, i);
            }
        }

        for(int i=0; i<m.multipliers.size(); ++i)
        {
            QDomElement multiplierElement = appendDomElement(movableElement, caf::animation::multiplier);
            multiplierElement.setAttribute(caf::name, m.multipliers[i]);
        }

        for(int i=0; i<m.divisors.size(); ++i)
        {
            QDomElement divisorElement = appendDomElement(movableElement, caf::animation::divisor);
            divisorElement.setAttribute(caf::name, m.divisors[i]);
        }

        QDomElement startElement = appendDomElement(movableElement, caf::animation::start);
        setQrealAttribute(startElement, "x", m.startX);
        setQrealAttribute(startElement, "y", m.startY);
        setQrealAttribute(startElement, "a", m.startTheta);

        for(int i=0; i<m.movementData.size(); ++i)
        {
            QDomElement movementElement = appendDomElement(movableElement, caf::animation::movement);
            m.movementData[i].saveToDomElement(movementElement);
        }

        QDomElement initScaleElement = appendDomElement(movableElement, caf::animation::initscale);
        setQrealAttribute(initScaleElement, "x", m.initScaleX);
        setQrealAttribute(initScaleElement, "y", m.initScaleY);

        for(int i=0; i<m.resizeData.size(); ++i)
        {
            QDomElement resizeElement = appendDomElement(movableElement, caf::animation::resize);
            m.resizeData[i].saveToDomElement(resizeElement);
        }

        QDomElement initColorElement = appendDomElement(movableElement, caf::animation::initcolor);
        QDomElement colorElement = appendDomElement(movableElement, caf::animation::color);
        m.colorData.saveToDomElements(initColorElement, colorElement);

        QDomElement transformOriginElement = appendDomElement(movableElement, caf::animation::transformorigin);
        setQrealAttribute(transformOriginElement, "x", m.transformOriginX);
        setQrealAttribute(transformOriginElement, "y", m.transformOriginY);

        for(int i=0; i<m.movablePortNames.size(); ++i)
        {
            QDomElement movingPortElement = appendDomElement(movableElement, caf::animation::movingport);
            movingPortElement.setAttribute(caf::animation::portname, m.movablePortNames[i]);
            setQrealAttribute(movingPortElement, caf::animation::startx, m.movablePortStartX[i]);
            setQrealAttribute(movingPortElement, caf::animation::starty, m.movablePortStartY[i]);
        }

        QDomElement relativeElement = appendDomElement(movableElement, caf::animation::relative);
        relativeElement.setAttribute(caf::idx, m.movableRelative);

        if(m.isAdjustable)
        {
            QDomElement adjustableElement = appendDomElement(movableElement, caf::animation::adjustable);
            setQrealAttribute(adjustableElement, caf::animation::xmin, m.adjustableMinX);
            setQrealAttribute(adjustableElement, caf::animation::xmax, m.adjustableMaxX);
            setQrealAttribute(adjustableElement, caf::animation::ymin, m.adjustableMinY);
            setQrealAttribute(adjustableElement, caf::animation::ymax, m.adjustableMaxY);
            adjustableElement.setAttribute(caf::animation::dataname, m.adjustableDataName);
            adjustableElement.setAttribute(caf::port, m.adjustablePort);
            setQrealAttribute(adjustableElement, caf::animation::xgain, m.adjustableGainX);
            setQrealAttribute(adjustableElement, caf::animation::ygain, m.adjustableGainY);
        }

        if(m.isSwitchable)
        {
            QDomElement switchableElement = appendDomElement(movableElement, caf::animation::switchable);
            switchableElement.setAttribute(caf::animation::dataname, m.switchableDataName);
            switchableElement.setAttribute(caf::port, m.switchablePort);

            setQrealAttribute(switchableElement, caf::animation::onvalue, m.switchableOnValue);
            setQrealAttribute(switchableElement, caf::animation::offvalue, m.switchableOffValue);

            if(m.isMomentary)
            {
                switchableElement.setAttribute(caf::animation::momentary, hmf::truetag);
            }
            else
            {
                switchableElement.setAttribute(caf::animation::momentary, hmf::falsetag);
            }

            if(m.hideIconOnSwitch)
            {
                switchableElement.setAttribute(caf::animation::hideicon, hmf::truetag);
            }
            else
            {
                switchableElement.setAttribute(caf::animation::hideicon, hmf::falsetag);
            }
        }

        if(m.isIndicator)
        {
            QDomElement indicatorElement = appendDomElement(movableElement, caf::animation::indicator);
            indicatorElement.setAttribute(caf::animation::dataname, m.indicatorDataName);
            indicatorElement.setAttribute(caf::port, m.indicatorPort);
        }
    }
}

ModelObjectAnimationMovableData *ModelObjectAnimationData::getMovablePtr(int idx)
{
    for(int i=0; i<movables.size(); ++i) {
        if(movables[i].idx == idx) {
            return &movables[i];
        }
    }
    return nullptr;
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

ModelObjectAppearance::ModelObjectAppearance(const ModelObjectAppearance &other)
{
    *this = other;
}

ModelObjectAppearance &ModelObjectAppearance::operator=(const ModelObjectAppearance &other)
{
    // Handle self assignment
    if (&other == this)
    {
        return *this;
    }

    mHmfFile = other.mHmfFile;
    mTypeName = other.mTypeName;
    mSubTypeName = other.mSubTypeName;
    mDisplayName = other.mDisplayName;
    mSourceCode = other.mSourceCode;
    mLibPath = other.mLibPath;
    mIsRecompilable = other.mIsRecompilable;
    mHelpText = other.mHelpText;
    mHelpPicture = other.mHelpPicture;
    mHelpHtmlPath = other.mHelpHtmlPath;
    mHelpLinks = other.mHelpLinks;
    mOverridedDefaultParameters = other.mOverridedDefaultParameters;
    mHiddenParameters = other.mHiddenParameters;
    mIsoIconAppearance = other.mIsoIconAppearance;
    mUserIconAppearance = other.mUserIconAppearance;
    mIsoIcon = other.mIsoIcon;
    mUserIcon = other.mUserIcon;
    mDefaultMissingIconPath = other.mDefaultMissingIconPath;
    mNameTextPos = other.mNameTextPos;
    mReplacementObjects = other.mReplacementObjects;
    mParameterSets = other.mParameterSets;

    // OK, this is a hack, the port appearance map is actually a map of shared pointers,
    // but we want a "deep copy" so lets fix that
    for (auto it = other.mPortAppearanceMap.begin(); it != other.mPortAppearanceMap.end(); ++it)
    {
        PortAppearance *pPA = new PortAppearance();
        *pPA = *(it.value().data()); // Copy
        addPortAppearance(it.key(), SharedPortAppearanceT(pPA));
    }

    mAnimationData = other.mAnimationData;
    mBasePath = other.mBasePath;

    return *this;
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

    // Check if given filepath is absolute or relative, if absolute assume we want relative to basepath
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
    // If relative assume relative to basepath, if empty then clear absolute paths
    if (mUserIconAppearance.mRelativePath.isEmpty())
    {
        mUserIconAppearance.mAbsolutePath.clear();
    }
    else
    {
        QFileInfo relUserPath;
        relUserPath.setFile(QDir(mBasePath), mUserIconAppearance.mRelativePath);
        mUserIconAppearance.mAbsolutePath = relUserPath.absoluteFilePath();
    }

    if (mIsoIconAppearance.mRelativePath.isEmpty())
    {
        mIsoIconAppearance.mAbsolutePath.clear();
    }
    else
    {
        QFileInfo relIsoPath;
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

//! @brief Returns a list of available parameter sets for model object
//! @returns Map with names and corresponding SSV file paths
QMap<QString, QString> ModelObjectAppearance::getParameterSets() const
{
    return mParameterSets;
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

const SharedPortAppearanceT ModelObjectAppearance::getPortAppearance(const QString &rPortName) const
{
    PortAppearanceMapT::const_iterator it = mPortAppearanceMap.find(rPortName);
    if (it != mPortAppearanceMap.end())
    {
        return it.value();
    }
    return SharedPortAppearanceT();
}

SharedPortAppearanceT ModelObjectAppearance::getPortAppearance(const QString &rPortName)
{
    PortAppearanceMapT::iterator it = mPortAppearanceMap.find(rPortName);
    if (it != mPortAppearanceMap.end())
    {
        return it.value();
    }
    return SharedPortAppearanceT();
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
void ModelObjectAppearance::addPortAppearance(const QString portName, SharedPortAppearanceT pPortAppearance)
{
    if (pPortAppearance)
    {
        mPortAppearanceMap.insert(portName, pPortAppearance);
    }
    else
    {
        mPortAppearanceMap.insert(portName, SharedPortAppearanceT(new PortAppearance()));
    }
}

//! @brief Get the base path that all icon paths are relative
QString ModelObjectAppearance::getBasePath() const
{
    return mBasePath;
}

//! @brief Get the component appearance XML file
QFileInfo ModelObjectAppearance::getXMLFile() const
{
    return mXMLFile;
}

//! @brief Read the ModelObjectAppearance contents from an XML DOM Element
void ModelObjectAppearance::readFromDomElement(QDomElement domElement)
{
    mHmfFile        = domElement.attribute(caf::hmffile, mHmfFile);
    mTypeName       = domElement.attribute(caf::typenametag, mTypeName);
    mSubTypeName    = domElement.attribute(caf::subtypename, mSubTypeName);
    mDisplayName    = domElement.attribute(caf::displayname, mDisplayName);
    QString newSourceCode     = domElement.attribute(caf::sourcecode, "");
    if(!newSourceCode.isEmpty())
        mSourceCode = newSourceCode;
    QString newLibPath        = domElement.attribute(caf::libpath, "");
    if(!newLibPath.isEmpty())
        mLibPath = newLibPath;
    mIsRecompilable   = parseAttributeBool(domElement, caf::recompilable, false);

    //Use typename if displayname not set
    if (mDisplayName.isEmpty())
    {
        mDisplayName = mTypeName;
    }

    QDomElement xmlHelp = domElement.firstChildElement(caf::help::root);
    if(!xmlHelp.isNull())
    {
        QDomElement xmlHelpText = xmlHelp.firstChildElement(caf::help::text);
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

        QDomElement xmlHelpPicture = xmlHelp.firstChildElement(caf::help::picture);
        if (!xmlHelpPicture.isNull())
        {
            mHelpPicture = xmlHelpPicture.text();
        }

        QDomElement xmlHelpLink = xmlHelp.firstChildElement(caf::help::link);
        while (!xmlHelpLink.isNull())
        {
            mHelpLinks.append(xmlHelpLink.text());
            xmlHelpLink = xmlHelpLink.nextSiblingElement(caf::help::link);
        }

        QDomElement xmlHelpHtml = xmlHelp.firstChildElement(caf::help::html);
        if (!xmlHelpHtml.isNull())
        {
            mHelpHtmlPath = xmlHelpHtml.text();
        }

        QDomElement xmlHelpMD = xmlHelp.firstChildElement(caf::help::markdown);
        if (!xmlHelpMD.isNull())
        {
            mHelpHtmlPath = xmlHelpMD.text();
        }
    }

    //Read parameter sets (name and .ssv file)
    QDomElement xmlParameterSets = domElement.firstChildElement(caf::parametersets);
    if(!xmlParameterSets.isNull())
    {
        QDomElement xmlParameterSet = xmlParameterSets.firstChildElement(caf::parameterset);
        while(!xmlParameterSet.isNull())
        {
            QString name = xmlParameterSet.attribute(caf::name);
            QString path = xmlParameterSet.attribute(caf::path);
            mParameterSets.insert(name, path);
            xmlParameterSet = xmlParameterSet.nextSiblingElement(caf::parameterset);
        }
    }

    QDomElement xmlParameters = domElement.firstChildElement(caf::parameters);
    if(!xmlParameters.isNull())
    {
        QDomElement xmlParameter = xmlParameters.firstChildElement(caf::parameter);
        while(!xmlParameter.isNull())
        {
            QString name = xmlParameter.attribute(caf::name);
            QString value = xmlParameter.text();
            mOverridedDefaultParameters.insert(name, value);
            if(parseAttributeBool(xmlParameter, caf::hidden, false))
            {
                mHiddenParameters.append(name);
            }
            xmlParameter = xmlParameter.nextSiblingElement(caf::parameter);
        }
    }

    //We assume only one icons element
    QDomElement xmlIcons = domElement.firstChildElement(caf::icons);
    QDomElement xmlIcon = xmlIcons.firstChildElement(caf::icon);
    while (!xmlIcon.isNull())
    {
        QString type = xmlIcon.attribute(caf::type);
        if (type == "iso")
        {
            mIsoIconAppearance.mRelativePath = xmlIcon.attribute(caf::path);
            mIsoIconAppearance.mScale = parseAttributeQreal(xmlIcon, caf::scale, 1.0);
            mIsoIconAppearance.mRotationBehaviour = xmlIcon.attribute(caf::iconrotation,mIsoIconAppearance.mRotationBehaviour);
        }
        else if (type == "user")
        {
            mUserIconAppearance.mRelativePath = xmlIcon.attribute(caf::path);
            mUserIconAppearance.mScale = parseAttributeQreal(xmlIcon, caf::scale, 1.0);
            mUserIconAppearance.mRotationBehaviour = xmlIcon.attribute(caf::iconrotation,mUserIconAppearance.mRotationBehaviour);
        }
        else if (type == "defaultmissing")
        {
            //! @todo maybe have a DefaultIconAppearance object to, an load all data
            mDefaultMissingIconPath = xmlIcon.attribute(caf::path);
        }
        //else ignore, maybe should give warning

        xmlIcon = xmlIcon.nextSiblingElement(caf::icon);
    }

    QDomElement xmlPorts = domElement.firstChildElement(caf::ports);
    while (!xmlPorts.isNull())
    {
        QDomElement xmlPort = xmlPorts.firstChildElement(caf::port);
        while (!xmlPort.isNull())
        {
            QString portname;
            PortAppearance portApp;
            parsePortDomElement(xmlPort, portname, portApp);
            if (mPortAppearanceMap.contains(portname))
            {
                // We need to copy data, not replace as there may be shared pointers to existing data
                *mPortAppearanceMap[portname].data() = portApp;
            }
            else
            {
                mPortAppearanceMap.insert(portname, SharedPortAppearanceT(new PortAppearance(portApp)));
            }
            xmlPort = xmlPort.nextSiblingElement(caf::port);
        }
        // There should only be one <ports>, but lets check for more just in case
        xmlPorts = xmlPorts.nextSiblingElement(caf::ports);
    }

    QDomElement xmlReplacables = domElement.firstChildElement(caf::replacables);
    while (!xmlReplacables.isNull())
    {
        QDomElement xmlReplacable = xmlReplacables.firstChildElement(caf::replacable);
        while (!xmlReplacable.isNull())
        {
            QString typeName = xmlReplacable.attribute(caf::typenametag);
            gpLibraryHandler->addReplacement(mTypeName, typeName); //!< @todo mainwindow and library should not be called in this file it is supposed to be a leaf class
            xmlReplacable = xmlReplacable.nextSiblingElement(caf::replacable);
        }

        xmlReplacables = xmlReplacables.nextSiblingElement(caf::replacables);
    }


    // vvvvvvvvvvvvvvvvvvvvv=== Bellow Reads old Format 0.2 Tags ===vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

    QDomElement xmlHelp_02 = domElement.firstChildElement(caf::help::root);
    if(!xmlHelp_02.isNull())
    {
        if (xmlHelp_02.hasAttribute(caf::help::picture))
        {
            mHelpPicture = xmlHelp_02.attribute(caf::help::picture);
        }

        if (xmlHelp_02.hasAttribute(caf::help::text))
        {
            mHelpText = xmlHelp_02.attribute(caf::help::text);
        }
    }

    QString portname;
    QDomElement xmlPorts_02 = domElement.firstChildElement(caf::portpositions);
    while (!xmlPorts_02.isNull())
    {
        QDomElement xmlPortPose = xmlPorts_02.firstChildElement(caf::portpose);
        while (!xmlPortPose.isNull())
        {
            PortAppearance portApp;
            parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
            mPortAppearanceMap.insert(portname, SharedPortAppearanceT(new PortAppearance(portApp)));
            xmlPortPose = xmlPortPose.nextSiblingElement(caf::portpose);
        }
        // There should only be one <ports>, but lets check for more just in case
        xmlPorts_02 = xmlPorts_02.nextSiblingElement(caf::portpositions);
    }
    
    QDomElement xmlAnimation = domElement.firstChildElement(caf::animation::root);
    mAnimationData.readFromDomElement(xmlAnimation, mBasePath);
    

    // vvvvvvvvvvvvvvvvvvvvv=== Bellow Reads old Format 0.1 Tags ===vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

    // Read old style icons
    QDomElement xmlIcon2 = domElement.firstChildElement(caf::icon);
    if (!xmlIcon2.isNull())
    {
        mIsoIconAppearance.mRelativePath = xmlIcon2.attribute(caf::isopath);
        mUserIconAppearance.mRelativePath = xmlIcon2.attribute(caf::userpath);
        mIsoIconAppearance.mRotationBehaviour = xmlIcon2.attribute(caf::animation::relative);
        mUserIconAppearance.mRotationBehaviour = xmlIcon2.attribute(caf::animation::relative);
        mUserIconAppearance.mScale = parseAttributeQreal(xmlIcon2, caf::userscasle, 1.0);
        mIsoIconAppearance.mScale = parseAttributeQreal(xmlIcon2, caf::isoscale, 1.0);
    }

    // Read old style portposes, where portposes were not contained inside a common "ports" element
    QDomElement xmlPortPose = domElement.firstChildElement(caf::portpose);
    while (!xmlPortPose.isNull())
    {
        PortAppearance portApp;
        parsePortPoseTag(xmlPortPose, portname, portApp.x, portApp.y, portApp.rot);
        mPortAppearanceMap.insert(portname, SharedPortAppearanceT(new PortAppearance(portApp)));
        xmlPortPose = xmlPortPose.nextSiblingElement(caf::portpose);
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
    QDomElement more = getOrAppendNewDomElement(parentDomElement, caf::modelobject);
    more.setAttribute(caf::typenametag, mTypeName);
    more.setAttribute(caf::displayname, mDisplayName);
    if (!mSubTypeName.isEmpty())
    {
        more.setAttribute(caf::subtypename, mSubTypeName);
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
    QDomElement xmlIcons = appendDomElement(xmlObject, caf::icons);
    if (hasIcon(UserGraphics))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, caf::icon);
        xmlIcon.setAttribute(caf::type, "user");
        xmlIcon.setAttribute(caf::path, mUserIconAppearance.mRelativePath);
        setQrealAttribute(xmlIcon, caf::scale, mUserIconAppearance.mScale, 6, 'g');
        xmlIcon.setAttribute(caf::iconrotation, mUserIconAppearance.mRotationBehaviour);
    }
    if (hasIcon(ISOGraphics))
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, caf::icon);
        xmlIcon.setAttribute(caf::type, "iso");
        xmlIcon.setAttribute(caf::path, mIsoIconAppearance.mRelativePath);
        setQrealAttribute(xmlIcon, caf::scale, mIsoIconAppearance.mScale, 6, 'g');
        xmlIcon.setAttribute(caf::iconrotation, mIsoIconAppearance.mRotationBehaviour);
    }

    // If default missing have changed, then save that data as well
    //! @todo not hardcoded should be defined
    if (mDefaultMissingIconPath != "missingcomponenticon.svg")
    {
        QDomElement xmlIcon = appendDomElement(xmlIcons, caf::icon);
        xmlIcon.setAttribute(caf::type, "defaultmissing");
        xmlIcon.setAttribute(caf::path, mDefaultMissingIconPath);
    }

    // Save help text and picture data
    if(!mHelpText.isEmpty() || !mHelpPicture.isEmpty() || !mHelpLinks.isEmpty() || !mHelpHtmlPath.isEmpty() )
    {
        QDomElement xmlHelp = appendDomElement(xmlObject, caf::help::root);
        if( !mHelpText.isEmpty() )
        {
            appendDomTextNode(xmlHelp, caf::help::text, mHelpText);
        }

        if( !mHelpPicture.isEmpty() )
        {
            appendDomTextNode(xmlHelp, caf::help::picture, mHelpPicture);
        }

        for( QString &link : mHelpLinks )
        {
            appendDomTextNode(xmlHelp, caf::help::link, link);
        }

        if( !mHelpHtmlPath.isEmpty() )
        {
            appendDomTextNode(xmlHelp, caf::help::html, mHelpHtmlPath);
        }
    }

    // Save port data
    //! @todo maybe make the port appearance  class capable of saving itself to DOM
    QDomElement xmlPorts = appendDomElement(xmlObject, caf::ports);
    PortAppearanceMapT::iterator pit;
    for (pit=mPortAppearanceMap.begin(); pit!=mPortAppearanceMap.end(); ++pit)
    {
        appendPortDomElement(xmlPorts, pit.key(), *pit.value().data());
    }

    QDomElement xmlAnimation;
    if(xmlObject.firstChildElement(caf::animation::root).isNull())
    {
        xmlAnimation = appendDomElement(xmlObject, caf::animation::root);
    }
    else
    {
        xmlAnimation = xmlObject.firstChildElement(caf::animation::root);
    }
    mAnimationData.saveToDomElement(xmlAnimation);
}

//! @brief Convenience function to save only specific ports to dom element, used to save dynamic parameter ports
void ModelObjectAppearance::saveSpecificPortsToDomElement(QDomElement &rDomElement, const QStringList &rPortNames)
{
    //! @todo maybe make the port appearance  class capable of saving itself to DOM
    QDomElement xmlModelObject = addModelObjectRootElement(rDomElement);

    // First check if the ports element already exist, else add the element
    QDomElement xmlPorts = getOrAppendNewDomElement(xmlModelObject, caf::ports);
    for (const QString &portName : rPortNames)
    {
        const SharedPortAppearanceT pData = mPortAppearanceMap.value(portName);
        if (pData)
        {
            appendPortDomElement(xmlPorts, portName, *pData.data());
        }
    }
}

//! @brief Save Appearancedata to XML file
void ModelObjectAppearance::saveToXMLFile(QString filename)
{
    //Save to file
    QDomDocument doc;
    QDomElement cafroot = doc.createElement(caf::root);
    doc.appendChild(cafroot);
    cafroot.setAttribute(caf::version, CAF_VERSIONNUM);
    this->saveToDomElement(cafroot);
    appendRootXMLProcessingInstruction(doc); //The xml "comment" on the first line
    saveXmlFile(filename, gpMessageHandler, [&](){return doc;});
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

//! @brief Access method to set the component appearance XML file
void ModelObjectAppearance::setXMLFile(const QFileInfo file)
{
    mXMLFile = file;
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
    if(rDomElement.hasAttribute(caf::idx))
    {
        idx = rDomElement.attribute(caf::idx).toInt();
    }

    if(iconPath.isEmpty() && !rDomElement.firstChildElement(caf::icon).isNull() && !rDomElement.firstChildElement(caf::icon).attribute(caf::userpath).isEmpty())
    {
        iconPath = rDomElement.firstChildElement(caf::icon).attribute(caf::userpath);
    }

    QDomElement dataElement = rDomElement.firstChildElement("data");
    if(!dataElement.isNull())
    {
        dataPorts.clear();
        dataNames.clear();
    }
    while(!dataElement.isNull())
    {
        int idx = dataElement.attribute(caf::idx).toInt();
        while(dataPorts.size() < idx+1)
        {
            dataPorts.append("");
            dataNames.append(""); //Assume names and ports always have same size
        }
        dataPorts[idx] = dataElement.attribute(caf::port);
        dataNames[idx] = dataElement.attribute(caf::animation::dataname);
        dataElement = dataElement.nextSiblingElement("data");
    }

    QDomElement multiplierElement = rDomElement.firstChildElement(caf::animation::multiplier);
    if(!multiplierElement.isNull())
    {
        multipliers.clear();
    }
    while(!multiplierElement.isNull())
    {
        multipliers.append(multiplierElement.attribute(caf::name));
        multiplierElement = multiplierElement.nextSiblingElement(caf::animation::multiplier);
    }

    QDomElement divisorElement = rDomElement.firstChildElement(caf::animation::divisor);
    if(!divisorElement.isNull())
    {
        divisors.clear();
    }
    while(!divisorElement.isNull())
    {
        divisors.append(divisorElement.attribute(caf::name));
        divisorElement = divisorElement.nextSiblingElement(caf::animation::divisor);
    }

    QDomElement xmlMovement = rDomElement.firstChildElement(caf::animation::movement);
    if(!xmlMovement.isNull())
    {
        movementData.clear();
    }
    while(!xmlMovement.isNull())
    {
        ModelObjectAnimationMovementData movement;
        movement.readFromDomElement(xmlMovement);
        movementData.append(movement);
        xmlMovement = xmlMovement.nextSiblingElement(caf::animation::movement);
    }
    if(!rDomElement.firstChildElement(caf::animation::start).isNull())
    {
        startX = rDomElement.firstChildElement(caf::animation::start).attribute("x").toDouble();
        startY = rDomElement.firstChildElement(caf::animation::start).attribute("y").toDouble();
        startTheta = rDomElement.firstChildElement(caf::animation::start).attribute("a").toDouble();
    }

    if(!rDomElement.firstChildElement(caf::animation::initscale).isNull())
    {
        initScaleX = rDomElement.firstChildElement(caf::animation::initscale).attribute("x").toDouble();
        initScaleY = rDomElement.firstChildElement(caf::animation::initscale).attribute("y").toDouble();
    }
    else
    {
        initScaleX = 1;
        initScaleY = 1;
    }

    QDomElement xmlResize = rDomElement.firstChildElement(caf::animation::resize);

    resizeData.clear();

    while(!xmlResize.isNull())
    {
        ModelObjectAnimationResizeData resize;
        resize.readFromDomElement(xmlResize);
        resizeData.append(resize);
        xmlResize = xmlResize.nextSiblingElement(caf::animation::resize);
    }


    QDomElement xmlInitColor = rDomElement.firstChildElement(caf::animation::initcolor);
    QDomElement xmlColor = rDomElement.firstChildElement(caf::animation::color);
    if(!xmlInitColor.isNull() && !xmlColor.isNull())
    {
        colorData.readFromDomElements(xmlInitColor, xmlColor);
    }

    if(!rDomElement.firstChildElement(caf::animation::transformorigin).isNull())
    {
        transformOriginX = rDomElement.firstChildElement(caf::animation::transformorigin).attribute("x").toDouble();
        transformOriginY = rDomElement.firstChildElement(caf::animation::transformorigin).attribute("y").toDouble();
    }

    QFileInfo movableIconFileInfo(iconPath);
    if (movableIconFileInfo.isRelative() && !iconPath.isEmpty())
    {
        movableIconFileInfo.setFile(QDir(basePath), iconPath);
        iconPath = movableIconFileInfo.absoluteFilePath();
    }
    QDomElement xmlAdjustable = rDomElement.firstChildElement(caf::animation::adjustable);
    if(!xmlAdjustable.isNull())
    {
        isAdjustable = true;
        adjustableMinX = xmlAdjustable.attribute(caf::animation::xmin).toDouble();
        adjustableMaxX = xmlAdjustable.attribute(caf::animation::xmax).toDouble();
        adjustableMinY = xmlAdjustable.attribute(caf::animation::ymin).toDouble();
        adjustableMaxY = xmlAdjustable.attribute(caf::animation::ymax).toDouble();
        adjustablePort = xmlAdjustable.attribute(caf::port);
        adjustableDataName = xmlAdjustable.attribute(caf::animation::dataname);
        adjustableGainX = xmlAdjustable.attribute(caf::animation::xgain).toDouble();
        adjustableGainY = xmlAdjustable.attribute(caf::animation::ygain).toDouble();
    }
    else
    {
        isAdjustable = false;
    }

    QDomElement xmlSwitchable = rDomElement.firstChildElement(caf::animation::switchable);
    if(!xmlSwitchable.isNull())
    {
        isSwitchable = true;
        isMomentary = parseAttributeBool(xmlSwitchable, caf::animation::momentary, false);
        switchableOffValue = xmlSwitchable.attribute(caf::animation::offvalue).toDouble();
        switchableOnValue = xmlSwitchable.attribute(caf::animation::onvalue).toDouble();
        switchablePort = xmlSwitchable.attribute(caf::port);
        switchableDataName = xmlSwitchable.attribute(caf::animation::dataname);
        hideIconOnSwitch = parseAttributeBool(xmlSwitchable, caf::animation::hideicon, false);
    }
    else
    {
        isSwitchable = false;
        isMomentary = false;
        switchableOffValue = 0;
        switchableOnValue = 0;
        switchablePort = QString();
        switchableDataName = QString();
        hideIconOnSwitch = false;
    }

    QDomElement xmlIndicaor = rDomElement.firstChildElement(caf::animation::indicator);
    if(!xmlIndicaor.isNull())
    {
        isIndicator = true;
        indicatorPort = xmlIndicaor.attribute(caf::port);
        indicatorDataName = xmlIndicaor.attribute(caf::animation::dataname);
    }
    else
    {
        isIndicator = false;
        indicatorPort = QString();
        indicatorDataName = QString();
    }


    QDomElement xmlMovingPorts = rDomElement.firstChildElement(caf::animation::movingport);
    if(!xmlMovingPorts.isNull())
    {
        movablePortNames.clear();
        movablePortStartX.clear();
        movablePortStartY.clear();
    }
    while(!xmlMovingPorts.isNull())
    {
        QString portName = xmlMovingPorts.attribute(caf::animation::portname);
        if(!portName.isEmpty() && !movablePortNames.contains(portName))   //Don't load tags without a port name, and tags that has already been loaded
        {
            movablePortNames.append(xmlMovingPorts.attribute(caf::animation::portname));
            movablePortStartX.append(xmlMovingPorts.attribute(caf::animation::startx).toDouble());
            movablePortStartY.append(xmlMovingPorts.attribute(caf::animation::starty).toDouble());
        }
        xmlMovingPorts = xmlMovingPorts.nextSiblingElement(caf::animation::movingport);
    }

    QDomElement xmlRelative = rDomElement.firstChildElement(caf::animation::relative);
    if(!xmlRelative.isNull())
    {
        movableRelative = xmlRelative.attribute(caf::idx).toInt();
    }
    else
    {
        movableRelative = -1;
    }
}


//! @brief Loads animation movement data from dom element
void ModelObjectAnimationMovementData::readFromDomElement(QDomElement &rDomElement)
{
    dataIdx = rDomElement.attribute(caf::idx).toDouble();
    x = rDomElement.attribute("x").toDouble();
    y = rDomElement.attribute("y").toDouble();
    theta = rDomElement.attribute("a").toDouble();
    divisor = rDomElement.attribute(caf::animation::divisor);
    multiplier = rDomElement.attribute(caf::animation::multiplier);
}


//! @brief Saves animation movement data to dom element
void ModelObjectAnimationMovementData::saveToDomElement(QDomElement &rDomElement) const
{
    setQrealAttribute(rDomElement, "x", x);
    setQrealAttribute(rDomElement, "y", y);
    setQrealAttribute(rDomElement, "a", theta);
    rDomElement.setAttribute(caf::idx, dataIdx);
    rDomElement.setAttribute(caf::animation::divisor, divisor);
    rDomElement.setAttribute(caf::animation::multiplier, multiplier);
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
    divisor = rDomElement.attribute(caf::animation::divisor);
    multiplier = rDomElement.attribute(caf::animation::multiplier);
}

void ModelObjectAnimationResizeData::saveToDomElement(QDomElement &rDomElement) const
{
    setQrealAttribute(rDomElement, "x", x);
    setQrealAttribute(rDomElement, "y", y);
    rDomElement.setAttribute("idx1", dataIdx1);
    rDomElement.setAttribute("idx2", dataIdx2);
    rDomElement.setAttribute(caf::animation::divisor, divisor);
    rDomElement.setAttribute(caf::animation::multiplier, multiplier);
}


void ModelObjectAnimationColorData::readFromDomElements(QDomElement &rInitDomElement, QDomElement &rDomElement)
{
    initR = rInitDomElement.attribute("r").toDouble();
    initG = rInitDomElement.attribute("g").toDouble();
    initB = rInitDomElement.attribute("b").toDouble();
    initA = rInitDomElement.attribute("a").toDouble();

    dataIdx = rDomElement.attribute(caf::idx).toInt();
    r = rDomElement.attribute("r").toDouble();
    g = rDomElement.attribute("g").toDouble();
    b = rDomElement.attribute("b").toDouble();
    a = rDomElement.attribute("a").toDouble();
    divisor = rDomElement.attribute(caf::animation::divisor);
    multiplier = rDomElement.attribute(caf::animation::multiplier);
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
    rDomElement.setAttribute(caf::idx, dataIdx);
    rDomElement.setAttribute(caf::animation::divisor, divisor);
    rDomElement.setAttribute(caf::animation::multiplier, multiplier);
}
