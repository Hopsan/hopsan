/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//$Id$

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
        double me;
        double *mpB;
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

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerPort("Pm1", "NodeMechanic");
            addInputVariable("B", "Viscous Friction", "Ns/m", 0.001, &mpB); // B, Must not be zero - velocity will become very oscillatory

            // Add constants
            addConstant("m_e", "Equivalent Mass", "kg", 1, me);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            //Initialization
            f1 = (*mpND_f1);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mDenX[0] = 0.0;
            mDenX[1] = (*mpB);
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = (*mpB);
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
            mDenX[1] = (*mpB)+Zx1;
            mDenV[0] = (*mpB)+Zx1;
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

