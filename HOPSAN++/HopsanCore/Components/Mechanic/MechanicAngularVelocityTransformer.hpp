//!
//! @file   MechanicAngularVelocityTransformer.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains an angular velocity transformer component
//!
//$Id$

#ifndef MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngularVelocityTransformer : public ComponentQ
    {

    private:
        double w;
        double signal, t, a, c, Zx;
        double *mpND_signal, *mpND_t, *mpND_a, *mpND_w, *mpND_c, *mpND_Zx;
        Integrator mInt;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicAngularVelocityTransformer("AngularVelocityTransformer");
        }

        MechanicAngularVelocityTransformer(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            w = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addPowerPort("out", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("w", "Generated angular velocity", "[rad/s]", w);
        }


        void initialize()
        {
            mpND_signal = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, w);
            mpND_t = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::TORQUE);
            mpND_a = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::ANGLE);
            mpND_w = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::CHARIMP);

            mInt.initialize(mTimestep, (*mpND_signal), 0.0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            signal = (*mpND_signal);
            c = (*mpND_c);
            Zx = (*mpND_Zx);

            //Spring equations
            a = mInt.update(signal);
            t = c + Zx*signal;

            //Write values to nodes
            (*mpND_t) = t;
            (*mpND_a) = a;
            (*mpND_w) = signal;
        }
    };
}

#endif // MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED




