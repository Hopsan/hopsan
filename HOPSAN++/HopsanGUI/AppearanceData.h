#ifndef APPEARANCEDATA_H
#define APPEARANCEDATA_H

#include <iostream>
#include <QString>
#include <QPointF>
#include <QVector>


class AppearanceData
{
public:
    AppearanceData();
    QString getTypeName();
    QString getIconPath();
    QString getIconPathISO();
    qreal getRotation();
    QPointF getNameTextPos();
    QVector<QPointF> &getPortCoordinateFractionVector();

    friend std::istream& operator >>(std::istream &is, AppearanceData &rData);
    friend std::ostream& operator <<(std::ostream &os, const AppearanceData &rData);

private:
    QString mTypeName;
    QString mIconPath;
    QString mIconPathISO;
    qreal mRotation;
    QPointF mNameTextPos;
    QVector<QPointF> mPortCoordinateFraction;


};

#endif // APPEARANCEDATA_H
