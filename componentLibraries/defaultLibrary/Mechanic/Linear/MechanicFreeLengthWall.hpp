/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
    double me, mStopPos;

    //Node data pointers
    double *mpB;
    double *mpP1_f, *mpP1_x, *mpP1_v, *mpP1_me, *mpP1_c, *mpP1_Zx;

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
        addConstant("m_e", "Equivalent Mass", "kg", 1, me); //!< @todo this constant is not needed since we have a start value
        addConstant("stop_pos", "The position of the stop", "Position", 0, mStopPos);
    }


    void initialize()
    {
        //Assign node data pointers
        mpP1_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
        mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
        mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
        mpP1_me = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);
        mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
        mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

        //Initialization
        mNumX[0] = 1.0;
        mNumX[1] = 0.0;
        mDenX[0] = 0.0;
        mDenX[1] = (*mpB);
        mNumV[0] = 1.0;
        mNumV[1] = 0.0;
        mDenV[0] = (*mpB);
        mDenV[1] = 0.0;

        mFilterX.initialize(mTimestep, mNumX, mDenX, -(*mpP1_c), (*mpP1_x));
        mFilterV.initialize(mTimestep, mNumV, mDenV, -(*mpP1_c), (*mpP1_v));

        (*mpP1_me) = me;
    }


    void simulateOneTimestep()
    {
        double f1, x1, v1, c1, Zx1;

        //Get variable values from nodes
        c1 = (*mpP1_c);
        Zx1 = (*mpP1_Zx);

        //Mass equations
        mDenX[1] = (*mpB)+Zx1;
        mDenV[0] = (*mpB)+Zx1;
        mFilterX.setDen(mDenX);
        mFilterV.setDen(mDenV);

        x1 = mFilterX.update(-c1);
        v1 = mFilterV.update(-c1);

        // Note! Negative x1 value means that we are moving into the port towards mStopPos
        //       mStopPos should have same coordinate system as port, (positive direction out of the component)
        if(x1<=mStopPos)
        {
            // We have hit the wall, Enforce position and zero velocity
            // this is kind of non-physical, to just stop instantly
            x1=mStopPos;
            v1=0.0;
            mFilterX.initializeValues(-c1, x1);
            mFilterV.initializeValues(-c1, v1);
            f1 = c1 + Zx1*v1;
        }
        // If we are not in contact, then set f1 = 0 since that end should be in "free space"
        else
        {
            f1 = 0;
        }

        //Write new values to nodes
        (*mpP1_f) = f1;
        (*mpP1_x) = x1;
        (*mpP1_v) = v1;
    }
};
}

#endif // MECHANICFREELENGTHWALL_HPP_INCLUDED

