#ifndef LOADEXTERNAL_H
#define LOADEXTERNAL_H

#include <string>
#include <iostream>

#ifdef WIN32
#include "windows.h"
#else
#include <dlfcn.h>
#endif


using namespace std;

class LoadExternal
{
public:
    LoadExternal();
    void Load(string libpath)
    {

#ifdef WIN32
        HINSTANCE lib_ptr;
        lib_ptr = LoadLibrary(libpath.c_str());

        if(!lib_ptr)
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
        lib_ptr = dlopen(libpath.c_str(), RTLD_NOW);
        //lib_ptr = dlopen(libpath.c_str(), RTLD_LAZY);
        if(!lib_ptr)
        {
            cout << "Error opening external lib: " << libpath << endl;
            cout << dlerror() << endl;
            //exit(-1);
        }
        else
        {
            cout << "Succes (probably) opening external lib: " << libpath << endl;
        }
#endif


    }
protected:
private:
};

#endif // LOADEXTERNAL_H
