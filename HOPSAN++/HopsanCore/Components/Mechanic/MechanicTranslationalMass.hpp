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

#ifndef MECHANICTRANSLATIONALMASS_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASS_HPP_INCLUDED

#include <sstream>
#include <math.h>

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
        double m, B, /*k,*/ xMin, xMax;
        double mLength;         //This length is not accesible by the user,
                                //it is set from the start values by the c-components in the ends
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2;  //Node data pointers
        double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;                                                    //Node data variables
        double mNum[3];
        double mDen[3];
        //SecondOrderFilter mFilter;
        //Integrator mInt;
        DoubleIntegratorWithDamping mIntegrator;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMass("TranslationalMass");
        }

        MechanicTranslationalMass(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            m = 100.0;
            B = 10;
            //k = 0.0;
            xMin = 0.0;
            xMax = 1.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("m", "Mass", "[kg]",                  m);
            registerParameter("B", "Viscous Friction", "[Ns/m]",    B);
            //registerParameter("k", "Spring Coefficient", "[N/m]",   k);
            registerParameter("x_min", "Minimum Position", "[m]",   xMin);
            registerParameter("x_max", "Maximum Position", "[m]",   xMax);
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
            f2 = (*mpND_f2);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);
            x2 = (*mpND_x2);

            mLength = x1+x2;

//            mNum[0] = 0.0;
//            mNum[1] = 1.0;
//            mNum[2] = 0.0;
//            mDen[0] = k;
//            mDen[1] = B;
//            mDen[2] = m;
//            mFilter.initialize(mTimestep, mNum, mDen, 0, -v1);
//            mInt.initialize(mTimestep, -v1, -x1+mLength);
            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);

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
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations
            //mDen[1] = B+Zx1+Zx2;
            //mFilter.setDen(mDen);
            //v2 = mFilter.update(c1-c2);
            //x2 = mInt.update(v2);

            mIntegrator.setDamping((B+Zx1+Zx2) / m * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/m);
            v2 = mIntegrator.valueFirst();
            x2 = mIntegrator.valueSecond();

            if(x2<xMin)
            {
                x2=xMin;
                v2=std::max(0.0, v2);
                mIntegrator.initializeValues(0.0, x2, v2);
            }
            if(x2>xMax)
            {
                x2=xMax;
                v2=std::min(0.0, v2);
                mIntegrator.initializeValues(0.0, x2, v2);
            }

            v1 = -v2;
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

