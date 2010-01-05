#ifndef LOADEXTERNAL_H
#define LOADEXTERNAL_H

#include <string>
#include "win32dll.h"

#include "Component.h"


using namespace std;

class DLLIMPORTEXPORT LoadExternal
{
private:
    ComponentFactory* mpComponentFactory;
    NodeFactory* mpNodeFactory;

public:
    LoadExternal();
    void load(string libpath);
    void setFactory(ComponentFactory* cfactory_ptr, NodeFactory* nfactory_ptr);
};

#endif // LOADEXTERNAL_H
