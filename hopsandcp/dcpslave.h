#ifndef DCPSLAVE_H
#define DCPSLAVE_H

#include <string>
#include <vector>

namespace hopsan {
class HopsanEssentials;
class ComponentSystem;
}

struct SlaveDescription_t;

class DcpSlave
{
public:
    DcpSlave(const std::string modelfile, const std::string host, int port);
    ~DcpSlave();

    void generateDescriptionFile(std::string &targetFile);

private:
    hopsan::HopsanEssentials *mpHopsanCore;
    hopsan::ComponentSystem *mpRootSystem;
    std::vector<std::string> mInputs, mOutputs;
    std::string mHost = "127.0.0.1";
    int mPort = 8080;
    void printWaitingMessages();
    SlaveDescription_t *getSlaveDescription();
};

#endif // DCPSLAVE_H
