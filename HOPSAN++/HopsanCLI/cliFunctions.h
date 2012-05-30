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
namespace hopsan{
class ComponentSystem;
}

// ===== Print functions =====
void printWaitingMessages(const bool printDebug=true);
void printTsInfo(const hopsan::ComponentSystem* pSystem);
void printSystemParams(hopsan::ComponentSystem* pSystem);
void printComponentHierarchy(hopsan::ComponentSystem *pSystem, std::string prefix="",
                             const bool doPrintTsInfo=false,
                             const bool doPrintSystemParams=false);
void setColor(unsigned int color);

// ===== Save Functions =====
void saveNodeDataToFile(hopsan::ComponentSystem* pSys, const std::string compName, const std::string portName, const std::string fileName);

// ===== Load Functions =====
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames);
void readNodesToSaveFromTxtFile(const std::string filePath, std::vector<std::string> &rComps, std::vector<std::string> &rPorts);

// ===== compare Functions =====
bool compareVectors(const std::vector<double> &rVec, const std::vector<double> &rRef, const double tol);
void performModelTest(std::string modelName);


#endif // CLIFUNCTIONS_H
