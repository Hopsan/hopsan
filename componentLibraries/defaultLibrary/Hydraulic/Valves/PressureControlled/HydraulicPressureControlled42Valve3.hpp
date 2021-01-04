/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicPressureControlled42Valve3.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-04-30
//!
//! @brief Contains a pressure controlled hydraulic 4/2 valve of Q-type with no closed position
//$Id: HydraulicPressureControlled42Valve3.hpp 8490 2015-11-26 08:05:39Z robbr48 $

#ifndef HYDRAULICPRESSURECONTROLLED42VALVE3_HPP_INCLUDED
#define HYDRAULICPRESSURECONTROLLED42VALVE3_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>

using namespace std;
namespace hopsan {

    class HydraulicPressureControlled42Valve3 : public ComponentQ
    {
    private:
        //Constants
        double mOmega_h, mDelta_h;
        
        //Input variable node data pointers
        
        double *mpFs_min, *mpFs_max, *mpC_q, *mpRho, *mpD, *mpF_pa, *mpF_bt, *mpF_pb, *mpF_at, *mpX_vmax;
        
        //Output variable node data pointers
        double *mpX_v;
        
        //Power port pointers
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpPC;
        
        //Power port node data pointers
        double *mpPP_q, *mpPP_p, *mpPP_c, *mpPP_Zc;
        double *mpPT_q, *mpPT_p, *mpPT_c, *mpPT_Zc;
        double *mpPA_q, *mpPA_p, *mpPA_c, *mpPA_Zc;
        double *mpPB_q, *mpPB_p, *mpPB_c, *mpPB_Zc;
        double *mpPC_q, *mpPC_p, *mpPC_c, *mpPC_Zc;
        
        //Transfer function for dynamics of spool position
        SecondOrderTransferFunction mSpoolPosTF;
        
        //Help objects for turbulent flow functions
        TurbulentFlowFunction qTurb_pa,qTurb_at,qTurb_pb,qTurb_bt;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureControlled42Valve3();
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
            addInputVariable("f_pa", "Fraction of spool diameter that is opening P-A", "", 1, &mpF_pa);
            addInputVariable("f_pb", "Fraction of spool diameter that is opening P-B", "", 1, &mpF_pb);
            addInputVariable("f_bt", "Fraction of spool diameter that is opening B-T", "", 1, &mpF_bt);
            addInputVariable("f_at", "Fraction of spool diameter that is opening A-T", "", 1, &mpF_at);
            addInputVariable("x_vmax", "Maximum spool position", "", 0.01, &mpX_vmax);

            //Register output variables
            addOutputVariable("x_v", "Spool position", "m", 0, &mpX_v);
            
            //Add power ports
            mpPP = addPowerPort("PP", "NodeHydraulic", "");
            mpPT = addPowerPort("PT", "NodeHydraulic", "");
            mpPA = addPowerPort("PA", "NodeHydraulic", "");
            mpPB = addPowerPort("PB", "NodeHydraulic", "");
            mpPC = addPowerPort("PC", "NodeHydraulic", "");
        }


