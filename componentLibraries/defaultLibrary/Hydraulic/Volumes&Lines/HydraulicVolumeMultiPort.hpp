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
//! @file   HydraulicVolumeMultiPort.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-02
//!
//! @brief Contains a Hydraulic Volume Component
//!
//$Id$

#ifndef HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
#define HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED

#include "ComponentEssentials.h"
//#include "ComponentSystem.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief A hydraulic volume component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVolumeMultiPort : public ComponentC
    {

    private:
        double mPh;
        Port *mpP1;
        size_t mNumPorts;
        std::vector<double*> mvpP1_p, mvpP1_q, mvpP1_c, mvpP1_Zc;
        double *mpAlpha, *mpBetae, *mpV;

        std::vector<double> mvCnew;
        std::vector<double> mvC0;

    public:
        static Component *Creator()
        {
            return new HydraulicVolumeMultiPort();
        }

        void configure()
        {
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");

            addConstant("P_high", "High pressure (for animation)", "Pa", 2e7, mPh);

            addInputVariable("V", "Volume", "m^3", 1.0e-3, &mpV);
            addInputVariable("Beta_e", "Bulkmodulus", "Pa", 1.0e9, &mpBetae);
            addInputVariable("alpha", "Low pass coefficient to dampen standing delayline waves", "-", 0.1, &mpAlpha);

            setDefaultStartValue(mpP1, NodeHydraulic::Flow, 0.0);
            setDefaultStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
        }


        void initialize()
        {
            double Zc, betae, alpha, V;
            betae = (*mpBetae);
            alpha = (*mpAlpha);
            V = (*mpV);

            mNumPorts = mpP1->getNumPorts();
            mvpP1_p.resize(mNumPorts);
            mvpP1_q.resize(mNumPorts);
            mvpP1_c.resize(mNumPorts);
            mvpP1_Zc.resize(mNumPorts);
            mvC0.resize(mNumPorts);
            mvCnew.resize(mNumPorts);

            Zc = double(mNumPorts)*betae/(2.0*V)*mTimestep/(1.0-alpha);

            double pTot=0.0;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpP1_p[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Pressure, 0.0);
                mvpP1_q[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Flow, 0.0);
                mvpP1_c[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::WaveVariable, 0.0);
                mvpP1_Zc[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpP1_p[i]  = getDefaultStartValue(mpP1, NodeHydraulic::Pressure, i);
                *mvpP1_q[i]  = getDefaultStartValue(mpP1, NodeHydraulic::Flow, i);
                pTot       += getDefaultStartValue(mpP1,NodeHydraulic::Pressure, i)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow, i);
                *mvpP1_Zc[i] = Zc;
            }
            pTot = pTot/double(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *mvpP1_c[i] = pTot*2.0-(*mvpP1_p[i]) - Zc*(*mvpP1_q[i]);
            }
        }


        void simulateOneTimestep()
        {
            double cTot = 0.0;
            double pAvg, Zc, V, betae, alpha;
            betae = (*mpBetae);
            alpha = (*mpAlpha);
            V = (*mpV);

            Zc = double(mNumPorts)*betae/(2.0*V)*mTimestep/(1.0-alpha);

            //Equations
            for (size_t i=0; i<mNumPorts; ++i)
            {
                cTot += (*mvpP1_c[i]) + 2.0*Zc*(*mvpP1_q[i]);
            }
            pAvg = cTot/double(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvC0[i] = pAvg*2.0-(*mvpP1_c[i]) - 2.0*Zc*(*mvpP1_q[i]);
                mvCnew[i] = alpha*(*mvpP1_c[i]) + (1.0-alpha)*mvC0[i];
            }

            //Write new values
            for(size_t i=0; i<mNumPorts; ++i)
            {
                (*mvpP1_Zc[i]) = Zc;
                (*mvpP1_c[i]) = mvCnew[i];
            }
        }


        void finalize()
        {
        }
    };
}

#endif // HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
