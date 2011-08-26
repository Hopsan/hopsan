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
//! @file   GUIModelObjectAppearance.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by GuiModelObjects and library widget
//!
//$Id$

#ifndef GUIMODELOBJECTAPPEARANCE_H
#define GUIMODELOBJECTAPPEARANCE_H

#include <QString>
#include <QPointF>
#include "../common.h"

#include "../Utilities/XMLUtilities.h"
#include "../GUIPortAppearance.h"

//Define for the root xml element name, and the element name for each component (modelobject)
#define CAF_ROOT "hopsanobjectappearance"
#define CAF_MODELOBJECT "modelobject"

class GUIModelObjectAppearance
{
public:
    GUIModelObjectAppearance();
    void setTypeName(QString name);
    void setName(QString name);
    void setHelpText(QString text);
    void setBasePath(QString path);
    void setIconPath(QString path, graphicsType gfxType);

    QString getTypeName();
    QString getName();
    QString getNonEmptyName();
    QString getHelpPicture();
    QString getHelpText();
    QString getBasePath();
    QString getFullAvailableIconPath(graphicsType gfxType=USERGRAPHICS);
    QString getIconPath(graphicsType gfxType=USERGRAPHICS);
    qreal   getIconScale(const graphicsType gfxType=USERGRAPHICS);
    QString getIconRotationBehaviour(const graphicsType gfxType=USERGRAPHICS);
    QPointF getNameTextPos();
    PortAppearanceMapT &getPortAppearanceMap();
    void erasePortAppearance(const QString portName);
    void addPortAppearance(const QString portName, GUIPortAppearance *pPortAppearance=0);

    bool hasIcon(const graphicsType gfxType);

    void readFromDomElement(QDomElement domElement);
    void saveToDomElement(QDomElement &rDomElement);
    void saveToXMLFile(QString filename);

private:
    QString mTypeName;
    QString mDisplayName;
    QString mHelpPicture;
    QString mHelpText;
    QString mIconUserPath;
    QString mIconIsoPath;
    qreal   mIconUserScale;
    qreal   mIconIsoScale;
    QString mIconIsoRotationBehaviour;
    QString mIconUserRotationBehaviour;
    QPointF mNameTextPos;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for path strings
    QString mBasePath;

    //Private help functions
    void makeSurePathsAbsolute();
    void makeSurePathsRelative();


};

#endif // APPEARANCEDATA_H
