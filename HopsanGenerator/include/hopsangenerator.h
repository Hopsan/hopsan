
#ifndef HOPSANGENERATOR_H
#define HOPSANGENERATOR_H

#include "hopsangenerator_win32dll.h"

extern "C" {

    typedef void (*messagehandler_t)(const char*, const char, void*);

    void setMessageHandler(messagehandler_t messageHandler);

    HOPSANGENERATOR_DLLAPI void callCppGenerator(const char* hppPath, const char* compilerPath, bool compile=false, const char* hopsanInstallPath="", messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI void callLibraryGenerator(const char*  outputPath, const char* const hppFiles[], const int numFiles, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI void callComponentLibraryCompiler(const char* outputPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI void callModelicaGenerator(const char* moFilePath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0, int solver=0, bool compile=false, const char* hopsanInstallPath="");

    HOPSANGENERATOR_DLLAPI void callFmuImportGenerator(const char* fmuFilePath, const char* targetPath, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI void callFmuExportGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, const char* compilerPath, int version=2, int architecture=64, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI void callSimulinkExportGenerator(const char* outputPath, const char* modelFile, void* pHopsanSystem, bool disablePortLabels, const char* hopsanInstallPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI void callLabViewSITGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

}

#endif // HOPSANGENERATOR_H
