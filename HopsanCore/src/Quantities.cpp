/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//$Id$

#include "Quantities.h"


hopsan::QuantityRegister::QuantityRegister()
{
    registerQuantity("Pressure", "Pa");
    registerQuantityAlias("Pressure", "Stress");
    registerQuantity("Flow", "m^3/s");

    registerQuantity("Force", "N");
    registerQuantity("Position", "m");
    registerQuantity("Velocity", "m/s");
    registerQuantity("Acceleration", "m/(s^2)");

    registerQuantity("Torque", "Nm");
    registerQuantity("Angle", "rad");
    registerQuantity("AngularVelocity", "rad/s");

    registerQuantity("Voltage", "V");
    registerQuantity("Current", "A");

    registerQuantity("Momentum", "kg m/s");
    registerQuantity("Energy", "J");
    registerQuantity("Power", "J/s");

    registerQuantity("Mass", "kg");
    registerQuantity("Area", "m^2");
    registerQuantity("Volume", "m^3");
    registerQuantity("Displacement", "m^3/rev");

    registerQuantity("MomentOfInertia", "kg m^2");

    registerQuantity("Density", "kg/m^3");

    registerQuantity("Frequency", "rad/s");
    registerQuantity("Time", "s");

    registerQuantity("Temperature", "K");

    registerQuantity("Resistance", "ohm");

    registerQuantity("LeakageCoefficient", "(m^3/s)/Pa");

    // This is a fake quantity for scaling between absolute and dB values
    registerQuantity("Magnitude", "");

    // Register quantity aliases
    registerQuantityAlias("Position", "Length");
    registerQuantityAlias("Position", "Altitude");
}

void hopsan::QuantityRegister::registerQuantity(const hopsan::HString &rQuantity, const hopsan::HString &rBaseUnit)
{
    mQuantity2BaseUnit.insert(std::pair<HString, HString>(rQuantity, rBaseUnit));
}

void hopsan::QuantityRegister::registerQuantityAlias(const hopsan::HString &rQuantity, const hopsan::HString &rAlias)
{
    mQuantityAliases.insert(std::pair<HString, HString>(rAlias, rQuantity));
}

hopsan::HString hopsan::QuantityRegister::lookupBaseUnit(const hopsan::HString &rQuantity) const
{
    // First check if alias, then lookup actual quantity
    std::map<HString, HString>::const_iterator it = mQuantityAliases.find(rQuantity);
    if (it != mQuantityAliases.end())
    {
        it = mQuantity2BaseUnit.find(it->second);
    }
    // else lookup directly
    else
    {
        it = mQuantity2BaseUnit.find(rQuantity);
    }
    // If quantity registered, then return its base unit
    if (it != mQuantity2BaseUnit.end())
    {
        return it->second;
    }
    // else return empty string
    return HString();
}

bool hopsan::QuantityRegister::haveQuantity(const hopsan::HString &rQuantity) const
{
    // First check if alias, then lookup actual quantity
    std::map<HString, HString>::const_iterator it = mQuantityAliases.find(rQuantity);
    if (it != mQuantityAliases.end())
    {
        it = mQuantity2BaseUnit.find(it->second);
    }
    // else lookup directly
    else
    {
        it = mQuantity2BaseUnit.find(rQuantity);
    }
    return (it != mQuantity2BaseUnit.end());
}

//! @brief Lookup a quantity base unit, or return as a unit if not a valid quantity is given
//! @param[in] rQuantityOrUnit The name of the quantity (or unit)
//! @param[out] rQuantity The quantity, empty if rQuantityOrUnit is not a valid quantity
//! @param[out] rUnitOrBaseUnit The base unit for the given quantity, or the value of rQuantityOrUnit if it is not a valid quantity
//! @returns true if rQuantityOrUnit is a valid quantity else false
bool hopsan::checkIfQuantityOrUnit(const hopsan::HString &rQuantityOrUnit, hopsan::HString &rQuantity, hopsan::HString &rUnitOrBaseUnit)
{
    rUnitOrBaseUnit = gpInternalCoreQuantityRegister->lookupBaseUnit(rQuantityOrUnit);
    // rUnitOrQuantity is treated as a unit because no valid quantity has been specified
    if (rUnitOrBaseUnit.empty())
    {
        rQuantity.clear();
        rUnitOrBaseUnit = rQuantityOrUnit;
        return false;
    }
    // rUnitOrQuantity is actually a quantity, and bu is the corresponding base unit
    else
    {
        rQuantity = rQuantityOrUnit;
        return true;
    }
}
