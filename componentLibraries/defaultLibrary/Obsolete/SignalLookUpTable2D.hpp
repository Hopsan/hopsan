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
//! @file   SignalLookUpTable2D.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-11-03
//!
//! @brief Contains a Look Up Table 2D
//!
//$Id$

#ifndef SIGNALLOOKUPTABLE2D_HPP_INCLUDED
#define SIGNALLOOKUPTABLE2D_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HopsanTypes.h"

namespace hopsan {

    //!
    //! @brief The data csv file should be  on , separated form or ; separated form
    //!
    //! @ingroup SignalComponents
    //!
    class SignalLookUpTable2D : public ComponentSignal
    {

    private:
        double *mpIn, *mpOut;

        int mInDataId, mOutDataId;
        bool mReloadCSV;
        HString mDataCurveFileName;
        HString mSeparatorChar;
        CSVParserNG mDataFile;
        LookupTable1D mLookupTable;

    public:
        static Component *Creator()
        {
            return new SignalLookUpTable2D();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            addOutputVariable("out", "", "", &mpOut);

            addConstant("filename", "Data file (absolute or relative model path)", "", "FilePath", mDataCurveFileName);
            addConstant("csvsep", "csv separator character", "", HString(","), mSeparatorChar);
            addConstant("outid", "csv file value column index", "", 1, mOutDataId);
            addConstant("reload","Reload csv file in initialize", "", true, mReloadCSV);

            mInDataId = 0;
        }


        void initialize()
        {
            if ( mLookupTable.isEmpty() || mReloadCSV )
            {
                bool isOK=false;
                mLookupTable.clear();

                isOK = mDataFile.openFile(findFilePath(mDataCurveFileName));
                if (isOK)
                {
                    if (mSeparatorChar.size() == 1)
                    {
                        mDataFile.setFieldSeparator(mSeparatorChar[0]);
                        mDataFile.indexFile();
                        isOK = true;
                    }
                    else
                    {
                        addErrorMessage("Separator character must be ONE character");
                        isOK = false;
                    }
                }
                if(!isOK)
                {
                    addErrorMessage("Unable to initialize CSV file: "+mDataCurveFileName+", "+mDataFile.getErrorString());
                    stopSimulation();
                    mDataFile.closeFile();
                    return;
                }
                else
                {
                    // Make sure that selected data vector is in range
                    size_t minCols, maxCols;
                    mDataFile.getMinMaxNumCols(minCols, maxCols);
                    if ( mInDataId >= int(maxCols) || mOutDataId >= int(maxCols) )
                    {
                        HString ss;
                        ss = "inid: "+to_hstring(mInDataId)+" or outid:"+to_hstring(mOutDataId)+" is out of range!";
                        addErrorMessage(ss);
                        stopSimulation();
                        mDataFile.closeFile();
                        return;
                    }

                    isOK = mDataFile.copyColumn(mInDataId, mLookupTable.getIndexDataRef());
                    isOK = isOK && mDataFile.copyColumn(mOutDataId, mLookupTable.getValueDataRef());
                    // Now the data is in the lookuptable and we can close the csv file and clear the index
                    mDataFile.closeFile();

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
                        addErrorMessage("The LookupTable data is not OK after reading from file: "+mDataCurveFileName);
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

        bool isObsolete() const
        {
            return true;
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
