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
//! @file   HydraulicPressureControlled42Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-04-30
//!
//! @brief Contains a pressure controlled hydraulic 2/2 valve of Q-type with default position closed
//$Id: HydraulicPressureControlled42Valve.hpp 8139 2015-06-17 12:30:13Z petno25 $

#ifndef HYDRAULICPRESSURECONTROLLED22VALVE_HPP_INCLUDED
#define HYDRAULICPRESSURECONTROLLED22VALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>

using namespace std;
namespace hopsan {

    class HydraulicPressureControlled22Valve : public ComponentQ
    {
    private:
        //Constants
        double mOmega_h, mDelta_h;

        //Input variable node data pointers

        double *mpFs_min, *mpFs_max, *mpC_q, *mpRho, *mpD, *mpF_pa, *mpF_bt, *mpX_vmax;

        //Output variable node data pointers
        double *mpX_v;

        //Power port pointers
        Port *mpPP, *mpPA,*mpPC;

        //Power port node data pointers
        double *mpPP_q, *mpPP_p, *mpPP_c, *mpPP_Zc;
        double *mpPA_q, *mpPA_p, *mpPA_c, *mpPA_Zc;
        double *mpPC_q, *mpPC_p, *mpPC_c, *mpPC_Zc;

        //Transfer function for dynamics of spool position
        SecondOrderTransferFunction mSpoolPosTF;

        //Help objects for turbulent flow functions
        TurbulentFlowFunction qTurb_pa;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureControlled22Valve();
        }

        void configure()
        {
            //Register constant parameters
            addConstant("omega_h", "Resonance frequency", "Frequency", 100, mOmega_h);
            addConstant("delta_h", "Damping factor", "-", 1, mDelta_h);

            //Register input variables
            addInputVariable("Fs_min", "Minimum pressure for opening the valve", "Pa", 100000, &mpFs_min);
            addInputVariable("Fs_max", "Pressure for fully opening the valve", "Pa", 1000000, &mpFs_max);
            addInputVariable("C_q", "Flow coefficient", "-", 0.67, &mpC_q);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
            addInputVariable("d", "Spool diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool diameter that is opening P-A ", "", 1, &mpF_pa);
            addInputVariable("f_bt", "Fraction of spool diameter that is opening B-T", "", 1, &mpF_bt);
            addInputVariable("x_vmax", "Maximum spool position", "", 0.01, &mpX_vmax);

            //Register output variables
            addOutputVariable("x_v", "Spool position", "m", 0, &mpX_v);

            //Add power ports
            mpPP = addPowerPort("PP", "NodeHydraulic", "");
            mpPA = addPowerPort("PA", "NodeHydraulic", "");
            mpPC = addPowerPort("PC", "NodeHydraulic", "");
        }


        void initialize()
        {
            //Get node data pointers from ports
            mpPP_q = getSafeNodeDataPtr(mpPP, NodeHydraulic::Flow);
            mpPP_p = getSafeNodeDataPtr(mpPP, NodeHydraulic::Pressure);
            mpPP_c = getSafeNodeDataPtr(mpPP, NodeHydraulic::WaveVariable);
            mpPP_Zc = getSafeNodeDataPtr(mpPP, NodeHydraulic::CharImpedance);
            mpPA_q = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpPA_p = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpPA_c = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpPA_Zc = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);
            mpPC_q = getSafeNodeDataPtr(mpPC, NodeHydraulic::Flow);
            mpPC_p = getSafeNodeDataPtr(mpPC, NodeHydraulic::Pressure);
            mpPC_c = getSafeNodeDataPtr(mpPC, NodeHydraulic::WaveVariable);
            mpPC_Zc = getSafeNodeDataPtr(mpPC, NodeHydraulic::CharImpedance);

             //Initiate transfer function (second order low-pass filter)
            double num[3] = {1.0, 0.0, 0.0};    //Numinator
            double den[3] = {1.0, 2.0*mDelta_h/mOmega_h, 1.0/(mOmega_h*mOmega_h)};    //Denominator
            //Arguments = timestep, numinator, denominator, initial input value, initial output value, minimum output value, maximum output value
            mSpoolPosTF.initialize(mTimestep, num, den, 0, 0, 0, (*mpX_vmax));
        }


        void simulateOneTimestep()
        {
             //Declare local variables
            double xv, Kcpa, qpa, Fs_min, Fs_max;
            double pp, qp, cp, Zcp, pa, qa, ca, Zca, pc, qc, cc, Cq, rho, xvmax, d, f_pa;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ca = (*mpPA_c);
            Zca = (*mpPA_Zc);
            cc = (*mpPC_c);
            Fs_min = (*mpFs_min);
            Fs_max = (*mpFs_max);
            Cq = (*mpC_q);
            rho = (*mpRho);
            xvmax = (*mpX_vmax);
            d = (*mpD);
            f_pa = (*mpF_pa);

            //Dynamics of spool position (second order low pass filter)
            //Min and max functions restricts input between Fs_min and Fs_max
            mSpoolPosTF.update(min(1.0, max(0.0, (cc-Fs_min)/(Fs_max-Fs_min)))*xvmax);
            xv = mSpoolPosTF.value();

            //Determine flow coefficient from spool position
            Kcpa = Cq*f_pa*pi*d*xv*sqrt(2.0/rho);

            //Calculate flow
            qTurb_pa.setFlowCoefficient(Kcpa);
            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);

            qp = -qpa;
            qa = qpa;

            //TLM equations
            pp = cp + qp*Zcp;
            pa = ca + qa*Zca;
            pc = cc;
            qc = 0;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
                cav = true;
            }
            if(pp < 0.0)
            {
                cp = 0.0;
                Zcp = 0;
                cav = true;
            }

            //Handle cavitation
            if(cav)
            {
                qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);

                if (xv >= 0.0)
                {
                    qp = -qpa;
                    qa = qpa;
                }
                else
                {
                    qp = 0;
                    qa = 0;
                }

                pp = cp + qp*Zcp;
                pa = ca + qa*Zca;
                pc = cc;
                qc = 0;
            }

            //Write new values to nodes
            (*mpPP_p) = pp;
            (*mpPP_q) = qp;
            (*mpPA_p) = pa;
            (*mpPA_q) = qa;
            (*mpPC_p) = pc;
            (*mpPC_q) = qc;
            (*mpX_v) = xv;
        }
    };
}

#endif //HYDRAULICPRESSURECONTROLLED22VALVE_HPP_INCLUDED


