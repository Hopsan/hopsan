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
//! @file   MechanicRotationalInertia.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-13
//!
//! @brief Contains a mechanic translational lossless connector (i.e. a moving body without inertia)
//!
//$Id$

#ifndef MECHANICTRANSLATIONALLOSSLESSCONNECTOR_HPP_INCLUDED
#define MECHANICTRANSLATIONALLOSSLESSCONNECTOR_HPP_INCLUDED

#include <sstream>

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalLosslessConnector : public ComponentQ
    {

    private:
        double mLength;         //This length is not accesible by the user,
                                //it is set from the start values by the c-components in the ends
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2;  //Node data pointers
        SecondOrderTransferFunction mFilter;
        Integrator mInt;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalLosslessConnector("TranslationalLosslessConnector");
        }

        MechanicTranslationalLosslessConnector(const std::string name) : ComponentQ(name)
        {
            //Set member attributes

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);

            mpND_f2 = getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::POSITION);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);

            //Initialization
            double x1 = (*mpND_x1);
            double v1 = (*mpND_v1);
            double x2 = (*mpND_x2);

            mLength = x1+x2;

            mInt.initialize(mTimestep, -v1, -x1+mLength);

            //Print debug message if velocities do not match
            if((*mpND_v1) != -(*mpND_v2))
            {
                std::stringstream ss;
                ss << "Start velocities does not match, {" << getName() << "::" << mpP1->getPortName() <<
                        "} and {" << getName() << "::" << mpP2->getPortName() << "}.";
                this->addDebugMessage(ss.str());
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double c1 = (*mpND_c1);
            double Zx1 = (*mpND_Zx1);
            double c2 = (*mpND_c2);
            double Zx2 = (*mpND_Zx2);

            //Connector equations
            double v2 = (c1-c2)/(Zx1+Zx2);
            double v1 = -v2;
            double x2 = mInt.update(v2);
            double x1 = -x2 + mLength;
            double f1 = c1 + Zx1*v1;
            double f2 = c2 + Zx2*v2;

            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_f2) = f2;
            (*mpND_x2) = x2;
            (*mpND_v2) = v2;
        }
    };
}

#endif // MECHANICTRANSLATIONALLOSSLESSCONNECTOR_HPP_INCLUDED

