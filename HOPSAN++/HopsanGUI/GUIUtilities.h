//!
//! @file   GUIUtilities.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-10-09
//!
//! @brief Contains a class for misc utilities
//!
//$Id$


#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QPointF>
#include <QString>
#include <QFileInfo>
#include <QTextStream>


class GUIPort;

QString readName(QTextStream &rTextStream);
QString readName(QString namestring);
QString addQuotes(QString str);
QString relativePath(QString pathtochange, QString basepath);
qreal deg2rad(qreal deg);
qreal rad2deg(qreal rad);
qreal normDeg180(qreal deg);
qreal normDeg360(qreal deg);
qreal normRad(qreal rad);
double dist(double x1,double y1, double x2, double y2);

QPointF getOffsetPointfromPort(GUIPort *pPort);

#endif // GUIUTILITIES_H
