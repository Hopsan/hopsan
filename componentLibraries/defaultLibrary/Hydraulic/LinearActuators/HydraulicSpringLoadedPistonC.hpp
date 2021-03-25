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
//! @file   HydraulicSpringLoadedPistonC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-05-07
//!
//! @brief Contains a Hydraulic Spring-Loaded Piston of C type
//!
//$Id$

#ifndef HYDRAULICSPRINGLOADEDPISTONC_HPP_INCLUDED
#define HYDRAULICSPRINGLOADEDPISTONC_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicSpringLoadedPistonC : public ComponentC
{
    private:
        double CxLim, ZxLim, wfak, alpha;
        bool mUseEndStops;
        
        double ci1, cl1, ci2, cl2;  //Members because old value need to be remembered (c1 and c2 are remembered through nodes)
        double mNum[2];
        double mDen[2];

        //Node data pointers
        std::vector<double*> mvpND_p1, mvpND_q1, mvpND_c1, mvpND_Zc1;
        std::vector<double*> mvpND_p2, mvpND_q2, mvpND_c2, mvpND_Zc2;
        double *mpA1, *mpA2, *mpSl, *mpV01, *mpV02, *mpBp, *mpBetae, *mpCLeak, *mpFs;

        double *mpf3, *mpx3, *mpv3, *mpc3, *mpZx3, *mpme;
        size_t mNumPorts1, mNumPorts2;

        //Ports
        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicSpringLoadedPistonC();
        }

        void configure()
        {
            //Set member attributes
            wfak = 0.1;
            alpha = 0.1;

            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");
            mpP2 = addPowerMultiPort("P2", "NodeHydraulic", "");
            mpP3 = addPowerPort("P3", "NodeMechanic");

            // Add constant parameters
            addConstant("use_sl", "Use end stops (stroke limitation)", "", true, mUseEndStops);

            //Register changeable parameters to the HOPSAN++ core
            addInputVariable("A_1", "Piston area 1", "m^2", 0.001, &mpA1);
            addInputVariable("A_2", "Piston area 2", "m^2", 0.001, &mpA2);
            addInputVariable("s_l", "Stroke", "m", 1.0, &mpSl);
            addInputVariable("F_s", "Spring force", "N", 1000.0, &mpFs);
            addInputVariable("V_1", "Dead volume in chamber 1", "m^3", 0.0003, &mpV01);
            addInputVariable("V_2", "Dead volume in chamber 2", "m^3", 0.0003, &mpV02);
            addInputVariable("B_p", "Viscous friction", "Ns/m", 1000.0, &mpBp);
            addInputVariable("Beta_e", "Bulk modulus", "Pa", 1000000000.0, &mpBetae);
            addInputVariable("c_leak", "Leakage coefficient", "LeakageCoefficient", 0.00000000001, &mpCLeak);
        }


        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mNumPorts2 = mpP2->getNumPorts();

            mvpND_p1.resize(mNumPorts1);
            mvpND_q1.resize(mNumPorts1);
            mvpND_c1.resize(mNumPorts1);
            mvpND_Zc1.resize(mNumPorts1);

            mvpND_p2.resize(mNumPorts2);
            mvpND_q2.resize(mNumPorts2);
            mvpND_c2.resize(mNumPorts2);
            mvpND_Zc2.resize(mNumPorts2);

            double A1 = (*mpA1);
            double A2 = (*mpA2);
            double sl = (*mpSl);
            double V01 = (*mpV01);
            double V02 = (*mpV02);
            double bp = (*mpBp);
            double betae = (*mpBetae);
            double cLeak = (*mpCLeak);

            //Assign node data pointers
            for (size_t i=0; i<mNumPorts1; ++i)
            {

                mvpND_p1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Pressure, 0.0);
                mvpND_q1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Flow, 0.0);
                mvpND_c1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::WaveVariable, 0.0);
                mvpND_Zc1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpND_p1[i] = getDefaultStartValue(mpP1, NodeHydraulic::Pressure);
                *mvpND_q1[i] = getDefaultStartValue(mpP1, NodeHydraulic::Flow)/double(mNumPorts1);
                *mvpND_c1[i] = getDefaultStartValue(mpP1, NodeHydraulic::Pressure);
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                mvpND_p2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::Pressure, 0.0);
                mvpND_q2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::Flow, 0.0);
                mvpND_c2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::WaveVariable, 0.0);
                mvpND_Zc2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpND_p2[i] = getDefaultStartValue(mpP2, NodeHydraulic::Pressure);
                *mvpND_q2[i] = getDefaultStartValue(mpP2, NodeHydraulic::Flow)/double(mNumPorts2);
                *mvpND_c2[i] = getDefaultStartValue(mpP2, NodeHydraulic::Pressure);
            }
            mpf3 = getSafeNodeDataPtr(mpP3, NodeMechanic::Force);
            mpx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::Position);
            mpv3 = getSafeNodeDataPtr(mpP3, NodeMechanic::Velocity);
            mpc3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WaveVariable);
            mpZx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CharImpedance);
            mpme = getSafeNodeDataPtr(mpP3, NodeMechanic::EquivalentMass);

            //Declare local variables;
            double p1, p2, x3, v3;
            double Zc1, Zc2, c3, Zx3;
            double qi1, qi2, V1, V2, qLeak, V1min, V2min;

            //Read variables from nodes
            p1 = (*mvpND_p1[0]);
            p2 = (*mvpND_p2[0]);
            x3 = (*mpx3);
            v3 = (*mpv3);

            //Size of volumes
            V1 = V01+A1*(-x3);
            V2 = V02+A2*(sl+x3);
            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*1.0); //me is not written to node yet.
            V2min = betae*mTimestep*mTimestep*A2*A2/(wfak*1.0);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;

            Zc1 = (double(mNumPorts1)+2.0) / 2.0 * betae/V1*mTimestep/(1.0-alpha);    //Number of ports in volume is 2 internal plus the external ones
            Zc2 = (double(mNumPorts2)+2.0) / 2.0 * betae/V2*mTimestep/(1.0-alpha);
            Zx3 = A1*A1*Zc1 +A2*A2*Zc2 + bp;

            //Internal flows
            qi1 = v3*A1;
            qi2 = -v3*A2;

            ci1 = p1 + Zc1*qi1;
            ci2 = p2 + Zc2*qi2;

            //Leakage flow
            qLeak = cLeak*(p1-p2);

            cl1 = p1 + Zc1*(-qLeak);
            cl2 = p2 + Zc2*qLeak;

            c3 = A1*ci1 - A2*ci2;

            //Write to nodes
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                *(mvpND_c1[i]) = p1 + Zc1*(*mvpND_q1[i]);
                *(mvpND_Zc1[i]) = Zc1;
            }
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                *(mvpND_c2[i]) = p2 + Zc2*(*mvpND_q2[i]);
                *(mvpND_Zc2[i]) = Zc2;
            }
            (*mpc3) = c3;
            (*mpZx3) = Zx3;
        }

        void simulateOneTimestep()
        {
            //Declare local variables;
            double V1, V2, qLeak, qi1, qi2, p1mean, p2mean, V1min, V2min, CxLim, ZxLim;;

            //Read variables from nodes
            double Zc1 = (*mvpND_Zc1[0]);          //All Zc should be the same and Q components shall
            double Zc2 = (*mvpND_Zc2[0]);          //never touch them, so let's just use first value
            double x3 = (*mpx3);
            double v3 = (*mpv3);
            double me = (*mpme);

            double A1 = (*mpA1);
            double A2 = (*mpA2);
            double sl = (*mpSl);
            double V01 = (*mpV01);
            double V02 = (*mpV02);
            double bp = (*mpBp);
            double betae = (*mpBetae);
            double cLeak = (*mpCLeak);
            double Fs = (*mpFs);

            //Leakage flow
            qLeak = cLeak*(cl1-cl2)/(1.0+cLeak*(Zc1+Zc2));

            //Internal flows
            qi1 = v3*A1;
            qi2 = -v3*A2;

            //Size of volumes
            V1 = V01+A1*(-x3);
            V2 = V02+A2*(sl+x3);
            if(me <= 0)        //Me must be bigger than zero
            {
                addDebugMessage("Me = "+to_hstring(me));

                //! @todo what the heck is this all about?
                if(mTime > mTimestep*1.5)
                {
                    addErrorMessage("The equivalent mass 'me' has to be greater than 0.");
                    stopSimulation();
                }
                else        //Don't check first time step, because C is executed before Q and Q may not have written me during initialization
                {
                    addWarningMessage("Equivalent mass 'me' not initialized to a value greater than 0.");
                    me = 1;
                }
            }

            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*me);
            V2min = betae*mTimestep*mTimestep*A2*A2/(wfak*me);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;

            // Cylinder volumes are modelled the same way as the multiport volume:
            //   c1 = Wave variable for external port
            //   ci1 = Wave variable for internal (mechanical) port
            //   cl1 = Wave variable for leakage port

            //Volume 1
            Zc1 = (double(mNumPorts1)+2.0) / 2.0 * betae/V1*mTimestep/(1.0-alpha);    //Number of ports in volume is 2 internal plus the external ones
            p1mean = (ci1 + Zc1*2.0*qi1) + (cl1 + Zc1*2.0*(-qLeak));
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                p1mean += (*mvpND_c1[i]) + 2.0*Zc1*(*mvpND_q1[i]);
            }
            p1mean = p1mean/(double(mNumPorts1)+2.0);
            ci1 = std::max(0.0, alpha * ci1 + (1.0 - alpha)*(p1mean*2.0 - ci1 - 2.0*Zc1*qi1));
            cl1 = std::max(0.0, alpha * cl1 + (1.0 - alpha)*(p1mean*2.0 - cl1 - 2.0*Zc1*(-qLeak)));

            //Volume 2
            Zc2 = ((double(mNumPorts2)+2.0) / 2.0) * betae/V2*mTimestep/(1.0-alpha);
            p2mean = (ci2 + Zc2*2.0*qi2) + (cl2 + Zc2*2.0*qLeak);
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                p2mean += (*mvpND_c2[i]) + 2.0*Zc2*(*mvpND_q2[i]);
            }
            p2mean = p2mean/(double(mNumPorts2)+2.0);
            ci2 = std::max(0.0, alpha * ci2 + (1.0 - alpha)*(p2mean*2.0 - ci2 - 2.0*Zc2*qi2));
            cl2 = std::max(0.0, alpha * cl2 + (1.0 - alpha)*(p2mean*2.0 - cl2 - 2.0*Zc2*qLeak));

            // Add extra force and Zc in end positions to simulate stop.
            // Stops could also be implemented in the mass component (connected Q-component)
            if (mUseEndStops)
            {
                limitStroke(CxLim, ZxLim, x3, v3, me, sl);
            }
            else
            {
                CxLim = 0;
                ZxLim = 0;
            }

            //Internal mechanical port
            double c3 = A1*ci1 - A2*ci2 + CxLim;
            double Zx3 = A1*A1*Zc1 + A2*A2*Zc2 + bp + ZxLim;
            //! @note End of stroke limitation currently turned off, because the piston gets stuck in the end position.
            //! @todo Either implement a working limitation, or remove it completely. It works just as well to have it in the mass component.

            //Write to nodes
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                *(mvpND_c1[i]) = std::max(0.0, alpha * (*mvpND_c1[i]) + (1.0 - alpha)*(p1mean*2 - (*mvpND_c1[i]) - 2*Zc1*(*mvpND_q1[i])));
                *(mvpND_Zc1[i]) = Zc1;
            }
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                *(mvpND_c2[i]) = std::max(0.0, alpha * (*mvpND_c2[i]) + (1.0 - alpha)*(p2mean*2 - (*mvpND_c2[i]) - 2*Zc2*(*mvpND_q2[i])));
                *(mvpND_Zc2[i]) = Zc2;
            }
            (*mpc3) = c3-Fs;
            (*mpZx3) = Zx3;
        }
        
        void limitStroke(double &CxLim, double &ZxLim, double x3, double v3, double me, double sl)
        {
            double FxLim=0.0, ZxLim0, NewCxLim, alfa;

            alfa = 0.5;

            //Equations
            if (-x3 > sl)
            {
                ZxLim0 = wfak*me/mTimestep;
                ZxLim = ZxLim0/(1.0 - alfa);
                FxLim = ZxLim0 * (x3 + sl) / mTimestep;
                NewCxLim = FxLim + ZxLim*v3;
            }
            else if (-x3 < 0.0)
            {
                ZxLim0 = wfak*me/mTimestep;
                ZxLim = ZxLim0/(1.0 - alfa);
                FxLim = ZxLim0*x3/mTimestep;
                NewCxLim = FxLim + ZxLim*v3;
            }
            else
            {
                ZxLim = 0.0;
                CxLim = 0.0;
                NewCxLim = FxLim + ZxLim*v3;
            }

            // Filtering of the characteristics
            CxLim = alfa * CxLim + (1.0 - alfa) * NewCxLim;
        }
    };
}

#endif


