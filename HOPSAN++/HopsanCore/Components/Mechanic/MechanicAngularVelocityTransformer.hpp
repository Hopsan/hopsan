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
        double signal, t, a, c, Zc;
        double *signal_ptr, *t_ptr, *a_ptr, *w_ptr, *c_ptr, *Zc_ptr;
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
            mTypeName = "MechanicAngularVelocityTransformer";
            w = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addPowerPort("out", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("w", "Generated angular velocity", "[rad/s]", w);
        }


        void initialize()
        {
            if(mpIn->isConnected())
                signal_ptr  = mpIn->getNodeDataPtr(NodeSignal::VALUE);
            else
                signal_ptr = new double(w);

            t_ptr = mpOut->getNodeDataPtr(NodeMechanicRotational::TORQUE);
            a_ptr = mpOut->getNodeDataPtr(NodeMechanicRotational::ANGLE);
            w_ptr = mpOut->getNodeDataPtr(NodeMechanicRotational::ANGULARVELOCITY);
            c_ptr = mpOut->getNodeDataPtr(NodeMechanicRotational::WAVEVARIABLE);
            Zc_ptr = mpOut->getNodeDataPtr(NodeMechanicRotational::CHARIMP);

            mInt.initialize(mTimestep, (*signal_ptr), 0.0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            signal = (*signal_ptr);
            c = (*c_ptr);
            Zc = (*Zc_ptr);

            //Spring equations
            a = mInt.update(signal);
            t = c + Zc*signal;

            //Write values to nodes
            (*t_ptr) = t;
            (*a_ptr) = a;
            (*w_ptr) = signal;
        }
    };
}

#endif // MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED




