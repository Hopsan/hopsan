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
#include "common.h"

#include "Utilities/XMLUtilities.h"
#include "GUIPortAppearance.h"

//Define for the root xml element name, and the element name for each component (modelobject)
#define CAF_VERSION "version"
#define CAF_ROOT "hopsanobjectappearance"
#define CAF_MODELOBJECT "modelobject"

enum AbsoluteRelativeT {ABSOLUTE, RELATIVE};

QDomElement appendOrGetCAFRootTag(QDomElement parentElement);

class ModelObjectIconAppearance
{
public:
    ModelObjectIconAppearance();
    QString mRelativePath;
    QString mAbsolutePath;
    QString mRotationBehaviour;
    qreal mScale;
    bool mIsValid;
};

class ModelObjectAnimationData
{
public:
    void readFromDomElement(QDomElement &rDomElement, QString basePath);

    QString baseIconPath;
    QStringList movableIconPaths;
    QStringList dataPorts;
    QStringList dataNames;
    QStringList multipliers;
    QStringList divisors;
    QVector<double> speedX;
    QVector<double> speedY;
    QVector<double> speedTheta;
    QVector<double> startX;
    QVector<double> startY;
    QVector<double> startTheta;
    QVector<double> transformOriginX;
    QVector<double> transformOriginY;
    QVector<bool> isAdjustable;
    QVector<double> adjustableMinX;
    QVector<double> adjustableMaxX;
    QVector<double> adjustableMinY;
    QVector<double> adjustableMaxY;
    QStringList adjustablePort;
    QStringList adjustableDataName;
    QVector<double> adjustableGainX;
    QVector<double> adjustableGainY;
    QVector<QStringList> movablePortNames;
    QVector<QList<double> > movablePortStartX;
    QVector<QList<double> > movablePortStartY;
};


class ModelObjectAppearance
{
public:
    ModelObjectAppearance();
    void setTypeName(QString name);
    void setName(QString name);
    void setHelpText(QString text);
    void setBasePath(QString path);
    void setIconPath(const QString path, const graphicsType gfxType, const AbsoluteRelativeT absrel);
    void setIconScale(const qreal scale, const graphicsType gfxType);

    QString getTypeName();
    QString getName();
    QString getNonEmptyName();
    QString getHelpPicture();
    QString getHelpText();
    QString getBasePath();
    QString getFullAvailableIconPath(graphicsType gfxType=USERGRAPHICS);
    QString getIconPath(const graphicsType gfxType, const AbsoluteRelativeT absrel);
    qreal   getIconScale(const graphicsType gfxType=USERGRAPHICS);
    QString getIconRotationBehaviour(const graphicsType gfxType=USERGRAPHICS);
    QPointF getNameTextPos();

    ModelObjectAnimationData *getAnimationDataPtr();

    
    PortAppearanceMapT &getPortAppearanceMap();
    void erasePortAppearance(const QString portName);
    void addPortAppearance(const QString portName, PortAppearance *pPortAppearance=0);

    bool hasIcon(const graphicsType gfxType);

    void readFromDomElement(QDomElement domElement);
    void saveToDomElement(QDomElement &rDomElement);
    void saveSpecificPortsToDomElement(QDomElement &rDomElement, const QStringList &rParametNames);
    void saveToXMLFile(QString filename);

private:
    QString mTypeName;
    QString mDisplayName;
    QString mHelpPicture;
    QString mHelpText;
    ModelObjectIconAppearance mIsoIconAppearance;
    ModelObjectIconAppearance mUserIconAppearance;
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
    graphicsType selectAvailableGraphicsType(const graphicsType type);

    ModelObjectAnimationData mAnimationData;
};


#endif // APPEARANCEDATA_H