        void initialize()
        {
            //Get node data pointers from ports
            mpPP_q = getSafeNodeDataPtr(mpPP, NodeHydraulic::Flow);
            mpPP_p = getSafeNodeDataPtr(mpPP, NodeHydraulic::Pressure);
            mpPP_c = getSafeNodeDataPtr(mpPP, NodeHydraulic::WaveVariable);
            mpPP_Zc = getSafeNodeDataPtr(mpPP, NodeHydraulic::CharImpedance);
            mpPT_q = getSafeNodeDataPtr(mpPT, NodeHydraulic::Flow);
            mpPT_p = getSafeNodeDataPtr(mpPT, NodeHydraulic::Pressure);
            mpPT_c = getSafeNodeDataPtr(mpPT, NodeHydraulic::WaveVariable);
            mpPT_Zc = getSafeNodeDataPtr(mpPT, NodeHydraulic::CharImpedance);
            mpPA_q = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpPA_p = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpPA_c = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpPA_Zc = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);
            mpPB_q = getSafeNodeDataPtr(mpPB, NodeHydraulic::Flow);
            mpPB_p = getSafeNodeDataPtr(mpPB, NodeHydraulic::Pressure);
            mpPB_c = getSafeNodeDataPtr(mpPB, NodeHydraulic::WaveVariable);
            mpPB_Zc = getSafeNodeDataPtr(mpPB, NodeHydraulic::CharImpedance);
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
            double xv;
            double Kcpa, Kcbt, Kcpb, Kcat;
            double qpa, qbt, qpb, qat;
            double Fs_min, Fs_max;
            double pp, qp, cp, Zcp;
            double pt, qt, ct, Zct;
            double pa, qa, ca, Zca;
            double pb, qb, cb, Zcb;
            double pc, qc, cc;
            double Cq, rho, xvmax, d, f_pa, f_bt, f_pb, f_at;
            bool cav = false;
            double xpanom,xpbnom,xatnom,xbtnom;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ct = (*mpPT_c);
            Zct = (*mpPT_Zc);
            ca = (*mpPA_c);
            Zca = (*mpPA_Zc);
            cb = (*mpPB_c);
            Zcb = (*mpPB_Zc);
            cc = (*mpPC_c);
            Fs_min = (*mpFs_min);
            Fs_max = (*mpFs_max);
            Cq = (*mpC_q);
            rho = (*mpRho);
            xvmax = (*mpX_vmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_bt = (*mpF_bt);
            f_pb = (*mpF_pb);
            f_at = (*mpF_at);

            //Dynamics of spool position (second order low pass filter)
            //Min and max functions restricts input between Fs_min and Fs_max
            mSpoolPosTF.update(min(1.0, max(0.0, (cc-Fs_min)/(Fs_max-Fs_min)))*xvmax);
            xv = mSpoolPosTF.value();

            //Determine flow coefficient from spool position
            xpanom = xvmax-xv;
            xpbnom = xv;
            xatnom = xv;
            xbtnom = xvmax-xv;

            //Calculate flow
            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcpb = Cq*f_pb*pi*d*xpbnom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);
            Kcbt = Cq*f_bt*pi*d*xbtnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_pb.setFlowCoefficient(Kcpb);
            qTurb_at.setFlowCoefficient(Kcat);
            qTurb_bt.setFlowCoefficient(Kcbt);
            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
            qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);

            qp = -qpa-qpb;
            qa = qpa-qat;
            qb = -qbt+qpb;
            qt = qat+qbt;

            //TLM equations
            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;
            pb = cb + qb*Zcb;
            pc = cc;
            qc = 0;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
                cav = true;
            }
            if(pb < 0.0)
            {
                cb = 0.0;
                Zcb = 0;
                cav = true;
            }
            if(pp < 0.0)
            {
                cp = 0.0;
                Zcp = 0;
                cav = true;
            }
            if(pt < 0.0)
            {
                ct = 0.0;
                Zct = 0;
                cav = true;
            }

            //Handle cavitation
            if(cav)
            {
	        qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
                qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
                qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
                qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);

                qp = -qpa-qpb;
                qa = qpa-qat;
                qb = -qbt+qpb;
                qt = qat+qbt;
                
                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
                pb = cb + qb*Zcb;
                pc = cc;
                qc = 0;
            }

            //Write new values to nodes
            (*mpPP_p) = pp;
            (*mpPP_q) = qp;
            (*mpPT_p) = pt;
            (*mpPT_q) = qt;
            (*mpPA_p) = pa;
            (*mpPA_q) = qa;
            (*mpPB_p) = pb;
            (*mpPB_q) = qb;
            (*mpPC_p) = pc;
            (*mpPC_q) = qc;
            (*mpX_v) = xv;
        }
    };
}

#endif //HYDRAULICPRESSURECONTROLLED42VALVE2_HPP_INCLUDED


