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
        Port /* *mpIn1, */*mpP1, *mpPA, *mpPB;
        size_t mNumPorts1;
        double *mpMovement;
        std::vector<double*> mvpND_p1, mvpND_q1, mvpND_c1, mvpND_Zc1;
        double *mpND_pa, *mpND_qa, *mpND_ca, *mpND_Zca;
        double *mpND_pb, *mpND_qb, *mpND_cb, *mpND_Zcb;
        double *mpND_Debug1, *mpND_Debug2, *mpND_Debug3;

        std::vector<double> p1, q1, c1, Zc1;
        std::vector<double> xva,xvb;

        Integrator mIntegrator;

        double R, Wg, Rf;
        double *mpPhiP, *mpPhi1, *mpPhi2, *mpDAlpha, *mpAlphaF, *mpTh1, *mpTh2, *mpRf, *mpRho;
        TurbulentFlowFunction qTurb;

    public:
        static Component *Creator()
        {
            return new HydraulicValvePlate();
        }

        void configure()
        {
            //Register changable parameters to the HOPSAN++ core
            addInputVariable("phi_P", "Length of grooves", "deg", 160, &mpPhiP);
            addInputVariable("phi_1", "Length of first pre-compression chamber", "deg", 6, &mpPhi1);
            addInputVariable("phi_2", "Length of second pre-compression chamber", "deg", 6, &mpPhi2);
            addInputVariable("Delta_alpha", "-", "deg", 5, &mpDAlpha);
            addInputVariable("alpha_f", "-", "deg", 30, &mpAlphaF);
            addInputVariable("R_f", "Radius to groove center line", "m", 0.03, &mpRf);
            addInputVariable("theta_1", "Angle 1", "deg", 6, &mpTh1);
            addInputVariable("theta_2", "Angle 2", "deg", 90, &mpTh2);
            addInputVariable("rho", "Oil Density", "kg/m^3", 890, &mpRho);
            addInputVariable("movement", "Movement", "rad/s", 160, &mpMovement);
            addOutputVariable("DEBUG1", "DEBUG1", "");
            addOutputVariable("DEBUG2", "DEBUG1", "");
            addOutputVariable("DEBUG3", "DEBUG1", "");

            //Add ports to the component
            //mpIn1 = addReadPort("movement", "NodeSignal");
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
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

            mpND_Debug1 = getSafeNodeDataPtr("DEBUG1", NodeSignal::Value);
            mpND_Debug2 = getSafeNodeDataPtr("DEBUG2", NodeSignal::Value);
            mpND_Debug3 = getSafeNodeDataPtr("DEBUG3", NodeSignal::Value);

            mIntegrator.initialize(mTimestep, 0, 0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double in1 = (*mpMovement);

            double phiP = (*mpPhiP);
            double phi1 = (*mpPhi1);
            double phi2 = (*mpPhi2);
            double dAlpha = (*mpDAlpha);
            double alphaF = (*mpAlphaF);
            double th1 = (*mpTh1);
            double th2 = (*mpTh2);
            Rf = (*mpRf);
            double rho = (*mpRho);

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
            double temp = 1.0/360.0*2*Rf*pi;
            x = x*temp;
            phi = phi*temp;
            g1 = g1*temp;
            g2 = g2*temp;
            r = r*temp;
            R = R*temp;

            if(x >= 0 && x <= g1)
            {
                th11 = th11/180.0*pi;
                th21 = th21/180.0*pi;
                double w1 = 2*g1*tan(th11)*tan(th21/2.0);
                double A1 = x*x/g1*w1/2.0;
                double A2 = x*x*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = std::min(A1, A2);
            }
            else if(x >= g1 && x <= g1+2*r)
            {
                double k3 = g1*g1*tan(th11)*tan(th11)*tan(th21/2.0);
                double d = 2*r-x+g1;
                double A = 2*r*r*acos(d/(2.0*r)) - 0.5*d*sqrt(4*r*r-d*d);
                ret = k3 + A;
            }
            else if(x >= g1+2*r && x <= R)
            {
                double k2 = r*r*pi/2.0;
                double k3 = g1*g1*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = k3 + k2 + (x-g1-2*r)*2.0*r + k2 ;
            }
            else if(x >= R && x <= R+g1)
            {
                th11 = th11/180.0*pi;
                th21 = th21/180.0*pi;
                double w1 = 2*g1*tan(th11)*tan(th21/2.0);
                double A1 = (g1+R-x)*(g1+R-x)/g1*w1/2.0;
                double A2 = (g1+R-x)*(g1+R-x)*tan(th11)*tan(th11)*tan(th21/2.0);
                double k2 = r*r*pi/2.0;
                ret = std::min(A1, A2) + k2 + (x-g1-2.0*r)*2.0*r + k2;
            }

            else if(x >= R+g1 && x <= g1+phi)
            {
                double k2 = r*r*pi/2.0;
                ret = k2 + (R-2.0*r)*2.0*r + k2;
            }
            else if(x >= g1+phi && x <= g1+phi+g2)
            {
                double k2 = r*r*pi/2.0;
                th11 = th11/180.0*pi;
                th21 = th21/180.0*pi;
                th12 = th12/180.0*pi;
                th22 = th22/180.0*pi;
                double w2 = 2*g2*tan(th12)*tan(th22/2.0);
                double A1 = (x-g1-phi)*(x-g1-phi)/g1*w2/2.0;
                double A2 = (x-g1-phi)*(x-g1-phi)*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = k2 + (R-2*r+phi+g1-x)*2.0*r + k2 + std::min(A1, A2);
            }
            else if(x >= g1+phi+g2 && x <= g1+phi+R-2*r)
            {
                double k2 = r*r*pi/2.0;
                double k5 = g2*g2*tan(th12)*tan(th12)*tan(th22/2.0);
                ret = k2 + (R-2*r+phi+g1-x)*2.0*r + k2 + k5;
            }
            else if(x >= g1+phi+R-2*r && x <= g1+phi+R)
            {
                double k5 = g2*g2*tan(th12)*tan(th12)*tan(th22/2.0);
                double d = x-g1-phi-R+2*r;
                double A = 2*r*r*acos(d/(2.0*r)) - 0.5*d*sqrt(4*r*r-d*d);
                ret = A + k5;
            }
            else if(x >= g1+phi+R && x <= g1+phi+R+g2)
            {
                th11 = th11/180.0*pi;
                th21 = th21/180.0*pi;
                th12 = th12/180.0*pi;
                th22 = th22/180.0*pi;
                double w2 = 2*g2*tan(th12)*tan(th22/2.0);
                double A1 = (R-x+g1+phi+g2)*(R-x+g1+phi+g2)/g1*w2/2.0;
                double A2 = (R-x+g1+phi+g2)*(R-x+g1+phi+g2)*tan(th11)*tan(th11)*tan(th21/2.0);
                ret = std::min(A1, A2);
            }

            return ret;
        }
    };
}

#endif // HYDRAULICVALVEPLATE_HPP_INCLUDED

