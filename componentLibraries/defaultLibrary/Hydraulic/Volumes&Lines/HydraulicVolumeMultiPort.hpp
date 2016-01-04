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
#include "ComponentSystem.h"
#include <vector>

namespace hopsan {

#ifdef MULTIPORTNDPSTRUCT
    class HydraulicVolumeMultiPort : public ComponentC
    {

    private:
        double mPh;
        Port *mpP1;
        size_t mNumPorts;
        std::vector<HydraulicNodeDataPointerStructT> mvpP1;
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
            getHydraulicMultiPortNodeDataPointers(mpP1, mvpP1);

            mvC0.resize(mNumPorts);
            mvCnew.resize(mNumPorts);

            Zc = double(mNumPorts)*betae/(2.0*V)*mTimestep/(1.0-alpha);

            double pTot=0.0;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *mvpP1[i].pP  = getDefaultStartValue(mpP1, NodeHydraulic::Pressure, i);
                *mvpP1[i].pQ  = getDefaultStartValue(mpP1, NodeHydraulic::Flow, i);
                pTot         += getDefaultStartValue(mpP1,NodeHydraulic::Pressure, i)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow, i);
                *mvpP1[i].pZc = Zc;
            }
            pTot = pTot/double(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *mvpP1[i].pC = pTot*2.0-(*mvpP1[i].pP) - Zc*(*mvpP1[i].pQ);
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
                cTot += (*mvpP1[i].pC) + 2.0*Zc*(*mvpP1[i].pQ);
            }
            pAvg = cTot/double(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvC0[i] = pAvg*2.0-(*mvpP1[i].pC) - 2.0*Zc*(*mvpP1[i].pQ);
                mvCnew[i] = alpha*(*mvpP1[i].pC) + (1.0-alpha)*mvC0[i];
            }

            //Write new values
            for(size_t i=0; i<mNumPorts; ++i)
            {
                (*mvpP1[i].pZc) = Zc;
                (*mvpP1[i].pC) = mvCnew[i];
            }
        }
    };
#elif defined MULTIPORTNDPSTRUCTMETHODS
    class HydraulicVolumeMultiPort : public ComponentC
    {

    private:
        double mPh;
        Port *mpP1;
        size_t mNumPorts;
        std::vector<HydraulicNodeDataPointerStructT> mvpP1;
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
            getHydraulicMultiPortNodeDataPointers(mpP1, mvpP1);

            mvC0.resize(mNumPorts);
            mvCnew.resize(mNumPorts);

            Zc = double(mNumPorts)*betae/(2.0*V)*mTimestep/(1.0-alpha);

            double pTot=0.0;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpP1[i].rp()  = getDefaultStartValue(mpP1, NodeHydraulic::Pressure, i);
                mvpP1[i].rq()  = getDefaultStartValue(mpP1, NodeHydraulic::Flow, i);
                pTot          += getDefaultStartValue(mpP1,NodeHydraulic::Pressure, i)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow, i);
                mvpP1[i].rZc() = Zc;
            }
            pTot = pTot/double(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpP1[i].rc() = pTot*2.0-mvpP1[i].p() - Zc*mvpP1[i].q();
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
                cTot += mvpP1[i].c() + 2.0*Zc*mvpP1[i].q();
            }
            pAvg = cTot/double(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvC0[i] = pAvg*2.0-mvpP1[i].c() - 2.0*Zc*mvpP1[i].q();
                mvCnew[i] = alpha*mvpP1[i].c() + (1.0-alpha)*mvC0[i];
            }

            //Write new values
            for(size_t i=0; i<mNumPorts; ++i)
            {
                mvpP1[i].rZc() = Zc;
                mvpP1[i].rc()  = mvCnew[i];
            }
        }
    };
