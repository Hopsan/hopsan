/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/
//!
//! @file                  Hydraulic22DirectionalCheckValve.hpp
//! @author                Paulo Teixeira
//! @date                  2014-07-01
//!
//! @brief This code is a combination of Hydraulic22DirectionalValve.hpp and HydraulicCheckValvePreLoaded.hpp. Contains a hydraulic on/off check valve of Q-type

#ifndef HYDRAULIC22DIRECTIONALCHECKVALVE_HPP_INCLUDED
#define HYDRAULIC22DIRECTIONALCHECKVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class Hydraulic22DirectionalCheckValve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mValveSpoolPosFilter;
        TurbulentFlowFunction qTurb;
        //Constants
        double mOmega_h, mDelta_h;
        //Input variable node data pointers
        double *mpIn, *mpKv_open, *mpKv_check, *mpF_s;
        //Output variable node data pointers
        double *mpOut;
        //Power port pointers
        Port *mpP1, *mpP2;
        //Power port node data pointers
        double *mpP1_q, *mpP1_p, *mpP1_c, *mpP1_Zc;
        double *mpP2_q, *mpP2_p, *mpP2_c, *mpP2_Zc;

    public:
        static Component *Creator()
        {
            return new Hydraulic22DirectionalCheckValve();
        }

        void configure()
        {
            //Register constant parameters
            addConstant("omega_h", "Resonance Frequency", "Frequency", 100, mOmega_h);
            addConstant("delta_h", "Damping Factor", "-", 1, mDelta_h);
            //Register input variables
            addInputVariable("in", "<0.5 (check), >0.5 (open)", "-", 0, &mpIn);
            addInputVariable("Kv_open", "Pressure-Flow Coefficient in opened position", "(m^3/s)/sqrt(Pa)", 5e-7, &mpKv_open);
            addInputVariable("Kv_check", "Pressure-Flow Coefficient in checked position", "(m^3/s)/sqrt(Pa)", 5e-7, &mpKv_check);
            addInputVariable("F_s", "Spring Pre-Load Tension", "Pa", 0.0, &mpF_s);
            //Register output variables
            addOutputVariable("out", "<0.5 (check), >0.5 (open)", "-", 0, &mpOut);
            //Add power ports
            mpP1 = addPowerPort("P1", "NodeHydraulic", "");
            mpP2 = addPowerPort("P2", "NodeHydraulic", "");
            //Set default power port start values
        }


        void initialize()
        {
            //Get node data pointers from ports
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
            
            //WRITE YOUR INITIALIZATION CODE HERE
            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDelta_h/mOmega_h, 1.0/(mOmega_h*mOmega_h)};
            double initialOut = limit(*mpOut, 0, 1);
            mValveSpoolPosFilter.initialize(mTimestep, num, den, initialOut, initialOut, 0, 1);
            simulateOneTimestep();
        }

        void simulateOneTimestep()
        {
            //Declare local variables
            double in, Kv_open, Kv_check, F_s;
            double out, outnom;
            double P1_q, P1_p, P1_c, P1_Zc, P2_q, P2_p, P2_c, P2_Zc;
            bool cav = false;

            //Read variable values from nodes
            P1_c = (*mpP1_c);
            P1_Zc = (*mpP1_Zc);
            P2_c = (*mpP2_c);
            P2_Zc = (*mpP2_Zc);
            in = (*mpIn);
            Kv_open = (*mpKv_open);
            Kv_check = (*mpKv_check);
            F_s = (*mpF_s);

            //WRITE YOUR EQUATIONS HERE
            if(doubleToBool(in))
            {
                mValveSpoolPosFilter.update(1);
            }
            else
            {
                mValveSpoolPosFilter.update(0);
            }

            out = mValveSpoolPosFilter.value();

            outnom = std::max(out,0.0);

            if(outnom>0.5)
            {
                qTurb.setFlowCoefficient(Kv_open);
                P2_q = qTurb.getFlow(P1_c, P2_c, P1_Zc, P2_Zc);
            }
            else
            {
                qTurb.setFlowCoefficient(Kv_check);
                if(P1_c > P2_c+F_s)
                    {
                    P2_q = qTurb.getFlow(P1_c, P2_c, P1_Zc, P2_Zc);
                    }
                else
                    {
                    P2_q = 0.0;
                    }
            }
            
            P1_q = -P2_q;

            P1_p = P1_c + P1_q*P1_Zc;
            P2_p = P2_c + P2_q*P2_Zc;

            // Cavitation check
            if(P1_p < 0.0)
            {
                P1_c = 0.0;
                P1_Zc = 0;
                cav = true;
            }
            if(P2_p < 0.0)
            {
                P2_c = 0.0;
                P2_Zc = 0;
                cav = true;
            }

            if(cav)
            {
           
                if(outnom>0.5)
                {
                    qTurb.setFlowCoefficient(Kv_open);
                    P2_q = qTurb.getFlow(P1_c, P2_c, P1_Zc, P2_Zc);
                }
                else
                {
                    qTurb.setFlowCoefficient(Kv_check);
                    if(P1_c > P2_c+F_s)
                    {
                        P2_q = qTurb.getFlow(P1_c, P2_c, P1_Zc, P2_Zc);
                    }
                    else
                    {
                        P2_q = 0.0;
                    }
                }

                P1_q = -P2_q;

                P1_p = P1_c + P1_q*P1_Zc;
                P2_p = P2_c + P2_q*P2_Zc;
            }

            //Write new values to nodes
            (*mpP1_q) = P1_q;
            (*mpP1_p) = P1_p;
            (*mpP2_q) = P2_q;
            (*mpP2_p) = P2_p;
            (*mpOut) = outnom;
        }
    };
}

#endif //HYDRAULIC22DIRECTIONALCHECKVALVE_HPP_INCLUDED


