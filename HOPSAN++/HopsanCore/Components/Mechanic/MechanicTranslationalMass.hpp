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
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2;  //Node data pointers
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
            f1 = (*mpND_f1);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);
            x2 = (*mpND_x2);

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
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

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
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_f2) = f2;
            (*mpND_x2) = x2;
            (*mpND_v2) = v2;
        }
    };
}

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

