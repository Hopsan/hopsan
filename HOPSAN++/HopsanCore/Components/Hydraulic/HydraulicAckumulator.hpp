//!
//! @file   HydraulicAckumulator.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a Hydraulic Ackumulator component with constant bulk modulus
//!
//$Id$

#ifndef HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED
#define HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicAckumulator : public ComponentQ
    {
    private:
        double mPmin, mVtot, mVoil, mVgas, mBetae, mKappa, mKce, mStartPressure, mStartFlow;
        Delay mDelayedP2, mDelayedC1, mDelayedZc1, mDelayedQ2;
        Port *mpP1, *mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicAckumulator("Ackumulator");
        }

        HydraulicAckumulator(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicAckumulator";
            mPmin                   = 1000000.0;
            mVtot                   = 0.005;
            mVoil                   = 0.0;
            mVgas                   = mVtot-mVoil;
            mBetae                  = 1000000000.0;
            mKappa                  = 1.4;
            mKce                    = 0.0000000001;
            mStartPressure          = 1000000.0;             //Start pressure, must beequal to or higher than pmin
            mStartFlow              = 0.0;                   //Initial flow into the accumulator

            mpP1 = addPowerPort("P1", "NodeHydraulic");     //External port
            mpOut = addWritePort("out", "NodeSignal");     //Internal pressure output

            registerParameter("Pmin", "Minimum Internal Pressure", "Pa", mPmin);
            registerParameter("Vtot", "Total Volume", "m^3", mVtot);
            registerParameter("Betae", "Effective Bulk Modulus", "Pa", mBetae);
            registerParameter("Kappa", "Polytropic Exponent", "-", mKappa);
            registerParameter("Kce", "Flow-Pressure Coefficient", "(m^3/s)/Pa", mKce);
            registerParameter("StartPressure", "Initial Internal Pressure", "Pa", mStartPressure);
            registerParameter("StartFlow", "Initial Flow Into Ackumulator", "m^3/s", mStartFlow);
        }


        void initialize()
        {
            double p1 = mpP1->readNode(NodeHydraulic::PRESSURE);
            double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);

            mpP1->writeNode(NodeHydraulic::PRESSURE, c1);
            mpP1->writeNode(NodeHydraulic::MASSFLOW, 0.0);

            if (mStartPressure < mPmin)         //User has selected an initial pressure lower than the minimum pressure, so use minimum pressure instead
            {
                mStartPressure = mPmin;
                if (mStartPressure < 0)         //User has selected a minimum pressure lower than zero, put it to zero
                {
                    mStartPressure = 0;
                }
                mVgas = mVtot;                  //Pressure is minimum, so ackumulator is empty
                mVoil = 0;
                mDelayedQ2.initialize(mTime, mStartFlow);        //"Previous" value for q2 first step
            }
            else
            {
                mDelayedQ2.initialize(mTime, mKce*(p1-mStartPressure));              //"Previous" value for q2 first step, calculated with orifice equation
                mVgas = pow(mPmin*pow(mVtot, mKappa)/mStartPressure, 1/mKappa);     //Initial gas volume, calculated from initial pressure
                mVoil = mVtot - mVgas;
            }
            mDelayedP2.initialize(mTime, mStartPressure);
            mDelayedC1.initialize(mTime, c1);
            mDelayedZc1.initialize(mTime, Zc1);

            mDelayedP2.setStepDelay(1);
            mDelayedC1.setStepDelay(1);
            mDelayedZc1.setStepDelay(1);
            mDelayedQ2.setStepDelay(1);

            std::cout << "Start Pressure: " << mStartPressure << std::endl;;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);

            //Ackumulator equations
            double e0, ct;
            double p1,q1,q2;
            double p2 = mDelayedP2.value();

            e0 = mPmin * pow(mVtot, mKappa);
            ct = mVgas / (p2*mKappa) + (mVtot-mVgas) / mBetae;
            q2 = (mDelayedQ2.value() * (2*ct*(mKce*mDelayedZc1.value()+1) - mKce*mTimestep) +
                  2*ct*mKce*(c1-mDelayedC1.value())) / (2*ct*(mKce*Zc1+1) + mKce*mTimestep);
            p1 = c1 - Zc1*q2;
            p2 = p1 - q2/mKce;

            if (mTime < 0.05)
            {
                std::cout << "p1 = " << p1 << ", p2 = " << p2 << ", q = " << q2 << ", ct = " << ct << std::endl;
            }


            if (p1 < 0.0)       //Cavitation!
            {
                c1 = 0.0;
                Zc1 = 0.0;
                q2 = (mDelayedQ2.value() * (2*ct*(mKce*mDelayedZc1.value()+1) - mKce*mTimestep) +
                      2*ct*mKce*(c1-mDelayedC1.value())) / (2*ct*(mKce*Zc1+1) + mKce*mTimestep);
                p1 = 0.0;
                p2 = -q2/mKce;
            }

            if (p2 < mPmin)     //Too low pressure (ack cannot be less than empty)
            {
                std::cout << "Empty!" << std::endl;
                mVgas = mVtot;
                q2 = 0.0;
                p1 = c1;
                p2 = mPmin;
            }

            mVgas = pow(e0/p2, 1/mKappa);
            mVoil = mVtot - mVgas;
            q1 = -q2;

            if (mTime < 0.05)
            {
                std::cout << "Voil = " << mVoil << std::endl;
            }

            mDelayedP2.update(p2);
            mDelayedQ2.update(q2);
            mDelayedC1.update(c1);
            mDelayedZc1.update(Zc1);

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
            mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
            if (mpOut->isConnected())
            {
                mpOut->writeNode(NodeSignal::VALUE, mVoil);
            }
        }
    };
}

#endif // HYDRAULICACKUMULATOR_HPP_INCLUDED
