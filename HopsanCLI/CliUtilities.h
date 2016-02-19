/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
void printColorMessage(const ColorsEnumT color, const std::string &rMessage, bool silent=false);
void setTerminalColor(const ColorsEnumT color);

// ===== Sys Functions =====
size_t getNumAvailibleCores();

// ===== Data Functions =====
bool compareVectors(const std::vector<double> &rVec, const std::vector<double> &rRef, const double tol);

// ===== Read File Functions =====
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames);

#endif // CLIUTILITIES_H
