#ifndef GUIROOTSYSTEM_H
#define GUIROOTSYSTEM_H

#include "HopsanCore.h"

class GUIRootSystem
{
public:
    GUIRootSystem();
    bool connect();
    bool disconnect();

private:
    //*****Core Interaction*****
    ComponentSystem *mpCoreComponentSystem;
    //**************************
};

#endif // GUIROOTSYSTEM_H
