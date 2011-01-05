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
        double *signal_ptr, *f_ptr, *x_ptr, *v_ptr, *c_ptr, *Zx_ptr;
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
            mTypeName = "MechanicVelocityTransformer";
            v = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addPowerPort("out", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("v", "Generated velocity", "[m/s]", v);
        }


        void initialize()
        {
            if(mpIn->isConnected()) { signal_ptr  = mpIn->getNodeDataPtr(NodeSignal::VALUE); }
            else { signal_ptr = new double(v); }

            f_ptr = mpOut->getNodeDataPtr(NodeMechanic::FORCE);
            x_ptr = mpOut->getNodeDataPtr(NodeMechanic::POSITION);
            v_ptr = mpOut->getNodeDataPtr(NodeMechanic::VELOCITY);
            c_ptr = mpOut->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx_ptr = mpOut->getNodeDataPtr(NodeMechanic::CHARIMP);

            mInt.initialize(mTimestep, (*signal_ptr), 0.0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            signal = (*signal_ptr);
            c = (*c_ptr);
            Zx = (*Zx_ptr);

            //Spring equations
            x = mInt.update(signal);
            f = c + Zx*signal;

            //Write values to nodes
            (*f_ptr) = f;
            (*x_ptr) = x;
            (*v_ptr) = signal;
        }
    };
}

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
