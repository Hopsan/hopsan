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
//! @file   Signal1DPLOLookUpTable.hpp
//! @author Peter Nordin <peter.nordin@liue.se>
//! @date   2014-10-03
//!
//! @brief Contains a one dimensional lookup table from PLO files
//!
//$Id$

#ifndef SIGNAL1DPLOLOOKUPTABLE_HPP_INCLUDED
#define SIGNAL1DPLOLOOKUPTABLE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HopsanTypes.h"

namespace hopsan {

    //!
    //! @brief 1d Lookup based on Hopsan classic plo format
    //!
    //! @ingroup SignalComponents
    //!
    class Signal1DPLOLookupTable : public ComponentSignal
    {

    private:
        double *mpIn, *mpOut;

        HString mInDataName, mOutDataName;
        bool mReloadPLO;
        bool mUseTextInput;
        HFilePath mPloFileName;
        HTextBlock mTextInput;
        PLOParser mPLOParser;
        LookupTable1D mLookupTable;

    public:
        static Component *Creator()
        {
            return new Signal1DPLOLookupTable();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            addOutputVariable("out", "", "", &mpOut);

            addConstant("filename", "Data file (absolute or relative to model path)", mPloFileName);
            addConstant("text", "Text input (instead of file)", mTextInput);
            addConstant("invar", "Name of input variable (in file)", "", "Time", mInDataName);
            addConstant("outvar", "Name of output variable (in file)", "", "Data", mOutDataName);
            addConstant("reload","Reload plo file in initialize", "", true, mReloadPLO);
        }


        void initialize()
        {
            mUseTextInput = !mTextInput.empty();

            if ( mLookupTable.isEmpty() || mReloadPLO )
            {
                bool isOK=false;
                mLookupTable.clear();

                if (mUseTextInput) {
                    isOK = mPLOParser.readText(mTextInput);
                }
                else {
                    isOK = mPLOParser.readFile(findFilePath(mPloFileName));
                }

                if(!isOK)
                {
                    HString msg = mUseTextInput ? "Unable to initialize PLO parser: "+mPLOParser.getErrorString() :
                                                  "Unable to initialize PLO file: "+mPloFileName+", "+mPLOParser.getErrorString();
                    addErrorMessage(msg);
                    stopSimulation();
                    return;
                }
                else
                {
                    // Make sure that selected data vectors are in range
                    int inId, outId;
                    inId = mPLOParser.getColIdxForDataName(mInDataName);
                    outId = mPLOParser.getColIdxForDataName(mOutDataName);

                    if ( inId < 0 || outId < 0 )
                    {
                        HString ss;
                        ss = "invar: "+mInDataName+" or outvar: "+mOutDataName+" does not exist in specified file!";
                        addErrorMessage(ss);
                        stopSimulation();
                        return;
                    }

                    mPLOParser.copyColumn(inId, mLookupTable.getIndexDataRef());
                    mPLOParser.copyColumn(outId, mLookupTable.getValueDataRef());
                    // Now the data is in the lookuptable and we can throw away the plo data to conserve memory
                    mPLOParser.clearData();

                    // Make sure strictly increasing (no sorting will be done if that is already the case)
                    mLookupTable.sortIncreasing();

                    // Check if data is OK before we continue
                    isOK = mLookupTable.isDataOK();
                    if(!isOK)
                    {
                        HString msg = "The LookupTable data is not OK";
                        if (!mUseTextInput) {
                            msg.append(" after reading from file: "+mPloFileName);
                        }
                        addErrorMessage(msg);
                        if (!mLookupTable.isDataSizeOK())
                        {
                            addErrorMessage("Something is wrong with the size of the index or data vectors");
                        }
                        if (!mLookupTable.allIndexStrictlyIncreasing())
                        {
                            addErrorMessage("Even after sorting, the index column is still not strictly increasing");
                        }
                        stopSimulation();
                    }
                }
            }
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mLookupTable.interpolate(*mpIn);
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
