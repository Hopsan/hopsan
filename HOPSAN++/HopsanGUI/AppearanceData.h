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

#include <iostream>
//#include <QTextIStream>
//#include <QTextOStream>
#include <QTextStream>
#include <QString>
#include <QPointF>
#include <QVector>
#include <QMap>

class PortAppearance
{
public:
    qreal x;
    qreal y;
    qreal rot;
};

typedef QMap<QString, PortAppearance> PortAppearanceMapT;

class AppearanceData
{
public:
    AppearanceData();
    void setTypeName(QString name);
    void setBasePath(QString path);
    void setIconPathUser(QString path);
    void setIconPathISO(QString path);

    QString getTypeName();
    QString getFullIconPath(bool useIso=false);
    QString getIconPathUser();
    QString getIconPathISO();
    QString getIconRotationBehaviour();
    QPointF getNameTextPos();
    size_t  getNumberOfPorts();
    PortAppearanceMapT &getPortAppearanceMap();

    bool haveIsoIcon();
    bool haveUserIcon();

    QString getBasePath();

    bool setAppearanceData(QTextStream &is);

    friend QTextStream& operator >>(QTextStream &is, AppearanceData &rData);
    friend QTextStream& operator <<(QTextStream &os, AppearanceData &rData);

private:
    QString mTypeName;
    QString mIconPathUser;
    QString mIconPathISO;
    QString mIconRotationBehaviour;
    qreal mRotation;
    QPointF mNameTextPos;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for path strings, mayb should not be stored in here
    QString mBasePath;
    //This bool signals wheter there were errors when reading the data from input stream
    bool mIsOK;




};



#endif // APPEARANCEDATA_H
