//!
//! @file   Hydraulic43Valve.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a hydraulic 4/3-valve of Q-type
#ifndef HYDRAULIC43VALVE_HPP_INCLUDED
#define HYDRAULIC43VALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/3-valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic43Valve : public ComponentQ
    {
    private:
        double mCq;
        double md;
        double mf;
        double mxvmax;
        double moverlap_pa;
        double moverlap_pb;
        double moverlap_at;
        double moverlap_bt;
        double momegah;
        double mdeltah;
        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturbpa;
        TurbulentFlowFunction mQturbpb;
        TurbulentFlowFunction mQturbat;
        TurbulentFlowFunction mQturbbt;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new Hydraulic43Valve("Hydraulic 4/3 Valve");
        }

        Hydraulic43Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic43Valve";
            mCq = 0.67;
            md = 0.01;
            mf = 1.0;
            mxvmax = 0.01;
            moverlap_pa = 0.0;
            moverlap_pb = 0.0;
            moverlap_at = 0.0;
            moverlap_bt = 0.0;
            momegah = 100.0;
            mdeltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", mCq);
            registerParameter("d", "Diameter", "[m]", md);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", mf);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", mxvmax);
            registerParameter("overlap_pa", "Spool Overlap From Port P To A", "[m]", moverlap_pa);
            registerParameter("overlap_pb", "Spool Overlap From Port P To B", "[m]", moverlap_pb);
            registerParameter("overlap_at", "Spool Overlap From Port A To T", "[m]", moverlap_at);
            registerParameter("overlap_pa", "Spool Overlap From Port B To T", "[m]", moverlap_bt);
            registerParameter("omegah", "Resonance Frequency", "[rad/s]", momegah);
            registerParameter("deltah", "Damping Factor", "[-]", mdeltah);
        }


        void initialize()
        {
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(momegah*momegah), 2.0*mdeltah/momegah, 1.0};
            myFilter.initialize(mTimestep, num, den, 0, 0, -mxvmax, mxvmax);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double cp  = mpPP->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zcp = mpPP->readNode(NodeHydraulic::CHARIMP);
            double ct  = mpPT->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zct = mpPT->readNode(NodeHydraulic::CHARIMP);
            double ca  = mpPA->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zca = mpPA->readNode(NodeHydraulic::CHARIMP);
            double cb  = mpPB->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zcb = mpPB->readNode(NodeHydraulic::CHARIMP);
            double xvin  = mpIn->readNode(NodeSignal::VALUE);

            myFilter.update(xvin);
            double xv = myFilter.value();

            //Valve equations
            double xpanom = std::max(xv-moverlap_pa,0.0);
            double xpbnom = std::max(-xv-moverlap_pb,0.0);
            double xatnom = std::max(-xv-moverlap_at,0.0);
            double xbtnom = std::max(xv-moverlap_bt,0.0);

            double Kcpa = mCq*mf*pi*md*xpanom*sqrt(2.0/890.0);
            double Kcpb = mCq*mf*pi*md*xpbnom*sqrt(2.0/890.0);
            double Kcat = mCq*mf*pi*md*xatnom*sqrt(2.0/890.0);
            double Kcbt = mCq*mf*pi*md*xbtnom*sqrt(2.0/890.0);

            //With TurbulentFlowFunction:
            mQturbpa.setFlowCoefficient(Kcpa);
            mQturbpb.setFlowCoefficient(Kcpb);
            mQturbat.setFlowCoefficient(Kcat);
            mQturbbt.setFlowCoefficient(Kcbt);

            double qpa = mQturbpa.getFlow(cp, ca, Zcp, Zca);
            double qpb = mQturbpb.getFlow(cp, cb, Zcp, Zcb);
            double qat = mQturbat.getFlow(ca, ct, Zca, Zct);
            double qbt = mQturbbt.getFlow(cb, ct, Zcb, Zct);

            //With sigsqrl:
            //        double qpa = 0.5*Kcpa*Kcpa*(Zcp+Zca)+sigsqrl(Kcpa*Kcpa*(cp-ca)+0.25*Kcpa*Kcpa*Kcpa*Kcpa*(Zcp+Zca)*(Zcp+Zca));
            //        double qpb = 0.5*Kcpb*Kcpb*(Zcp+Zcb)+sigsqrl(Kcpb*Kcpb*(cp-cb)+0.25*Kcpb*Kcpb*Kcpb*Kcpb*(Zcp+Zcb)*(Zcp+Zcb));
            //        double qat = 0.5*Kcat*Kcat*(Zca+Zct)+sigsqrl(Kcat*Kcat*(ca-ct)+0.25*Kcat*Kcat*Kcat*Kcat*(Zca+Zct)*(Zca+Zct);
            //        double qbt = 0.5*Kcbt*Kcbt*(Zcb+Zct)+sigsqrl(Kcbt*Kcbt*(cb-ct)+0.25*Kcbt*Kcbt*Kcbt*Kcbt*(Zcb+Zct)*(Zcb+Zct));

            double qp, qa, qb, qt;
            if (xv >= 0.0)
            {
                qp = -qpa;
                qa = qpa;
                qb = -qbt;
                qt = qbt;
            }
            else
            {
                qp = -qpb;
                qa = -qat;
                qb = qpb;
                qt = qat;
            }

            double pp = cp + qp*Zcp;
            double pa = ca + qa*Zca;
            double pb = cb + qb*Zcb;
            double pt = ct + qt*Zct;

            //Write new values to nodes

            mpPP->writeNode(NodeHydraulic::PRESSURE, pp);
            mpPP->writeNode(NodeHydraulic::FLOW, qp);
            mpPT->writeNode(NodeHydraulic::PRESSURE, pt);
            mpPT->writeNode(NodeHydraulic::FLOW, qt);
            mpPA->writeNode(NodeHydraulic::PRESSURE, pa);
            mpPA->writeNode(NodeHydraulic::FLOW, qa);
            mpPB->writeNode(NodeHydraulic::PRESSURE, pb);
            mpPB->writeNode(NodeHydraulic::FLOW, qb);
        }
    };







    //!
    //! @brief Hydraulic 4/3-valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOptimized43Valve : public ComponentQ
    {
    private:
        double mCq;
        double md;
        double mf;
        double mxvmax;
        double moverlap_pa;
        double moverlap_pb;
        double moverlap_at;
        double moverlap_bt;
        double momegah;
        double mdeltah;
        double *cp, *Zcp, *ct, *Zct, *ca, *Zca, *cb, *Zcb, *xvin;
        double *pp, *qp, *pt, *qt, *pa, *qa, *pb, *qb;
        double xv, xv_abs, xpanom, xpbnom, xatnom, xbtnom, Kcpa, Kcpb, Kcat, Kcbt, qpa, qpb, qat, qbt;

            //Filter
        double mFilterDelayU[2];
        double mFilterDelayY[2];
        double mCoeffU[3];
        double mCoeffY[3];
            //

#define pi 3.14159
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicOptimized43Valve("Hydraulic 4/3 Valve");
        }

        HydraulicOptimized43Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicOptimized43Valve";
            mCq = 0.67;
            md = 0.01;
            mf = 1.0;
            mxvmax = 0.01;
            moverlap_pa = 0.0;
            moverlap_pb = 0.0;
            moverlap_at = 0.0;
            moverlap_bt = 0.0;
            momegah = 100.0;
            mdeltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", mCq);
            registerParameter("d", "Diameter", "[m]", md);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", mf);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", mxvmax);
            registerParameter("overlap_pa", "Spool Overlap From Port P To A", "[m]", moverlap_pa);
            registerParameter("overlap_pb", "Spool Overlap From Port P To B", "[m]", moverlap_pb);
            registerParameter("overlap_at", "Spool Overlap From Port A To T", "[m]", moverlap_at);
            registerParameter("overlap_pa", "Spool Overlap From Port B To T", "[m]", moverlap_bt);
            registerParameter("omegah", "Resonance Frequency", "[rad/s]", momegah);
            registerParameter("deltah", "Damping Factor", "[-]", mdeltah);
        }


        void initialize()
        {
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/momegah*momegah, 2.0*mdeltah/momegah, 1.0};

                //Filter
            mFilterDelayU[0] = 0;
            mFilterDelayU[1] = 0;
            mFilterDelayY[0] = 0;
            mFilterDelayY[1] = 0;

            mCoeffU[0] = num[2]*(mTimestep*mTimestep) - 2.0*num[1]*mTimestep + 4.0*num[0];
            mCoeffU[1] = 2.0*num[2]*(mTimestep*mTimestep) - 8.0*num[0];
            mCoeffU[2] = num[2]*(mTimestep*mTimestep) + 2.0*num[1]*mTimestep + 4.0*num[0];

            mCoeffY[0] = den[2]*(mTimestep*mTimestep) - 2.0*den[1]*mTimestep + 4.0*den[0];
            mCoeffY[1] = 2.0*den[2]*(mTimestep*mTimestep) - 8.0*den[0];
            mCoeffY[2] = den[2]*(mTimestep*mTimestep) + 2.0*den[1]*mTimestep + 4.0*den[0];
                //

            cp =   mpPP->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zcp =  mpPP->getNodeDataPtr(NodeHydraulic::CHARIMP);
            ct =   mpPT->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zct =  mpPT->getNodeDataPtr(NodeHydraulic::CHARIMP);
            ca =   mpPA->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zca =  mpPA->getNodeDataPtr(NodeHydraulic::CHARIMP);
            cb =   mpPB->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zcb =  mpPB->getNodeDataPtr(NodeHydraulic::CHARIMP);
            xvin = mpIn->getNodeDataPtr(NodeSignal::VALUE);

            pp = mpPP->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qp = mpPP->getNodeDataPtr(NodeHydraulic::FLOW);
            pt = mpPT->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qt = mpPT->getNodeDataPtr(NodeHydraulic::FLOW);
            pa = mpPA->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qa = mpPA->getNodeDataPtr(NodeHydraulic::FLOW);
            pb = mpPB->getNodeDataPtr(NodeHydraulic::PRESSURE);
            qb = mpPB->getNodeDataPtr(NodeHydraulic::FLOW);
        }


        void simulateOneTimestep()
        {
                //Filter
            xv = 1.0/mCoeffY[2]*(mCoeffU[2] * *xvin + mCoeffU[1]*mFilterDelayU[0] +
                 mCoeffU[0]*mFilterDelayU[1] - (mCoeffY[1]*mFilterDelayY[0] + mCoeffY[0]*mFilterDelayY[1]));
            mFilterDelayU[1] = mFilterDelayU[0];
            mFilterDelayU[0] = *xvin;
            mFilterDelayY[1] = mFilterDelayY[0];
            mFilterDelayY[0] = xv;
                //

            if (xv>mxvmax)
            {
                xv = mxvmax;
            }
            else if(xv < -mxvmax)
            {
                xv = -mxvmax;
            }

            if((xv-moverlap_pa) > 0)
                xpanom = xv-moverlap_pa;
            else
                xpanom = 0;
            if((-xv-moverlap_pb) > 0)
                xpbnom = -xv-moverlap_pb;
            else
                xpbnom = 0;
            if((-xv-moverlap_at) > 0)
                xatnom = -xv-moverlap_at;
            else
                xatnom = 0;
            if((xv-moverlap_bt) > 0)
                xbtnom = xv-moverlap_bt;
            else
                xbtnom = 0;

            Kcpa = mCq*mf*pi*md*xpanom*sqrt(2.0/890.0);
            Kcpb = mCq*mf*pi*md*xpbnom*sqrt(2.0/890.0);
            Kcat = mCq*mf*pi*md*xatnom*sqrt(2.0/890.0);
            Kcbt = mCq*mf*pi*md*xbtnom*sqrt(2.0/890.0);

                //Flow functions
            if (*cp > *ca)
                qpa = Kcpa*(sqrt(*cp-*ca+((*Zcp+*Zca)*(*Zcp+*Zca)) * (Kcpa*Kcpa)/4) - Kcpa*(*Zcp+*Zca)/2);
            else
                qpa = Kcpa*(Kcpa*(*Zcp+*Zca)/2 - sqrt(*ca-*cp+((*Zcp+*Zca)*(*Zcp+*Zca))*(Kcpa*Kcpa)/4));

            if (*cp > *cb)
                qpb = Kcpb*(sqrt(*cp-*cb+((*Zcp+*Zcb)*(*Zcp+*Zcb)) * (Kcpb*Kcpb)/4) - Kcpb*(*Zcp+*Zcb)/2);
            else
                qpb = Kcpb*(Kcpb*(*Zcp+*Zcb)/2 - sqrt(*cb-*cp+((*Zcp+*Zcb)*(*Zcp+*Zcb))*(Kcpb*Kcpb)/4));

            if (*ca > *ct)
                qat = Kcat*(sqrt(*ca-*ct+((*Zca+*Zct)*(*Zca+*Zct)) * (Kcat*Kcat)/4) - Kcat*(*Zca+*Zct)/2);
            else
                qat = Kcat*(Kcat*(*Zca+*Zct)/2 - sqrt(*ct-*ca+((*Zca+*Zct)*(*Zca+*Zct))*(Kcat*Kcat)/4));

            if (*cb > *ct)
                qbt = Kcbt*(sqrt(*cb-*ct+((*Zcb+*Zct)*(*Zcb+*Zct)) * (Kcbt*Kcbt)/4) - Kcbt*(*Zcb+*Zct)/2);
            else
                qbt = Kcbt*(Kcbt*(*Zcb+*Zct)/2 - sqrt(*ct-*cb+((*Zcb+*Zct)*(*Zcb+*Zct))*(Kcbt*Kcbt)/4));
                //


            if (xv >= 0.0)
            {
                *qp = -qpa;
                *qa = qpa;
                *qb = -qbt;
                *qt = qbt;
            }
            else
            {
                *qp = -qpb;
                *qa = -qat;
                *qb = qpb;
                *qt = qat;
            }

            *pp = *cp + *qp * *Zcp;
            *pa = *ca + *qa * *Zca;
            *pb = *cb + *qb * *Zcb;
            *pt = *ct + *qt * *Zct;
        }
    };




}

#endif // HYDRAULIC43VALVE_HPP_INCLUDED

