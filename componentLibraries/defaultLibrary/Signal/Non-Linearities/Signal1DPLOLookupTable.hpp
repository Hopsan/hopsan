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
        HString mDataCurveFileName;
        PLOParser mDataFile;
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

            addConstant("filename", "Data file (absolute or relative to model path)", "", "FilePath", mDataCurveFileName);
            addConstant("invar", "Name of input variable (in file)", "", "Time", mInDataName);
            addConstant("outvar", "Name of output variable (in file)", "", "Data", mOutDataName);
            addConstant("reload","Reload plo file in initialize", "", true, mReloadPLO);
        }


        void initialize()
        {
            if ( mLookupTable.isEmpty() || mReloadPLO )
            {
                bool isOK=false;
                mLookupTable.clear();

                isOK = mDataFile.readFile(findFilePath(mDataCurveFileName));
                if(!isOK)
                {
                    addErrorMessage("Unable to initialize PLO file: "+mDataCurveFileName+", "+mDataFile.getErrorString());
                    stopSimulation();
                    return;
                }
                else
                {
                    // Make sure that selected data vectors are in range
                    int inId, outId;
                    inId = mDataFile.getColIdxForDataName(mInDataName);
                    outId = mDataFile.getColIdxForDataName(mOutDataName);

                    if ( inId < 0 || outId < 0 )
                    {
                        HString ss;
                        ss = "invar: "+mInDataName+" or outvar: "+mOutDataName+" does not exist in specified file!";
                        addErrorMessage(ss);
                        stopSimulation();
                        return;
                    }

                    mDataFile.copyColumn(inId, mLookupTable.getIndexDataRef());
                    mDataFile.copyColumn(outId, mLookupTable.getValueDataRef());
                    // Now the data is in the lookuptable and we can throw away the plo data to conserve memory
                    mDataFile.clearData();

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

        bool isExperimental() const
        {
            return true;
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
