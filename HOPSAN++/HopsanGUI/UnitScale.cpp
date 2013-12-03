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
//$Id: common.h 6112 2013-11-06 11:43:30Z robbr48 $

#include "UnitScale.h"

//! @brief Constructor for a unit / scale (double) combination
UnitScale::UnitScale(const QString &rUnit, const double scale) : mUnit(rUnit)
{
    setScale(scale);
}

//! @brief Constructor for a unit / scale (QString) combination
UnitScale::UnitScale(const QString &rUnit, const QString &rScale) : mUnit(rUnit), mScale(rScale)
{
    //Everything is done in initializer list
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

//! @brief Check if scale is empty
//! @returns True if empty else false
bool UnitScale::isEmpty() const
{
    return mScale.isEmpty();
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
