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

TempDirectoryHandle::TempDirectoryHandle(HString prefix)
{
        mPath = getTempDirectory();
        mPath.append("/Hopsan/"+prefix);
        mPath.append(generateRandomNumericString());

#if __cplusplus >= 201703L
        mPath = std::filesystem::create_directory(mPath.c_str());
#else
        mIsValid = createDirectory(mPath);
#endif
}

TempDirectoryHandle::~TempDirectoryHandle()
{
    removeDirectory(mPath);
}

HString TempDirectoryHandle::path()
{
    return mPath;
}

bool TempDirectoryHandle::isValid()
{
    return mIsValid;
}


HString TempDirectoryHandle::getTempDirectory() {
#if __cplusplus >= 201703L
        mPath = std::filesystem::temp_directory_path().c_str();
#else
#ifdef _WIN32
    char tempdirbuff[MAX_PATH+1];
    GetTempPathA(MAX_PATH+1, tempdirbuff);
    return HString(tempdirbuff);
#else
    char * val = getenv("TMPDIR");
    if(val != nullptr) {
        return val;
    }
    val = getenv("TMP");
    if(val != nullptr) {
        return val;
    }
    val = getenv("TEMP");
    if(val != nullptr) {
        return val;
    }
    val = getenv("TEMPDIR");
    if(val != nullptr) {
        return val;
    }
    return "/tmp";
#endif
#endif
}

HString TempDirectoryHandle::generateRandomNumericString()
{
#if __cplusplus >= 201103L
        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_int_distribution<> distrib(0, 999999999);
        int random = distrib(gen);
#else
        int random = rand() % 1000000000;
#endif
        std::stringstream ss;
        ss << std::setw(6) << std::setfill('0') << random;
        return ss.str().c_str();
}

bool TempDirectoryHandle::createDirectory(HString path)
{
#if __cplusplus >= 201703L
        return std::filesystem::create_directory(path.c_str());
#else
    HString currentLevel = "";
    std::string level;
    std::stringstream ss(path.c_str());

    while(std::getline(ss, level, '/')) {
        currentLevel.append(level.c_str());
#ifdef _WIN32
        if (!directoryExists(currentLevel) && _mkdir(currentLevel.c_str()) != 0) {
            return false;
        }
#else
        if(!currentLevel.empty()) {
            if (!directoryExists(currentLevel)) {
                if(mkdir(currentLevel.c_str(), 0755) != 0) {
                    return false;
                }
            }
        }
#endif

        currentLevel += "/"; // don't forget to append a slash
    }
#endif
    return directoryExists(path);
}


bool TempDirectoryHandle::directoryExists(HString &foldername)
{
    struct stat st;
    stat(foldername.c_str(), &st);
    return st.st_mode & S_IFDIR;
}

bool TempDirectoryHandle::removeDirectory(HString path)
{
#if __cplusplus >= 201703L
    std::filesystem::remove_all(path.c_str());
#else
#ifdef _WIN32
    WCHAR szDir[MAX_PATH+1];
    SHFILEOPSTRUCTW fos = {0};
    std::string tempStr(path.c_str());
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
    DIR *dir = opendir(path.c_str());
    size_t path_len = path.size();
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

                snprintf(buf, len, "%s/%s", path.c_str(), p->d_name);
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
        success = (0 == rmdir(path.c_str()));
    }

    return success;
#endif
#endif
}

