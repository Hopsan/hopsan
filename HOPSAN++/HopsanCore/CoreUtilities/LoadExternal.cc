//!
//! @file   LoadExternal.cc
//! @author <peter.nordin@liu.se>
//! @date   2009-12-22
//!
//! @brief Contains the ExternalLoader class
//!
//$Id$

#include "LoadExternal.h"
#include "Component.h"
#include <iostream>

#ifdef WIN32
#include "windows.h"
#else
#include "dlfcn.h"
#endif

LoadExternal::LoadExternal()
{
    //ctor
}

void LoadExternal::load(string libpath)
{
    //typedef void (*register_contents_t)(ComponentFactory::FactoryPairVectorT *factory_vector_ptr);
    typedef void (*register_contents_t)(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr);

#ifdef WIN32
    HINSTANCE lib_ptr;
    lib_ptr = LoadLibrary(libpath.c_str());

    if (!lib_ptr)
    {
        cout << "Error opening external lib: " << libpath << endl;
        //cout << dlerror() << endl;
        //exit(-1);
    }
    else
    {
        cout << "Succes (probably) opening external lib: " << libpath << endl;
    }
    //Now load the register function
    register_contents_t register_contents = (register_contents_t)GetProcAddress(lib_ptr, "register_contents");
    if (!register_contents)
    {
        cout << "Cannot load symbol 'register_contents': " << GetLastError() << endl;
        //dlclose(handle);
        //return 1;
    }
#else
    void *lib_ptr;
    //First open the lib
    lib_ptr = dlopen(libpath.c_str(), RTLD_NOW);
    //lib_ptr = dlopen(libpath.c_str(), RTLD_LAZY);
    if (!lib_ptr)
    {
        cout << "Error opening external lib: " << libpath << endl;
        cout << dlerror() << endl;
        //exit(-1);
    }
    else
    {
        cout << "Succes (probably) opening external lib: " << libpath << endl;
    }
    //Now load the register function
    register_contents_t register_contents = (register_contents_t)dlsym(lib_ptr, "register_contents");
    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        cout << "Cannot load symbol 'register_contents': " << dlsym_error << endl;
        //dlclose(handle);
        //return 1;
    }

#endif

////    //Cant send factory ptr as it contain static memberfunctions for registering. Local static functions in dll will be used then = VERY BAD
////    ComponentFactory::FactoryPairVectorT new_components;
////    new_components.clear();                 //Make sure clean
////    register_contents(&new_components);     //Send vector to dll for registration info
////
////    //Register all components given by dll or so
////    ComponentFactory::FactoryPairVectorT::iterator it;
////    for (it = new_components.begin(); it != new_components.end(); ++it)
////    {
////        ComponentFactory::RegisterCreatorFunction((*it).first, (*it).second);
////    }

    register_contents(mpComponentFactory, mpNodeFactory);

}

void LoadExternal::setFactory(ComponentFactory* cfactory_ptr, NodeFactory* nfactory_ptr)
{
    mpComponentFactory = cfactory_ptr;
    mpNodeFactory = nfactory_ptr;
}
