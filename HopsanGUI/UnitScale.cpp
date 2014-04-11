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
//! @param[in] def Default value to use if scle is NULL
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
    return mScale == "1";
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
    return mScale.toDouble()*value;
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
