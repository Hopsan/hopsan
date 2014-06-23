/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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

        int mInDataId, mOutDataId;
        bool mReloadCSV;
        HString mDataCurveFileName;
        CSVParserNG mDataFile;
        LookupTableND<1> mLookupTable;

    public:
        static Component *Creator()
        {
            return new Signal1DLookupTable();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            addOutputVariable("out", "", "", &mpOut);

            addConstant("filename", "Data file (absolute or relativ to model path)", "", "FilePath", mDataCurveFileName);
            addConstant("inid", "csv file index column (0-based index)", "", 0, mInDataId);
            addConstant("outid", "csv file value column (0-based index)", "", 1, mOutDataId);
            addConstant("reload","Reload csv file in initialize", "", true, mReloadCSV);
        }


        void initialize()
        {
            if ( mLookupTable.isEmpty() || mReloadCSV )
            {
                bool isOK=false;
                mLookupTable.clear();

                isOK = mDataFile.setFile(findFilePath(mDataCurveFileName));
                if (isOK)
                {
                    isOK = mDataFile.parseEntireFile();
                }
                if(!isOK)
                {
                    addErrorMessage("Unable to initialize CSV file: "+mDataCurveFileName+", "+mDataFile.getErrorString());
                    stopSimulation();
                }
                else
                {
                    // Make sure that selected data vector is in range
                    const int nDataCols = mDataFile.getNumDataCols();
                    if ( mInDataId >= nDataCols || mOutDataId >= nDataCols )
                    {
                        HString ss;
                        ss = "inid: "+to_hstring(mInDataId)+" or outid:"+to_hstring(mOutDataId)+" is out of range!";
                        addErrorMessage(ss);
                        stopSimulation();
                        return;
                    }

                    mDataFile.copyColumn(mInDataId, mLookupTable.getIndexDataRef());
                    mDataFile.copyColumn(mOutDataId, mLookupTable.getValueDataRef());
                    // Now the data is in the lookuptable and we can throw away the csv data to conservev memory
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
