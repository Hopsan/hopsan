
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
//! @file   MechanicTranslationalSpringWithSlack.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-06-30
//!
//! @brief Contains a mechanical translational spring component with slack
//!
//$Id: MechanicSpeedSensor.hpp 5559 2013-06-24 15:43:06Z bjoer $

#ifndef MECHANICTRANSLATIONALSPRINGWITHSLACK_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRINGWITHSLACK_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalSpringWithSlack : public ComponentC
    {

    private:
        double *mpK;
        double *mpND_f1, *mpND_f2, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zc1, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zc2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalSpringWithSlack();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changeable parameters to the HOPSAN++ core
            addInputVariable("k", "Spring Coefficient", "N/m", 100.0,  &mpK);
        }


        void initialize()
        {
            mpND_f1 =  getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpND_f2 =  getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            //! @todo Is this correct? Ask Petter!
            //(*mpND_c1) = (*mpND_f2)+2.0*Zc*(*mpND_v2);
            //(*mpND_c2) = (*mpND_f1)+2.0*Zc*(*mpND_v1);
            (*mpND_Zc1) = (*mpK)*mTimestep;
            (*mpND_Zc2) = (*mpK)*mTimestep;
        }


        void simulateOneTimestep()
        {
            double c1,c2,Zc;

            //Get variable values from nodes
            const double x1 = (*mpND_x1);
            const double x2 = (*mpND_x2);
            if(x1+x2 > 0)
            {
                c1 = 0;
                c2 = 0;
                Zc = 0;
            }
            else
            {
                const double v1 = (*mpND_v1);
                const double lastc1 = (*mpND_c1);
                const double v2 = (*mpND_v2);
                const double lastc2 = (*mpND_c2);

                //Spring equations
                Zc = (*mpK)*mTimestep;
                c1 = lastc2 + 2.0*Zc*v2;
                c2 = lastc1 + 2.0*Zc*v1;
            }

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRINGWITHSLACK_HPP_INCLUDED
