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
        double *mpND_signal,*mpND_t, *mpND_c, *mpND_Zx;
        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicTorqueTransformer("TorqueTransformer");
        }

        MechanicTorqueTransformer(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            t = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("t", "Generated Torque", "[Nm]", t);

            disableStartValue(mpP1, NodeMechanicRotational::TORQUE);
        }


        void initialize()
        {
            mpND_signal = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, t);
            mpND_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::TORQUE);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);

            (*mpND_t) = t;
            (*mpND_Zx) = 0.0;
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpND_signal);
            (*mpND_Zx) = 0.0;
        }
    };
}
#endif // MECHANICTORQUETRANSFORMER_HPP_INCLUDED
