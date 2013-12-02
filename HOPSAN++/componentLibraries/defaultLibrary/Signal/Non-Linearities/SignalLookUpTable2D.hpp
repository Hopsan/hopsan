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
        double *mpND_in, *mpND_out;

        int mOutDataId;
        bool mReloadCSV;
        HString mDataCurveFileName;
        CSVParser *mpDataCurve;

        void deleteDataCurve()
        {
            if (mpDataCurve!=0)
            {
                delete mpDataCurve;
                mpDataCurve=0;
            }
        }

    public:
        static Component *Creator()
        {
            return new SignalLookUpTable2D();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "", "", &mpND_out);

            addConstant("filename", "Data file (absolute or relativ model path)", "", "FilePath", mDataCurveFileName);
            addConstant("outid", "csv file value column index", "", 1, mOutDataId);
            addConstant("reload","Reload csv file in initialize", "", true, mReloadCSV);

            mpDataCurve = 0;
        }


        void initialize()
        {
            if ( (mpDataCurve==0) || mReloadCSV )
            {
                bool success=false;
                deleteDataCurve();

                mpDataCurve = new CSVParser(success, findFilePath(mDataCurveFileName));
                if(!success)
                {
                    addErrorMessage("Unable to initialize CSV file: "+mDataCurveFileName+", "+mpDataCurve->getErrorString());
                    stopSimulation();
                }
                else
                {
                    // Make sure that selected data vector is in range
                    if (mOutDataId >= int(mpDataCurve->getNumDataCols()))
                    {
                        HString ss;
                        ss = "outid:" + to_hstring(mOutDataId) + " is out of range, limiting to: ";
                        mOutDataId = int(mpDataCurve->getNumDataCols())-1;
                        ss += to_hstring(mOutDataId);
                        addWarningMessage(ss);
                    }


                    if (mpDataCurve->getIncreasingOrDecresing(0) != 1)
                    {
                        mpDataCurve->sortIncreasing(0);
                        mpDataCurve->calcIncreasingOrDecreasing();
                    }

                    success = (mpDataCurve->getIncreasingOrDecresing(0) == 1);
                    if(!success)
                    {
                        addErrorMessage("Unable to initialize CSV file: "+mDataCurveFileName+", "+"Even after sorting, index column is still not strictly increasing");
                        stopSimulation();
                    }
                }
            }
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mpDataCurve->interpolate(*mpND_in, mOutDataId);
        }

        void finalize()
        {
            // Cleanup data curve
            if (mReloadCSV)
            {
                deleteDataCurve();
            }
        }

        void deconfigure()
        {
            // If data still remains (due to not reloading) then delete it
            deleteDataCurve();
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
