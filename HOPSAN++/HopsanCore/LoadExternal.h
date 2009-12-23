#ifndef LOADEXTERNAL_H
#define LOADEXTERNAL_H

#include <string>
#include <iostream>
#include <dlfcn.h>

using namespace std;

class LoadExternal
{
    public:
        LoadExternal();
        void Load(string libpath)
        {
            void *lib_ptr;
            lib_ptr = dlopen(libpath.c_str(), RTLD_NOW);
            if(lib_ptr == NULL){
               cout << "Error opening external lib: " << libpath << endl;
               //cerr << dlerror() << endl;
               //exit(-1);
            }

        }
    protected:
    private:
};

#endif // LOADEXTERNAL_H
