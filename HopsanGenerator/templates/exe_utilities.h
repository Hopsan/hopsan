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
//! @file   exe_utilities.h
//! @author Robert Braun
//! @date   2019-05-27
//!
//! @brief Contains utility functions for exported executable models
//!
//$Id$

#ifndef MODELUTILITIES_H
#define MODELUTILITIES_H

#include <string>
#include <vector>
#include "HopsanEssentials.h"

enum SaveResults {Final, Full};
enum SaveDescriptions {NameOnly, NameAliasUnit};
void saveResults(hopsan::ComponentSystem *pSys, const std::string &rFileName, const SaveResults howMany, const SaveDescriptions descriptions, std::string prefix="", std::ofstream *pFile=0);
void transposeCSVresults(const std::string &rFileName);
void importParameterValuesFromCSV(const std::string filePath, hopsan::ComponentSystem* pSystem);
bool startsWith(std::string str, std::string match);
bool setParameter(std::string &rParName, std::string &rParValue, hopsan::ComponentSystem *pSystem);
void printHelpText(hopsan::ComponentSystem *pSystem);
void printParameters(hopsan::ComponentSystem *pSystem);

#endif // MODELUTILITIES_H
