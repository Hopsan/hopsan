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
        double v;
        double signal, f, vin, x, c, Zx;
        double *mpND_xin, *mpND_vin, *mpND_f, *mpND_x, *mpND_v, *mpND_c, *mpND_Zx;
        Integrator mInt;
        Port *mpXin, *mpVin, *mpPm1;

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
            mpXin = addReadPort("xin", "NodeSignal", Port::NOTREQUIRED);
            mpVin = addReadPort("vin", "NodeSignal", Port::NOTREQUIRED);
            mpPm1 = addPowerPort("Pm1", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("v", "Generated Velocity", "[m/s]", v);
        }


        void initialize()
        {
            mpND_xin  = getSafeNodeDataPtr(mpXin, NodeSignal::VALUE, x);
            mpND_vin  = getSafeNodeDataPtr(mpVin, NodeSignal::VALUE, v);

            mpND_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::FORCE);
            mpND_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::POSITION);
            mpND_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::VELOCITY);
            mpND_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx = getSafeNodeDataPtr(mpPm1, NodeMechanic::CHARIMP);

            mInt.initialize(mTimestep, (*mpND_v), (*mpND_x));


            if(mpXin->isConnected() && !mpVin->isConnected())
            {
                std::stringstream ss;
                ss << "Position input is connected but velocity is constant, kinematic relationsship must be manually enforced.";
                addWarningMessage(ss.str());
            }
            else if(mpXin->isConnected() && mpVin->isConnected())
            {
                std::stringstream ss;
                ss << "Both position and velocity inputs are connected, kinematic relationsship must be manually enforced.";
                addWarningMessage(ss.str());
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            vin = (*mpND_vin);
            c = (*mpND_c);
            Zx = (*mpND_Zx);

            //Source equations
            if(mpXin->isConnected())
            {
                x = (*mpND_xin);
            }
            else
            {
                x = mInt.update(vin);
            }
            f = c + Zx*vin;

            //Write values to nodes
            (*mpND_f) = f;
            (*mpND_x) = x;
            (*mpND_v) = vin;
        }
    };
}

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
