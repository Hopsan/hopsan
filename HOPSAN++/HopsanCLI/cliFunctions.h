/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   cliFunctions.h
//! @author FluMeS
//! @date   2012-05-30
//!
//! @brief Contains helpfunctions for CLI
//!
//$Id$

#ifndef CLIFUNCTIONS_H
#define CLIFUNCTIONS_H

#include <string>
#include <vector>

// Forward Declaration
namespace hopsan {
class ComponentSystem;
}

// ===== Help Functions =====
void splitFilePath(const std::string fullPath, std::string &rBasePath, std::string &rFileName);
void splitFileName(const std::string fileName, std::string &rBaseName, std::string &rExt);
void splitStringOnDelimiter(const std::string &rString, const char delim, std::vector<std::string> &rSplitVector);

// ===== Print functions =====
enum ColorsEnumT {Red, Green, Blue, Yellow, White, Reset};
void printWaitingMessages(const bool printDebug=true);
void printErrorMessage(const std::string &rError);
void printWarningMessage(const std::string &rWarning);
void printColorMessage(const ColorsEnumT color, const std::string &rMessage);
void printTsInfo(const hopsan::ComponentSystem* pSystem);
void printSystemParams(hopsan::ComponentSystem* pSystem);
void printComponentHierarchy(hopsan::ComponentSystem *pSystem, std::string prefix="",
                             const bool doPrintTsInfo=false,
                             const bool doPrintSystemParams=false);
void setTerminalColor(const ColorsEnumT color);

// ===== Save Functions =====
enum SaveResults {Final, Full};
void saveNodeDataToFile(hopsan::ComponentSystem* pSys, const std::string compName, const std::string portName, const std::string fileName);
void saveResults(hopsan::ComponentSystem *pSys, const std::string &rFileName, const SaveResults howMany, std::string prefix="", std::ofstream *pFile=0);
void transposeCSVresults(const std::string &rFileName);
void exportParameterValuesToCSV(const std::string &rFileName, hopsan::ComponentSystem* pSystem, std::string prefix="", std::ofstream *pFile=0);

// ===== Load Functions =====
void importParameterValuesFromCSV(const std::string filePath, hopsan::ComponentSystem* pSystem);
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames);
void readNodesToSaveFromTxtFile(const std::string filePath, std::vector<std::string> &rComps, std::vector<std::string> &rPorts);

// ===== compare Functions =====
bool compareVectors(const std::vector<double> &rVec, const std::vector<double> &rRef, const double tol);
bool performModelTest(const std::string hvcFilePath);


#endif // CLIFUNCTIONS_H
