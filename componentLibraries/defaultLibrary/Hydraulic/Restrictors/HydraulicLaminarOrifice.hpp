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

//$Id$

#ifndef HYDRAULICLAMINARORIFICE_HPP_INCLUDED
#define HYDRAULICLAMINARORIFICE_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

#ifdef PORTNDPSTRUCT
    class HydraulicLaminarOrifice : public ComponentQ
    {
    private:
        double *mpKc;
        HydraulicNodeDataPointerStructT mP1, mP2;
        Port *mpP1, *mpP2, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrifice();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addInputVariable("Kc", "Pressure-Flow Coefficient", "m^5/Ns", 1.0e-11, &mpKc);
        }


        void initialize()
        {
//            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
//            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
//            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
//            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

//            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
//            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
//            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
//            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
            getHydraulicPortNodeDataPointers(mpP1, mP1);
            getHydraulicPortNodeDataPointers(mpP2, mP2);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

            //Get variable values from nodes
            c1 = (*mP1.pC);
            Zc1 = (*mP1.pZc);
            c2 = (*mP2.pC);
            Zc2 = (*mP2.pZc);
            const double Kc = fabs(*mpKc);

            //Orifice equations
            q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            bool cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(cav)
            {
                q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
            (*mP1.pP) = p1;
            (*mP1.pQ) = q1;
            (*mP2.pP) = p2;
            (*mP2.pQ) = q2;
        }
    };
#elif defined PORTNDPSTRUCTMETHODS
class HydraulicLaminarOrifice : public ComponentQ
{
private:
    double *mpKc;
    HydraulicNodeDataPointerStructT mP1, mP2;
    Port *mpP1, *mpP2, *mpIn;

public:
    static Component *Creator()
    {
        return new HydraulicLaminarOrifice();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");
        mpIn = addInputVariable("Kc", "Pressure-Flow Coefficient", "m^5/Ns", 1.0e-11, &mpKc);
    }


    void initialize()
    {
//            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
//            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
//            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
//            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

//            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
//            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
//            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
//            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        getHydraulicPortNodeDataPointers(mpP1, mP1);
        getHydraulicPortNodeDataPointers(mpP2, mP2);
    }


    void simulateOneTimestep()
    {
        double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

        //Get variable values from nodes
//        c1 = (*mP1.pC);
//        Zc1 = (*mP1.pZc);
//        c2 = (*mP2.pC);
//        Zc2 = (*mP2.pZc);
        const double Kc = fabs(*mpKc);

        //Orifice equations
        q2 = Kc*(mP1.c()-mP2.c())/(1.0+Kc*(mP1.Zc()+mP2.Zc()));
        q1 = -q2;
        p1 = mP1.c() + q1*mP1.Zc();
        p2 = mP2.c() + q2*mP2.Zc();

        //Cavitation check
        bool cav = false;
        if(p1 < 0.0)
        {
            c1 = 0.0;
            Zc1 = 0.0;
            cav = true;
        }
        if(p2 < 0.0)
        {
            c2 = 0.0;
            Zc2 = 0.0;
            cav = true;
        }
        if(cav)
        {
            q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;
            if(p1 < 0.0) { p1 = 0.0; }
            if(p2 < 0.0) { p2 = 0.0; }
        }

        //Write new variables to nodes
        mP1.rp() = p1;
        mP1.rq() = q1;
        mP2.rp() = p2;
        mP2.rq() = q2;
    }
};

