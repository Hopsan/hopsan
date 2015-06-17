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
//! @file   SignalConstant.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Constant Component
//!
//$Id$

#ifndef SIGNALCONSTANT_HPP_INCLUDED
#define SIGNALCONSTANT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalConstant : public ComponentSignal
    {

    private:
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalConstant();
        }

        void configure()
        {
            addOutputVariable("y", "Constant value", "", 1.0, &mpOut);
        }


        void initialize()
        {
            // Nothing to do
        }

        void simulateOneTimestep()
        {
            //Nothing to do (only one write port can exist in the node, so no one else shall write to the value)
        }
    };
}

#endif // SIGNALCONSTANT_HPP_INCLUDED
