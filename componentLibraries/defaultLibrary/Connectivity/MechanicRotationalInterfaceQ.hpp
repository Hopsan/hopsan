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
//! @file   MechanicRotationalInterfaceQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-30
//!
//! @brief Contains a rotational mechanic interface component of Q-type
//!
//$Id$

#ifndef MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED
#define MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInterfaceQ : public ComponentQ
    {

    private:
        Port *mpP1;
        double *mpND_t, *mpND_w, *mpND_c, *mpND_Zx;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInterfaceQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        }

        void initialize()
        {
            mpND_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpND_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpND_Zx = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);
        }

        void simulateOneTimestep()
        {
            //! @todo If this works, do same in other Q-type interface components

            //Calculate torque from c and Zx
            double w = (*mpND_w);
            double c = (*mpND_c);
            double Zx = (*mpND_Zx);

            (*mpND_t) = c + Zx*w;
        }
    };
}

#endif // MECHANICROTATIONALINTERFACEQ_HPP_INCLUDED




