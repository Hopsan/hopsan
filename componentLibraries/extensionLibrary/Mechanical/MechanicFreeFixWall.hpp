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

#ifndef MECHANICFREEFIXWALL_HPP_INCLUDED
#define MECHANICFREEFIXWALL_HPP_INCLUDED

#include <sstream>
#include <math.h>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicFreeFixWall : public ComponentQ
{

private:
    double mStopPos;

    //Node data pointers
    double *mpB;
    MechanicNodeDataPointerStructT mP1;

    double mNumX[2];//, mNumV[2];
    double mDenX[2];//, mDenV[2];
    FirstOrderTransferFunction mFilterX;
    //FirstOrderTransferFunction mFilterV;
    Port *mpP1;
    bool mIsFixed;

public:
    static Component *Creator()
    {
        return new MechanicFreeFixWall();
    }

    void configure()
    {
        //Add ports to the component
        mpP1 = addPowerPort("Pm1", "NodeMechanic");
        addInputVariable("B", "Viscous Friction", "Ns/m", 0.001, &mpB); // B, Must not be zero - velocity will become very oscillatory

        // Add constants
        addConstant("stop_pos", "The position of the stop", "Position", 0, mStopPos);
        addConstant("fixed", "Fixed or free wall","", false, mIsFixed);
    }


    void initialize()
    {
        //Assign node data pointers
        getMechanicPortNodeDataPointers(mpP1, mP1);

        if (mIsFixed)
        {
            mP1.rv() = 0;
        }
        else
        {
            //Initialization
            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mDenX[0] = 0.0;
            mDenX[1] = (*mpB);
//            mNumV[0] = 1.0;
//            mNumV[1] = 0.0;
//            mDenV[0] = (*mpB);
//            mDenV[1] = 0.0;

            mFilterX.initialize(mTimestep, mNumX, mDenX, -(mP1.c()), (mP1.x()));
//            mFilterV.initialize(mTimestep, mNumV, mDenV, -(mP1.c()), (mP1.v()));
        }
    }


    void simulateOneTimestep()
    {
        if (mIsFixed)
        {
            mP1.rf() = mP1.rc();
        }
        else
        {
            double f1, x1, v1;
            const double c1 = mP1.c();
            const double Zx1 = mP1.Zc();

            //Mass equations
            mDenX[1] = (*mpB)+Zx1;
//            mDenV[0] = (*mpB)+Zx1;
            mFilterX.setDen(mDenX);
//            mFilterV.setDen(mDenV);

            x1 = mFilterX.update(-c1);
//            v1 = mFilterV.update(-c1);
            v1 = -c1/(*mpB+Zx1);

            // Note! Negative x1 value means that we are moving into the port towards mStopPos
            //       mStopPos should have same coordinate system as port, (positive direction out of the component)
            if(x1<=mStopPos)
            {
                // We have hit the wall, Enforce position and zero velocity
                // this is kind of non-physical, to just stop instantly
                x1=mStopPos;
                v1=0.0;
                mFilterX.initializeValues(-c1, x1);
                // I dont think that we should reinitialize the velocity filter, that would create or remove energy
                //mFilterV.initializeValues(-c1, v1);
                f1 = c1 + Zx1*v1;
            }
            // If we are not in contact, then set f1 = 0 since that end should be in "free space"
            else
            {
                f1 = 0;
            }

            //Write new values to nodes
            mP1.rf() = f1;
            mP1.rx() = x1;
            mP1.rv() = v1;
        }
    }
};
}

#endif // MECHANICFREELENGTHWALL_HPP_INCLUDED

