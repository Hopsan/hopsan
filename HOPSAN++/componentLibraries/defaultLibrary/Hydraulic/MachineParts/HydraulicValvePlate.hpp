/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   MechanicValvePlate.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-12
//!
//! @brief Contains a hydraulic valve plate component
//!
//$Id$

#ifndef HYDRAULICVALVEPLATE_HPP_INCLUDED
#define HYDRAULICVALVEPLATE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class HydraulicValvePlate : public ComponentQ
    {

    private:
        Port *mpIn1, *mpP1, *mpPA, *mpPB;
        Port *mpDebug1, *mpDebug2, *mpDebug3;
        size_t mNumPorts1;
        double *mpND_in1;
        std::vector<double*> mvpND_p1, mvpND_q1, mvpND_c1, mvpND_Zc1;
        double *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca;
        double *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb;
        double *mpND_Debug1, *mpND_Debug2, *mpND_Debug3;

        std::vector<double> p1, q1, c1, Zc1;
        std::vector<double> xva,xvb;

        Integrator mIntegrator;

        double phiP, Wg, phi1, phi2, dAlpha, alphaF, th1, th2, R, Rf, rho;
        TurbulentFlowFunction qTurb;

    public:
        static Component *Creator()
        {
            return new HydraulicValvePlate();
        }

        void configure()
        {
            phiP = 160;
            phi1 = 6.0;
            phi2 = 6.0;
            dAlpha = 5.0;
            alphaF = 30;
            th1 = 6;
            th2 = 90;
            rho = 890;
            Rf = 0.03;

            //Register changable parameters to the HOPSAN++ core
            registerParameter("phi_P", "Length of grooves", "[deg]", phiP);
            registerParameter("phi_1", "Length of first pre-compression chamber", "[deg]", phi1);
            registerParameter("phi_2", "Length of second pre-compression chamber", "[deg]", phi2);
            registerParameter("Delta_alpha", "-", "[deg]", dAlpha);
            registerParameter("alpha_f", "-", "[deg]", alphaF);
            registerParameter("R_f", "Radius to groove center line", "[m]", Rf);
            registerParameter("theta_1", "Angle 1", "[deg]", th1);
            registerParameter("theta_2", "Angle 2", "[deg]", th2);
            registerParameter("rho", "Oil Density", "[kg/m^3]", rho);

            //Add ports to the component
            mpIn1 = addReadPort("movement", "NodeSignal");
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpDebug1 = addWritePort("DEBUG1", "NodeSignal", Port::NotRequired);
            mpDebug2 = addWritePort("DEBUG2", "NodeSignal", Port::NotRequired);
            mpDebug3 = addWritePort("DEBUG3", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mvpND_p1.resize(mNumPorts1);
            mvpND_q1.resize(mNumPorts1);
            mvpND_c1.resize(mNumPorts1);
            mvpND_Zc1.resize(mNumPorts1);
            p1.resize(mNumPorts1);
            q1.resize(mNumPorts1);
            c1.resize(mNumPorts1);
            Zc1.resize(mNumPorts1);
            xva.resize(mNumPorts1);
            xvb.resize(mNumPorts1);

            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::Value);

            for(size_t i=0; i<mNumPorts1; ++i)
            {
                mvpND_p1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Pressure);
                mvpND_q1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Flow);
                mvpND_c1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::WaveVariable);
                mvpND_Zc1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::CharImpedance);
            }

            mpND_pa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpND_qa = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpND_ca = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpND_Zca = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            mpND_pb = getSafeNodeDataPtr(mpPB, NodeHydraulic::Pressure);
            mpND_qb = getSafeNodeDataPtr(mpPB, NodeHydraulic::Flow);
            mpND_cb = getSafeNodeDataPtr(mpPB, NodeHydraulic::WaveVariable);
            mpND_Zcb = getSafeNodeDataPtr(mpPB, NodeHydraulic::CharImpedance);

            mpND_Debug1 = getSafeNodeDataPtr(mpDebug1, NodeSignal::Value);
            mpND_Debug2 = getSafeNodeDataPtr(mpDebug2, NodeSignal::Value);
            mpND_Debug3 = getSafeNodeDataPtr(mpDebug3, NodeSignal::Value);

            mIntegrator.initialize(mTimestep, 0, 0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double in1 = (*mpND_in1);
            //double cx1  = (*mpND_cx1);
            //double Zx1 = (*mpND_Zx1);
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                c1[i] = (*mvpND_c1[i]);
                Zc1[i] = (*mvpND_Zc1[i]);
            }
            double ca = (*mpND_ca);
            double Zca = (*mpND_Zca);
            double cb = (*mpND_cb);
            double Zcb = (*mpND_Zcb);

            //Calculate angle
            double a1 = mIntegrator.update(in1);

            //Calculate opening areas
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                double start = i*360.0/mNumPorts1;
                double start2 = start+180;

                xva[i] = groove(a1*180.0/pi, start, phiP, dAlpha, alphaF, phi1, th1, th2, phi2, th1, th2);
                xvb[i] = groove(a1*180.0/pi, start2, phiP, dAlpha, alphaF, phi1, th1, th2, phi2, th1, th2);
            }

            //Print debug output
            (*mpND_Debug1) = xva[0];
            (*mpND_Debug2) = xvb[0];
            (*mpND_Debug3) = xva[2];

            //Calculate flows
            double qa=0;
            double qb=0;

            for(size_t i=0; i<mNumPorts1; ++i)
            {
                q1[i] = 0;
                double Kca = 0.67*/*A**/xva[i]*sqrt(2.0/rho);
                qTurb.setFlowCoefficient(Kca);
                double q1a = qTurb.getFlow(c1[i], ca, Zc1[i], Zca);
                q1[i] -= q1a;
                qa += q1a;
            }

            for(size_t i=0; i<mNumPorts1; ++i)
            {
                double Kcb = 0.67/**A*/*xvb[i]*sqrt(2.0/rho);
                qTurb.setFlowCoefficient(Kcb);
                double q1b = qTurb.getFlow(c1[i], cb, Zc1[i], Zcb);
                q1[i] -= q1b;
                qb += q1b;
                p1[i] = c1[i]+q1[i]*Zc1[i];
            }

            //Calculate pressures
            double pa = ca + qa*Zca;
            double pb = cb + qb*Zcb;

            //Write new values to nodes
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_p1[i]) = p1[i];
                (*mvpND_q1[i]) = q1[i];
            }
            (*mpND_pa) = pa;
            (*mpND_qa) = qa;
            (*mpND_pb) = pb;
            (*mpND_qb) = qb;
        }


