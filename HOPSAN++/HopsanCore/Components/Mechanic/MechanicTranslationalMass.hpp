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

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMass : public ComponentQ
    {

    private:
        double m, B, k, xMin, xMax;
        double mLength;         //This length is not accesible by the user,
                                //it is set from the start values by the c-components in the ends
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_me1, *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2, *mpND_me2;  //Node data pointers
        double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;                                                    //Node data variables
        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];
        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
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
            k = 0.0;
            xMin = 0.0;
            xMax = 1.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("m", "Mass", "[kg]",                  m);
            registerParameter("B", "Viscous Friction", "[Ns/m]",    B);
            registerParameter("k", "Spring Coefficient", "[N/m]",   k);
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
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EQMASS);

            mpND_f2 = getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::POSITION);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);
            mpND_me2 = getSafeNodeDataPtr(mpP2, NodeMechanic::EQMASS);

            //Initialization
            f1 = (*mpND_f1);
            f2 = (*mpND_f2);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);
            x2 = (*mpND_x2);
            v2 = (*mpND_v2);

            mLength = x1+x2;

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = k;
            mDenX[1] = B;
            mDenX[2] = m;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = B;
            mDenV[1] = m;

            mFilterX.initialize(mTimestep, mNumX, mDenX, f1-f2, x2);
            mFilterV.initialize(mTimestep, mNumV, mDenV, f1-f2 - k*x2, v2);

            (*mpND_me1) = m;
            (*mpND_me2) = m;

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
            mDenX[1] = B+Zx1+Zx2;
            mDenV[0] = B+Zx1+Zx2;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            x2 = mFilterX.update(c1-c2);
            v2 = mFilterV.update(c1-c2 - k*x2);

//            if((mTime > 6) && (mTime < 6.01))
//            {
//                double apa = c1-c2;
//                stringstream ss;
//                ss << "t: " << mTime << "   c1-c2 = " << apa << "   v2 = " << v2;
//                addDebugMessage(ss.str());
//            }

            if(x2<xMin)
            {
                x2=xMin;
                v2=0.0;
                mFilterX.initializeValues(c1-c2, x2);
                mFilterV.initializeValues(c1-c2, 0.0);
            }
            if(x2>xMax)
            {
                x2=xMax;
                v2=0.0;
                mFilterX.initializeValues(c1-c2, x2);
                mFilterV.initializeValues(c1-c2, 0.0);
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
            (*mpND_me1) = m;
            (*mpND_me2) = m;


//            if((mTime>.5) && (mTime<.5001))
//            {
//                std::stringstream ss;
//                ss << "mTime: " << mTime << "     c1-c2: " << c1-c2 << "    v2: " << v2 << std::endl;
//                addInfoMessage(ss.str());
//            }
        }
    };
}

#endif // MECHANICTRANSLATIONALMASS_HPP_INCLUDED

