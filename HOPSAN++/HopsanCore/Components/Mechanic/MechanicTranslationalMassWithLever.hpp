//!
//! @file   MechanicTranslationalMassWithLever.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-03-11
//!
//! @brief Contains a Lever with a Mass
//!
//$Id$

#ifndef MECHANICTRANSLATIONALMASSWITHLEVER_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASSWITHLEVER_HPP_INCLUDED

#include <sstream>

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //Verified with AMESim 2011-03-21
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMassWithLever : public ComponentQ
    {

    private:
        double L1, L2, w, m, B, k;
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1,
               *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2;  //Node data pointers
        double f1, x1, v1, c1, Zx1,
               f2, x2, v2, c2, Zx2; //Node data variables
        double mNum[3];
        double mDen[3];
        SecondOrderFilter mFilter;
        Integrator mInt;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMassWithLever("TranslationalMassWithLever");
        }

        MechanicTranslationalMassWithLever(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            L1 = 1;
            L2 = 1;
            m = 1.0;
            B = 10;
            k = 0.0;
            w = 1.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("L_1", "Length", "[m]",               L1);
            registerParameter("L_2", "Length", "[m]",               L2);
            registerParameter("m",  "Mass", "[kg]",                m);
            registerParameter("B",  "Viscous Friction", "[Ns/m]",  B);
            registerParameter("k",  "Spring Coefficient", "[N/m]", k);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1  = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_x1  = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_v1  = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_c1  = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);

            mpND_f2  = getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_x2  = getSafeNodeDataPtr(mpP2, NodeMechanic::POSITION);
            mpND_v2  = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_c2  = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);

            //Initialization
            f1 = (*mpND_f1);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);
            x2 = (*mpND_x2);

            w = (L1+L2)/L1;

            mNum[0] = 0.0;
            mNum[1] = 1.0;
            mNum[2] = 0.0;
            mDen[0] = m;
            mDen[1] = B;
            mDen[2] = k;
            mFilter.initialize(mTimestep, mNum, mDen, -f1/w, -v1*w);
            mInt.initialize(mTimestep, -v1*w, -x1*w);

            //Print debug message if velocities do not match
            if(mpP1->readNode(NodeMechanic::VELOCITY)*w != -mpP2->readNode(NodeMechanic::VELOCITY))
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
            c1 = (*mpND_c1)/w;
            Zx1 = (*mpND_Zx1)/pow(w, 2.0);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations
            mDen[1] = B + Zx1 + Zx2;

            mFilter.setDen(mDen);
            v2 = mFilter.update(c1-c2);
            v1 = -v2/w;
            x2 = mInt.update(v2);
            x1 = -x2/w;
            f1 = (c1 - Zx1*v2)*w;
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

#endif // MECHANICTRANSLATIONALMASSWITHLEVER_HPP_INCLUDED

