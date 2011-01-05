//$Id$

#ifndef MECHANICTRANSLATIONALMASS_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASS_HPP_INCLUDED

#include <sstream>

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMass : public ComponentQ
    {

    private:
        double m, B, k;
        double mLength;         //This length is not accesible by the user,
                                //it is set from the start values by the c-components in the ends
        double *f1_ptr, *x1_ptr, *v1_ptr, *c1_ptr, *Zx1_ptr, *f2_ptr, *x2_ptr, *v2_ptr, *c2_ptr, *Zx2_ptr;  //Node data pointers
        double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;                                                    //Node data variables
        double mNum[3];
        double mDen[3];
        SecondOrderFilter mFilter;
        Integrator mInt;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMass("TranslationalMass");
        }

        MechanicTranslationalMass(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            mTypeName = "MechanicTranslationalMass";
            m = 1.0;
            B    = 10;
            k   = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("m", "Mass", "[kg]",                  m);
            registerParameter("B", "Viscous Friction", "[Ns/m]",    B);
            registerParameter("k", "Spring Coefficient", "[N/m]",   k);
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
            f1 = (*f1_ptr);
            x1 = (*x1_ptr);
            v1 = (*v1_ptr);
            x2 = (*x2_ptr);

            mLength = x1+x2;

            mNum[0] = 0.0;
            mNum[1] = 1.0;
            mNum[2] = 0.0;
            mDen[0] = m;
            mDen[1] = B;
            mDen[2] = k;
            mFilter.initialize(mTimestep, mNum, mDen, -f1, -v1);
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
            c1 = (*c1_ptr);
            Zx1 = (*Zx1_ptr);
            c2 = (*c2_ptr);
            Zx2 = (*Zx2_ptr);

            //Mass equations
            mDen[1] = B+Zx1+Zx2;

            mFilter.setDen(mDen);
            v2 = mFilter.update(c1-c2);
            v1 = -v2;
            x2 = mInt.update(v2);
            x1 = -x2 + mLength;
            f1 = c1 + Zx1*v1;
            f2 = c2 + Zx2*v2;

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

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

