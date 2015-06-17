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
//! @file   HydraulicTankQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-12
//!
//! @brief Contains a Hydraulic Tank Component of Q-type
//!
//$Id$

#ifndef HYDRAULICTANKQ_HPP_INCLUDED
#define HYDRAULICTANKQ_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTankQ : public ComponentQ
    {
    private:
        double *mpP;

        double *mpND_p, *mpND_q, *mpND_c, *mpND_Zc;

        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicTankQ("TankQ");
        }

        HydraulicTankQ(const std::string name) : ComponentQ(name)
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            addInputVariable("p", "Default Pressure", "Pa", 1e5, &mpP);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double q, c, Zc;

            //Get variable values from nodes
            c = (*mpND_c);
            Zc = (*mpND_Zc);

            //Equations
            q = ((*mpP) - c)/Zc;

            //Write variables to nodes
            (*mpND_p) = p;
            (*mpND_q) = q;
        }
    };
}

#endif // HYDRAULICTANKQ_HPP_INCLUDED
