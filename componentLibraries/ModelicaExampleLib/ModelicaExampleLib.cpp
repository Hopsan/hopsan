#include <iostream>
#include "ComponentEssentials.h"
#include "ModelicaMass.hpp"
#include "ModelicaOrifice.hpp"
#include "Modelica22Valve.hpp"
#include "ModelicaSecondOrderTransferFunction.hpp"
#include "ModelicaAccumulator.hpp"
#include "ModelicaIfElse.hpp"
#include "ModelicaCylinderQ.hpp"
#include "ModelicaElectricMotor.hpp"

using namespace hopsan;

extern "C" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    //Register Components
    pComponentFactory->registerCreatorFunction("ModelicaMass", ModelicaMass::Creator);
    pComponentFactory->registerCreatorFunction("ModelicaOrifice", ModelicaOrifice::Creator);
    pComponentFactory->registerCreatorFunction("Modelica22Valve", Modelica22Valve::Creator);
    pComponentFactory->registerCreatorFunction("ModelicaSecondOrderTransferFunction", ModelicaSecondOrderTransferFunction::Creator);
    pComponentFactory->registerCreatorFunction("ModelicaAccumulator", ModelicaAccumulator::Creator);
    pComponentFactory->registerCreatorFunction("ModelicaIfElse", ModelicaIfElse::Creator);
    pComponentFactory->registerCreatorFunction("ModelicaCylinderQ", ModelicaCylinderQ::Creator);
    pComponentFactory->registerCreatorFunction("ModelicaElectricMotor", ModelicaElectricMotor::Creator);

    //Register custom nodes (if any)
    HOPSAN_UNUSED(pNodeFactory);
}

extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;
    pHopsanExternalLibInfo->libName = (char*)"ModelicaExampleLib";
}
