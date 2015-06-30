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
//! @file   SignalVariableTimeDelay.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-02-22
//!
//! @brief Contains a Signal Time Delay Component
//!
//$Id$

#ifndef SIGNALVARIABLETIMEDELAY_HPP_INCLUDED
#define SIGNALVARIABLETIMEDELAY_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalVariableTimeDelay : public ComponentSignal
    {

    private:
        double *mpTimeDelay;
        double mMaxMemSize;
        Delay *mpDelay;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalVariableTimeDelay();
        }

        void configure()
        {
            mpDelay = 0;

            addInputVariable("dT", "Time delay", "s", 1.0, &mpTimeDelay);
            addConstant("maxMem", "Maximum allowed memory consumption", "MB", 50, mMaxMemSize);

            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "", "", &mpND_out);
        }


        void initialize()
        {
            if (*mpTimeDelay < 0)
            {
                addWarningMessage("Can not have timedelay < 0 s");
//                stopSimulation();
//                return;
            }

            mpDelay = new Delay;
            mpDelay->initialize((*mpTimeDelay), mTimestep, (*mpND_in));
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            // Check if delay have changed, using int truncation and + 0.5  to round to nearest int
            // First make sure timedelay not negative
            const size_t nSamps = int(std::max((*mpTimeDelay),0.0)/mTimestep+0.5);

            if ( nSamps != mpDelay->getSize())
            {
                if (nSamps > size_t((mMaxMemSize*1e6)/double(sizeof(double))) )
                {
                    addErrorMessage("Trying to allocate to much memory with current timestep and requested time delay: "+to_hstring(*mpTimeDelay)+" s!");
                    stopSimulation();
                }

                // Create a new empty delay
                Delay *pNewDelay = new Delay();

                // Check if it should be populated
                if (nSamps != 0)
                {
                    try
                    {
                        pNewDelay->initialize(int(nSamps),0);
                    }
                    catch (int e)
                    {
                        addErrorMessage("Exception nr: "+to_hstring(e)+" occured");
                        stopSimulation();
                    }

                    // Copy old data, depending on longer or shorter delay
                    if (nSamps < mpDelay->getSize())
                    {
                        // Keep the oldest values truncate the newest ones
                        for (size_t i=0; i<nSamps; ++i)
                        {
                            pNewDelay->update(mpDelay->getOldIdx(i));
                        }
                    }
                    else
                    {
                        // Keep all old values, fill with the newest value
                        for (size_t i=0; i<mpDelay->getSize(); ++i)
                        {
                            pNewDelay->update(mpDelay->getOldIdx(i));
                        }
                        for (size_t i=mpDelay->getSize(); i<nSamps; ++i)
                        {
                            if (mpDelay->getSize() > 0)
                            {
                                pNewDelay->update(mpDelay->getNewest());
                            }
                            else
                            {
                                pNewDelay->update(*mpND_in);
                            }
                        }
                    }
                }

                delete mpDelay;
                mpDelay = pNewDelay;
            }


            // If delay is populated, use it , else not
            if (mpDelay->getSize() == 0)
            {
                *mpND_out = *mpND_in;
            }
            else
            {
                (*mpND_out) =  mpDelay->update(*mpND_in);
            }
        }

        void finalize()
        {
            if (mpDelay)
            {
                delete mpDelay;
                mpDelay = 0;
            }
        }
    };
}

#endif
