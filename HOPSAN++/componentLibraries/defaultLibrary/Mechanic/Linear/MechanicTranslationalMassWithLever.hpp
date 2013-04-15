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

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //Verified with another simulation software 2011-03-21
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMassWithLever : public ComponentQ
    {

    private:
        double L1, L2, w;
        double *mpM, *mpB;
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_me1, *mpND_c1, *mpND_Zx1,
               *mpND_f2, *mpND_x2, *mpND_v2, *mpND_me2, *mpND_c2, *mpND_Zx2;  //Node data pointers
        double f1, x1, v1, c1, Zx1,
               f2, x2, v2, c2, Zx2; //Node data variables
        double mNum[3];
        double mDen[3];
        DoubleIntegratorWithDamping mIntegrator;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMassWithLever();
        }

        void configure()
        {
            //! @todo set eqmass to some good values, should consider lever.

            //Set member attributes
            w = 1.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            addConstant("L_1", "Length", "[m]", 1,              L1);
            addConstant("L_2", "Length", "[m]", 1,              L2);
            addInputVariable("m",  "Mass", "[kg]",  1.0,              &mpM);
            addInputVariable("B",  "Viscous Friction", "[Ns/m]", 10,  &mpB);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1  = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1  = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1  = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);
            mpND_c1  = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            mpND_f2  = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpND_x2  = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpND_v2  = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpND_me2 = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);
            mpND_c2  = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            //Initialization
            f1 = (*mpND_f1);
            f2 = (*mpND_f2);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);

            w = (L1+L2)/L1;

            mIntegrator.initialize(mTimestep, 0, (f1-f2)/(*mpM), -x1*w, -v1*w);
            (*mpND_x2) = -x1*w;
            (*mpND_v2) = -v1*w;

            //Print debug message if velocities do not match
            if(mpP1->readNode(NodeMechanic::Velocity)*w != -mpP2->readNode(NodeMechanic::Velocity))
            {
                std::stringstream ss;
                ss << "Start velocities does not match, {" << getName() << "::" << mpP1->getName() <<
                        "} and {" << getName() << "::" << mpP2->getName() << "}.";
                this->addDebugMessage(ss.str());
            }

            (*mpND_me1) = (*mpM)*w;
            (*mpND_me2) = (*mpM);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1)/w;
            Zx1 = (*mpND_Zx1)/pow(w, 2.0);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);
            const double m = (*mpM);
            const double B = (*mpB);

            //Mass equations
            mIntegrator.setDamping((B+Zx1+Zx2) / m * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/m);
            v2 = mIntegrator.valueFirst();
            x2 = mIntegrator.valueSecond();
            v1 = -v2/w;
            x1 = -x2/w;
            f1 = (c1 - Zx1*v2)*w;
            f2 = c2 + Zx2*v2;

            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_me1) = m*w;
            (*mpND_f2) = f2;
            (*mpND_x2) = x2;
            (*mpND_v2) = v2;
            (*mpND_me2) = m;
        }
    };
}

#endif // MECHANICTRANSLATIONALMASSWITHLEVER_HPP_INCLUDED

