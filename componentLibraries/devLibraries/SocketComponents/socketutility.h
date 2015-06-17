/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef SOCKETUTILITY_H
#define SOCKETUTILITY_H

#include <vector>
#include <string>

// Forward declaration
class SocketUtility_d;

class SocketUtility
{
public:
    SocketUtility();
    ~SocketUtility();

    bool openSocket(const std::string &rOtherAddress, const std::string &rOtherPort, const std::string &rThisPort);
    void closeSocket();

    long sendBytes(const char *pData, size_t nBytes);

    size_t writeSocket(const char value);
    size_t writeSocket(const int value);
    size_t writeSocket(const double value);
    size_t writeSocket(std::vector<double> &rData);
    size_t writeSocket(const std::string &rString);

    bool readSocket(char &rValue);
    bool readSocket(char &rValue, const int timeoutMS);
    bool readSocket(double &rValue);
    bool readSocket(double &rValue, const int timeoutMS);
    bool readSocket(std::vector<double> &rData, const size_t nElements);
    bool readSocket(std::string &rString, const size_t nElements, const int timeoutMS=0);

    size_t poll();

    const std::string &getErrorString();
    size_t numWaitingBytes() const;

    void setSleepUS(int us);


private:
    size_t readBytes(char *pTargetBuffer, size_t nBytes);

    int mSleepUS;
    SocketUtility_d *_d;
    std::string mErrorString;
};

#endif // SOCKETUTILITY_H
