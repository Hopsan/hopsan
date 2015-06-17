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

//$Id$

#ifndef SIGNALDEADZONE_HPP_INCLUDED
#define SIGNALDEADZONE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDeadZone : public ComponentSignal
    {


    private:
        double *mpStartDead;
        double *mpEndDead;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalDeadZone();
            }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "", "", &mpND_out);

            addInputVariable("u_dstart", "Start of Dead Zone", "", -1.0, &mpStartDead);
            addInputVariable("u_dend", "End of Dead Zone", "", 1.0, &mpEndDead);
        }

        void initialize()
        {
            simulateOneTimestep();
        }

        void simulateOneTimestep()
        {
            //Deadzone equations
            if ( (*mpND_in) < (*mpStartDead))
            {
                (*mpND_out) = (*mpND_in) - (*mpStartDead);
            }
            else if ( (*mpND_in) > (*mpStartDead) && (*mpND_in) < (*mpEndDead))
            {
                (*mpND_out) = 0;
            }
            else
            {
                (*mpND_out) = (*mpND_in) - (*mpEndDead);
            }
        }
    };
}

#endif // SIGNALDEADZONE_HPP_INCLUDED
