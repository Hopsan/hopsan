#ifndef NUMHOPHELPER_H
#define NUMHOPHELPER_H

#include "HopsanTypes.h"

namespace hopsan {

class Component;
class ComponentSystem;
class NumHopHelperPrivate;

class NumHopHelper
{
public:
    NumHopHelper();
    ~NumHopHelper();

    void setSystem(ComponentSystem *pSystem);
    void setComponent(Component *pComponent);

    void registerDataPtr(const HString &name, double *pData);

    bool evalNumHopScript(const HString &script, double &rValue, bool doPrintOutput, HString &rOutput);
    bool interpretNumHopScript(const HString &script, bool doPrintOutput, HString &rOutput);
    bool eval(double &rValue, bool doPrintOutput, HString &rOutput);

private:
    ComponentSystem *mpSystem;
    Component *mpComponent;
    NumHopHelperPrivate *mpPrivate;
};

}

#endif // NUMHOPHELPER_H
