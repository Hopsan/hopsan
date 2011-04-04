//!
//! @file   SignalStopSimulation.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-15
//!
//! @brief Contains a component for stopping a simulation
//!
//$Id$

#ifndef SIGNALSTOPSIMULATION_HPP_INCLUDED
#define SIGNALSTOPSIMULATION_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStopSimulation : public ComponentSignal
    {

    private:
        double *mpND_in;
        Port *mpIn;

    public:
        static Component *Creator()
        {
            return new SignalStopSimulation("StopSim");
        }

        SignalStopSimulation(const std::string name) : ComponentSignal(name)
        {

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, boolToDouble(false));
        }


        void simulateOneTimestep()
        {
            if(doubleToBool(*mpND_in))
            {
                this->stopSimulation();
            }
        }
    };
}
#endif // SIGNALSTOPSIMULATION_HPP_INCLUDED
