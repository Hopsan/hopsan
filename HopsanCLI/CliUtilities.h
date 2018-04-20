/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HopsanCLI/CliUtilities.h
//! @author peter.nordin@liu.se
//! @date   2014-12-09
//!
//! @brief Contains helpfunctions for CLI
//!
//$Id$

#ifndef CLIUTILITIES_H
#define CLIUTILITIES_H

#include <string>
#include <vector>
#include <sstream>

// ===== Help Functions for String Paths =====
void splitFilePath(const std::string fullPath, std::string &rBasePath, std::string &rFileName);
void splitFileName(const std::string fileName, std::string &rBaseName, std::string &rExt);
void splitStringOnDelimiter(const std::string &rString, const char delim, std::vector<std::string> &rSplitVector);
std::string relativePath(std::string basePath, std::string fullPath);
std::string getCurrentExecPath();

// ===== Print functions =====
enum ColorsEnumT {Red, Green, Blue, Yellow, White, Reset};
void printErrorMessage(const std::string &rError, bool silent=false);
void printWarningMessage(const std::string &rWarning, bool silent=false);
void printMessage(const std::string &rMessage, bool silent=false);
void printColorMessage(const ColorsEnumT color, const std::string &rMessage, bool silent=false);
void setTerminalColor(const ColorsEnumT color);

// ===== Sys Functions =====
size_t getNumAvailibleCores();

// ===== Data Functions =====
bool compareVectors(const std::vector<double> &rVec, const std::vector<double> &rRef, const double tol);

// ===== Read File Functions =====
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames);

#endif // CLIUTILITIES_H
