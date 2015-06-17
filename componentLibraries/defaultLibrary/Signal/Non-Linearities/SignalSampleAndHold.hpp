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
//! @file   SignalSampleAndHold.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-12-18
//!
//! @brief Contains a signal sample-and-hold component
//!
//$Id$
#ifndef SIGNALSAMPLEANDHOLD_HPP_INCLUDED
#define SIGNALSAMPLEANDHOLD_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

class SignalSampleAndHold : public ComponentSignal
{
private:
    double *mpFs;
    double *mpIn, *mpOut;
    double mNextT, mTs;

public:
    static Component *Creator()
    {
        return new SignalSampleAndHold();
    }

    void configure()
    {
        addInputVariable("f_s", "Sampling Frequency", "Hz", 100, &mpFs);
        addInputVariable("in", "", "", 0, &mpIn);
        addOutputVariable("out", "", "", &mpOut);
    }

    void initialize()
    {
        (*mpOut) = (*mpIn);
        mNextT = mTime;
        mTs = 1.0/(*mpFs);
    }

    void simulateOneTimestep()
    {
        if(mTime >= mNextT)
        {
            (*mpOut) = (*mpIn);
            mNextT += mTs;
        }
    }
};
}

#endif // SIGNALSAMPLEANDHOLD_HPP_INCLUDED
