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
//! @file   MechanicFixedPositionMultiPort.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-03-21
//!
//! @brief Contains a Mechanic Fixed Position Component
//!
//$Id$

#ifndef MECHANICFIXEDPOSITIONMULTIPORT_HPP_INCLUDED
#define MECHANICFIXEDPOSITIONMULTIPORT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class MechanicFixedPositionMultiPort : public ComponentQ
    {

    private:
        double me;
        Port *mpMP;
        std::vector<double*> mND_f_vec;
        std::vector<double*> mND_x_vec;
        std::vector<double*> mND_v_vec;
        std::vector<double*> mND_c_vec;
        std::vector<double*> mND_Zc_vec;
        std::vector<double*> mND_me_vec;
        size_t mNumPorts;

    public:
        static Component *Creator()
        {
            return new MechanicFixedPositionMultiPort();
        }

        void configure()
        {
            mpMP = addPowerMultiPort("Pm1", "NodeMechanic");

            addConstant("m_e", "Equivalent Mass", "kg", 1, me);
        }

        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();

            //! @todo write help function to set the size and contents of a these vectors automatically
            mND_f_vec.resize(mNumPorts);
            mND_x_vec.resize(mNumPorts);
            mND_v_vec.resize(mNumPorts);
            mND_c_vec.resize(mNumPorts);
            mND_Zc_vec.resize(mNumPorts);
            mND_me_vec.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mND_f_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Force);
                mND_x_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Position);
                mND_v_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Velocity);
                mND_c_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::WaveVariable);
                mND_Zc_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::CharImpedance);
                mND_me_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::EquivalentMass);

                *(mND_v_vec[i]) = 0;
                *(mND_me_vec[i]) = me;
            }
        }


        void simulateOneTimestep()
        {
            //Equations
            for(size_t i=0; i<mNumPorts; ++i)
            {
                *(mND_f_vec[i]) = *(mND_c_vec[i]);
            }
        }
    };
}

#endif // MECHANICFIXEDPOSITIONMULTIPORT_HPP_INCLUDED
