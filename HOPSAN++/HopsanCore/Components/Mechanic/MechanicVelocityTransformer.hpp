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

//$Id$

#ifndef MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicVelocityTransformer : public ComponentQ
    {

    private:
        double v;
        double signal, f, x, c, Zx;
        double *mpND_signal, *mpND_f, *mpND_x, *mpND_v, *mpND_c, *mpND_Zx;
        Integrator mInt;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicVelocityTransformer("VelocityTransformer");
        }

        MechanicVelocityTransformer(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            v = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addPowerPort("out", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("v", "Generated velocity", "[m/s]", v);
        }


        void initialize()
        {
            mpND_signal  = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, v);

            mpND_f = getSafeNodeDataPtr(mpOut, NodeMechanic::FORCE);
            mpND_x = getSafeNodeDataPtr(mpOut, NodeMechanic::POSITION);
            mpND_v = getSafeNodeDataPtr(mpOut, NodeMechanic::VELOCITY);
            mpND_c = getSafeNodeDataPtr(mpOut, NodeMechanic::WAVEVARIABLE);
            mpND_Zx = getSafeNodeDataPtr(mpOut, NodeMechanic::CHARIMP);

            mInt.initialize(mTimestep, (*mpND_signal), 0.0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            signal = (*mpND_signal);
            c = (*mpND_c);
            Zx = (*mpND_Zx);

            //Spring equations
            x = mInt.update(signal);
            f = c + Zx*signal;

            //Write values to nodes
            (*mpND_f) = f;
            (*mpND_x) = x;
            (*mpND_v) = signal;
        }
    };
}

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
