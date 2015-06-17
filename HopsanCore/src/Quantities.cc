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


#include "Quantities.h"


hopsan::QuantityRegister::QuantityRegister()
{
    registerQuantity("Pressure", "Pa");
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

    registerQuantity("Mass", "Kg");
    registerQuantity("Area", "m^2");
    registerQuantity("Volume", "m^3");
    registerQuantity("Displacement", "m^3/rev");

    registerQuantity("Frequency", "rad/s");
    registerQuantity("Time", "s");

}

void hopsan::QuantityRegister::registerQuantity(const hopsan::HString &rQuantity, const hopsan::HString &rBaseUnit)
{
    mQuantity2BaseUnit.insert(std::pair<HString, HString>(rQuantity, rBaseUnit));
}

hopsan::HString hopsan::QuantityRegister::lookupBaseUnit(const hopsan::HString &rQuantity)
{
    std::map<HString, HString>::iterator it = mQuantity2BaseUnit.find(rQuantity);
    if (it != mQuantity2BaseUnit.end())
    {
        return it->second;
    }
    return HString();
}

bool hopsan::QuantityRegister::haveQuantity(const hopsan::HString &rQuantity) const
{
    return (mQuantity2BaseUnit.find(rQuantity) != mQuantity2BaseUnit.end());
}

hopsan::QuantityRegister hopsan::gHopsanQuantities;
