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


class ModelObjectAnimationResizeData
{
public:
    ModelObjectAnimationResizeData(double xi=0, double yi=0, int dataIdx1i=0, int dataIdx2i=-1, QString divisori="", QString multiplieri="")
        : x(xi), y(yi), dataIdx1(dataIdx1i), dataIdx2(dataIdx2i), divisor(divisori), multiplier(multiplieri), divisorValue(1), multiplierValue(1) {}
    void readFromDomElement(QDomElement &rDomElement);
    void saveToDomElement(QDomElement &rDomElement) const;
    double x;
    double y;
    int dataIdx1;
    int dataIdx2;
    QString divisor;
    QString multiplier;
    double divisorValue;
    double multiplierValue;
};


class ModelObjectAnimationMovementData
{
public:
    ModelObjectAnimationMovementData(double xi=0, double yi=0, double thetai=0, int dataIdxi=0, QString divisori="", QString multiplieri="")
        : x(xi), y(yi), theta(thetai), dataIdx(dataIdxi), divisor(divisori), multiplier(multiplieri), divisorValue(1), multiplierValue(1) {}
    void readFromDomElement(QDomElement &rDomElement);
    void saveToDomElement(QDomElement &rDomElement) const;
    double x;
    double y;
    double theta;
    int dataIdx;
    QString divisor;
    QString multiplier;
    double divisorValue;
    double multiplierValue;
};

class ModelObjectAnimationColorData
{
public:
    ModelObjectAnimationColorData(double initRi=0, double initGi=0, double initBi=0, double initAi=0, double ri=0,
                                  double gi=0, double bi=0, double ai=0, int dataIdxi=0, QString divisori="", QString multiplieri="")
        : initR(initRi), initG(initGi), initB(initBi), initA(initAi), r(ri), g(gi), b(bi), a(ai), dataIdx(dataIdxi),
          divisor(divisori), multiplier(multiplieri), divisorValue(1), multiplierValue(1) {}
    void readFromDomElements(QDomElement &rInitDomElement, QDomElement &rDomElement);
    void saveToDomElements(QDomElement &rInitDomElement, QDomElement &rDomElement) const;
    double initR;
    double initG;
    double initB;
    double initA;
    double r;
    double g;
    double b;
    double a;
    int dataIdx;
    QString divisor;
    QString multiplier;
    double divisorValue;
    double multiplierValue;
};


class ModelObjectAnimationMovableData
{
public:
    void readFromDomElement(QDomElement &rDomElement, QString basePath);

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
    QList<ModelObjectAnimationMovementData> movementData;

    //Resize
    double initScaleX;
    double initScaleY;
    QList<ModelObjectAnimationResizeData> resizeData;

    //Color
    ModelObjectAnimationColorData colorData;

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
    bool hideIconOnSwitch;

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

    //Movables
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
    QString getHelpHtmlPath() const;
    const QMap<QString, QString> &getOverridedDefaultParameters() const;
    bool isParameterHidden(const QString &name) const;
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
    QString mHelpText,mHelpPicture,mHelpLink,mHelpHtmlPath;
    QMap<QString,QString> mOverridedDefaultParameters;
    QStringList mHiddenParameters;
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
