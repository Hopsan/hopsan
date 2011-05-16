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
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

QString readName(QTextStream &rTextStream);
QString readName(QString namestring);
QString addQuotes(QString str);
//QString relativePath(QString pathtochange, QDir basedir);
QString relativePath(QFileInfo pathtochange, QDir basedir);
qreal deg2rad(qreal deg);
qreal rad2deg(qreal rad);
qreal normDeg180(qreal deg);
qreal normDeg360(qreal deg);
qreal normRad(qreal rad);
qreal dist(QPointF &rPoint);
qreal dist(QPointF &rPoint1, QPointF &rPoint2);
double dist(double x1,double y1, double x2, double y2);
QString parseVariableDescription(QString input);
QString parseVariableUnit(QString input);

#endif // GUIUTILITIES_H
