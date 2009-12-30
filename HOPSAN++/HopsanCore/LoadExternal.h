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
    LoadExternal();
    void Load(string libpath)
    {

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
        typedef void (*register_contents_t)(ComponentFactory *factory_ptr);

        register_contents_t register_contents = (register_contents_t)dlsym(lib_ptr, "register_contents");
        const char *dlsym_error = dlerror();
        if (dlsym_error)
        {
            cout << "Cannot load symbol 'register_contents': " << dlsym_error << endl;
            //dlclose(handle);
            //return 1;
        }

        ///TODO: this feels ugnly, I create an instance of ClasFactory (which has all members static) so that a pointer can be sent into the register function.
        ComponentFactory cfactory;  //It seems to be ok to let this temp factory go out of scope, all contents are static anyway
        register_contents(&cfactory);



#endif


    }
protected:
private:
};

#endif // LOADEXTERNAL_H
