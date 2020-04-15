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
//! @file   UnitScale.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-12-03
//!
//! @brief Contains the UnitConverter class
//!
//$Id$

#include "UnitScale.h"
#include "CoreAccess.h"

//! @brief Constructor for a unit / scale (double) combination
UnitConverter::UnitConverter(const QString &rUnit, const double scale, const double offset) : mUnit(rUnit)
{
    setScaleAndOffset(scale, offset);
}

//! @brief Constructor for a quantity / unit / scale (QString) combination
UnitConverter::UnitConverter(const QString &rQuantity, const QString &rUnit, const QString &rScale, const QString &rOffset) :
    mQuantity(rQuantity), mUnit(rUnit), mScale(rScale), mOffset(rOffset)
{
    // Check if valid
    bool isOK=false, isOK2=true;
    mScale.toDouble(&isOK);
    if (!mOffset.isEmpty())
    {
        mOffset.toDouble(&isOK2);
    }
    if (!(isOK && isOK2))
    {
        clear();
    }
}

UnitConverter::UnitConverter(UnitConverter::ConverterExpression, const QString &quantity, const QString &unit, const QString &toBaseExpression, const QString &fromBaseExpression) :
    mQuantity(quantity), mUnit(unit), mToBaseExpression(toBaseExpression), mFromBaseExpression(fromBaseExpression)
{

}


//! @brief Default constructor (empty UnitConverter)
UnitConverter::UnitConverter()
{
    // Default constructor
}

//! @brief Clear the UnitConverter
void UnitConverter::clear()
{
    mQuantity.clear();
    mUnit.clear();
    mScale.clear();
    mOffset.clear();
}


//! @brief Convert scale string to double (return default value if this UnitConverter is NULL)
//! @param[in] defaultValue Default value to use if scale is NULL
//! @returns Scale as double
double UnitConverter::scaleToDouble(const double defaultValue) const
{
    if (mScale.isEmpty())
    {
        return defaultValue;
    }
    else
    {
        return mScale.toDouble();
    }
}

double UnitConverter::offsetToDouble(const double defaultValue) const
{
    if (mOffset.isEmpty())
    {
        return defaultValue;
    }
    else
    {
        return mOffset.toDouble();
    }
}

bool UnitConverter::isExpression() const
{
    return !(mToBaseExpression.isEmpty() || mToBaseExpression.isEmpty());
}

//! @brief Check if scale is empty
//! @returns True if empty else false
bool UnitConverter::isEmpty() const
{
    return isScaleEmpty() && isOffsetEmpty() && !isExpression();
}

bool UnitConverter::isScaleEmpty() const
{
    return mScale.isEmpty();
}

bool UnitConverter::isOffsetEmpty() const
{
    return mOffset.isEmpty();
}

bool UnitConverter::isScaleOne() const
{
    //! @todo this may not work if scale is very close to 1 due to truncation
    return (mScale == "1" || (mScale == "1.0")) && mOffset.isEmpty();
}

bool UnitConverter::isScaleMinusOne() const
{
    return ((mScale == "-1") || (mScale == "-1.0")) && mOffset.isEmpty();
}

bool UnitConverter::isScaleLesserThan (const UnitConverter &lhs, const UnitConverter &rhs)
{
    return (lhs.mScale.toDouble() < rhs.mScale.toDouble());
}

//! @brief Set the scale from a double
//! @param [in] The scale value
void UnitConverter::setScaleAndOffset(const double scale, const double offset)
{
    mScale = QString("%1").arg(scale);
    // Note! For simplicity (in some other functions) we clear offset if zero
    if (offset != 0)
    {
        mOffset = QString("%1").arg(offset);
    }
    else
    {
        mOffset.clear();
    }
}

//! @brief Set only scale, clearing the "unit" and "quantity"
//! @param [in] The scale value
void UnitConverter::setOnlyScaleAndOffset(const double scale, const double offset)
{
    mUnit.clear();
    setScaleAndOffset(scale, offset);
}

//! @brief Rescale a value with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
double UnitConverter::convertToBase(const double value) const
{
    if (isExpression()) {
        return evalWithNumHop(QString("x=%1;%2").arg(value).arg(mToBaseExpression));
    }
    if (isScaleEmpty()) {
        return value+mOffset.toDouble();
    }
    else {
        return mScale.toDouble()*value+mOffset.toDouble();
    }
}

//! @brief Rescale a value expressed as string with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
//! @note String conversion may truncate value
QString UnitConverter::convertToBase(const QString value) const
{
    return QString("%1").arg(convertToBase(value.toDouble()));
}

//! @brief Inverted rescaling of a value with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
double UnitConverter::convertFromBase(const double value) const
{
    if (isExpression()) {
        return evalWithNumHop(QString("x=%1;%2").arg(value).arg(mFromBaseExpression));
    }
    else if (isScaleEmpty()) {
        return (value - mOffset.toDouble());
    }
    else {
        return (value - mOffset.toDouble()) / mScale.toDouble();
    }
}

//! @brief Inverted rescaling of a value expressed as string with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
//! @note String conversion may truncate value
QString UnitConverter::convertFromBase(const QString value) const
{
    return QString("%1").arg(convertFromBase(value.toDouble()));
}


bool UnitConverter::operator== (const UnitConverter &rOther)
{
    return  !(*this != rOther);
}

bool UnitConverter::operator!= (const UnitConverter &rOther)
{
    return (mQuantity != rOther.mQuantity) || (mScale != rOther.mScale) || (mUnit != rOther.mUnit) || (mOffset != rOther.mOffset);
}
