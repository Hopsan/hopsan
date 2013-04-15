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
#include <sstream>

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
        double *mpND_f, *mpND_x, *mpND_v, *mpND_c, *mpND_Zx, *mpND_me;
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
            mpVPort = addInputVariable("v", "Generated Velocity", "[m/s]", 0.0, &mpND_v);
            mpXPort = addInputVariable("x", "Generated Position", "m", 0.0, &mpND_x);

            // add constants
            addConstant("m_e", "Equivalent Mass", "[kg]", 10, me);
        }


        void initialize()
        {
            mpND_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
            mpND_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
            mpND_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
            mpND_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
            mpND_Zx = getSafeNodeDataPtr(mpPm1, NodeMechanic::CharImpedance);
            mpND_me = getSafeNodeDataPtr(mpPm1, NodeMechanic::EquivalentMass);

            mInt.initialize(mTimestep, (*mpND_v), (*mpND_x));
            (*mpND_me) = me;

            if (mpXPort->isConnected())
            {
               mXIsConnected = true;
            }
            else
            {
               mXIsConnected = false;
            }

            if(mXIsConnected && !mpVPort->isConnected())
            {
                addWarningMessage("Position input is connected but velocity is constant, kinematic relationsship must be manually enforced.");
            }
            else if(mXIsConnected && mpVPort->isConnected())
            {
                addWarningMessage("Both position and velocity inputs are connected, kinematic relationsship must be manually enforced.");
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            const double v = (*mpND_v);
            const double c = (*mpND_c);
            const double Zx = (*mpND_Zx);
            double x;

            //Source equations
            if(mXIsConnected)
            {
                x = (*mpND_x);
            }
            else
            {
                x = mInt.update(v);
            }

            //Write values to nodes
            (*mpND_f) = c + Zx*v;
            (*mpND_x) = x;
            (*mpND_v) = v;
        }
    };
}

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
