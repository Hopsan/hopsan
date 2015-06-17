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
//! @file   HydraulicMultiTankC.hpp
//! @author Robert Braun
//! @date   2011-09-02
//!
//! @brief Contains a Hydraulic Multi Port Tank Component of C-type
//!
//$Id$

#ifndef HYDRAULICMULTITANKC_HPP_INCLUDED
#define HYDRAULICMULTITANKC_HPP_INCLUDED

#include <vector>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicMultiTankC : public ComponentC
    {
    private:
        double mP;

        Port *mpMP;
        size_t mNumPorts;
        std::vector<double*> mvpMP_p; //!< @todo Do we really need this, also in non multiport example
        std::vector<double*> mvpMP_q;
        std::vector<double*> mvpMP_c;
        std::vector<double*> mvpMP_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicMultiTankC();
        }

        void configure()
        {
            mpMP = addPowerMultiPort("MP", "NodeHydraulic");
            addConstant("p", "Default pressure", "Pa", 1.0e5, mP);

            disableStartValue(mpMP, NodeHydraulic::Pressure);
            disableStartValue(mpMP, NodeHydraulic::WaveVariable);
            disableStartValue(mpMP, NodeHydraulic::CharImpedance);
        }


        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();
            //! @todo write help function to set the size and contents of a these vectors automatically
            mvpMP_p.resize(mNumPorts);
            mvpMP_q.resize(mNumPorts);
            mvpMP_c.resize(mNumPorts);
            mvpMP_Zc.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpMP_p[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Pressure);
                mvpMP_q[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Flow);
                mvpMP_c[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::WaveVariable);
                mvpMP_Zc[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::CharImpedance);

                *(mvpMP_p[i]) = mP;    //Override the startvalue for the pressure
                *(mvpMP_q[i]) = getDefaultStartValue(mpMP, NodeHydraulic::Flow);
                *(mvpMP_c[i]) = mP;
                *(mvpMP_Zc[i]) = 0.0;
            }
        }


        void simulateOneTimestep()
        {
            //Nothing will change
        }
    };
}

#endif // HYDRAULICTANKC_HPP_INCLUDED
