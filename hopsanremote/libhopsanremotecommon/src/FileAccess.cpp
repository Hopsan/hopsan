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

#include "hopsanremotecommon/FileAccess.h"

#include <sstream>
#include <iostream>
#include <errno.h>
#include <cstring>

#ifdef _WIN32
#  include <direct.h>
#  ifdef __MINGW32__
#    include <dirent.h>
#  elif defined _MSC_VER
//   Use local copy of https://github.com/tronkko/dirent
#    include "dirent.h" 
#  endif
#else
#  include <unistd.h>
#  include <dirent.h>
#endif

#include <sys/stat.h>

using namespace std;

FileAccess::FileAccess()
{
    mCurrentDir = ".";
}


bool FileAccess::enterDir(const std::string &rDir)
{
    DIR *pDir = opendir(rDir.c_str());
    if (pDir != nullptr)
    {
        mCurrentDir = rDir.c_str();
        (void)closedir(pDir);
        return true;
    }
    return false;
}

std::string FileAccess::currentDir() const
{
    return mCurrentDir;
}

bool FileAccess::createDir(string dirPath)
{
    string fullDirPath = mCurrentDir+"/";

    stringstream ss(dirPath);
    string dirname;
    while(std::getline(ss, dirname, '/'))
    {
        if (!dirname.empty())
        {
            fullDirPath += dirname;
#ifdef _WIN32
            int rc = _mkdir(fullDirPath.c_str());
#else
            int rc = mkdir(fullDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

            if (rc == 0)
            {
                std::cout << " Created dir: " << fullDirPath << std::endl;
            }
            else if ( (rc < 0) && (errno != EEXIST) )
            {
                std::cout << " Could not create dir: " << fullDirPath << std::endl;
                cout << "Errno: " << errno << " = " << strerror(errno) << endl;
                return false;
            }
            fullDirPath+="/";
        }
    }
    return true;
}

std::vector<std::string> FileAccess::findFilesWithSuffix(std::string suffix, bool doRecursiveSearch)
{
    vector<std::string> files;
    DIR *pDir = opendir(mCurrentDir.c_str());
    dirent *pEntry;
    while ((pEntry = readdir(pDir)) != nullptr)
    {
        std::string entrypath = mCurrentDir+"/"+string(pEntry->d_name);
        struct stat s;
        stat(entrypath.c_str(), &s);
        //cout << "pEntery->d_name: " << pEntry->d_name << endl;
        //cout << "stat->st_mode: " << s.st_mode << "\t path: " << entrypath <<  endl;
        if ( S_ISDIR(s.st_mode) && doRecursiveSearch &&
             !((strcmp(pEntry->d_name, ".")==0 || strcmp(pEntry->d_name, "..")==0)) )
        {
            FileAccess fa;
            //cout << "Entering: " << entrypath << endl;
            fa.enterDir(entrypath);
            vector<std::string> subfiles = fa.findFilesWithSuffix(suffix, doRecursiveSearch);
            files.insert(files.end(), subfiles.begin(), subfiles.end());
        }
        else
        {
            string name(pEntry->d_name);
            size_t p = name.find_last_of(".");
            if (p != string::npos)
            {
                if (name.substr(p) == suffix)
                {
                    files.push_back(mCurrentDir+"/"+name);
                }
            }
        }
    }
    (void)closedir(pDir);
    return files;
}


void splitFilePath(const string &rFullPath, string &rPath, string &rFileName)
{
#ifdef _WIN32
    // On windows search first for \ if we can find it then test with / (in case we use posix path strings)
    size_t e = rFullPath.find_last_of("\\");
    if (e == string::npos)
    {
        e = rFullPath.find_last_of("/");
    }
#else
    // If not windows then use POSIX path separator
    size_t e = rFullPath.find_last_of("/");
#endif
    if (e != string::npos)
    {
        rPath = rFullPath.substr(0, e+1);
        rFileName = rFullPath.substr(e+1);
    }
    else
    {
        rPath.clear();
        rFileName = rFullPath;
    }
}
