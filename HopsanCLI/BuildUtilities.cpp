
//$Id$

#include "BuildUtilities.h"

#include "CoreUtilities/GeneratorHandler.h"
#include "CliUtilities.h"

#include <iostream>

using namespace std ;

bool buildComponentLibrary(const std::string &rLibraryXML, std::string &rOutput)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        string path = getCurrentExecPath();
        // Expected include and bin path for HopsanCore
        string hopsanIncludePath = path+"../HopsanCore/include/";
        string hopsanBinPath = path;

        pHandler->callComponentLibraryCompiler(rLibraryXML.c_str(), "", "", hopsanIncludePath.c_str(), hopsanBinPath.c_str(), "", false);
        delete(pHandler);
        return true;
    }
    delete(pHandler);
    return false;
}
