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
//! @file   ClassFactoryStatusCheck.hpp
//! @author <peter.nordin@liu.se>
//! @date   2011-05-30
//!
//! @brief Contains template function ClassFactoryStatusCheck
//!
//$Id$

#ifndef CLASSFACTORYSTATUSCHECK_HPP
#define CLASSFACTORYSTATUSCHECK_HPP

#include "ClassFactory.hpp"
#include "HopsanCoreMessageHandler.h"

namespace hopsan {

//! @brief Template help function that checks status messages from the registration status in a class factory
//! @details This functionality is not included in the actual class factory as we want to keep the class factory clean
//! Hopsan should depend on the ClassFactory but the ClassFactory should not depend on Hopsan
template<typename FactoryT>
void checkClassFactoryStatus(FactoryT  *pFactory, HopsanCoreMessageHandler *pMessenger)
{
    typename FactoryT::RegStatusVectorT statusMap = pFactory->getRegisterStatus();
    typename FactoryT::RegStatusVectorT::iterator it;

    for ( it=statusMap.begin(); it!=statusMap.end(); ++it)
    {
        if ( it->second == FactoryT::RegisteredOk )
        {
            pMessenger->addDebugMessage("Registered: "+ it->first + " in core", "successfulregister");
        }
        else if ( it->second == FactoryT::AllreadyRegistered )
        {
            pMessenger->addWarningMessage("Keyvalue: "+ it->first + " was already registerd in core. Your new Component or Node will NOT be availiable!", "alreadyregistered");
        }
        else if ( it->second == FactoryT::NotRegistered )
        {
            pMessenger->addWarningMessage("Keyvalue: "+ it->first + " has not been registered in core", "notregistered");
        }
    }
}

}
#endif // CLASSFACTORYSTATUSCHECK_HPP
