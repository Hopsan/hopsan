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
//! @file   HydraulicPressureSource.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Tank Component of C-type
//!
//$Id$

#ifndef HYDRAULICTANKC_HPP_INCLUDED
#define HYDRAULICTANKC_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTankC : public ComponentC
    {
    private:
        double mPressure;

        Port *mpP1;
        double *mpP1_p, *mpP1_c, *mpP1_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicTankC();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            addConstant("p", "Default Pressure", "Pa", 1.0e5, mPressure);

            setDefaultStartValue(mpP1, NodeHydraulic::Flow, 0.0);
            disableStartValue(mpP1, NodeHydraulic::Pressure);
            disableStartValue(mpP1, NodeHydraulic::WaveVariable);
            disableStartValue(mpP1, NodeHydraulic::CharImpedance);
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            //Override the start value
            (*mpP1_p) = mPressure;
            (*mpP1_c) = mPressure;
            (*mpP1_Zc) = 0.0;
        }


        void simulateOneTimestep()
        {
            //Nothing will change
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICTANKC_HPP_INCLUDED
