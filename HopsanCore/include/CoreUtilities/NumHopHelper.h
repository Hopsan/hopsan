#ifndef NUMHOPHELPER_H
#define NUMHOPHELPER_H

#include "HopsanTypes.h"

namespace hopsan {

class ComponentSystem;
class NumHopHelperPrivate;

class NumHopHelper
{
public:
    NumHopHelper();
    ~NumHopHelper();

    void setSystem(ComponentSystem *pSystem);
    void evalNumHopScript(const HString &script, bool doPrintOutput, HString &rOutput);

private:
    ComponentSystem *mpSystem;
    NumHopHelperPrivate *mpPrivate;
};

}

#endif // NUMHOPHELPER_H
