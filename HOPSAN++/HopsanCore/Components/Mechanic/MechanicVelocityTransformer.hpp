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
        double mSignal;
        Integrator mInt;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicVelocityTransformer("VelocityTransformer");
        }

        MechanicVelocityTransformer(const std::string name) : ComponentQ(name)
        {
            mSignal = 0.0;

            //Set member attributes
            mTypeName = "MechanicVelocityTransformer";

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addPowerPort("out", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("Speed", "Generated speed", "[m/s]", mSignal);
        }


        void initialize()
        {
            double signal;
            if(mpIn->isConnected())
                signal  = mpIn->readNode(NodeSignal::VALUE);
            else
                signal = mSignal;
            mInt.initialize(mTimestep, signal, 0.0);
        }


        void simulateOneTimestep()
        {
            double signal;
            //Get variable values from nodes
            if(mpIn->isConnected())
                signal  = mpIn->readNode(NodeSignal::VALUE);
            else
                signal = mSignal;
            double c =mpOut->readNode(NodeMechanic::WAVEVARIABLE);
            double Zc =mpOut->readNode(NodeMechanic::CHARIMP);

            //Spring equations
            double v = signal;
            double x = mInt.update(v);
            double F = c + Zc*v;

            //Write new values to nodes
            mpOut->writeNode(NodeMechanic::POSITION, x);
            mpOut->writeNode(NodeMechanic::VELOCITY, v);
            mpOut->writeNode(NodeMechanic::FORCE, F);
        }
    };
}

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
