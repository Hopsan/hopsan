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

//!
//! @file   MechanicTorsionalSpring.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a torsional spring
//!
//$Id$

#ifndef MECHANICTORSIONALSPRING_HPP_INCLUDED
#define MECHANICTORSIONALSPRING_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTorsionalSpring : public ComponentC
    {

    private:
        double k;
        double w1, c1, lastc1, w2, c2, lastc2, Zx;
        double *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_w2, *mpND_c2, *mpND_Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTorsionalSpring();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
            addConstant("k", "Spring Coefficient", "Nm/rad", 100.0, k);
        }


        void initialize()
        {
            mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            Zx = k*mTimestep;

            (*mpND_Zx1) = Zx;
            (*mpND_Zx2) = Zx;

        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            w1 = (*mpND_w1);
            lastc1 = (*mpND_c1);
            w2 = (*mpND_w2);
            lastc2 = (*mpND_c2);

            //Spring equations
            c1 = lastc2 + 2.0*Zx*w2;
            c2 = lastc1 + 2.0*Zx*w1;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_c2) = c2;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