//        double groove(double x, double start, double sep, double dAlpha, double precL1, double precW1, double precL2, double precW2)
//        {
//            double ret=0;
//            x = fmod(x-start, 360.0);

//            double min_angle = sep/2.0;
//            double max_angle = 180-sep/2.0;

//            if(x >= min_angle && x <= max_angle)
//            {
//                if(x < min_angle+dAlpha)
//                {
//                    double y = (x-min_angle)/dAlpha;
//                    ret = -(y*y)+2*(y);
//                }
//                else if(x > max_angle-dAlpha)
//                {
//                    double y = (x-max_angle+dAlpha)/dAlpha+1;
//                    ret = -(y*y)+2*(y);
//                }
//                else
//                {
//                    ret = 1;
//                }
//            }

//            if(x > min_angle-precL1 && x < min_angle+precL1)
//            {
//                ret = std::max(ret, (x-min_angle+precL1)/(precL1*2) * precW1);
//            }
//            else if(x > max_angle-precL2 && x < max_angle+precL2)
//            {
//                ret = std::max(ret, precW2 - (x-max_angle+precL2)/(precL2*2) * precW2);
//            }

//            return ret;
//        }


        double groove(double x, double start, double phi, double r, double R, double g1, double th11, double th21, double g2, double th12, double th22)
        {
            double min_angle = 90-phi/2.0-g1-r;

            double ret=0;

            x = fmod(x-start-min_angle, 360);
            x = x/360.0*2*Rf*pi;
            phi = phi/360.0*2*Rf*pi;
            g1 = g1/360.0*2*Rf*pi;
            g2 = g2/360.0*2*Rf*pi;
            r = r/360.0*2*Rf*pi;
            R = R/360.0*2*Rf*pi;
            th11 = th11/180.0*pi;
            th21 = th21/180.0*pi;
            th12 = th12/180.0*pi;
            th22 = th22/180.0*pi;

            double w1 = 2*g1*tan(th11)*tan(th21/2.0);
            double w2 = 2*g2*tan(th12)*tan(th22/2.0);

            double k2 = r*r*pi/2.0;
            double k3 = g1*g1*tan(th11)*tan(th11)*tan(th21/2.0);
            double k5 = g2*g2*tan(th12)*tan(th12)*tan(th22/2.0);

            if(x >= 0 && x <= g1)
            {
                double A1 = x*x/g1*w1/2.0;
                double A2 = x*x*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = std::min(A1, A2);
            }
            else if(x >= g1 && x <= g1+2*r)
            {
                double d = 2*r-x+g1;
                double A = 2*r*r*acos(d/(2.0*r)) - 0.5*d*sqrt(4*r*r-d*d);
                ret = k3 + A;
            }
            else if(x >= g1+2*r && x <= R)
            {
                ret = k3 + k2 + (x-g1-2*r)*2.0*r + k2 ;
            }
            else if(x >= R && x <= R+g1)
            {
                double A1 = (g1+R-x)*(g1+R-x)/g1*w1/2.0;
                double A2 = (g1+R-x)*(g1+R-x)*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = std::min(A1, A2) + k2 + (x-g1-2.0*r)*2.0*r + k2;
            }

            else if(x >= R+g1 && x <= g1+phi)
            {
                ret = k2 + (R-2.0*r)*2.0*r + k2;
            }
            else if(x >= g1+phi && x <= g1+phi+g2)
            {
                double A1 = (x-g1-phi)*(x-g1-phi)/g1*w2/2.0;
                double A2 = (x-g1-phi)*(x-g1-phi)*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = k2 + (R-2*r+phi+g1-x)*2.0*r + k2 + std::min(A1, A2);
            }
            else if(x >= g1+phi+g2 && x <= g1+phi+R-2*r)
            {
                ret = k2 + (R-2*r+phi+g1-x)*2.0*r + k2 + k5;
            }
            else if(x >= g1+phi+R-2*r && x <= g1+phi+R)
            {
                double d = x-g1-phi-R+2*r;
                double A = 2*r*r*acos(d/(2.0*r)) - 0.5*d*sqrt(4*r*r-d*d);
                ret = A + k5;
            }
            else if(x >= g1+phi+R && x <= g1+phi+R+g2)
            {
                double A1 = (R-x+g1+phi+g2)*(R-x+g1+phi+g2)/g1*w2/2.0;
                double A2 = (R-x+g1+phi+g2)*(R-x+g1+phi+g2)*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = std::min(A1, A2);
            }

            return ret;
        }
    };
}

#endif // HYDRAULICVALVEPLATE_HPP_INCLUDED

