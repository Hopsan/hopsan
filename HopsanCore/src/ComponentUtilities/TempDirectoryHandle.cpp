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


#include "ComponentUtilities/num2string.hpp"

#include <iostream>
#if __cplusplus >= 201703L
#include <filesystem>
#endif
#if __cplusplus >= 201103L
#include <random>
#include <chrono>
#endif
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <shellapi.h>
#include <strsafe.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif



#include "ComponentUtilities/TempDirectoryHandle.h"

using namespace hopsan;

TempDirectoryHandle::TempDirectoryHandle(const HString &rPrefix)
    : mIsValid(false)
{
    mPath = getTempDirectory();
    mPath.append("/Hopsan/"+rPrefix);
    mPath.append(generateRandomNumericString());

    mIsValid = createPath(mPath);
}

TempDirectoryHandle::~TempDirectoryHandle()
{
    removeDirectory(mPath);
}

const HString &TempDirectoryHandle::path() const
{
    return mPath;
}

bool TempDirectoryHandle::isValid() const
{
    return mIsValid;
}


const HString TempDirectoryHandle::getTempDirectory() const {
#if __cplusplus >= 201703L
    return std::filesystem::temp_directory_path().c_str();
#else
#ifdef _WIN32
    char tempdirbuff[MAX_PATH+1];
    GetTempPathA(MAX_PATH+1, tempdirbuff);
    return HString(tempdirbuff);
#else
    char * val = getenv("TMPDIR");
    if(val != NULL) {
        return val;
    }
    val = getenv("TMP");
    if(val != NULL) {
        return val;
    }
    val = getenv("TEMP");
    if(val != NULL) {
        return val;
    }
    val = getenv("TEMPDIR");
    if(val != NULL) {
        return val;
    }
    return "/tmp";
#endif
#endif
}

const HString TempDirectoryHandle::generateRandomNumericString() const
{
#if __cplusplus >= 201103L

#ifdef __MINGW32__
    // "Bug" in MinGW (using constant seed) supposedly fixed in GCC 9.2, using current time as seed instead
    static std::mt19937 gen(static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
#else
    static std::mt19937 gen(std::random_device{}());
#endif
    static std::uniform_int_distribution<> distrib(0, 999999999);
    int random = distrib(gen);
#else
    int random = rand() % 1000000000;
#endif
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << random;
    return ss.str().c_str();
}

bool TempDirectoryHandle::createDir(const HString &rName) const
{
#ifdef _WIN32
        return _mkdir(rName.c_str()) == 0;
#else
        int ret = mkdir(rName.c_str(), 0777) == 0;
        return (ret != 0);
#endif
}

bool TempDirectoryHandle::createPath(const HString &rPath) const
{
#if __cplusplus >= 201703L
        return std::filesystem::create_directory(rPath.c_str());
#else
    HString currentLevel = "";
    std::string level;
    std::stringstream ss(rPath.c_str());

    while(std::getline(ss, level, '/')) {
        currentLevel.append(level.c_str());
        if(!currentLevel.empty()) {
            if (!directoryExists(currentLevel) && !createDir(currentLevel)) {
                return false;
            }
        }
        if(currentLevel.empty() || currentLevel[currentLevel.size()-1] != '/') {
            currentLevel += "/"; // don't forget to append a slash
        }
    }
#endif
    return directoryExists(rPath);
}


bool TempDirectoryHandle::directoryExists(const HString &rName) const
{
    struct stat st;
    if(stat(rName.c_str(), &st) != 0) {
        return false;
    }
    else if(st.st_mode & S_IFDIR) {
        return true;
    }
    return false;
}

bool TempDirectoryHandle::removeDirectory(const HString &rPath) const
{
#if __cplusplus >= 201703L
    std::filesystem::remove_all(rPath.c_str());
#else
#ifdef _WIN32
    WCHAR szDir[MAX_PATH+1];
    SHFILEOPSTRUCTW fos = {0};
    std::string tempStr(rPath.c_str());
    std::wstring stemp = std::wstring(tempStr.begin(), tempStr.end());
    StringCchCopyW(szDir, MAX_PATH,STRSAFE_LPCWSTR(stemp.c_str()));
    int len = lstrlenW(szDir);
    szDir[len+1] = 0; // Null termination

    //Delete the folder including files and subfolders
    fos.wFunc = FO_DELETE;
    fos.pFrom = szDir;
    fos.fFlags = FOF_NO_UI;
    return (0 == SHFileOperationW( &fos ));

#else
    DIR *dir = opendir(rPath.c_str());
    size_t path_len = rPath.size();
    bool success = false;

    if(dir) {
        struct dirent *p;

        success = true;
        while (success && (p=readdir(dir))) {
            char *buf;
            size_t len;

            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = static_cast<char*>(malloc(len));

            if (buf) {
                struct stat statbuf;

                snprintf(buf, len, "%s/%s", rPath.c_str(), p->d_name);
                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        success = removeDirectory(buf);
                    }
                    else {
                        success = (unlink(buf) == 0);
                    }
                }
                free(buf);
            }
        }
        closedir(dir);
    }

    if (success) {
        success = (0 == rmdir(rPath.c_str()));
    }

    return success;
#endif
#endif
}

