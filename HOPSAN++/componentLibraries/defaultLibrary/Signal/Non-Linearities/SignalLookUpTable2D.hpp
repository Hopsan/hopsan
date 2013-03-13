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
#include <algorithm>
#include <sstream>

namespace hopsan {

    //!
    //! @brief The data csv file should be  on , separated form or ; separated form
    //!
    //! @ingroup SignalComponents
    //!
    class SignalLookUpTable2D : public ComponentSignal
    {

    private:
        Port *mpIn, *mpOut;
        double *mpND_in, *mpND_out;

        int mOutDataId;
        std::string mDataCurveFileName;
        CSVParser *myDataCurve;

    public:
        static Component *Creator()
        {
            return new SignalLookUpTable2D();
        }

        void configure()
        {
            mpIn = addReadPort("in", "NodeSignal", Port::NotRequired);
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            mOutDataId=1;
            mDataCurveFileName = "FilePath";
            registerParameter("filename", "Data file (abs. path or located at model path)", "", mDataCurveFileName);
            registerParameter("outid", "csv file value column index", "", mOutDataId);

            myDataCurve = 0;
        }


        void initialize()
        {
            bool success=false;
            if (myDataCurve!=0)
            {
                delete myDataCurve;
                myDataCurve=0;
            }

            myDataCurve = new CSVParser(success, findFilePath(mDataCurveFileName));
            if(!success)
            {
                std::stringstream ss;
                ss << "Unable to initialize CSV file: " << mDataCurveFileName << ", " << myDataCurve->getErrorString();
                addErrorMessage(ss.str());
                stopSimulation();
            }
            else
            {
                // Make sure that selected data vector is in range
                if (mOutDataId >= int(myDataCurve->getNumDataCols()))
                {
                    std::stringstream ss;
                    ss << "outid:" << mOutDataId << " is out of range, limiting to: ";
                    mOutDataId = int(myDataCurve->getNumDataCols())-1;
                    ss << mOutDataId;
                    addWarningMessage(ss.str());
                }


                if (myDataCurve->getIncreasingOrDecresing(0) != 1)
                {
                    myDataCurve->sortIncreasing(0);
                    myDataCurve->calcIncreasingOrDecreasing();
                }

                success = (myDataCurve->getIncreasingOrDecresing(0) == 1);
                if(!success)
                {
                    std::stringstream ss;
                    ss << "Unable to initialize CSV file: " << mDataCurveFileName << ", " << "Even after sorting, index column is still not strictly increasing";
                    addErrorMessage(ss.str());
                    stopSimulation();
                }
            }

            if(success)
            {
                //                std::stringstream ss;
                //                ss << mGain << "  " << myDataCurve->interpolate(mGain);
                //                addInfoMessage(ss.str());

                mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
                mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
            }
        }


        void simulateOneTimestep()
        {
//            (*mpND_out) = myDataCurve->interpolate_old(*mpND_in, 1);
//            (*mpND_out) = myDataCurve->interpolate(*mpND_in, 1);
//            (*mpND_out) = myDataCurve->interpolateInc(*mpND_in, 1);
            (*mpND_out) = myDataCurve->interpolate(*mpND_in, mOutDataId);
        }

        void finalize()
        {
            //! @todo actually this is only needed in destructor to cleanup
            //Cleanup data curve
            if (myDataCurve!=0)
            {
                delete myDataCurve;
                myDataCurve=0;
            }
        }
    };
}

#endif // SIGNALLOOKUPTABLE2D_HPP_INCLUDED
