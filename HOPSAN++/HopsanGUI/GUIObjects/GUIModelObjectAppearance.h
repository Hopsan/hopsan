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


class GUIModelObjectAppearance
{
public:
    GUIModelObjectAppearance();
    void setTypeName(QString name);
    void setName(QString name);
    void setHelpText(QString text);
    void setBasePath(QString path);
    void setRelativeIconPath(QString path, graphicsType gfxType);

    QString getTypeName();
    QString getName();
    QString getNonEmptyName();
    QString getHelpPicture();
    QString getHelpText();
    QString getBasePath();
    QString getFullIconPath(graphicsType gfxType=USERGRAPHICS);
    QString getRelativeIconPath(graphicsType gfxType=USERGRAPHICS);
    QString getIconRotationBehaviour();
    QPointF getNameTextPos();
    PortAppearanceMapT &getPortAppearanceMap();
    void erasePortAppearance(const QString portName);
    void addPortAppearance(const QString portName, GUIPortAppearance *pPortAppearance=0);

    bool haveIsoIcon();
    bool haveUserIcon();

    void readFromDomElement(QDomElement domElement);
    void saveToDomElement(QDomElement &rDomElement);
    void saveToXML(QString filename);

private:
    QString mTypeName;
    QString mDisplayName;
    QString mHelpPicture;
    QString mHelpText;
    QString mIconUserPath;
    QString mIconIsoPath;
    QString mIconRotationBehaviour;
    QPointF mNameTextPos;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for path strings
    QString mBasePath;

};

#endif // APPEARANCEDATA_H
