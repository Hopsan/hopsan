#ifndef LOADEXTERNAL_H
#define LOADEXTERNAL_H

#include <string>
#include <iostream>
#include "Component.h"
#include "win32dll.h"

#ifdef WIN32
#include "windows.h"
#else
#include "dlfcn.h"
#endif


using namespace std;

class DLLIMPORTEXPORT LoadExternal
{
public:
    LoadExternal(){};
    void Load(string libpath)
    {
        typedef void (*register_contents_t)(ComponentFactory::FactoryVectorT *factory_vector_ptr);

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

        //Cant send factory ptr as it contain static memberfunctions for registering. Local static functions in dll will be used then = VERY BAD
        ComponentFactory::FactoryVectorT new_components;
        new_components.clear();                 //Make sure clean
        register_contents(&new_components);     //Send vector to dll for registration info

        //Register all components given by dll or so
        ComponentFactory::FactoryVectorT::iterator it;
        for (it = new_components.begin(); it != new_components.end(); ++it)
        {
            ComponentFactory::RegisterCreatorFunction((*it).first, (*it).second);
        }


    }
protected:
private:
};

#endif // LOADEXTERNAL_H
