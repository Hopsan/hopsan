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
//! @file   SignalUndefinedConnection.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-05
//!
//! @brief Contains a Signal Undefined Connection Component (for Real-Time targets)
//!
//$Id$

#ifndef SIGNALUNDEFINEDCONNECTION_HPP_INCLUDED
#define SIGNALUNDEFINEDCONNECTION_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalUndefinedConnection : public ComponentSignal
    {

    private:
        Port *mpOut;
        double X;
        double *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalUndefinedConnection();
        }

        void configure()
        {
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("X",  "Value", "[-]",  X);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            (*mpND_out) = X;
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = X;
        }
    };
}

#endif // SIGNALUNDEFINEDCONNECTION_HPP_INCLUDED
