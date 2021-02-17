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
//! @file   Signal1DLookUpTable.hpp
//! @author Peter Nordin <peter.nordin@liue.se>
//! @date   2014-06-19
//!
//! @brief Contains a one dimensional lookup table
//!
//$Id$

#ifndef SIGNAL1DLOOKUPTABLE_HPP_INCLUDED
#define SIGNAL1DLOOKUPTABLE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HopsanTypes.h"

namespace hopsan {

    //!
    //! @brief The data csv file should be  on , separated form or ; separated form
    //!
    //! @ingroup SignalComponents
    //!
    class Signal1DLookupTable : public ComponentSignal
    {

    private:
        double *mpIn, *mpOut;

        int mInDataId, mOutDataId, mNumLinesToSkip;
        bool mReloadCSV;
        bool mUseTextInput;
        HFilePath mFileName;
        HTextBlock mTextInput;
        HString mSeparatorChar;
        HString mCommentChar;
        CSVParserNG mCSVParser;
        LookupTable1D mLookupTable;

    public:
        static Component *Creator()
        {
            return new Signal1DLookupTable();
        }

        void configure()
        {
            mUseTextInput = false;

            addInputVariable("in", "", "", 0.0, &mpIn);
            addOutputVariable("out", "", "", &mpOut);

            addConstant("filename", "Data file (absolute or relative to model path)", mFileName);
            addConstant("text", "Text input (instead of file)", mTextInput);
            addConstant("csvsep", "csv separator character", "", HString(","), mSeparatorChar);
            addConstant("inid", "csv file index column (0-based index)", "", 0, mInDataId);
            addConstant("outid", "csv file value column (0-based index)", "", 1, mOutDataId);
            addConstant("numlineskip", "The number of lines to skip (from the top)", "", 0, mNumLinesToSkip);
            addConstant("comment", "Skip initial lines starting with character", "", "", mCommentChar);
            addConstant("reload","Reload csv file in initialize", "", true, mReloadCSV);
        }


        void initialize()
        {
            mUseTextInput = !mTextInput.empty();

            if ( mLookupTable.isEmpty() || mReloadCSV )
            {
                bool isOK=false;
                mLookupTable.clear();

                if (mUseTextInput) {
                    isOK = mCSVParser.openText(mTextInput);
                }
                else {
                    isOK = mCSVParser.openFile(findFilePath(mFileName));
                }

                if (isOK)
                {
                    mNumLinesToSkip = std::max(mNumLinesToSkip, 0);
                    if (!mCommentChar.empty()) {
                        if (mCommentChar.size()>1) {
                            addErrorMessage("Comment character must be one character");
                            isOK = false;
                        }
                        else {
                            mCSVParser.setCommentChar(mCommentChar[0]);
                        }
                    }

                    if(isOK) {
                        if (mSeparatorChar.size() == 1) {
                            mCSVParser.setLinesToSkip(mNumLinesToSkip);
                            mCSVParser.setFieldSeparator(mSeparatorChar[0]);
                            mCSVParser.indexFile();
                            isOK = true;
                        }
                        else {
                            addErrorMessage("Separator character must be ONE character");
                            isOK = false;
                        }
                    }
                }
                if(!isOK)
                {
                    HString msg = mUseTextInput ? "Unable to initialize CSV parser: "+mCSVParser.getErrorString() :
                                                  "Unable to initialize CSV file: "+mFileName+", "+mCSVParser.getErrorString();
                    addErrorMessage(msg);
                    stopSimulation();
                    mCSVParser.closeFile();
                    return;
                }
                else
                {
                    // Make sure that selected data vector is in range
                    size_t minCols, maxCols;
                    mCSVParser.getMinMaxNumCols(minCols, maxCols);
                    if ( mInDataId >= int(maxCols) || mOutDataId >= int(maxCols) )
                    {
                        HString ss;
                        ss = "inid: "+to_hstring(mInDataId)+" or outid:"+to_hstring(mOutDataId)+" is out of range!";
                        addErrorMessage(ss);
                        stopSimulation();
                        mCSVParser.closeFile();
                        return;
                    }

                    isOK = mCSVParser.copyColumn(mInDataId, mLookupTable.getIndexDataRef());
                    isOK = isOK && mCSVParser.copyColumn(mOutDataId, mLookupTable.getValueDataRef());
                    // Now the data is in the lookuptable and we can close the csv file and clear the index
                    mCSVParser.closeFile();

                    if (!isOK)
                    {
                        addErrorMessage("There were parsing errors in either the input or output data columns");
                        stopSimulation();
                        return;
                    }

                    // Make sure strictly increasing (no sorting will be done if that is already the case)
                    mLookupTable.sortIncreasing();

                    // Check if data is OK before we continue
                    isOK = mLookupTable.isDataOK();
                    if(!isOK)
                    {
                        HString msg = "The LookupTable data is not OK";
                        if (!mUseTextInput) {
                            msg.append(" after reading from file: "+mFileName);
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
