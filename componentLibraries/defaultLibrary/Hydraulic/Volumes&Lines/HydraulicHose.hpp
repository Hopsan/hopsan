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
//! @file   HydraulicHose.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-04
//!
//! @brief Contains a Hydraulic Hose Component
//! Written by Petter Krus 910407
//! Revised 920415
//! Translated to Hopsan NG by Robert Braun 110504
//$Id$

#ifndef HYDRAULICHOSE_HPP_INCLUDED
#define HYDRAULICHOSE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicHose : public ComponentC
    {

    private:

        // Member variables
        double kappa;
        double Rtot, RL1d, RL2d, Rw, a, TL, NT, TN, area, areaC, Zc0, alfa, den, BC0, BC1, AC1;
        double RQ10, RQ20, RQF1D, RQF2D, RQEF1D, RQEF2D, C1F1, C2F1, RQ1, RL1, RQ2, RL2, RL;
        double W1, W2, W3, W4, C1F, C2F, RQF1, RQF2, RQEF1, RQEF2;
        FirstOrderTransferFunction FilterC1F, FilterC2F, FilterC1F1, FilterC2F1;
        double numC1F[2], denC1F[2];
        double numC2F[2], denC2F[2];
        double numC1F1[2], denC1F1[2];
        double numC2F1[2], denC2F1[2];

        // Cyclic memmory
        //! @todo The memory size should not be hardcoded
        int NTIME, NTIME1, NTMAX;
        double c1i[1001];       //1001 because 1000 with starting at 1 (stupid Fortran thing)
        double c2i[1001];

        // Constants
        double betae, wallVisc;

        // Ports and I/O Varibales
        double *mpRho, *mpVisc, *mpD, *mpL;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicHose();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            //Register changeable parameters to the HOPSAN++ core
            addInputVariable("rho",    "Density",                   "kg/m^3", 870.0, &mpRho);
            addInputVariable("eta",    "Dynamic oil viscosity",     "Ns/m^2", 0.03, &mpVisc);
            addInputVariable("d",      "Line diameter",             "m",      0.03, &mpD);
            addInputVariable("l",      "Line length",               "m",      1.0, &mpL);

            addConstant("beta_e", "Bulk modulus",              "Pa",     1e9, betae);
            addConstant("eta_w",  "Equivalent wall viscosity", "Ns/m^2", 0.03, wallVisc);
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

                //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, rho, visc, d, l;

                //Read variables from nodes
            p1 = (*mpP1_p);
            q1 = (*mpP1_q);
            p2 = (*mpP2_p);
            q2 = (*mpP2_q);
            rho = (*mpRho);
            visc = (*mpVisc);
            d = (*mpD);
            l = (*mpL);

            NTMAX = 1000;
            kappa = 1.25;

                // Line resistance
            Rtot = 128.0*visc*l/(pi*d*d*d*d);
            RL1d = Rtot;
            RL2d = Rtot;
            Rw = 128.0*wallVisc*l/(pi*d*d*d*d);

                // Speed of sound in the line
            a=sqrt(betae/rho);

                // Characteristic impedance
            TL = l/a;
            NT = TL/mTimestep+0.5;

            TN = NT*mTimestep;

            area = pi*d*d/4.0;
            areaC = area*TL/TN;
            Zc0= rho*a/areaC;
            if(NT < 1) NT = 1;
            TN = NT*mTimestep;

            alfa  = Rtot/(Zc0*TN);

            den = (2.0*kappa*NT + 1.0);
            BC0 = 1.0/den;
            BC1 = 1.0/den;
            AC1 = -(2.0*kappa*NT - 1.0)/den;

            Zc1 = Zc0 + BC0*Rtot;
            Zc2 = Zc0 + BC0*Rtot;

            for(int i=1; i<NTMAX+1; ++i)
            {
                c1i[i] = p1 + Zc0*q1;
                c2i[i] = p2 + Zc0*q2;
            }

            NTIME=1;

            RQ10 = Rtot*q1;
            RQ20 = Rtot*q2;
            RQF1D = RQ10*(1.0 - BC0);
            RQF2D = RQ20*(1.0 - BC0);
            RQEF1D = 0.0;
            RQEF2D = 0.0;

            C1F1 = p1 + Zc0*q1;
            C2F1 = p2 + Zc0*q2;

            c1 = p1 - Zc1*q1;
            c2 = p2 - Zc2*q2;

            FilterC1F.initialize(mTimestep, numC1F, denC1F, c1i[NTIME], c1i[NTIME]);        //! @todo Not sure if start values for filters are correct
            FilterC2F.initialize(mTimestep, numC2F, denC2F, c2i[NTIME], c2i[NTIME]);
            FilterC1F1.initialize(mTimestep, numC1F1, denC1F1, c1i[NTIME], C1F1);
            FilterC2F1.initialize(mTimestep, numC2F1, denC2F1, c2i[NTIME], C2F1);

            //Write new values to nodes
            (*mpP1_c) = c1;
            (*mpP1_Zc) = Zc1;
            (*mpP2_c) = c2;
            (*mpP2_Zc) = Zc2;
        }


        void simulateOneTimestep()
        {
                //Declare local variables
            double /*p1,*/ q1, c1, Zc1, /*p2,*/ q2, c2, Zc2, rho, visc, d, l;

                //Read variables from nodes
            //p1 = (*mpND_p1);
            q1 = (*mpP1_q);
            c1 = (*mpP1_c);
            //p2 = (*mpND_p2);
            q2 = (*mpP2_q);
            c2 = (*mpP2_c);
            rho = (*mpRho);
            visc = (*mpVisc);
            d = (*mpD);
            l = (*mpL);

                //Cyclic memory
            NTIME = NTIME+1;
            if(NTIME > NTMAX)
            {
                NTIME=1;
            }

            NTIME1 = NTIME-1*NT + 1;
            if(NTIME1 < 1)
            {
                NTIME1 = NTIME1 + NTMAX;
            }

                //-------- Line model -----------
            RQ1 = rq(RL1,q1,d,l,rho,visc);
            RQ2 = rq(RL2,q2,d,l,rho,visc);
            RL1 = (RL1 + RL1d)/2.0;
            RL2 = (RL2 + RL2d)/2.0;
            RL1d = RL1;
            RL2d = RL2;
            RL = (RL1+RL2)/2.0;

            Zc1 = Zc0 + BC0*RL1;
            Zc2 = Zc0 + BC0*RL2;

            c1i[NTIME] = c1 + 2.0*Zc0*q1;
            c2i[NTIME] = c2 + 2.0*Zc0*q2;

                // Low pass filtering of transmitted signals
            W1 = 1.0/(kappa*TN);
            W2 = W1*exp(RL/(2.0*Zc0));
            W3 = Rw/(2.0*Zc0*TN);
            W4 = W3*exp(Rw/(2.0*Zc0));


            numC1F[1] = 1.0/W2;
            numC1F[0] = 1.0;
            denC1F[1] = 1.0/W1;
            denC1F[0] = 1.0;
            FilterC1F.setNumDen(numC1F, denC1F);
            C1F = FilterC1F.update(c1i[NTIME1]);

            numC2F[1] = 1.0/W2;
            numC2F[0] = 1.0;
            denC2F[1] = 1.0/W1;
            denC2F[0] = 1.0;
            FilterC2F.setNumDen(numC2F, denC2F);
            C2F = FilterC2F.update(c2i[NTIME1]);

            numC1F1[1] = 1.0/W4;
            numC1F1[0] = 1.0;
            denC1F1[1] = 1.0/W3;
            denC1F1[0] = 1.0;
            FilterC1F1.setNumDen(numC1F1, denC1F1);
            C1F1 = FilterC1F1.update(C1F);

            numC2F1[1] = 1.0/W4;
            numC2F1[0] = 1.0;
            denC2F1[1] = 1.0/W3;
            denC2F1[0] = 1.0;
            FilterC2F1.setNumDen(numC2F1, denC2F1);
            C2F1 = FilterC2F1.update(C2F);

            RQF1D = RQF1D + BC0*RL1*q1;
            RQF2D = RQF2D + BC0*RL2*q2;

            RQF1 = -AC1*RQF1D + BC1*RL1*q1;
            RQF2 = -AC1*RQF2D + BC1*RL2*q2;

            RQF1D = RQF1;
            RQF2D = RQF2;

            RQEF1 = -AC1*RQEF1D + (BC0+BC1)*(RQ1-RL1*q1);
            RQEF2 = -AC1*RQEF2D + (BC0+BC1)*(RQ2-RL2*q2);

            RQEF1D = RQEF1;
            RQEF2D = RQEF2;

                // Calculation of characteristics in N1 and N2.
            c1 = C2F1 + RQF1 + BC0*(-RL1*q1) + RQEF1;
            c2 = C1F1 + RQF2 + BC0*(-RL2*q2) + RQEF2;

                // Write new values to nodes
            (*mpP1_c) = c1;
            (*mpP1_Zc) = Zc1;
            (*mpP2_c) = c2;
            (*mpP2_Zc) = Zc2;

        }


            //! @brief Subroutine for RQ-factorization
            //! Translated from old Hopsan with only syntax changes
        double rq(double &RL, double Q, double D, double L, double rho, double visc)
        {
            double RE,F;
            double RECRIT = 2300;
            double RQ;

            RE = fabs(4*rho*Q/(pi*D*visc));

            if(RE < RECRIT)
            {
              RL=128*visc*L/(pi*D*D*D*D);
              RQ=RL*Q;
            }
            else
            {
              F = 0.079*pow(RE,-0.25);
              RL= 1.75*32*rho*F*L*fabs(Q)/(pi*pi*D*D*D*D*D);
              RQ = 32*rho*F*L*fabs(Q)*Q/(pi*pi*D*D*D*D*D);
            }

            return RQ;
        }
    };
}

#endif // HYDRAULICHOSE_HPP_INCLUDED

