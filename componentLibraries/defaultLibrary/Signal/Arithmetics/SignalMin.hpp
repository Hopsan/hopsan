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
//! @file   SignalMin.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-21
//!
//! @brief Contains a maximum component
//!
//$Id$

#ifndef SIGNALMIN_HPP_INCLUDED
#define SIGNALMIN_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMin : public ComponentSignal
    {

    private:
        size_t nInputs;
        std::vector<double *> mNDp_in_vec;
        double *mpND_out;
        Port *mpMultiInPort;

    public:
        static Component *Creator()
        {
            return new SignalMin();
        }

        void configure()
        {
            mpMultiInPort = addReadMultiPort("in", "NodeSignal", "", Port::NotRequired);
            addOutputVariable("out", "Min of the inputs", "", &mpND_out);
        }


        void initialize()
        {
            // We need at least one dummy port even if no port is connected
            nInputs = std::max(mpMultiInPort->getNumPorts(),size_t(1));

            mNDp_in_vec.resize(nInputs);
            for (size_t i=0; i<nInputs; ++i)
            {
                mNDp_in_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInPort, i, NodeSignal::Value);
            }

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            double min = (*mNDp_in_vec[0]);
            for (size_t i=1; i<nInputs; ++i)
            {
                if(*mNDp_in_vec[i]<min)
                {
                    min = *mNDp_in_vec[i];
                }
            }
            (*mpND_out) = min; //Write value to output node
        }
    };
}
#endif
