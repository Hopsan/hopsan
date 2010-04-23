#ifndef APPEARANCEDATA_H
#define APPEARANCEDATA_H

#include <iostream>
//#include <QTextIStream>
//#include <QTextOStream>
#include <QTextStream>
#include <QString>
#include <QPointF>
#include <QVector>

class PortAppearance
{
public:
    qreal x;
    qreal y;
    qreal rot;
};

class AppearanceData
{
public:
    AppearanceData();
    QString getTypeName();
    QString getIconPath();
    QString getIconPathISO();
    QString getIconRotationBehaviour();
    QPointF getNameTextPos();
    QVector<PortAppearance> &getPortAppearanceVector();

    QString getBasePath();
    void setBasePath(QString path);

    friend QTextStream& operator >>(QTextStream &is, AppearanceData &rData);
    friend QTextStream& operator <<(QTextStream &os, const AppearanceData &rData);

private:
    QString mTypeName;
    QString mIconPath;
    QString mIconPathISO;
    QString mIconRotationBehaviour;
    qreal mRotation;
    size_t mnPorts;
    QPointF mNameTextPos;
    QVector<PortAppearance> mPortAppearanceVector;

    //BaseDir for path strings, mayb should not be stored in here
    QString mBasePath;


};

#endif // APPEARANCEDATA_H
