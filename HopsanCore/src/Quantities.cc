
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
