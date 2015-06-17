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
//! @file   HydraulicUndefinedConnectionC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-03-09
//!
//! @brief Contains a Hydraulic Undefined Connection Component of C-type
//!
//$Id$

#ifndef HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED
#define HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicUndefinedConnectionC : public ComponentC
    {
    private:
        Port *mpP1;
        double C, Z;
        double *mpND_c, *mpND_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicUndefinedConnectionC();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            C = 1e5;
            Z = 1;

            registerParameter("C",  "Wave Variable",     "[-]",  C);
            registerParameter("Z",  "Impedance",         "[-]",  Z);
        }


        void initialize()
        {
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            (*mpND_c) = C;
            (*mpND_Zc) = Z;
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = C;
            (*mpND_Zc) = Z;
        }
    };
}

#endif // HYDRAULICUNDEFINEDCONNECTIONC_HPP_INCLUDED
