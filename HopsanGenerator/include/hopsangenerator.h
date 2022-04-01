
#ifndef HOPSANGENERATOR_H
#define HOPSANGENERATOR_H

#include "hopsangenerator_win32dll.h"
#include <stdbool.h>

extern "C" {

    typedef void (*messagehandler_t)(const char*, const char, void*);

    HOPSANGENERATOR_DLLAPI bool callCppGenerator(const char* hppPath, const char* compilerPath, bool compile=false, const char* hopsanInstallPath="", messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callLibraryGenerator(const char*  outputPath, const char* const hppFiles[], const int numFiles, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callComponentLibraryCompiler(const char* outputPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callModelicaGenerator(const char* moFilePath, const char* compilerPath, messagehandler_t messageHandler=0, void* pMessageObject=0, bool compile=false, const char* hopsanInstallPath="");

    HOPSANGENERATOR_DLLAPI bool callFmuExportGenerator(const char* outputPath, void* pHopsanSystem,  const char* const externalLibraries[], const int numLibraries, const char* hopsanInstallPath, const char* compilerPath, int version=2, int architecture=64, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callSimulinkExportGenerator(const char* outputPath, const char* modelFile, void* pHopsanSystem, const char* const externalLibraries[], const int numLibraries, bool disablePortLabels, const char* hopsanInstallPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callLabViewSITGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callCheckComponentLibrary(const char* libraryXMLPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callExeExportGenerator(const char* outputPath, void* pHopsanSystem,  const char* const externalLibraries[], const int numLibraries, const char* hopsanInstallPath, const char* compilerPath, int architecture=64, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callAddComponentToLibrary(const char* libraryXMLPath, const char *targetPath, const char* typeName, const char* displayName, const char* cqsType, const char *transform, const char * const constantNames[], const int numConstantNames, const char * const constantDisplayNames[], const int numConstantDisplayNames, const char * const constantUnits[], const int numConstantUnits, const char * const constantInits[], const int numConstantInits, const char * const inputNames[], const int numInputNames, const char * const inputDescriptions[], const int numInputDescriptions, const char * const inputUnits[], const int numInputUnits, const char * const inputInits[], const int numInputInits, const char * const outputNames[], const int numOutputNames, const char * const outputDescriptions[], const int numOutputDescriptions, const char * const outputUnits[], const int numOutputUnits, const char * const outputInits[], const int numOutputInits, const char * const portNames[], const int numPortNames, const char * const portDescriptions[], const int numPortDescriptions, const char * const portTypes[], const int numPortTypes, const int portsRequired[], const int numPortsRequired, bool modelica, messagehandler_t messageHandler=nullptr, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callAddExistingComponentToLibrary(const char* libraryXMLPath, const char* cafPath, messagehandler_t messageHandler=0, void* pMessageObject=0);

    HOPSANGENERATOR_DLLAPI bool callRemoveComponentFromLibrary(const char* libraryXmlPath, const char* cafPath, const char *sourceCodePath, bool deleteFiles, messagehandler_t messageHandler, void *pMessageObject);
}

#endif // HOPSANGENERATOR_H
