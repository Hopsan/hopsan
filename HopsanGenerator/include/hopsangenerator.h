
#ifndef HOPSANGENERATOR_H
#define HOPSANGENERATOR_H

#include "hopsangenerator_win32dll.h"
#include <stdbool.h>

extern "C" {

    typedef void (*messagehandler_t)(const char*, const char, void*);

    HOPSANGENERATOR_DLLAPI bool callCppGenerator(const char* hppPath, const char* compilerPath, bool compile=false, const char* hopsanInstallPath="", messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callLibraryGenerator(const char*  outputPath, const char* const hppFiles[], const int numFiles, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callComponentLibraryCompiler(const char* outputPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callModelicaGenerator(const char* moFilePath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0, int solver=0, bool compile=false, const char* hopsanInstallPath="");

    HOPSANGENERATOR_DLLAPI bool callFmuImportGenerator(const char* fmuFilePath, const char* targetPath, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callFmuExportGenerator(const char* outputPath, void* pHopsanSystem,  const char* const externalLibraries[], const int numLibraries, const char* hopsanInstallPath, const char* compilerPath, int version=2, int architecture=64, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callSimulinkExportGenerator(const char* outputPath, const char* modelFile, void* pHopsanSystem, const char* const externalLibraries[], const int numLibraries, bool disablePortLabels, const char* hopsanInstallPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callLabViewSITGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callCheckComponentLibrary(const char* libraryXMLPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callExeExportGenerator(const char* outputPath, void* pHopsanSystem,  const char* const externalLibraries[], const int numLibraries, const char* hopsanInstallPath, const char* compilerPath, int architecture=64, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callAddComponentToLibrary(const char* libraryXMLPath, const char* typeName, const char* displayName, messagehandler_t messageHandler=0, void* pMessageObject=0);
}

#endif // HOPSANGENERATOR_H
