#ifndef HYDRAULIC43VALVE_HPP_INCLUDED
#define HYDRAULIC43VALVE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

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
    #define pi 3.14159
    enum {PP, PT, PA, PB, PX};
    double sign(double x)
    {
        if (x>=0.0)
        {
            return 1.0;
        }
        else
        {
            return -1.0;
        }
    }
public:
    static Component *Creator()
    {
        std::cout << "running 4/3-valve creator" << std::endl;
        return new Hydraulic43Valve("Default43ValveName");
    }

    Hydraulic43Valve(const string name,
                             const double Cq         = 0.67,
                             const double fraction   = 1.0,
                             const double diameter   = 0.01,
                             const double xvmax      = 0.01,
                             const double overlap_pa = 0.0,
                             const double overlap_pb = 0.0,
                             const double overlap_at = 0.0,
                             const double overlap_bt = 0.0,
                             const double resfrequency = 100.0,
                             const double damping = 0.0,
                             const double timestep   = 0.001)
        : ComponentQ(name, timestep)
    {

        mTypeName = "Hydraulic43Valve";
        mCq = Cq;
        md = diameter;
        mf = fraction;
        mxvmax = xvmax;
        moverlap_pa = overlap_pa;
        moverlap_pb = overlap_pb;
        moverlap_at = overlap_at;
        moverlap_bt = overlap_bt;
        momegah = resfrequency;
        mdeltah = damping;

        addPowerPort("PP", "NodeHydraulic", PP);
        addPowerPort("PT", "NodeHydraulic", PT);
        addPowerPort("PA", "NodeHydraulic", PA);
        addPowerPort("PB", "NodeHydraulic", PB);
        addReadPort("PX", "NodeSignal", PX);

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
        double den[3] = {1.0/pow(momegah,2.0), 2.0*mdeltah/momegah, 1.0};
        myFilter.initialize(mTime, mTimestep, num, den);
    }

    void simulateOneTimestep()
    {

        //Get variable values from nodes
        double cp  = mPortPtrs[PP]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zcp = mPortPtrs[PP]->readNode(NodeHydraulic::CHARIMP);
        double ct  = mPortPtrs[PT]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zct = mPortPtrs[PT]->readNode(NodeHydraulic::CHARIMP);
        double ca  = mPortPtrs[PA]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zca = mPortPtrs[PA]->readNode(NodeHydraulic::CHARIMP);
        double cb  = mPortPtrs[PB]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zcb = mPortPtrs[PB]->readNode(NodeHydraulic::CHARIMP);
        double xvin  = mPortPtrs[PX]->readNode(NodeSignal::VALUE);

        double xv = myFilter.value(xvin);

        //Valve equations
        if (fabs(xv)>mxvmax)
        {
         xv = mxvmax*sign(xv);
        }
        double xpanom = max(xv-moverlap_pa,0.0);
        double xpbnom = max(-xv-moverlap_pb,0.0);
        double xatnom = max(-xv-moverlap_at,0.0);
        double xbtnom = max(xv-moverlap_bt,0.0);

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
//        double qpa = 0.5*pow(Kcpa,2.0)*(Zcp+Zca)+sigsqrl(pow(Kcpa,2.0)*(cp-ca)+0.25*pow(Kcpa,4.0)*pow(Zcp+Zca,2.0));
//        double qpb = 0.5*pow(Kcpb,2.0)*(Zcp+Zcb)+sigsqrl(pow(Kcpb,2.0)*(cp-cb)+0.25*pow(Kcpb,4.0)*pow(Zcp+Zcb,2.0));
//        double qat = 0.5*pow(Kcat,2.0)*(Zca+Zct)+sigsqrl(pow(Kcat,2.0)*(ca-ct)+0.25*pow(Kcat,4.0)*pow(Zca+Zct,2.0));
//        double qbt = 0.5*pow(Kcbt,2.0)*(Zcb+Zct)+sigsqrl(pow(Kcbt,2.0)*(cb-ct)+0.25*pow(Kcbt,4.0)*pow(Zcb+Zct,2.0));

        //Without siqsqrl
//        double qpa = 0.5*pow(Kcpa,2.0)*(Zcp+Zca)+sqrt(pow(Kcpa,2.0)*(cp-ca)+0.25*pow(Kcpa,4.0)*pow(Zcp+Zca,2.0))*sign(pow(Kcpa,2.0)*(cp-ca)+0.25*pow(Kcpa,4.0)*pow(Zcp+Zca,2.0));
//        double qpb = 0.5*pow(Kcpb,2.0)*(Zcp+Zcb)+sqrt(pow(Kcpb,2.0)*(cp-cb)+0.25*pow(Kcpb,4.0)*pow(Zcp+Zcb,2.0))*sign(pow(Kcpb,2.0)*(cp-cb)+0.25*pow(Kcpb,4.0)*pow(Zcp+Zcb,2.0));
//        double qat = 0.5*pow(Kcat,2.0)*(Zca+Zct)+sqrt(pow(Kcat,2.0)*(ca-ct)+0.25*pow(Kcat,4.0)*pow(Zca+Zct,2.0))*sign(pow(Kcat,2.0)*(ca-ct)+0.25*pow(Kcat,4.0)*pow(Zca+Zct,2.0));
//        double qbt = 0.5*pow(Kcbt,2.0)*(Zcb+Zct)+sqrt(pow(Kcbt,2.0)*(cb-ct)+0.25*pow(Kcbt,4.0)*pow(Zcb+Zct,2.0))*sign(pow(Kcbt,2.0)*(cb-ct)+0.25*pow(Kcbt,4.0)*pow(Zcb+Zct,2.0));

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

        mPortPtrs[PP]->writeNode(NodeHydraulic::PRESSURE, pp);
        mPortPtrs[PP]->writeNode(NodeHydraulic::MASSFLOW, qp);
        mPortPtrs[PT]->writeNode(NodeHydraulic::PRESSURE, pt);
        mPortPtrs[PT]->writeNode(NodeHydraulic::MASSFLOW, qt);
        mPortPtrs[PA]->writeNode(NodeHydraulic::PRESSURE, pa);
        mPortPtrs[PA]->writeNode(NodeHydraulic::MASSFLOW, qa);
        mPortPtrs[PB]->writeNode(NodeHydraulic::PRESSURE, pb);
        mPortPtrs[PB]->writeNode(NodeHydraulic::MASSFLOW, qb);

        //Update Filter:
        //myFilter.update(xvin);
    }
};

#endif // HYDRAULIC43VALVE_HPP_INCLUDED

