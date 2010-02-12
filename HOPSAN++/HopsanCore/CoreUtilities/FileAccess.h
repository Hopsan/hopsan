//!
//! @file   FileAccess.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-03
//!
//! @brief Contains the file access functions
//!
//$Id$

#ifndef FILEACCESS_H_INCLUDED
#define FILEACCESS_H_INCLUDED

#include "win32dll.h"
#include <string>
#include "HopsanCore.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

class DLLIMPORTEXPORT FileAccess
{
public:
    FileAccess();
    FileAccess(string filename);
    void setFilename(string filename);
    ComponentSystem loadModel(double *startTime, double *stopTime, string *plotComponent, string *plotPort);
    ComponentSystem loadModel(string filename, double *startTime, double *stopTime, string *plotComponent, string *plotPort);
    void saveModel(ComponentSystem mainModel);

private:
    string mFilename;
    ComponentSystem mModel;
    void saveComponentSystem(ofstream& modelFile, ComponentSystem& motherModel, string motherSystemName);
};

#endif // FILEACCESS_INCLUDED

