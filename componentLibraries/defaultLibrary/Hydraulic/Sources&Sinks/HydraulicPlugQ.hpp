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
//! @file   HydraulicPlugQ.hpp
//! @author FluMeS
//! @date   2013-05-02
//!
//! @brief Contains a Hydraulic Plug of Q-type
//!
//$Id$

#ifndef HYDRAULICPLUG_HPP_INCLUDED
#define HYDRAULICPLUG_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPlugQ : public ComponentQ
    {
    private:
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicPlugQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p, q, c, Zc;

            //Read variables from nodes
            c = (*mpP1_c);
            Zc = (*mpP1_Zc);

            //Flow source equations
            q = 0.0;
            p = c + q*Zc;

            if(p<0)
            {
                p=0;
            }

            (*mpP1_p) = p;
            (*mpP1_q) = q;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICPLUG_HPP_INCLUDED
