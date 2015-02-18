//$Id$

#include "FileAccess.h"

#include <dirent.h>

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