#elif defined MULTIPORTVALUESTRUCT
    class HydraulicVolumeMultiPort : public ComponentC
    {

    private:
        double mPh;
        Port *mpP1;
        size_t mNumPorts;
        std::vector<HydraulicNodeDataValueStructT> mvP1;
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
            mvP1.resize(mNumPorts);
            readHydraulicMultiPortValues_all(mpP1, mvP1);

            mvC0.resize(mNumPorts);
            mvCnew.resize(mNumPorts);

            Zc = double(mNumPorts)*betae/(2.0*V)*mTimestep/(1.0-alpha);

            double pTot=0.0;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvP1[i].p  = getDefaultStartValue(mpP1, NodeHydraulic::Pressure, i);
                mvP1[i].q  = getDefaultStartValue(mpP1, NodeHydraulic::Flow, i);
                pTot         += getDefaultStartValue(mpP1,NodeHydraulic::Pressure, i)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow, i);
                mvP1[i].Zc = Zc;
            }
            pTot = pTot/double(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvP1[i].c = pTot*2.0-mvP1[i].p - Zc*mvP1[i].q;
                writeHydraulicMultiPort_cZc(mpP1, i, mvP1[i].c, mvP1[i].Zc);
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

            readHydraulicMultiPortValues_all(mpP1, mvP1);


            //Equations
            for (size_t i=0; i<mNumPorts; ++i)
            {
                cTot += mvP1[i].c + 2.0*Zc*mvP1[i].q;
            }
            pAvg = cTot/double(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvC0[i] = pAvg*2.0-mvP1[i].c - 2.0*Zc*mvP1[i].q;
                mvCnew[i] = alpha*mvP1[i].c + (1.0-alpha)*mvC0[i];
            }

            //Write new values
            for(size_t i=0; i<mNumPorts; ++i)
            {
                mvP1[i].Zc = Zc;
                mvP1[i].c = mvCnew[i];
                writeHydraulicMultiPort_cZc(mpP1, i, mvP1[i].c, mvP1[i].Zc);
            }
        }
    };

#elif defined MULTIPORTREADWRITE

    class HydraulicVolumeMultiPort : public ComponentC
    {

    private:
        double mPh;
        Port *mpP1;
        size_t mNumPorts;
        std::vector<double> mvP1_p, mvP1_q, mvP1_c, mvP1_Zc;
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
            mvP1_p.resize(mNumPorts);
            mvP1_q.resize(mNumPorts);
            mvP1_c.resize(mNumPorts);
            mvP1_Zc.resize(mNumPorts);
            mvC0.resize(mNumPorts);
            mvCnew.resize(mNumPorts);

            Zc = double(mNumPorts)*betae/(2.0*V)*mTimestep/(1.0-alpha);

            double pTot=0.0;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mpP1->writeNode(NodeHydraulic::Pressure, getDefaultStartValue(mpP1, NodeHydraulic::Pressure, i), i);
                mpP1->writeNode(NodeHydraulic::Flow, getDefaultStartValue(mpP1, NodeHydraulic::Flow, i), i);
                pTot       += getDefaultStartValue(mpP1,NodeHydraulic::Pressure, i)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow, i);
                mpP1->writeNode(NodeHydraulic::CharImpedance, Zc, i);
            }
            pTot = pTot/double(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                double p = mpP1->readNode(NodeHydraulic::Pressure);
                double q = mpP1->readNode(NodeHydraulic::Flow);
                mpP1->writeNode(NodeHydraulic::WaveVariable, pTot*2.0-p - Zc*q, i);
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
                mvP1_c[i] = mpP1->readNode(NodeHydraulic::WaveVariable, i);
                mvP1_q[i] = mpP1->readNode(NodeHydraulic::Flow, i);
                cTot += mvP1_c[i] + 2.0*Zc*mvP1_q[i];
            }
            pAvg = cTot/double(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvC0[i] = pAvg*2.0-mvP1_c[i] - 2.0*Zc*mvP1_q[i];
                mvCnew[i] = alpha*mvP1_c[i] + (1.0-alpha)*mvC0[i];
            }

            //Write new values
            for(size_t i=0; i<mNumPorts; ++i)
            {
                mpP1->writeNode(NodeHydraulic::CharImpedance, Zc, i);
                mpP1->writeNode(NodeHydraulic::WaveVariable, mvCnew[i], i);
            }
        }


        void finalize()
        {
        }
    };

#else
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
#endif
}

#endif // HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
