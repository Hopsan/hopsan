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


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "../win32dll.h"
#include "../ComponentEssentials.h"
#include "../HopsanEssentials.h"

using namespace std;

class DLLIMPORTEXPORT FileAccess
{
public:
    FileAccess();
    FileAccess(string filename);
    void setFilename(string filename);
    ComponentSystem* loadModel(HopsanEssentials* pHopsan, double *startTime, double *stopTime, string *plotComponent, string *plotPort);
    ComponentSystem* loadModel(HopsanEssentials* pHopsan, string filename, double *startTime, double *stopTime, string *plotComponent, string *plotPort);
    void saveModel(string fileName, ComponentSystem* pMainModel, double startTime, double stopTime, string plotComponent, string plotPort);

private:
    string mFilename;
    ComponentSystem mModel;
    void saveComponentSystem(ofstream& modelFile, ComponentSystem* pMotherModel, string motherSystemName);
};

#endif // FILEACCESS_INCLUDED

