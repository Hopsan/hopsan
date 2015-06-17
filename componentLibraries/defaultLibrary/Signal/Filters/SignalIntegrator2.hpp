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
//! @file   SignalIntegrator2.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Integrator Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALINTEGRATOR2_HPP_INCLUDED
#define SIGNALINTEGRATOR2_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegrator2 : public ComponentSignal
    {

    private:
        Integrator mIntegrator;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalIntegrator2();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "", "", 0.0, &mpND_out);
        }


        void initialize()
        {
            double startY = (*mpND_out);
            double startU = (*mpND_in);
            mIntegrator.initialize(mTimestep, startU, startY);
        }


        void simulateOneTimestep()
        {
            //Filter equation
           (*mpND_out) = mIntegrator.update((*mpND_in));
        }
    };
}

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


