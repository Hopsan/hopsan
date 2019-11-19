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

class Options {
public:
    bool set(std::string &name, std::string &value) {
        if("descriptions" == name) {
            if(value == "namesonly") {
                descriptions = NameOnly;
            }
        }
        else if("progress" == name) {
            progress = (value == "true");
        }
        else if(name == "start") {
            startT = atof(value.c_str());
        }
        else if("stop" == name) {
            stopT = atof(value.c_str());
        }
        else if("step" == name) {
            stepT = atof(value.c_str());
        }
        else if("samples" == name) {
            nSamples = atof(value.c_str());
        }
        else if("transpose" == name) {
            transpose = (value == "true");
        }
        else if("results" == name) {
            if(value == "final") {
                results = Final;
            }
        }
        else {
            return false;
        }
        return true;
    }

    double startT = 0;
    double stopT = 10;
    double stepT = 0.001;
    double nSamples = 2048;
    bool transpose = false;
    bool progress = true;
    SaveResults results = Full;
    SaveDescriptions descriptions = NameAliasUnit;
};

void saveResults(hopsan::ComponentSystem *pSys, const std::string &rFileName, const SaveResults howMany, const SaveDescriptions descriptions, std::string prefix="", std::ofstream *pFile=0);
void transposeCSVresults(const std::string &rFileName);
void importParameterValuesFromCSV(const std::string filePath, hopsan::ComponentSystem* pSystem);
bool startsWith(std::string str, std::string match);
bool setParameter(std::string &rParName, std::string &rParValue, hopsan::ComponentSystem *pSystem);
void printHelpText(hopsan::ComponentSystem *pSystem);
void printParameters(hopsan::ComponentSystem *pSystem);
void readConfigFile(std::string &filePath, Options &options);
void printWaitingMessages(hopsan::HopsanEssentials& hopsanCore, bool printDebug, bool silent);

#endif // MODELUTILITIES_H
