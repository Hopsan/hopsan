#include "Components.h"
#include "../Component.h"


DLLIMPORTEXPORT void register_components(ComponentFactory* cfact_ptr)
{
    cfact_ptr->RegisterCreatorFunction();
}
