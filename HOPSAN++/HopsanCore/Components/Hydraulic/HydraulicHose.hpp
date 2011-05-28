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

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicHose : public ComponentC
    {

    private:

        double betae, rho, visc, wallVisc, d, l;
        double NTMAX, kappa;
        int NTIME, NTIME1;
        double Rtot, RL1d, RL2d, Rw, a, TL, NT, TN, area, areaC, Zc0, alfa, den, BC0, BC1, AC1;
        double RQ10, RQ20, RQF1D, RQF2D, RQEF1D, RQEF2D, C1F1, C2F1, RQ1, RL1, RQ2, RL2, RL;
        double W1, W2, W3, W4, C1F, C2F, RQF1, RQF2, RQEF1, RQEF2;
        double c1i[1001];       //1001 because 1000 with starting at 1 (stupid Fortran thing)
        double c2i[1001];
        FirstOrderFilter FilterC1F, FilterC2F, FilterC1F1, FilterC2F1;
        double numC1F[2], denC1F[2];
        double numC2F[2], denC2F[2];
        double numC1F1[2], denC1F1[2];
        double numC2F1[2], denC2F1[2];

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicHose("Hose");
        }

        HydraulicHose(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            betae = 1e9;
            rho = 870;
            visc = 0.03;
            wallVisc = 0.03;
            d = 0.03;
            l = 1;

            //pi = 3.1415926;
            NTMAX = 1000;
            kappa = 1.25;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("beta_e", "Bulk modulus",              "[Pa]",     betae);
            registerParameter("rho",    "Density",                   "[kg/m^3]", rho);
            registerParameter("eta",    "Dynamic oil viscosity",     "[Ns/m^2]", visc);
            registerParameter("eta_w",  "Equivalent wall viscosity", "[Ns/m^2]", wallVisc);
            registerParameter("d",      "Line diameter",             "[m]",      d);
            registerParameter("l",      "Line length",               "[m]",      l);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

                //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

                //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);

                // Line resistance
            Rtot = 128*visc*l/(pi*d*d*d*d);
            RL1d = Rtot;
            RL2d = Rtot;
            Rw = 128*wallVisc*l/(pi*d*d*d*d);

                // Speed of sound in the line
            a=sqrt(betae/rho);

                // Characteristic impedance
            TL = l/a;
            NT = TL/mTimestep+0.5;

            TN = NT*mTimestep;

            area = pi*d*d/4;
            areaC = area*TL/TN;
            Zc0= rho*a/areaC;
            if(NT < 1) NT = 1;
            TN = NT*mTimestep;

            alfa  = Rtot/(Zc0*TN);

            den = (2*kappa*NT + 1);
            BC0 = 1/den;
            BC1 = 1/den;
            AC1 = -(2*kappa*NT - 1)/den;

            Zc1 = Zc0 + BC0*Rtot;
            Zc2 = Zc0 + BC0*Rtot;

            for(size_t i=1; i<NTMAX+1; ++i)
            {
                c1i[i] = p1 + Zc0*q1;
                c2i[i] = p2 + Zc0*q2;
            }

            NTIME=1;

            RQ10 = Rtot*q1;
            RQ20 = Rtot*q2;
            RQF1D = RQ10*(1 - BC0);
            RQF2D = RQ20*(1 - BC0);
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
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
        }


        void simulateOneTimestep()
        {
                //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

                //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);

                //Cyclic memory
            NTIME = NTIME+1;
            if(NTIME > NTMAX) NTIME=1;

            NTIME1 = NTIME-1*NT + 1;
            if(NTIME1 < 1) NTIME1 = NTIME1 + NTMAX;

                //-------- Line modell -----------
            RQ1 = rq(RL1,q1,d,l,rho,visc);
            RQ2 = rq(RL2,q2,d,l,rho,visc);
            RL1 = (RL1 + RL1d)/2;
            RL2 = (RL2 + RL2d)/2;
            RL1d = RL1;
            RL2d = RL2;
            RL = (RL1+RL2)/2;

            Zc1 = Zc0 + BC0*RL1;
            Zc2 = Zc0 + BC0*RL2;

            c1i[NTIME] = c1 + 2*Zc0*q1;
            c2i[NTIME] = c2 + 2*Zc0*q2;

                // Low pass filtering of transmitted signals
            W1 = 1/(kappa*TN);
            W2 = W1*exp(RL/(2*Zc0));
            W3 = Rw/(2*Zc0*TN);
            W4 = W3*exp(Rw/(2*Zc0));


            numC1F[0] = 1/W2;
            numC1F[1] = 1;
            denC1F[0] = 1/W1;
            denC1F[1] = 1;
            FilterC1F.setNumDen(numC1F, denC1F);
            C1F = FilterC1F.update(c1i[NTIME1]);

            numC2F[0] = 1/W2;
            numC2F[1] = 1;
            denC2F[0] = 1/W1;
            denC2F[1] = 1;
            FilterC2F.setNumDen(numC2F, denC2F);
            C2F = FilterC2F.update(c2i[NTIME1]);

            numC1F1[0] = 1/W4;
            numC1F1[1] = 1;
            denC1F1[0] = 1/W3;
            denC1F1[1] = 1;
            FilterC1F1.setNumDen(numC1F1, denC1F1);
            C1F1 = FilterC1F1.update(C1F);

            numC2F1[0] = 1/W4;
            numC2F1[1] = 1;
            denC2F1[0] = 1/W3;
            denC2F1[1] = 1;
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

                // Calculation of caracteristics in N1 and N2.
            c1 = C2F1 + RQF1 + BC0*(-RL1*q1) + RQEF1;
            c2 = C1F1 + RQF2 + BC0*(-RL2*q2) + RQEF2;

                // Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;

        }


            //! @brief Subroutine for RQ-factorization
            //! Translated from old Hopsan with only syntax changes
        double rq(double &RL, double Q, double D, double L, double rho, double visc)
        {
            double PI = 3.14159;
            double RE,F;
            double RECRIT = 2300;
            double RQ;

            RE = fabs(4*rho*Q/(PI*D*visc));

            if(RE < RECRIT)
            {
              RL=128*visc*L/(PI*D*D*D*D);
              RQ=RL*Q;
            }
            else
            {
              F = 0.079*pow(RE,-0.25);
              RL= 1.75*32*rho*F*L*fabs(Q)/(PI*PI*D*D*D*D*D);
              RQ = 32*rho*F*L*fabs(Q)*Q/(PI*PI*D*D*D*D*D);
            }

            return RQ;
        }
    };
}

#endif // HYDRAULICHOSE_HPP_INCLUDED

