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

#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include <string>
#include <iostream>
#include <map>
#include <fstream>

#include "FileAccess.h"
#include "Messages.h"

class FileReceiver
{
public:
    void setFileDestination(const std::string &rDestination)
    {
        // We do not allow empty destinations as that would cause writing of files relative root directory if absolute path files are added
        if (!rDestination.empty())
        {
            mFileDestination = rDestination;
        }
    }

    std::string fileDestination() const
    {
        return mFileDestination;
    }

    bool hasFile(const std::string &rName)
    {
        return mFileMap.find(rName) != mFileMap.end();
    }

    bool isFileComplete(const std::string &rName)
    {
        auto it = mFileMap.find(rName);
        if (it != mFileMap.end())
        {
            return it->second;
        }
        return false;
    }

    bool addFilePart(const CmdmsgSendFile &rFilePart, std::string &rErrorMessage)
    {
        // As a safety this must be set to avoid accidentally overwriting stuff on the local disc
        // well you can do that anyway if you set this to something stupid
        if (mFileDestination.empty())
        {
            return false;
        }

        std::string dirPath, fileName;
        splitFilePath(rFilePart.filename, dirPath, fileName);

        FileAccess fa;
        if(fa.enterDir(mFileDestination))
        {
            fa.createDir(dirPath);
        }
        std::string fullFilePath=mFileDestination+"/"+rFilePart.filename;

        std::ofstream file;
        //! @todo what if we have file and send it again, should we truncate or just abort somehow, here it would be nice to know checksum of entire file and tell client we already have it
        //! @todo maybe better to have a "have file request instead"
        if (hasFile(fullFilePath))
        {
            // Open to append
            file.open(fullFilePath, std::ios::out | std::ios::app | std::ios::binary);
        }
        else
        {
            // Open to replace
            file.open(fullFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
        }

        if (file.is_open())
        {
            // Write data to file
            file.write(rFilePart.data.data(), rFilePart.data.size());

            // Remember file
            mFileMap.insert(std::pair<std::string, bool>(fullFilePath, rFilePart.islastpart));

            // Return success
            //! @todo should check for errors in writing
            file.close();
            return true;
        }
        rErrorMessage = " Could not open " + fullFilePath + " for writing!";
        return false;
    }

    void clear()
    {
        // If we forget all files they will be replaced the next time they are received
        mFileMap.clear();
        mFileDestination = "./";
    }

private:
    //! @todo some file meta data struct instead of iscomplete bool
    std::map<std::string, bool> mFileMap;
    std::string mFileDestination = "./";

};

#endif
