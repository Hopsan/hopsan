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
//! @file   TempDirectoryHandle.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2021-02-02
//!
//! @brief Contains a temp directory handle class
//!
//$Id$

#ifndef TEMPDIRECTORYHANDLE_H
#define TEMPDIRECTORYHANDLE_H

#include "win32dll.h"
#include "HopsanTypes.h"

namespace hopsan {

    class HOPSANCORE_DLLAPI TempDirectoryHandle
    {
    public:
        TempDirectoryHandle(const HString &rPrefix);
        ~TempDirectoryHandle();
        const HString &path() const;
        bool isValid() const;
        bool addSubDirectory(const HString &rName);

    private:
        HString mPath;
        bool mIsValid;
        const HString getTempDirectory() const;
        const HString generateRandomNumericString() const;
        bool createPath(const HString &rPath) const;
        bool createDir(const HString &rName) const;
        bool directoryExists(const HString &rName) const;
        bool removeDirectory(const HString &path) const;
    };
}

#endif // TEMPDIRECTORYHANDLE_H
