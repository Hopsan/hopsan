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
//! @file   SignalLogger.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2020-01-15
//!
//! @brief Contains a signal logger component
//!
//$Id$

#ifndef SIGNALLOGGER_HPP_INCLUDED
#define SIGNALLOGGER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentSystem.h"
#include "ComponentUtilities.h"
#include <fstream>
#include <ctime>


namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalLogger : public ComponentSignal
    {

    private:
        Port *mpIn;
        double *mpDt;
        HFilePath mFilePath;
        HString mNames, mAliases, mUnits;
        enum FileTypes {PlainColumnWiseCSV, PlainRowWiseCSV, HopsanRowWiseCSV, HopsanPLO};
        int mFileType;
        std::ofstream mFile;
        size_t mNumVars;
        double mLastLogTime;
        std::vector<double *> mpInData;
        std::vector<HString> mRows; //Used by row-wise file types, to write everything in finalize

    public:
        static Component *Creator()
        {
            return new SignalLogger();
        }

        void configure()
        {
            std::vector<HString> filetypes;
            filetypes.push_back("Plain column-wise CSV");
            filetypes.push_back("Plain row-wise CSV");
            filetypes.push_back("Hopsan column-wise CSV");
            filetypes.push_back("Hopsan PLO");

            mpIn = addReadMultiPort("in", "NodeSignal", "", Port::NotRequired);
            addConstant("path", "Path to output log file", mFilePath);
            addConditionalConstant("filetype", "File type", filetypes,0,mFileType);
            addInputVariable("dt", "Logging Interval (0 = log every sample)", "Time", 0, &mpDt);
            addConstant("names", "Variable names (comma separated)", "", "", mNames);
            addConstant("aliases", "Variable aliases (comma separated)", "", "", mAliases);
            addConstant("units", "Variable units (comma separated)", "", "", mUnits);
        }


        void initialize()
        {
            mRows.clear();

            mNumVars = mpIn->getNumConnectedPorts();
            mpInData.resize(mNumVars);
            for (size_t i=0; i<mNumVars; ++i)
            {
                mpInData[i] = getSafeMultiPortNodeDataPtr(mpIn, i, NodeSignal::Value);
            }
            HVector<HString> namesVector = mNames.split(',');
            HVector<HString> aliasesVector = mAliases.split(',');
            HVector<HString> unitsVector =mUnits.split(',');

            //Check that number of names/aliases/units are the same
            //(only important for Hopsan row-wise CSV, where aliases and units are used)
            if(mNumVars != namesVector.size()) {
                stopSimulation("Number of variable names must equal number of input variables");
                return;
            }
            if(mFileType == HopsanRowWiseCSV && mNumVars != aliasesVector.size()) {
                stopSimulation("Number of variable aliases must equal number of input variables");
                return;
            }
            if(mFileType == HopsanRowWiseCSV && mNumVars != unitsVector.size()) {
                stopSimulation("Number of variable units must equal number of input variables");
                return;
            }

            //Open file for writing
            mFile.open(mFilePath.c_str());
            if (!mFile.is_open()) {
                stopSimulation("Could not open file for writing: "+ mFilePath);
                return;
            }

            //Write header lines/rows depending on file type
            if(mFileType == PlainColumnWiseCSV) {
                //First row should be variable names
                mFile << "Time,";
                for(size_t i=0; i<mNumVars; ++i) {
                    mFile << namesVector[i].c_str();
                    if(i < mNumVars-1) {
                        mFile << ",";
                    }
                }
                mFile << "\n";
            }
            else if(mFileType == PlainRowWiseCSV) {
                //First column should be variable names
                //Create one row for each variable and insert names first
                mRows.resize(mNumVars+1);
                mRows[0].append("Time");
                for(size_t i=0; i<mNumVars; ++i) {
                    mRows[i+1].append(namesVector[i]);
                }
            }
            else if(mFileType == HopsanRowWiseCSV) {
                //First three columns should be variable names, aliases and units
                //Create one row for each variable and insert this first
                mRows.resize(mNumVars+1);
                mRows[0].append("Time,,");
                for(size_t i=0; i<mNumVars; ++i) {
                    mRows[i+1].append(namesVector[i]+",");
                    mRows[i+1].append(aliasesVector[i]+",");
                    mRows[i+1].append(unitsVector[i]);
                }
            }
            if(mFileType == HopsanPLO) {
                //Write custom PLO header rows

                //Extract target file name
                HVector<HString> temp = mFilePath.split('/');
                temp = temp[temp.size()-1].split('\\');
                HString fileName = temp[temp.size()-1];

                //Fetch parent system name (hmf filename is unknown)
                HString modelName = getSystemParent()->getName();

                //Generate date and time string
                time_t rawtime;
                struct tm * timeinfo;
                char timestr[100];
                time (&rawtime);
                timeinfo = localtime(&rawtime);
                std::strftime(timestr,sizeof(timestr),"%a %b %d %H:%M:%S %Y",timeinfo);
                HString dateTime = HString(timestr);

                //Generate HopsanCore version string
                HString version = "HopsanCore "+HString(HOPSANCOREVERSION);

                mRows.push_back("    'VERSION'");
                mRows.push_back("    3");
                mRows.push_back("    '"+fileName+"' '"+modelName+"' '"+dateTime+"' '"+version+"'");
                mRows.push_back("");   //Placeholder for cols and rows (number of rows unknown at this point)
                mRows.push_back("    'Time'");
                for(size_t i=0; i<mNumVars; ++i) {
                    //Variable names
                    mRows[mRows.size()-1].append(",    '"+namesVector[i]+"'");
                }
                mRows.push_back("  Time");
                for(size_t i=0; i<mNumVars; ++i) {
                    //Variable scaling (always 1, for backwards compatibility)
                    mRows[mRows.size()-1].append(" 1");
                }
            }

            //Close and re-open file in append mode
            mFile.close();
            mFile.open(mFilePath.c_str(),std::ios_base::app);
            if (!mFile.is_open()) {
                stopSimulation("Could not open file for writing: "+mFilePath);
                return;
            }

            mLastLogTime = mTime-(*mpDt);
            simulateOneTimestep();
        }



        void simulateOneTimestep()
        {
            if(mTime < mLastLogTime+(*mpDt)) {
                return;
            }
            mLastLogTime = mTime;

            if(mFileType == PlainColumnWiseCSV) {
                mFile << mTime << ",";
                for(size_t i=0; i<mNumVars; ++i) {
                    mFile << (*mpInData[i]);
                    if(i < mNumVars-1) {
                        mFile << ",";
                    }
                }
                mFile << "\n";
            }
            else if(mFileType == PlainRowWiseCSV || mFileType == HopsanRowWiseCSV) {
                mRows[0].append(","+HString(to_hstring(mTime).c_str()));
                for(size_t i=0; i<mNumVars; ++i) {
                    mRows[i+1].append(","+HString(to_hstring((*mpInData[i])).c_str()));
                }
            }
            else if(mFileType == HopsanPLO) {
                std::stringstream ss;
                ss << std::scientific << mTime;
                mRows.push_back("  "+HString(ss.str().c_str()));
                for(size_t i=0; i<mNumVars; ++i) {
                    if ((*mpInData[i]) < 0) {
                        mRows[mRows.size()-1].append(" ");
                    }
                    else {
                        mRows[mRows.size()-1].append("  ");
                    }
                    ss.str("");
                    ss << std::scientific << (*mpInData[i]);
                    mRows[mRows.size()-1].append(HString(ss.str().c_str()));
                }
            }
        }

        void finalize()
        {
            //Write number of rows for PLO file
            if(mFileType == HopsanPLO) {
                mRows[3] = "    "+HString(to_hstring(mNumVars+1).c_str())+ "    "+HString(to_hstring(mRows.size()-1).c_str());
            }

            //Write all rows for row-wise file types
            if(mFileType == PlainRowWiseCSV || mFileType == HopsanRowWiseCSV || mFileType == HopsanPLO) {
                for(size_t i=0; i<mRows.size(); ++i) {
                    mFile << mRows[i].c_str() << "\n";
                }
            }

            mFile.close();
        }
    };
}

#endif // SIGNALSINK_HPP_INCLUDED
