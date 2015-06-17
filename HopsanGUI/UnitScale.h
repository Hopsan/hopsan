/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
