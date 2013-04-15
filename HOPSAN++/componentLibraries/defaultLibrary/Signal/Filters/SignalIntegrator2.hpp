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


