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
//! @file   SignalAdd.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical summator component
//!
//$Id$

#ifndef SIGNALADD_HPP_INCLUDED
#define SIGNALADD_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAdd : public ComponentSignal
    {

    private:
        double *mpND_in1, *mpND_in2, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalAdd();
        }

        void configure()
        {
            addInputVariable("in1", "", "", 0, &mpND_in1);
            addInputVariable("in2", "", "", 0, &mpND_in2);
            addOutputVariable("out", "The sum of the inputs", "", &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_in1) + (*mpND_in2);
        }
    };
}
#endif // SIGNALADD_HPP_INCLUDED
