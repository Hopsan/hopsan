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
    qreal getRotation();
    QPointF getNameTextPos();
    QVector<PortAppearance> &getPortAppearanceVector();

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


};

#endif // APPEARANCEDATA_H
