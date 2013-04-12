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
//! @file   SignalLP2Filter.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2010-06-10
//!
//! @brief Contains a Signal Second Order Low Pass Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALLP2FILTER_HPP_INCLUDED
#define SIGNALLP2FILTER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalLP2Filter : public ComponentSignal
    {

    private:
        SecondOrderTransferFunction mTF2;
        double mW, mD, mMin, mMax;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalLP2Filter();
        }

        void configure()
        {

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mW=1000.0;
            mD=1.0;

            addInputVariable("in","","", 0.0, &mpND_in);
            addOutputVariable("out", "","",0.0, &mpND_out);

            registerParameter("omega", "Break frequency", "[rad/s]", mW, Constant);
            registerParameter("delta", "Damp coefficient", "[-]", mD, Constant);
            registerParameter("y_min", "Lower output limit", "[-]", mMin, Constant);
            registerParameter("y_max", "Upper output limit", "[-]", mMax, Constant);
        }


        void initialize()
        {
            double num[3];
            double den[3];

            num[2] = 0.0;
            num[1] = 0.0;
            num[0] = 1.0;
            den[2] = 1.0/(mW*mW);
            den[1] = 2.0*mD/mW;
            den[0] = 1.0;

            mTF2.initialize(mTimestep, num, den, (*mpND_in), (*mpND_out), mMin, mMax);

            //Writes out the value for time "zero"
            //(*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            //Write new values to nodes
            (*mpND_out) = mTF2.update((*mpND_in));
        }
    };
}

#endif // SIGNALLP2FILTER_HPP_INCLUDED


