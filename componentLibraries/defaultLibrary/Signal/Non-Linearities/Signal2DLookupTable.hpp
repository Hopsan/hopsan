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
//! @file   Signal2DLookUpTable.hpp
//! @author Peter Nordin <peter.nordin@liue.se>
//! @date   2014-06-19
//!
//! @brief Contains a two dimensional lookup table
//!
//$Id$

#ifndef SIGNAL2DLOOKUPTABLE_HPP_INCLUDED
#define SIGNAL2DLOOKUPTABLE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HopsanTypes.h"

namespace hopsan {

    //!
    //! @brief The data csv file should be  on , separated form or ; separated form
    //!
    //! @ingroup SignalComponents
    //!
    class Signal2DLookupTable : public ComponentSignal
    {

    private:
        double *mpInRow, *mpInCol, *mpOut;

        int mNumLinesToSkip;
        bool mReloadCSV;
        bool mUseTextInput;
        HFilePath mFileName;
        HString mCommentChar;
        HTextBlock mTextInput;
        CSVParserNG mCSVParser;
        LookupTable2D mLookupTable;

    public:
        static Component *Creator()
        {
            return new Signal2DLookupTable();
        }

        void configure()
        {
            addInputVariable("row", "", "", 0.0, &mpInRow);
            addInputVariable("col", "", "", 0.0, &mpInCol);
            addOutputVariable("out", "", "", &mpOut);

            addConstant("filename", "Data file (absolute or relative to model path)", mFileName);
            addConstant("text", "Text input (instead of file)", mTextInput);
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

                if (isOK)
                {
                    mCSVParser.indexFile();
                    isOK = true;
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
                    const size_t nDataCols = mCSVParser.getNumDataCols();
                    if ( !mCSVParser.allRowsHaveSameNumCols() || nDataCols != 3 )
                    {
                        addErrorMessage(HString("Wrong number of data columns: ")+to_hstring(nDataCols)+" != 3");
                        stopSimulation();
                        mCSVParser.closeFile();
                        return;
                    }

                    std::vector<long int> rowscols;
                    isOK = mCSVParser.copyRow(mCSVParser.getNumDataRows()-1,rowscols);
                    if (!isOK)
                    {
                        HString msg = "Could not parse the number of rows and columns (last line)";
                        if (!mUseTextInput) {
                            msg.append(" from CSV file: "+mFileName);
                        }
                        addErrorMessage(msg);
                        stopSimulation();
                        return;
                    }

                    size_t nRows = rowscols[0];
                    size_t nCols = rowscols[1];

                    // Copy row and column index vectors (ignoring the final row with nRows and nCols)
                    isOK = mCSVParser.copyEveryNthFromColumn(0, nCols, mLookupTable.getIndexDataRef(0));
                    isOK = isOK && mCSVParser.copyRangeFromColumn(1, 0, nCols, mLookupTable.getIndexDataRef(1));

                    if (!isOK)
                    {
                        addErrorMessage("Could not parse one or both of the csv index columns");
                        stopSimulation();
                        mCSVParser.closeFile();
                        return;
                    }

                    // Remove "extra element (num rows)" from row index column, cols not needed since we did not even fetch all values
                    if (mLookupTable.getDimSize(0) == nRows+1)
                    {
                        mLookupTable.getIndexDataRef(0).pop_back();
                    }

                    // Copy values
                    isOK = mCSVParser.copyRangeFromColumn(2, 0, mCSVParser.getNumDataRows()-1, mLookupTable.getValueDataRef());
                    if (!isOK)
                    {
                        addErrorMessage("Could not parse the csv value column");
                        stopSimulation();
                        mCSVParser.closeFile();
                        return;
                    }

                    // Now the data is in the lookup table and we can throw away the csv data to conserve memory
                    mCSVParser.closeFile();

                    // Make sure the correct number of rows and columns are available
                    if ( (nRows != mLookupTable.getDimSize(0)) || (nCols != mLookupTable.getDimSize(1)) )
                    {
                        addErrorMessage(HString("The actual number of extracted rows: "+to_hstring(mLookupTable.getDimSize(0))+
                                                " and cols: "+to_hstring(mLookupTable.getDimSize(1))+
                                                ", Does not match the specification (last line): "+to_hstring(nRows)+
                                                " "+to_hstring(nCols)));
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
                            addErrorMessage("Even after sorting, one or more index columns are still not strictly increasing");
                        }
                        stopSimulation();
                    }
                }
            }
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpOut) = mLookupTable.interpolate(*mpInRow, *mpInCol);
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
