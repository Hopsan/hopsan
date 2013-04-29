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

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicVelocityTransformer : public ComponentQ
    {

    private:
        bool mXIsConnected;
        double me;
        double *mpPm1_f, *mpPm1_x, *mpPm1_v, *mpPm1_c, *mpPm1_Zx, *mpPm1_me;
        double *mpIn_x, *mpIn_v;
        Integrator mInt;
        Port *mpXPort, *mpVPort, *mpPm1;

    public:
        static Component *Creator()
        {
            return new MechanicVelocityTransformer();
        }

        void configure()
        {
            //Add ports to the component
            mpPm1 = addPowerPort("Pm1", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            mpVPort = addInputVariable("v", "Generated Velocity", "[m/s]", 0.0, &mpIn_v);
            mpXPort = addInputVariable("x", "Generated Position", "m", 0.0, &mpIn_x);

            // add constants
            addConstant("m_e", "Equivalent Mass", "[kg]", 10, me);
        }


        void initialize()
        {
            mpPm1_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
            mpPm1_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
            mpPm1_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
            mpPm1_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
            mpPm1_Zx = getSafeNodeDataPtr(mpPm1, NodeMechanic::CharImpedance);
            mpPm1_me = getSafeNodeDataPtr(mpPm1, NodeMechanic::EquivalentMass);

            mXIsConnected = mpXPort->isConnected();
            if(mXIsConnected && !mpVPort->isConnected())
            {
                addWarningMessage("Position input is connected but velocity is constant, kinematic relationsship must be manually enforced.");
            }
            else if(mXIsConnected && mpVPort->isConnected())
            {
                addWarningMessage("Both position and velocity inputs are connected, kinematic relationsship must be manually enforced.");
            }

            // Initialize node values
            mInt.initialize(mTimestep, (*mpIn_v), (*mpIn_x));
            (*mpPm1_me) = me;

            (*mpPm1_f) = (*mpPm1_c) + (*mpPm1_Zx)*(*mpIn_v);
            (*mpPm1_x) = (*mpIn_x);
            (*mpPm1_v) = (*mpIn_v);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            const double v = (*mpIn_v);
            const double c = (*mpPm1_c);
            const double Zx = (*mpPm1_Zx);
            double x;

            //Source equations
            if(mXIsConnected)
            {
                x = (*mpIn_x);
            }
            else
            {
                x = mInt.update(v);
            }

            //Write values to nodes
            (*mpPm1_f) = c + Zx*v;
            (*mpPm1_x) = x;
            (*mpPm1_v) = v;
        }
    };
}

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
