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
//! @file   Hydraulic22PoppetValve.hpp
//! @author Bjorn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-08-26
//!
//! @brief Contains a hydraulic 2/2 poppet valve of Q-type
//$Id$

#ifndef HYDRAULIC22POPPET_HPP_INCLUDED
#define HYDRAULIC22POPPET_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 2/2 poppet of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22PoppetValve : public ComponentQ
    {
    private:
        // Member variables
        IntegratorLimited xIntegrator;
        TurbulentFlowFunction qTurb_pASAN;
        TurbulentFlowFunction qTurb_AC;
        double mAN, mAS, mAC;

        // Port and node data pointers
        Port *mpAN, *mpAS, *mpAC;
        double *mpAN_p, *mpAN_q, *mpAN_c, *mpAN_Zc,
               *mpAS_p, *mpAS_q, *mpAS_c, *mpAS_Zc,
               *mpAC_p, *mpAC_q, *mpAC_c, *mpAC_Zc;
        double *mpCq, *mpF, *mpRho, *mpD1, *mpD2, *mpK, *mpF0, *mpXvout;

        // Constants
        double mDd, mXvmax;


    public:
        static Component *Creator()
        {
            return new Hydraulic22PoppetValve();
        }

        void configure()
        {
            mpAN = addPowerPort("PN", "NodeHydraulic");
            mpAS = addPowerPort("PS", "NodeHydraulic");
            mpAC = addPowerPort("PC", "NodeHydraulic");

            addOutputVariable("xv_out", "Spool position", "", 0.0, &mpXvout);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
            addInputVariable("d_1", "Small diameter", "m", 10e-3, &mpD1);
            addInputVariable("d_2", "Big diameter", "m", 10e-3, &mpD2);
            addInputVariable("k", "Spring constant", "N/m", 1e4, &mpK);
            addInputVariable("F_0", "Spring pre-load", "N", 100.0, &mpF0);
            addInputVariable("f", "Fraction of poppet diameter that is opening", "-", 1.0, &mpF);

            addConstant("x_vmax", "Maximum Spool Displacement", "m", 0.01, mXvmax);
            addConstant("d_d", "Damp orifice diam.", "m", 0.1e-3, mDd);
        }


        void initialize()
        {
            mpAN_p = getSafeNodeDataPtr(mpAN, NodeHydraulic::Pressure);
            mpAN_q = getSafeNodeDataPtr(mpAN, NodeHydraulic::Flow);
            mpAN_c = getSafeNodeDataPtr(mpAN, NodeHydraulic::WaveVariable);
            mpAN_Zc = getSafeNodeDataPtr(mpAN, NodeHydraulic::CharImpedance);

            mpAS_p = getSafeNodeDataPtr(mpAS, NodeHydraulic::Pressure);
            mpAS_q = getSafeNodeDataPtr(mpAS, NodeHydraulic::Flow);
            mpAS_c = getSafeNodeDataPtr(mpAS, NodeHydraulic::WaveVariable);
            mpAS_Zc = getSafeNodeDataPtr(mpAS, NodeHydraulic::CharImpedance);

            mpAC_p = getSafeNodeDataPtr(mpAC, NodeHydraulic::Pressure);
            mpAC_q = getSafeNodeDataPtr(mpAC, NodeHydraulic::Flow);
            mpAC_c = getSafeNodeDataPtr(mpAC, NodeHydraulic::WaveVariable);
            mpAC_Zc = getSafeNodeDataPtr(mpAC, NodeHydraulic::CharImpedance);

            double rho = (*mpRho);
            //double frac_d = (*mpF);
            double d1 = (*mpD1);
            double d2 = (*mpD2);
            double Cq = (*mpCq);

            mAN = pi*d1*d1/4.0;
            mAC = pi*d2*d2/4.0;
            mAS = mAC - mAN;

            xIntegrator.initialize(mTimestep, 0.0, limit(*mpXvout,0.0,mXvmax), 0.0, mXvmax);

            //double K21 = Cq*pi*d1*frac_d*0.0*sqrt(2.0/rho); //Main flow coeff.
            double K3  = Cq*pi*mDd*mDd/4.0*sqrt(2.0/rho); //Damping orifice flow coeff.
            qTurb_AC.setFlowCoefficient(K3);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double cAN, ZcAN, pAN, qAN,
                   cAS, ZcAS, pAS, qAS,
                   cAC, ZcAC, pAC, qAC;
            double pc;
            bool cav = false;

            //Get variable values from nodes
            cAN  = (*mpAN_c);
            ZcAN = (*mpAN_Zc);
            cAS  = (*mpAS_c);
            ZcAS = (*mpAS_Zc);
            cAC  = (*mpAC_c);
            ZcAC = (*mpAC_Zc);

            double rho = (*mpRho);
            double frac_d = (*mpF);
            double d1 = (*mpD1);
            double F0 = (*mpF0);
            double k = (*mpK);
            double Cq = (*mpCq);


            double K21 = Cq*pi*d1*frac_d*xIntegrator.value()*sqrt(2.0/rho);
            qTurb_pASAN.setFlowCoefficient(K21);

            qAN = qTurb_pASAN.getFlow(cAS, cAN, ZcAN, ZcAS);
            qAS = -qAN;

            pAN = cAN + ZcAN*qAN;
            pAS = cAS + ZcAS*qAS;

            pc = (mAN*pAN + mAS*pAS -(F0+k*xIntegrator.value()))/mAC;

            qAC = qTurb_AC.getFlow(pc, cAC, 0.0, ZcAC);
            double v = qAC/mAC;

            pAC = cAC + ZcAC*qAC;

            //Cavitation check
            if(pAN < 0.0)
            {
                cAN  = 0.0;
                ZcAN = 0.0;
                cav  = true;
            }
            if(pAS < 0.0)
            {
                cAS  = 0.0;
                ZcAS = 0.0;
                cav  = true;
            }
            if(pAC < 0.0)
            {
                cAC  = 0.0;
                ZcAC = 0.0;
                cav  = true;
            }

            if(cav)
            {
                qAN = qTurb_pASAN.getFlow(cAS, cAN, ZcAN, ZcAS);
                qAS = -qAN;

                pAN = cAN + ZcAN*qAN;
                pAS = cAS + ZcAS*qAS;

                pc = (mAN*pAN + mAS*pAS -(F0+k*xIntegrator.value()))/mAC;

                v = qTurb_AC.getFlow(pc, cAC, 0.0, ZcAC)/mAC;
            }

            xIntegrator.update(v);

            //Write new values to nodes
            (*mpAN_p) = pAN;
            (*mpAN_q) = qAN;
            (*mpAS_p) = pAS;
            (*mpAS_q) = qAS;
            (*mpAC_p) = pAC;
            (*mpAC_q) = qAC;
            (*mpXvout) = xIntegrator.value();
        }
    };
}

#endif // HYDRAULIC22POPPETVALVE_HPP_INCLUDED
