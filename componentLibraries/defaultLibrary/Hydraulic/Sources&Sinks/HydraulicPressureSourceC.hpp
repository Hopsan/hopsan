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
//! @file   HydraulicPressureSourceC.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCEC_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSourceC : public ComponentC
    {
    private:
        Port *mpP1;
        double *mpP, *mpP1_p, *mpP1_c, *mpP1_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSourceC();
        }

        void configure()
        {
            addInputVariable("p", "Set pressure", "Pa", 1.0e5, &mpP);

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            disableStartValue(mpP1, NodeHydraulic::Pressure);
            disableStartValue(mpP1, NodeHydraulic::WaveVariable);
            disableStartValue(mpP1, NodeHydraulic::CharImpedance);
            setDefaultStartValue(mpP1, NodeHydraulic::Flow, 0.0);
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            (*mpP1_p) = (*mpP);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            *mpP1_c = *mpP;
            *mpP1_Zc = 0.0;
        }
    };
}

#endif // HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
