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
//! @file   SignalStopSimulation.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-15
//!
//! @brief Contains a component for stopping a simulation
//!
//$Id$

#ifndef SIGNALSTOPSIMULATION_HPP_INCLUDED
#define SIGNALSTOPSIMULATION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStopSimulation : public ComponentSignal
    {

    private:
        double *mpIn;
        HString mMessage;

    public:
        static Component *Creator()
        {
            return new SignalStopSimulation();
        }

        void configure()
        {
            addInputVariable("in", "Stop simulation if >0.5", "", boolToDouble(false), &mpIn);
            addConstant("message", "Message to show when stopping", "", "", mMessage);
        }


        void initialize()
        {
            // Nothing
        }


        void simulateOneTimestep()
        {
            if(doubleToBool(*mpIn))
            {
                if (mMessage.empty())
                {
                    stopSimulation(HString("Caused by component: ")+getName());
                }
                else
                {
                    stopSimulation(mMessage);
                }
            }
        }
    };
}
#endif // SIGNALSTOPSIMULATION_HPP_INCLUDED
