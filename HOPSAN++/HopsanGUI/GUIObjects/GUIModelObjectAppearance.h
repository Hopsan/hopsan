//!
//! @file   GUIModelObjectAppearance.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by guiobjects and library widget
//!
//$Id$

#ifndef GUIMODELOBJECTAPPEARANCE_H
#define GUIMODELOBJECTAPPEARANCE_H

#include <QTextStream>
#include <QString>
#include <QPointF>
#include "common.h"

#include "../Utilities/XMLUtilities.h"
#include "../GUIPortAppearance.h"


class GUIModelObjectAppearance
{
public:
    GUIModelObjectAppearance();
    void setTypeName(QString name);
    void setName(QString name);
    void setBasePath(QString path);
    void setIconPathUser(QString path);
    void setIconPathISO(QString path);

    QString getTypeName();
    QString getName();
    QString getNonEmptyName();
    QString getFullIconPath(graphicsType gfxType=USERGRAPHICS);
    QString getIconPathUser();
    QString getIconPathISO();
    QString getIconRotationBehaviour();
    QPointF getNameTextPos();
    PortAppearanceMapT &getPortAppearanceMap();

    bool haveIsoIcon();
    bool haveUserIcon();

    QString getBasePath();

    void readFromTextStream(QTextStream &is);

    void readFromDomElement(QDomElement domElement);
    void saveToDomElement(QDomElement &rDomElement);
    void saveToXML(QString filename);

    //This bool signals wheter there were errors when reading the data from input stream
    //! @todo should we really have it
    bool mIsReadOK;

private:
    QString mTypeName;
    QString mName;
    QString mIconPathUser;
    QString mIconPathISO;
    QString mIconRotationBehaviour;
    //qreal mRotation;
    QPointF mNameTextPos;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for path strings, mayb should not be stored in here
    QString mBasePath;

};

#endif // APPEARANCEDATA_H
