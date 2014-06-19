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
//! @date   2010-11-03
//!
//! @brief Contains a one dimensional lookup table
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

        bool mReloadCSV;
        HString mDataCurveFileName;
        CSVParserNG mDataFile;
        LookupTableND<2> mLookupTable;

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

            addConstant("filename", "Data file (absolute or relativ to model path)", "", "FilePath", mDataCurveFileName);
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
                    if ( nDataCols != 3 )
                    {
                        addErrorMessage(HString("Wrong number of data columns: ")+to_hstring(nDataCols)+" != 3");
                        stopSimulation();
                        return;
                    }

                    std::vector<long int> rowscols;
                    isOK = mDataFile.copyRow(mDataFile.getNumDataRows()-1,rowscols);
                    if (!isOK)
                    {
                        addErrorMessage("Could not parse the number of rows and columns (last line) from CSV file: "+mDataCurveFileName);
                        stopSimulation();
                        return;
                    }

                    size_t nRows = rowscols[0];
                    size_t nCols = rowscols[1];

                    // Copy row and column index vectors (ignoring the final row with nRows and nCols)
                    mDataFile.copyEveryNthFromColumn(0, nCols, mLookupTable.getIndexDataRef(0));
                    mDataFile.copyRangeFromColumn(1, 0, nCols, mLookupTable.getIndexDataRef(1));

                    // Remove "extra element (num rows)" from row index column
                    if (mLookupTable.getDimSize(0) == nRows+1)
                    {
                        mLookupTable.getIndexDataRef(0).pop_back();
                    }

                    // Copy values
                    mDataFile.copyRangeFromColumn(2, 0, mDataFile.getNumDataRows()-1, mLookupTable.getValueDataRef());

                    // Make sure the correct number of rows and columns are available
                    if ( (nRows != mLookupTable.getDimSize(0)) || (nCols != mLookupTable.getDimSize(1)) )
                    {
                        addErrorMessage(HString("Wrong number of rows: "+to_hstring(mLookupTable.getDimSize(0))+
                                                " "+to_hstring(mLookupTable.getDimSize(1))+
                                                " Should have been: "+to_hstring(nRows)+
                                                " "+to_hstring(nCols)));
                        stopSimulation();
                        return;
                    }

                    // Now the data is in the lookuptable and we can throw away the csv data to conserve memory
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

        bool isExperimental() const
        {
            return true;
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
