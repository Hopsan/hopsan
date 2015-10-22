//$Id$

#ifndef FILEACCESS_H
#define FILEACCESS_H

#include <string>
#include <vector>

class FileAccess
{
public:
    FileAccess();
    bool enterDir(const std::string &rDir);
    std::string currentDir() const;
    bool createDir(std::string dirPath);
    std::vector<std::string> findFilesWithSuffix(std::string suffix, bool doRecursiveSearch=false);
private:
    std::string mCurrentDir;
};

void splitFilePath(const std::string &rFullPath, std::string &rPath, std::string &rFileName);

#endif // FILEACCESS_H
