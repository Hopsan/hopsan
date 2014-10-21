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
//! @file   ModelObjectAppearance.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by GuiModelObjects and library widget
//!
//$Id$

#ifndef ModelObjectAppearance_H
#define ModelObjectAppearance_H

#include <QString>
#include <QPointF>
#include <QIcon>
#include "common.h"

#include "Utilities/XMLUtilities.h"
#include "GUIPortAppearance.h"

//Define for the root xml element name, and the element name for each component (modelobject)
#define CAF_VERSION "version"
#define CAF_ROOT "hopsanobjectappearance"
#define CAF_MODELOBJECT "modelobject"

enum AbsoluteRelativeEnumT {Absolute, Relative};

QDomElement appendOrGetCAFRootTag(QDomElement parentElement);

class ModelObjectIconAppearance
{
public:
    ModelObjectIconAppearance();
    QString mRelativePath;
    QString mAbsolutePath;
    QString mRotationBehaviour;
    double mScale;
    bool mIsValid;
};



class ModelObjectAnimationMovableData
{
public:
    QString iconPath;
    int idx;

    //Data
    QStringList dataPorts;
    QStringList dataNames;
    QStringList multipliers;
    QStringList divisors;

    //Movement
    double startX;
    double startY;
    double startTheta;
    QList<double> movementX;
    QList<double> movementY;
    QList<double> movementTheta;
    QList<int> movementDataIdx;

    //Resize
    double initScaleX;
    double initScaleY;
    QList<double> resizeX;
    QList<double> resizeY;
    QList<int> scaleDataIdx1;
    QList<int> scaleDataIdx2;

    //Color
    double initColorR;
    double initColorG;
    double initColorB;
    double initColorA;
    double colorR;
    double colorG;
    double colorB;
    double colorA;
    int colorDataIdx;

    //Transform origin
    double transformOriginX;
    double transformOriginY;

    //Movable ports
    QStringList movablePortNames;
    QList<double> movablePortStartX;
    QList<double> movablePortStartY;

    //Relative movable
    int movableRelative;

    //Adjustable
    bool isAdjustable;
    double adjustableMinX;
    double adjustableMaxX;
    double adjustableMinY;
    double adjustableMaxY;
    QString adjustablePort;
    QString adjustableDataName;
    double adjustableGainX;
    double adjustableGainY;

    //Switchable
    bool isSwitchable;
    double switchableOffValue;
    double switchableOnValue;
    QString switchablePort;
    QString switchableDataName;

    //Indicator
    bool isIndicator;
    QString indicatorPort;
    QString indicatorDataName;

    //Calculated at initialization
    double multiplierValue, divisorValue;
    bool useMultipliers, useDivisors;
};


class ModelObjectAnimationData
{
public:
    void readFromDomElement(QDomElement &rDomElement, QString basePath);
    void saveToDomElement(QDomElement &rDomElement);

    //Movabels
    QList<ModelObjectAnimationMovableData> movables;

    //Icon path
    QString baseIconPath;

    //Container-specific data
    double flowSpeed;
    double hydraulicMinPressure;
    double hydraulicMaxPressure;
};




class ModelObjectAppearance
{
public:
    ModelObjectAppearance();
    void cacheIcons();

    void setTypeName(const QString type);
    void setSubTypeName(const QString subtype);
    void setDisplayName(const QString name);
    void setHelpText(const QString text);
    void setBasePath(const QString path);
    void setIconPath(const QString path, const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrel);
    void setIconScale(const double scale, const GraphicsTypeEnumT gfxType);

    QString getHmfFile() const;
    QString getTypeName() const;
    QString getSubTypeName() const;
    QString getDisplayName() const;
    QString getNonEmptyName() const;
    const QString &getHelpPicture() const;
    const QString &getHelpText() const;
    const QString &getHelpLink() const;
    const QMap<QString, QString> &getOverridedDefaultParameters() const;
    QString getBasePath() const;
    QString getFullAvailableIconPath(GraphicsTypeEnumT gfxType=UserGraphics);
    QString getIconPath(const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrel);
    QIcon &getIcon(const GraphicsTypeEnumT gfxType);
    QString getDefaultMissingIconPath() const;
    double   getIconScale(const GraphicsTypeEnumT gfxType=UserGraphics);
    QString getIconRotationBehaviour(const GraphicsTypeEnumT gfxType=UserGraphics);
    QPointF getNameTextPos() const;
    QString getSourceCodeFile() const;
    QString getLibPath() const;
    bool isRecompilable() const;

    ModelObjectAnimationData *getAnimationDataPtr();

    
    PortAppearanceMapT &getPortAppearanceMap();
    const PortAppearance *getPortAppearance(const QString &rPortName) const;
    PortAppearance *getPortAppearance(const QString &rPortName);
    void erasePortAppearance(const QString portName);
    void addPortAppearance(const QString portName, PortAppearance *pPortAppearance=0);

    bool hasIcon(const GraphicsTypeEnumT gfxType);

    void readFromDomElement(QDomElement domElement);
    void saveToDomElement(QDomElement &rDomElement);
    void saveSpecificPortsToDomElement(QDomElement &rDomElement, const QStringList &rParametNames);
    void saveToXMLFile(QString filename);

private:
    QString mHmfFile;
    QString mTypeName;
    QString mSubTypeName;
    QString mDisplayName;
    QString mSourceCode;
    QString mLibPath;
    bool mIsRecompilable;
    QString mHelpText,mHelpPicture,mHelpLink;
    QMap<QString,QString> mOverridedDefaultParameters;
    ModelObjectIconAppearance mIsoIconAppearance;
    ModelObjectIconAppearance mUserIconAppearance;
    QIcon mIsoIcon;
    QIcon mUserIcon;
    QString mDefaultMissingIconPath;
    QPointF mNameTextPos;
    QStringList mReplacementObjects;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for relative paths
    QString mBasePath;

    //Private help functions
    QDomElement addModelObjectRootElement(QDomElement parentDomElement);
    void setRelativePathFromAbsolute();
    void setAbsoultePathFromRelative();
    void refreshIconValid();
    GraphicsTypeEnumT selectAvailableGraphicsType(const GraphicsTypeEnumT type);

    ModelObjectAnimationData mAnimationData;
};


#endif // APPEARANCEDATA_H
