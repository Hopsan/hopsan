#ifndef SAVERESTORESIMULATIONPOINT_H
#define SAVERESTORESIMULATIONPOINT_H

#include "HopsanTypes.h"

namespace hopsan {

class ComponentSystem;

void saveSimulationPoint(HString fileName, ComponentSystem* pRootSystem);
void restoreSimulationPoint(HString fileName, ComponentSystem* pRootSystem);

}

#endif // SAVERESTORESIMULATIONPOINT_H
