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
