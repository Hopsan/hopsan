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
