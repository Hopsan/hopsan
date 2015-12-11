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
        std::vector<double*> mvPm1_f;
        std::vector<double*> mvPm1_x;
        std::vector<double*> mvPm1_v;
        std::vector<double*> mvPm1_c;
        std::vector<double*> mvPm1_Zc;
        std::vector<double*> mvPm1_me;
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
            mvPm1_f.resize(mNumPorts);
            mvPm1_x.resize(mNumPorts);
            mvPm1_v.resize(mNumPorts);
            mvPm1_c.resize(mNumPorts);
            mvPm1_Zc.resize(mNumPorts);
            mvPm1_me.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvPm1_f[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Force);
                mvPm1_x[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Position);
                mvPm1_v[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::Velocity);
                mvPm1_c[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::WaveVariable);
                mvPm1_Zc[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::CharImpedance);
                mvPm1_me[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeMechanic::EquivalentMass);

                *(mvPm1_v[i]) = 0;
                *(mvPm1_me[i]) = me;
            }
        }


        void simulateOneTimestep()
        {
            //Equations
            for(size_t i=0; i<mNumPorts; ++i)
            {
                *(mvPm1_f[i]) = *(mvPm1_c[i]);
            }
        }
    };
}

#endif // MECHANICFIXEDPOSITIONMULTIPORT_HPP_INCLUDED
