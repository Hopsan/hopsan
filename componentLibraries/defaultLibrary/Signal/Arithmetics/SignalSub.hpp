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
//! @file   SignalSub.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2012-02-29
//!
//! @brief Contains a mathematical multiport subtractor component
//!
//$Id$

#ifndef SIGNALSUB_HPP_INCLUDED
#define SIGNALSUB_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSub : public ComponentSignal
    {

    private:
        size_t nSumInputs, nSubInputs;
        std::vector<double *> mNDp_in_sum_vec, mNDp_in_sub_vec;
        double *mpND_out;
        Port *mpMultiInSumPort, *mpMultiInSubPort;

    public:
        static Component *Creator()
        {
            return new SignalSub();
        }

        void configure()
        {
            mpMultiInSumPort = addReadMultiPort("insum", "NodeSignal", "", Port::NotRequired);
            mpMultiInSubPort = addReadMultiPort("insub", "NodeSignal", "", Port::NotRequired);
            addOutputVariable("out", "sum(insum)-sum(insub)", "", &mpND_out);
        }


        void initialize()
        {
            //We need at least one dummy port even if no port is connected
            nSumInputs = std::max(mpMultiInSumPort->getNumPorts(), size_t(1));
            nSubInputs = std::max(mpMultiInSubPort->getNumPorts(), size_t(1));

            mNDp_in_sum_vec.resize(nSumInputs);
            for (size_t i=0; i<nSumInputs; ++i)
            {
                mNDp_in_sum_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInSumPort, i, NodeSignal::Value);
            }
            mNDp_in_sub_vec.resize(nSubInputs);
            for (size_t i=0; i<nSubInputs; ++i)
            {
                mNDp_in_sub_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInSubPort, i, NodeSignal::Value);
            }
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            double sum = 0;
            for (size_t i=0; i<nSumInputs; ++i)
            {
                sum += *mNDp_in_sum_vec[i];
            }
            for (size_t i=0; i<nSubInputs; ++i)
            {
                sum -= *mNDp_in_sub_vec[i];
            }
            (*mpND_out) = sum; //Write value to output node
        }
    };
}
#endif
