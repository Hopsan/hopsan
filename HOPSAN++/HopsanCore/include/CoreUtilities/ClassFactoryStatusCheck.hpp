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
#include <string>

namespace hopsan {

//! @brief Template help function that checks status messages from the registration status in a class factory
//! @details This functionality is not included in the actual class factory as we want to keep the class factory clean
//! Hopsan should depend on the ClassFactory but the ClassFactory should not depend on Hopsan
template<typename FactoryT>
void checkClassFactoryStatus(FactoryT  *pFactory, HopsanCoreMessageHandler *pMessenger)
{
    typename FactoryT::RegStatusVectorT statusMap = pFactory->getRegisterStatusMap();
    typename FactoryT::RegStatusVectorT::iterator it;

    for ( it=statusMap.begin(); it!=statusMap.end(); ++it)
    {
        if ( it->second == FactoryT::REGISTEREDOK )
        {
            pMessenger->addDebugMessage(std::string("Registered: ") + std::string(it->first) + std::string(" in core"), "successfulregister");
        }
        else if ( it->second == FactoryT::ALLREADYREGISTERED )
        {
            pMessenger->addWarningMessage(std::string("Keyvalue: ") + std::string(it->first) + std::string(" was already registerd in core. Your new Component or Node will NOT be availiable!"), "alreadyregistered");
        }
        else if ( it->second == FactoryT::NOTREGISTERED )
        {
            pMessenger->addWarningMessage(std::string("Keyvalue: ") + std::string(it->first) + std::string(" has not been registered in core"), "notregistered");
        }
    }
}

}
#endif // CLASSFACTORYSTATUSCHECK_HPP
