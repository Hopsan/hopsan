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
        double mMass;
        double mB;
        double mk;
        double mLength; //This length is not accesible by the user,
                        //it is set from the start values by the c-components in the ends
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
            mMass = 1.0;
            mB    = 10;
            mk   = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("m", "Mass", "[kg]",            mMass);
            registerParameter("B", "Viscous Friction", "[Ns/m]", mB);
            registerParameter("k", "Spring Coefficient", "[N/m]",  mk);
        }


        void initialize()
        {
            //mFilter.initialize(0.0,0.0, mTime);
            double x1  = mpP1->readNode(NodeMechanic::POSITION);
            double x2  = mpP2->readNode(NodeMechanic::POSITION);
            double v1  = mpP1->readNode(NodeMechanic::VELOCITY);
            double F1  = mpP1->readNode(NodeMechanic::FORCE);
            mLength = x1+x2;
            //cout << "x0 = " << x1 << endl;
            mNum[0] = 0.0;
            mNum[1] = 1.0;
            mNum[2] = 0.0;
            mDen[0] = mMass;
            mDen[1] = mB;
            mDen[2] = mk;
            mFilter.initialize(mTimestep, mNum, mDen, -F1, -v1);
            mInt.initialize(mTimestep, -v1, -x1+mLength);
            //std::cout << "apa: " << mLength << std::endl;
            //mFilter.update(0);
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
            double Zx1  = mpP1->readNode(NodeMechanic::CHARIMP);
            double c1  = mpP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx2  = mpP2->readNode(NodeMechanic::CHARIMP);
            double c2  = mpP2->readNode(NodeMechanic::WAVEVARIABLE);

            //Mass equations
            mDen[1] = mB+Zx1+Zx2;

            mFilter.setDen(mDen);
            double v2 = mFilter.update(c1-c2);
            double v1 = -v2;
            double x2 = mInt.update(v2);
            double x1 = -x2 + mLength;
            double F1 = c1 + Zx1*v1;
            double F2 = c2 + Zx2*v2;

            //Write new values to nodes
            mpP1->writeNode(NodeMechanic::FORCE, F1);
            mpP2->writeNode(NodeMechanic::FORCE, F2);
            mpP1->writeNode(NodeMechanic::VELOCITY, v1);
            mpP2->writeNode(NodeMechanic::VELOCITY, v2);
            mpP1->writeNode(NodeMechanic::POSITION, x1);
            mpP2->writeNode(NodeMechanic::POSITION, x2);
            //Update Filter:
            //mFilter.update(c1-c2);
        }
    };
}

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

