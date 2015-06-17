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
