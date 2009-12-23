#include "Nodes.h"
#include "ClassFactory.h"

string NodeHydraulic::iDummyId = CClassFactory<string, Node>::RegisterCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
string NodeMechanic::iDummyId = CClassFactory<string, Node>::RegisterCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
