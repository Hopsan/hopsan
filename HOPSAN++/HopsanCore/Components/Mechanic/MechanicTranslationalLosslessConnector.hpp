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
        double *f1_ptr, *x1_ptr, *v1_ptr, *c1_ptr, *Zx1_ptr, *f2_ptr, *x2_ptr, *v2_ptr, *c2_ptr, *Zx2_ptr;  //Node data pointers
        SecondOrderFilter mFilter;
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
            mTypeName = "MechanicTranslationalLosslessConnector";

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");
        }


        void initialize()
        {
            //Assign node data pointers
            f1_ptr = mpP1->getNodeDataPtr(NodeMechanic::FORCE);
            x1_ptr = mpP1->getNodeDataPtr(NodeMechanic::POSITION);
            v1_ptr = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            c1_ptr = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);

            f2_ptr = mpP2->getNodeDataPtr(NodeMechanic::FORCE);
            x2_ptr = mpP2->getNodeDataPtr(NodeMechanic::POSITION);
            v2_ptr = mpP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            c2_ptr = mpP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2_ptr = mpP2->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Initialization
            double f1 = (*f1_ptr);
            double x1 = (*x1_ptr);
            double v1 = (*v1_ptr);
            double x2 = (*x2_ptr);

            mLength = x1+x2;

            mInt.initialize(mTimestep, -v1, -x1+mLength);

            //Print debug message if velocities do not match
            if(mpP1->readNode(NodeMechanic::VELOCITY) != -mpP2->readNode(NodeMechanic::VELOCITY))
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
            double c1 = (*c1_ptr);
            double Zx1 = (*Zx1_ptr);
            double c2 = (*c2_ptr);
            double Zx2 = (*Zx2_ptr);

            //Connector equations
            double v2 = (c1-c2)/(Zx1+Zx2);
            double v1 = -v2;
            double x2 = mInt.update(v2);
            double x1 = -x2 + mLength;
            double f1 = c1 + Zx1*v1;
            double f2 = c2 + Zx2*v2;

            //Write new values to nodes
            (*f1_ptr) = f1;
            (*x1_ptr) = x1;
            (*v1_ptr) = v1;
            (*f2_ptr) = f2;
            (*x2_ptr) = x2;
            (*v2_ptr) = v2;
        }
    };
}

#endif // MECHANICTRANSLATIONALLOSSLESSCONNECTOR_HPP_INCLUDED

