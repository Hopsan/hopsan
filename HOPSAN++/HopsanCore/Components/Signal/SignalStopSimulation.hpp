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

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStopSimulation : public ComponentSignal
    {

    private:
        Port *mpIn;

    public:
        static Component *Creator()
        {
            return new SignalStopSimulation("StopSim");
        }

        SignalStopSimulation(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalStopSimulation";

            mpIn = addReadPort("in", "NodeSignal");
        }


        void initialize()
        {
        }


        void simulateOneTimestep()
        {
            if (mpIn->isConnected())
            {
                if(mpIn->readNode(NodeSignal::VALUE) > 0.5)
                    this->stopSimulation();
            }
        }


        void finalize()
        {
        }
    };
}
#endif // SIGNALSTOPSIMULATION_HPP_INCLUDED
