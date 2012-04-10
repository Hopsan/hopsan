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

//$Id: MechanicFreeLengthWall.hpp 3510 2011-10-17 15:07:52Z petno25 $

#ifndef MECHANICFREELENGTHWALL_HPP_INCLUDED
#define MECHANICFREELENGTHWALL_HPP_INCLUDED

#include <sstream>
#include <math.h>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicFreeLengthWall : public ComponentQ
    {

    private:
        double B, me;
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_me1, *mpND_c1, *mpND_Zx1;  //Node data pointers
        double f1, x1, v1, c1, Zx1;                                                    //Node data variables
        double mNumX[2], mNumV[2];
        double mDenX[2], mDenV[2];
        FirstOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicFreeLengthWall();
        }

        MechanicFreeLengthWall() : ComponentQ()
        {
            //Set member attributes
            B = 0.001;              //Must not be zero - velocity will become very oscillative
            me = 1;

            //Add ports to the component
            mpP1 = addPowerPort("Pm1", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("B", "Viscous Friction", "[Ns/m]",    B);
            registerParameter("m_e", "Equivalent Mass", "[kg]", me);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EQMASS);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);

            //Initialization
            f1 = (*mpND_f1);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mDenX[0] = 0.0;
            mDenX[1] = B;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = B;
            mDenV[1] = 0.0;

            mFilterX.initialize(mTimestep, mNumX, mDenX, -f1, x1);
            mFilterV.initialize(mTimestep, mNumV, mDenV, -f1, v1);

            (*mpND_me1) = me;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);

            //Mass equations
            mDenX[1] = B+Zx1;
            mDenV[0] = B+Zx1;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            x1 = mFilterX.update(-c1);
            v1 = mFilterV.update(-c1);

            if(x1<0.0)
            {
                x1=0.0;
                v1=0.0;
                mFilterX.initializeValues(-c1, x1);
                mFilterV.initializeValues(-c1, v1);
            }

            f1 = c1 + Zx1*v1;

            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
        }
    };
}

#endif // MECHANICFREELENGTHWALL_HPP_INCLUDED

