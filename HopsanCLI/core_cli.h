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
//! @file   HopsanCLI/core_cli.h
//! @author peter.nordin@liu.se
//! @date   2014-12-09
//!
//! @brief Contains core related declarations and help functions for CLI
//!
//$Id$

#ifndef CORE_CLI_H
#define CORE_CLI_H

// Forward Declaration
namespace hopsan {
class Port;
class ComponentSystem;
class HopsanEssentials;
class HString;
}

extern hopsan::HopsanEssentials gHopsanCore;

void printWaitingMessages(const bool printDebug=true, bool silent=false);

#endif // CORE_CLI_H
