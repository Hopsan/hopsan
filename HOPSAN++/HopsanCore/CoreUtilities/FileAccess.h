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

namespace hopsan {

    std::string readName(std::stringstream &rTextStream);
    std::string readName(std::string namestring);
    std::string addQuotes(std::string str);

    class DLLIMPORTEXPORT FileAccess
    {
    public:
        FileAccess();
        FileAccess(std::string filename);
        void setFilename(std::string filename);
        ComponentSystem* loadModel(HopsanEssentials* pHopsan, double *startTime, double *stopTime);
        ComponentSystem* loadModel(HopsanEssentials* pHopsan, std::string filename, double *startTime, double *stopTime);
        void loadModelContents(std::stringstream &rLoaddatastream, ComponentSystem* pSubsystem, HopsanEssentials* pHopsan);

        void saveModel(std::string fileName, ComponentSystem* pMainModel, double startTime, double stopTime);


    private:
        std::string mFilename;
        ComponentSystem mModel;
        void saveComponentSystem(std::ofstream& modelFile, ComponentSystem* pMotherModel, std::string motherSystemName);

    };
}

#endif // FILEACCESS_INCLUDED

