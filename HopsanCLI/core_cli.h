#ifndef GLOBAL_H
#define GLOBAL_H

// Forward Declaration
namespace hopsan {
class Port;
class ComponentSystem;
class HopsanEssentials;
class HString;
}

extern hopsan::HopsanEssentials gHopsanCore;

void printWaitingMessages(const bool printDebug=true);

#endif // GLOBAL_H
