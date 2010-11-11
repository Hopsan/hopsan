//!
//! @file   HydraulicPressureSource.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCE_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureSource : public ComponentC
    {
    private:
        double mZc;
        double mPressure;
        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureSource("PressureSource");
        }

        HydraulicPressureSource(const std::string name) : ComponentC(name)
        {
            mTypeName = "HydraulicPressureSource";
            mPressure       = 1.0e5;
            mZc             = 0.0;

            mpIn = addReadPort("In", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("P", "Default pressure", "Pa", mPressure);

            setStartValue(mpP1, NodeHydraulic::PRESSURE, mPressure);
            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
        }


        void initialize()
        {
            mpP1->writeNode(NodeHydraulic::PRESSURE, mPressure); //Override the startvalue for the pressure
            setStartValue(mpP1, NodeHydraulic::PRESSURE, mPressure); //This is here to show the user that the start value is hard coded!
        }


        void simulateOneTimestep()
        {
            //Pressure source equation
            double c;
            if (mpIn->isConnected())
            {
                c = mpIn->readNode(NodeSignal::VALUE);         //We have a signal!
            }
            else
            {
                c = mPressure;                                  //No signal, use internal parameter
            }

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c);
            mpP1->writeNode(NodeHydraulic::CHARIMP, mZc);
        }
    };




    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOptimizedPressureSource : public ComponentC
    {
    private:
        //double mStartPressure;
        double mStartFlow;
        double mZc;
        double mPressure;
        Port *mpIn, *mpP1;

        double *input, *p, *q, *c, *Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicOptimizedPressureSource("PressureSource");
        }

        HydraulicOptimizedPressureSource(const std::string name) : ComponentC(name)
        {
            mTypeName = "HydraulicOptimizedPressureSource";
            mStartFlow      = 0.0;
            mPressure       = 1.0e5;
            mZc             = 0.0;

            mpIn = addReadPort("In", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeHydraulic");

            registerParameter("P", "Default pressure", "Pa", mPressure);
        }


        void initialize()
        {
            if(mpIn->isConnected())
                input = mpIn->getNodeDataPtr(NodeSignal::VALUE);

            p = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            c = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);

            *p = mPressure;
            *q = mStartFlow;
        }


        void simulateOneTimestep()
        {
            //Pressure source equation
            if (mpIn->isConnected())
            {
                *c = *input;
            }
            else
            {
                *c = mPressure;                                  //No signal, use internal parameter
            }

            *Zc = mZc;
        }
    };
}

#endif // HYDRAULICPRESSURESOURCE_HPP_INCLUDED
