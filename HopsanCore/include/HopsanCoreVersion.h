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

#ifndef HOPSANCOREVERSION_H
#define HOPSANCOREVERSION_H

#include "HopsanCoreMacros.h"

// We need to use this include because external dependencies will need the revision of the core when it was compiled last time,
// not the latest revision that you get when compiling the external component.
#include "HopsanCoreGitVersion.h"

#define HOPSANBASEVERSION "2.9.0"
#define HOPSANCOREVERSION HOPSANBASEVERSION "." TO_STR(HOPSANCORE_COMMIT_TIMESTAMP)
#define HOPSANCOREMODELFILEVERSION "0.4"

#ifdef DEBUGCOMPILING
 #define DEBUGRELEASECOMPILED "DEBUG"
#elif defined  RELEASECOMPILING
 #define DEBUGRELEASECOMPILED "RELEASE"
#else
 //#warning You must specify Debug or Release compiling by defining DEBUGCOMPILING or RELEASECOMPILING
 #define DEBUGRELEASECOMPILED "UNDEFINED"
#endif

// Include compiler info macros
#include "compiler_info.h"

#endif // HOPSANCOREVERSION_H
