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
//! @file   HydraulicSymmetricCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-09-02
//!
//! @brief A symmetric piston component of C-type. Inherits HydraulicCylinderC.
//!
//$Id$

#ifndef HYDRAULICSYMMETRICCYLINDERC_H
#define HYDRAULICSYMMETRICCYLINDERC_H


#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HydraulicCylinderC.hpp"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicSymmetricCylinderC : public HydraulicCylinderC
{

    private:
        double *mpA;

    public:
        static Component *Creator()
        {
            return new HydraulicSymmetricCylinderC();
        }

        void configure()
        {
            HydraulicCylinderC::configure();

            removePort("A_1");
            removePort("A_2");

            addInputVariable("A", "Piston Area", "m^2", 0.001, &mpA);
        }

        void initialize()
        {
            mpA1 = mpA;
            mpA2 = mpA;
            HydraulicCylinderC::initialize();
        }

        void simulateOneTimestep()
        {
            HydraulicCylinderC::simulateOneTimestep();
        }
    };
}

#endif // HYDRAULICSYMMETRICCYLINDERC_H
