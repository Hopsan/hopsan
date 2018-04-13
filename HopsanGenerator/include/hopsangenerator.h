
#ifndef HOPSANGENERATOR_H
#define HOPSANGENERATOR_H

#include "hopsangenerator_win32dll.h"

extern "C" {

HOPSANGENERATOR_DLLAPI void setMessageHandler(void (*message_handler)(const char* msg, char type));

HOPSANGENERATOR_DLLAPI void callCppGenerator(const char* hppPath, const char* compilerPath, bool compile=false, const char* hopsanInstallPath="");

HOPSANGENERATOR_DLLAPI void callLibraryGenerator(const char*  outputPath, const char* const hppFiles[], const int numFiles, bool quiet=false);

HOPSANGENERATOR_DLLAPI void callComponentLibraryCompiler(const char* outputPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, bool quiet=false);

HOPSANGENERATOR_DLLAPI void callModelicaGenerator(const char* moFilePath, const char* compilerPath, bool quiet=false, int solver=0, bool compile=false, const char* hopsanInstallPath="");

HOPSANGENERATOR_DLLAPI void callFmuImportGenerator(const char* fmuFilePath, const char* targetPath, const char* hopsanInstallPath, const char* compilerPath, bool quiet=false);

HOPSANGENERATOR_DLLAPI void callFmuExportGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, const char* compilerPath, int version=2, int architecture=64, bool quiet=false);

HOPSANGENERATOR_DLLAPI void callSimulinkExportGenerator(const char* outputPath, const char* modelFile, void* pHopsanSystem, bool disablePortLabels, const char* hopsanInstallPath, bool quiet=false);

HOPSANGENERATOR_DLLAPI void callLabViewSITGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, bool quiet=false);

}

#endif // HOPSANGENERATOR_H
