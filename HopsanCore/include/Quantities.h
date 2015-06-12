//!
//! @file   Quantities.h
//! @author peter.nordin@liu.se
//! @date   2015-06-11
//! @brief Contains the default quantity <-> base unit lookup
//!
//$Id$

#ifndef QUANTITIES_H
#define QUANTITIES_H

#include <map>
#include "HopsanTypes.h"

namespace hopsan {

class QuantityRegister
{
public:
    QuantityRegister();

    void registerQuantity(const HString &rQuantity, const HString &rBaseUnit);
    HString lookupBaseUnit(const HString &rQuantity);
    bool haveQuantity(const HString &rQuantity) const;

private:
    std::map<HString, HString> mQuantity2BaseUnit;
};

extern QuantityRegister gHopsanQuantities;


}
#endif // QUANTITIES_H

