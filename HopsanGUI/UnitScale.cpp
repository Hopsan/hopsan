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
//! @file   UnitScale.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-12-03
//!
//! @brief Contains the UnitScale class
//!
//$Id$

#include "UnitScale.h"

//! @brief Constructor for a unit / scale (double) combination
UnitScale::UnitScale(const QString &rUnit, const double scale) : mUnit(rUnit)
{
    setScale(scale);
}

//! @brief Constructor for a quantity / unit / scale (QString) combination
UnitScale::UnitScale(const QString &rQuantity, const QString &rUnit, const QString &rScale) :
    mQuantity(rQuantity), mUnit(rUnit), mScale(rScale)
{
    // Check if valid
    bool isOK;
    mScale.toDouble(&isOK);
    if (!isOK)
    {
        clear();
    }
}

//! @brief Constructor for a unit / scale (QString) combination
UnitScale::UnitScale(const QString &rUnit, const QString &rScale) : mUnit(rUnit), mScale(rScale)
{
    // Check if valid
    bool isOK;
    mScale.toDouble(&isOK);
    if (!isOK)
    {
        clear();
    }
}

//! @brief Default constructor (empty unitscale)
UnitScale::UnitScale()
{
    // Default constructor
}

//! @brief Clear the UnitScale
void UnitScale::clear()
{
    mUnit.clear();
    mScale.clear();
}

//! @brief Convert scale string to double
//! @returns Scale as double
double UnitScale::toDouble() const
{
    return mScale.toDouble();
}

//! @brief Convert scale string to double (return default value if this UnitScale is NULL)
//! @param[in] def Default value to use if scale is NULL
//! @returns Scale as double
double UnitScale::toDouble(const double def) const
{
    if (mScale.isEmpty())
    {
        return def;
    }
    else
    {
        return toDouble();
    }
}

//! @brief Check if scale is empty
//! @returns True if empty else false
bool UnitScale::isEmpty() const
{
    return mScale.isEmpty();
}

bool UnitScale::isOne() const
{
    //! @todo this may not work if scale is very close to 1 due to truncation
    return mScale == "1" || (mScale == "1.0");
}

bool UnitScale::isMinusOne() const
{
    return (mScale == "-1") || (mScale == "-1.0");
}

//! @brief Set the scale from a double
//! @param [in] The scale value
void UnitScale::setScale(const double scale)
{
    mScale = QString("%1").arg(scale);
}

//! @brief Set only scale, clearing the "unit"
//! @param [in] The scale value
void UnitScale::setOnlyScale(const double scale)
{
    mUnit.clear();
    mScale = QString("%1").arg(scale);
}

//! @brief Rescale a value with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
double UnitScale::rescale(const double value) const
{
    if (isEmpty())
    {
        return value;
    }
    else
    {
        return mScale.toDouble()*value;
    }
}

//! @brief Rescale a value expressed as string with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
//! @note String conversion may truncate value
QString UnitScale::rescale(const QString value) const
{
    return QString("%1").arg(rescale(value.toDouble()));
}

//! @brief Inverted rescaling of a value with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
double UnitScale::invRescale(const double value) const
{
    return value / mScale.toDouble();
}

//! @brief Inverted rescaling of a value expressed as string with this unit scale
//! @param[in] value The value to rescale
//! @returns The rescaled value
//! @note String conversion may truncate value
QString UnitScale::invRescale(const QString value) const
{
    return QString("%1").arg(invRescale(value.toDouble()));
}


bool UnitScale::operator== (const UnitScale &rOther)
{
    return  !(*this != rOther);
}

bool UnitScale::operator!= (const UnitScale &rOther)
{
    return (mQuantity != rOther.mQuantity) || (mScale != rOther.mScale) || (mUnit != rOther.mUnit);
}
