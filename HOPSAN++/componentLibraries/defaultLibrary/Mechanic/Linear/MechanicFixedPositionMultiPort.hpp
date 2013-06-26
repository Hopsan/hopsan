/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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
