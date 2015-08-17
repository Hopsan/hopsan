//$Id$

#include "FileAccess.h"

#include <dirent.h>
#include <sstream>
#include <iostream>
#include <errno.h>
#include <cstring>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

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
            int rc = mkdir(fullDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

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

std::vector<std::string> FileAccess::findFilesWithSuffix(std::string suffix)
{
    vector<std::string> files;
    DIR *pDir = opendir(mCurrentDir.c_str());
    dirent *pEntery;
    while ((pEntery = readdir(pDir)) != nullptr)
    {
        string name(pEntery->d_name);
        size_t p = name.find_last_of(".");
        if (p != string::npos)
        {
            if (name.substr(p) == suffix)
            {
                files.push_back(mCurrentDir+"/"+name);
            }
        }
    }
    (void)closedir(pDir);
    return files;
}


void splitFilePath(const string &rFullPath, string &rPath, string &rFileName)
{
    size_t e = rFullPath.find_last_of("/");
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
