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
//$Id$

#ifndef UNITSCALE_H
#define UNITSCALE_H

#include <QString>

class UnitScale
{
public:
    UnitScale();
    UnitScale(const QString &rUnit, const QString &rScale);
    UnitScale(const QString &rUnit, const double scale);
    UnitScale(const QString &rQuantity, const QString &rUnit, const QString &rScale);
    void clear();
    double toDouble() const;
    double toDouble(const double def) const;
    bool isEmpty() const;
    bool isOne() const;
    bool isMinusOne() const;
    void setScale(const double scale);
    void setOnlyScale(const double scale);
    double rescale(const double value) const;
    QString rescale(const QString value) const;
    double invRescale(const double value) const;
    QString invRescale(const QString value) const;

    bool operator== (const UnitScale &rOther);
    bool operator!= (const UnitScale &rOther);

    QString mQuantity;
    QString mUnit;
    QString mScale;
};
#endif // UNITSCALE_H
