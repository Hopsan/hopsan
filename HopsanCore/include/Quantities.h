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
    void registerQuantityAlias(const HString &rQuantity, const HString &rAlias);
    HString lookupBaseUnit(const HString &rQuantity) const;
    bool haveQuantity(const HString &rQuantity) const;

private:
    std::map<HString, HString> mQuantity2BaseUnit;
    std::map<HString, HString> mQuantityAliases;
};

bool checkIfQuantityOrUnit(const HString &rQuantityOrUnit, HString &rQuantity, HString &rUnitOrBaseUnit);

//! @todo this global pointer is not safe, will code it away later (maybe) /Peter
extern QuantityRegister *gpInternalCoreQuantityRegister; // Do not use this pointer outside of HopsanCore
}
#endif // QUANTITIES_H

