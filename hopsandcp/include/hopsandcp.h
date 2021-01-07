#ifndef HOPSANDCP_H
#define HOPSANDCP_H

#include <string>

// Forward Declaration
namespace hopsan {
    class Port;
    class ComponentSystem;
    class HopsanEssentials;
    class HString;
}

extern hopsan::HopsanEssentials gHopsanCore;
extern std::string gHost;
extern int gPort;

#endif // HOPSANDCP_H
