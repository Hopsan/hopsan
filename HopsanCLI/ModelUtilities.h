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

#ifndef MODELUTILITIES_H
#define MODELUTILITIES_H

#include <string>
#include <vector>
#include "core_cli.h"

void printTsInfo(const hopsan::ComponentSystem* pSystem);
void printSystemParams(hopsan::ComponentSystem* pSystem);
void printComponentHierarchy(hopsan::ComponentSystem *pSystem, std::string prefix="",
                             const bool doPrintTsInfo=false,
                             const bool doPrintSystemParams=false);

// ===== Save Functions =====
enum SaveResults {Final, Full};
void saveResults(hopsan::ComponentSystem *pSys, const std::string &rFileName, const SaveResults howMany, std::string prefix="", std::ofstream *pFile=0);
void transposeCSVresults(const std::string &rFileName);
void exportParameterValuesToCSV(const std::string &rFileName, hopsan::ComponentSystem* pSystem, std::string prefix="", std::ofstream *pFile=0);

// ===== Load Functions =====
void importParameterValuesFromCSV(const std::string filePath, hopsan::ComponentSystem* pSystem);
void readNodesToSaveFromTxtFile(const std::string filePath, std::vector<std::string> &rComps, std::vector<std::string> &rPorts);

// ===== Help Functions =====
void generateFullSystemHierarchyName(const hopsan::ComponentSystem *pSys, hopsan::HString &rFullSysName);
hopsan::HString generateFullPortVariableName(const hopsan::Port *pPort, const int dataId);


#endif // MODELUTILITIES_H
