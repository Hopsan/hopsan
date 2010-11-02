//!
//! @file   AppearanceData.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by guiobjects and library widget
//!
//$Id$

#ifndef APPEARANCEDATA_H
#define APPEARANCEDATA_H

#include <QTextStream>
#include <QString>
#include <QPointF>
#include <QVector>
//#include <QHash>
//#include <QPen>
#include "common.h"
#include "GUIPortAppearance.h"

#include "loadObjects.h"

class AppearanceData
{
public:
    AppearanceData();
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

    void readFromDomElement(QDomElement &rDomElement);
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
    qreal mRotation;
    QPointF mNameTextPos;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for path strings, mayb should not be stored in here
    QString mBasePath;

};

#endif // APPEARANCEDATA_H
