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

            addConstant("filename", "Data file (absolute or relative model path)", "", "FilePath", mDataCurveFileName);
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

        bool isObsolete() const
        {
            return true;
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
