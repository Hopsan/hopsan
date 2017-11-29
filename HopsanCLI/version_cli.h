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
//! @file   HopsanCLI/version_cli.h
//!
//! @brief Contains version number macro for the Hopsan CLI
//!
//$Id$

#ifndef VERSION_CLI_H
#define VERSION_CLI_H

#include "HopsanCoreVersion.h"

// If we don't have the revision number then define UNKNOWN
// On real release  builds, UNKNOWN will be replaced by actual revnum by external script
#ifndef HOPSANCLI_COMMIT_TIMESTAMP
#define HOPSANCLI_COMMIT_TIMESTAMP UNKNOWN
#endif

#define HOPSANCLIVERSION HOPSANBASEVERSION "." TO_STR(HOPSANCLI_COMMIT_TIMESTAMP)

#endif // VERSION_CLI_H
