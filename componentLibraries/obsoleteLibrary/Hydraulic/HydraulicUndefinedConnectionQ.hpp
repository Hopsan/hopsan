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
//! @file   HydraulicUndefinedConnectionQ.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-03-09
//!
//! @brief Contains a Hydraulic Undefined Connection Component of Q-type
//!
//$Id$

#ifndef HYDRAULICUNDEFINEDCONNECTIONQ_HPP_INCLUDED
#define HYDRAULICUNDEFINEDCONNECTIONQ_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicUndefinedConnectionQ : public ComponentQ
    {
    private:
        Port *mpP1;
        double P, Q;
        double *mpND_p, *mpND_q;

    public:
        static Component *Creator()
        {
            return new HydraulicUndefinedConnectionQ();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            P = 100000;
            Q = 0;

            registerParameter("P",  "Pressure",     "[-]",  P);
            registerParameter("Q",  "Flow",         "[-]",  Q);
        }


        void initialize()
        {
            mpND_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);

            (*mpND_p) = P;
            (*mpND_q) = Q;
        }


        void simulateOneTimestep()
        {
            (*mpND_p) = P;
            (*mpND_q) = Q;
        }
    };
}

#endif // HYDRAULICUNDEFINEDCONNECTIONQ_HPP_INCLUDED
