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
//! @file   HydraulicMultiPressureSourceC.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Multi Port Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICMULTIPRESSURESOURCEC_HPP_INCLUDED
#define HYDRAULICMULTIPRESSURESOURCEC_HPP_INCLUDED

#include <vector>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicMultiPressureSourceC : public ComponentC
    {
    private:
        double Zc;
        Port *mpIn, *mpMP;

        size_t mNumPorts;
        double *mpND_in;
        std::vector<double*> mND_p_vec;
        std::vector<double*> mND_q_vec;
        std::vector<double*> mND_c_vec;
        std::vector<double*> mND_Zc_vec;

    public:
        static Component *Creator()
        {
            return new HydraulicMultiPressureSourceC();
        }

        void configure()
        {
            Zc = 0.0;

            mpMP = addPowerMultiPort("MP", "NodeHydraulic");
            mpIn = addInputVariable("p", "Default pressure", "Pa", 1.0e5);

            disableStartValue(mpMP, NodeHydraulic::Pressure);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::Value);

            mNumPorts = mpMP->getNumPorts();

            //! @todo write help function to set the size and contents of a these vectors automatically
            mND_p_vec.resize(mNumPorts);
            mND_q_vec.resize(mNumPorts);
            mND_c_vec.resize(mNumPorts);
            mND_Zc_vec.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mND_p_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Pressure);
                mND_q_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::Flow);
                mND_c_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::WaveVariable);
                mND_Zc_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::CharImpedance);

                *(mND_p_vec[i]) = (*mpND_in);    //Override the startvalue for the pressure
                *(mND_q_vec[i]) = getStartValue(mpMP, NodeHydraulic::Flow);
                *(mND_c_vec[i]) = (*mpND_in);
                *(mND_Zc_vec[i]) = Zc;
            }
        }


        void simulateOneTimestep()
        {
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *(mND_c_vec[i]) = (*mpND_in);
                *(mND_Zc_vec[i]) = Zc;
            }
        }
    };
}

#endif // HYDRAULICPRESSURESOURCEC_HPP_INCLUDED
