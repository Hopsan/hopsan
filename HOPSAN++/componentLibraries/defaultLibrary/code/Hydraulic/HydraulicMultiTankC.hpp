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
        double Zc;
        double p;
        Port *mpMP;

        size_t mNumPorts;
        std::vector<double*> mND_p_vec; //!< @todo Do we really need this, also in non multiport example
        std::vector<double*> mND_c_vec;
        std::vector<double*> mND_Zc_vec;

    public:
        static Component *Creator()
        {
            return new HydraulicMultiTankC();
        }

        void configure()
        {
            p         = 1.0e5;
            Zc        = 0.0;

            mpMP = addPowerMultiPort("MP", "NodeHydraulic"); //addPowerPort("MP", "NodeHydraulic");

            registerParameter("p", "Default pressure", "[Pa]", p);

            //! @todo should we set startvalues in one or all ports
            disableStartValue(mpMP, NodeHydraulic::PRESSURE);
            setStartValue(mpMP, NodeHydraulic::FLOW, 0.0);
        }


        void initialize()
        {
            mNumPorts = mpMP->getNumPorts();

            //! @todo write help function to set the size and contents of a these vectors automatically
            mND_p_vec.resize(mNumPorts);
            mND_c_vec.resize(mNumPorts);
            mND_Zc_vec.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mND_p_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::PRESSURE);
                mND_c_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::WAVEVARIABLE);
                mND_Zc_vec[i] = getSafeMultiPortNodeDataPtr(mpMP, i, NodeHydraulic::CHARIMP);

                //! @todo how should we divide startvalues among connected ports
                *(mND_p_vec[i]) = p;    //Override the startvalue for the pressure
                *(mND_c_vec[i]) = p;
                *(mND_Zc_vec[i]) = Zc;
            }
            mpMP->setStartValue(NodeHydraulic::PRESSURE, p); //This is here to show the user that the start value is hard coded!
        }


        void simulateOneTimestep()
        {
            //Nothing will change
        }
    };
}

#endif // HYDRAULICTANKC_HPP_INCLUDED
