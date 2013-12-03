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
//! @file   UnitScale.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-12-03
//!
//! @brief Contains the UnitScale class
//!
//$Id: common.h 6112 2013-11-06 11:43:30Z robbr48 $

#ifndef UNITSCALE_H
#define UNITSCALE_H

#include <QString>

class UnitScale
{
public:
    UnitScale();
    UnitScale(const QString &rUnit, const QString &rScale);
    UnitScale(const QString &rUnit, const double scale);
    void clear();
    double toDouble() const;
    bool isEmpty() const;
    void setScale(const double scale);
    void setOnlyScale(const double scale);
    QString mUnit;
    QString mScale;
};
#endif // UNITSCALE_H
