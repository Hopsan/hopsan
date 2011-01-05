//!
//! @file   MechanicTorqueTransformer.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a mechanic prescribed torque component
//!
//$Id$

#ifndef MECHANICTORQUETRANSFORMER_HPP_INCLUDED
#define MECHANICTORQUETRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTorqueTransformer : public ComponentC
    {

    private:
        double t;
        double *signal_ptr, *c_ptr, *Zx_ptr;
        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicTorqueTransformer("TorqueTransformer");
        }

        MechanicTorqueTransformer(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "MechanicForceTransformer";
            t = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("t", "Generated Torque", "[Nm]", t);
        }


        void initialize()
        {
            if(mpIn->isConnected()) { signal_ptr = mpIn->getNodeDataPtr(NodeSignal::VALUE); }
            else { signal_ptr = new double(t); }

            c_ptr = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx_ptr = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            (*c_ptr) = (*signal_ptr);
            (*Zx_ptr) = 0.0;
        }
    };
}
#endif // MECHANICTORQUETRANSFORMER_HPP_INCLUDED
