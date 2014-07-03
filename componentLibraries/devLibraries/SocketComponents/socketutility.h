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


private:
    size_t readBytes(char *pTargetBuffer, size_t nBytes);


    SocketUtility_d *_d;
    std::string mErrorString;
};

#endif // SOCKETUTILITY_H