#elif defined PORTVALUESTRUCT
    class HydraulicLaminarOrifice : public ComponentQ
    {
    private:
        Port *mpP1, *mpP2, *mpIn;
        double *mpKc;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrifice();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addInputVariable("Kc", "Pressure-Flow Coefficient", "m^5/Ns", 1.0e-11, &mpKc);
        }


        void initialize()
        {
//            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
//            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
//            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
//            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

//            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
//            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
//            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
//            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            //double p1, q1, c1, Zc1, p2, q2, c2, Zc2;
            HydraulicNodeDataValueStructT P1, P2;

            readHydraulicPort_all(mpP1, P1);
            readHydraulicPort_all(mpP2, P2);

            //Get variable values from nodes
//            c1 = (*mP1.c);
//            Zc1 = (*mP1.Zc);
//            c2 = (*mP2.c);
//            Zc2 = (*mP2.Zc);
            const double Kc = fabs(*mpKc);

            //Orifice equations
            double q2 = Kc*(P1.c-P2.c)/(1.0+Kc*(P1.Zc+P2.Zc));
            double q1 = -q2;
            double p1 = P1.c + q1*P1.Zc;
            double p2 = P2.c + q2*P2.Zc;

            //Cavitation check
            bool cav = false;
            if(p1 < 0.0)
            {
                P1.c = 0.0;
                P1.Zc = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                P2.c = 0.0;
                P2.Zc = 0.0;
                cav = true;
            }
            if(cav)
            {
//                q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
//                q1 = -q2;
//                p1 = c1 + q1*Zc1;
//                p2 = c2 + q2*Zc2;
                double q2 = Kc*(P1.c-P2.c)/(1.0+Kc*(P1.Zc+P2.Zc));
                double q1 = -q2;
                p1 = P1.c + q1*P1.Zc;
                p2 = P2.c + q2*P2.Zc;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
//            (*mP1.p) = p1;
//            (*mP1.q) = q1;
//            (*mP2.p) = p2;
//            (*mP2.q) = q2;
            writeHydraulicPort_pq(mpP1, p1, q1 );
            writeHydraulicPort_pq(mpP2, p2, q2 );
        }
    };
#elif defined PORTVALUES
    class HydraulicLaminarOrifice : public ComponentQ
    {
    private:
        Port *mpP1, *mpP2, *mpIn;
        double *mpKc;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrifice();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addInputVariable("Kc", "Pressure-Flow Coefficient", "m^5/Ns", 1.0e-11, &mpKc);
        }


        void initialize()
        {
//            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
//            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
//            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
//            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

//            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
//            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
//            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
//            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

            readHydraulicPort_cZc(mpP1, c1, Zc1);
            readHydraulicPort_cZc(mpP2, c2, Zc2);

            //Get variable values from nodes
//            c1 = (*mP1.c);
//            Zc1 = (*mP1.Zc);
//            c2 = (*mP2.c);
//            Zc2 = (*mP2.Zc);
            const double Kc = fabs(*mpKc);

            //Orifice equations
            q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            bool cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(cav)
            {
                q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
//            (*mP1.p) = p1;
//            (*mP1.q) = q1;
//            (*mP2.p) = p2;
//            (*mP2.q) = q2;
            writeHydraulicPort_pq(mpP1, p1, q1 );
            writeHydraulicPort_pq(mpP2, p2, q2 );
        }
    };
#elif defined PORTREADWRITE
    class HydraulicLaminarOrifice : public ComponentQ
    {
    private:
        Port *mpP1, *mpP2, *mpIn;
        double *mpKc;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrifice();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addInputVariable("Kc", "Pressure-Flow Coefficient", "m^5/Ns", 1.0e-11, &mpKc);
        }


        void initialize()
        {
//            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
//            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
//            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
//            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

//            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
//            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
//            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
//            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

            //Get variable values from nodes
            c1 = mpP1->readNode(NodeHydraulic::WaveVariable);
            Zc1 = mpP1->readNode(NodeHydraulic::CharImpedance);
            c2 = mpP2->readNode(NodeHydraulic::WaveVariable);
            Zc2 = mpP2->readNode(NodeHydraulic::CharImpedance);
            const double Kc = fabs(*mpKc);

            //Orifice equations
            q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            bool cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(cav)
            {
                q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
            mpP1->writeNode(NodeHydraulic::Pressure, p1);
            mpP1->writeNode(NodeHydraulic::Flow, q1);
            mpP2->writeNode(NodeHydraulic::Pressure, p2);
            mpP2->writeNode(NodeHydraulic::Flow, q2);
        }
    };
#else
    //!
    //! @brief A hydraulic laminar orifice component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicLaminarOrifice : public ComponentQ
    {
    private:
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc, *mpKc;
        Port *mpP1, *mpP2, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrifice();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addInputVariable("Kc", "Pressure-Flow Coefficient", "m^5/Ns", 1.0e-11, &mpKc);
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
        }


        void simulateOneTimestep()
        {
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

            //Get variable values from nodes
            c1 = (*mpP1_c);
            Zc1 = (*mpP1_Zc);
            c2 = (*mpP2_c);
            Zc2 = (*mpP2_Zc);
            const double Kc = fabs(*mpKc);

            //Orifice equations
            q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            bool cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(cav)
            {
                q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
        }
    };
#endif

}

#endif // HYDRAULICLAMINARORIFICE_HPP_INCLUDED
