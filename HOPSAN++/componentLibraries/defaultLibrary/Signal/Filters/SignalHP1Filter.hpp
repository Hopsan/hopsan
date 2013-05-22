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
//! @file   SignalHP1Filter.hpp
//! @author Robert Braun <karl.pettersson@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a Signal First Order High Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALHP1FILTER_HPP_INCLUDED
#define SIGNALHP1FILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalHP1Filter : public ComponentSignal
    {

    private:
        FirstOrderTransferFunction mTF;
        double mW, mMin, mMax;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalHP1Filter();
        }

        void configure()
        {
            addInputVariable("in","","", 0.0, &mpND_in);
            addOutputVariable("out", "","",0.0, &mpND_out);

            addConstant("omega", "Break frequency", "[rad/s]", 1000.0, mW);
            addConstant("y_min", "Lower output limit", "[-]", -1.5E+300, mMin);
            addConstant("y_max", "Upper output limit", "[-]", 1.5E+300, mMax);
        }


        void initialize()
        {
            double num[2];
            double den[2];

            num[1] = 1.0/mW;
            num[0] = 0.0;
            den[1] = 1.0/mW;
            den[0] = 1.0;

            mTF.initialize(mTimestep, num, den, (*mpND_in), (*mpND_out), mMin, mMax);

            // Do not init output as the startvalue given in that port will initialize the filter output
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = mTF.update((*mpND_in));
        }
    };
}

#endif // SIGNALHP1FILTER_HPP_INCLUDED


