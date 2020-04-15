/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

class UnitConverter
{
public:
    static bool isScaleLesserThan (const UnitConverter &lhs, const UnitConverter &rhs);
    enum ConverterExpression {Expression};

    UnitConverter();
    UnitConverter(const QString &rUnit, const double scale, const double offset=0);
    UnitConverter(const QString &rQuantity, const QString &rUnit, const QString &rScale, const QString &rOffset);
    UnitConverter(ConverterExpression, const QString &quantity, const QString &unit, const QString &toBaseExpression, const QString& fromBaseExpression);
    void clear();

    double scaleToDouble(const double defaultValue=1.0) const;
    double offsetToDouble(const double defaultValue=0.0) const;

    bool isEmpty() const;
    bool isScaleEmpty() const;
    bool isOffsetEmpty() const;
    bool isScaleOne() const;
    bool isScaleMinusOne() const;
    bool isExpression() const;

    void setScaleAndOffset(const double scale, const double offset=0.0);
    void setOnlyScaleAndOffset(const double scale, const double offset=0.0);

    double convertToBase(const double value) const;
    QString convertToBase(const QString value) const;
    double convertFromBase(const double value) const;
    QString convertFromBase(const QString value) const;

    bool operator== (const UnitConverter &rOther);
    bool operator!= (const UnitConverter &rOther);

    QString mQuantity;
    QString mUnit;
    QString mScale;
    QString mOffset;
    QString mToBaseExpression;
    QString mFromBaseExpression;
};
#endif // UNITSCALE_H
