//!
//! @file   LoadExternal.h
//! @author <peter.nordin@liu.se>
//! @date   2009-12-22
//!
//! @brief Contains the ExternalLoader class
//!
//$Id$

#ifndef LOADEXTERNAL_H
#define LOADEXTERNAL_H

#include <string>
#include "../win32dll.h"
#include "../Component.h"

using namespace std;

//! @brief This class handles loading and unloading of external component and node libs
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
