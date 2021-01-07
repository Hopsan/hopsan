/*-----------------------------------------------------------------------------

//! @file   VariableMachine.hpp
//! @author Samuel Kärnell <samuel.karnell@liu.se>
//! @date   2021-01-07

-----------------------------------------------------------------------------*/
// Header guard to avoid inclusion of the same code twice
// Every hpp file in your library need to have its own UNIQUE header guard
#ifndef VARIABLEMACHINEW_H
#define VARIABLEMACHINEW_H
// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>
// Put your component class inside the hopsan namespace (optional)
namespace hopsan {
// Define a new Class that inherits from ComponentC, ComponentQ or ComponentSignal
// This depends on the type of component you want to create, a C, Q or signal component
class VariableMachineW : public ComponentQ
{
private:
    double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2,*mpND_p3, *mpND_q3, *mpND_c3, *mpND_Zc3, *mpND_t4, *mpND_a4, *mpND_w4, *mpND_c4, *mpND_Zx4;
	double mViscFric, mExtLeak, mIntLeak, mDp, mJ;	
	double *mpEps;
    Port *mpP1, *mpP2, *mpP3, *mpP4;
	FirstOrderTransferFunction mTF;
	Integrator mIntegrator;
	double mNum [2] , mDen [2];
public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    // This static function is mandatory
    static Component *Creator()
    {
        return new VariableMachineW();
    }
    // The Configure function that is run ONCE when a new object of the class is created
    // Use this function to set initial member variable values and to register Ports, Parameters and Startvalues
    // This function is mandatory
    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");
		mpP3 = addPowerPort("P3", "NodeHydraulic");
		mpP4 = addPowerPort("P4", "NodeMechanicRotational");		
		
		addInputVariable("eps", "Displacement setting", "-", 1, &mpEps);				
        addConstant("D_p", "Displacement", "m^3/rev", 60e-6, mDp);
		addConstant("J_m", "Inertia Load", "kgm^2", 0.1, mJ);
		addConstant("viscFric", "Viscous friction", "Nms/rad", 0.1, mViscFric);
		addConstant("intLeak", "Internal leakage coefficient", "(m^3/s)/Pa", 1e-12, mIntLeak);
		addConstant("extLeak", "External leakage coefficient", "(m^3/s)/Pa", 1e-12, mExtLeak);
    }
    // The initialize function is called before simulation begins.
    // It may be called multiple times
    // In this function you can read or write from/to nodes
    // This function is optional but most likely needed
    void initialize()
    {
        mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
        mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
        mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
        mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
		
        mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
        mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
        mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
        mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
		
		mpND_p3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::Pressure);
        mpND_q3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::Flow);
        mpND_c3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::WaveVariable);
        mpND_Zc3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::CharImpedance);
		
		mpND_t4 = getSafeNodeDataPtr(mpP4, NodeMechanicRotational::Torque);
        mpND_a4 = getSafeNodeDataPtr(mpP4, NodeMechanicRotational::Angle);
        mpND_w4 = getSafeNodeDataPtr(mpP4, NodeMechanicRotational::AngularVelocity);
        mpND_c4 = getSafeNodeDataPtr(mpP4, NodeMechanicRotational::WaveVariable);
        mpND_Zx4 = getSafeNodeDataPtr(mpP4, NodeMechanicRotational::CharImpedance);
		
		mNum [0] = 1.0;
		mNum [1] = 0.0;
		mDen [0] = (mViscFric);
		mDen [1] = (mJ)*pi*pi;

		mTF.initialize(mTimestep , mNum , mDen , pi*(-(*mpND_p2)*(mDp) + (*mpND_p1)*(mDp) + (*mpND_t4)*2*pi)/2 , *mpND_w4);
		mIntegrator.initialize(mTimestep, *mpND_a4 , *mpND_w4 );
    }
    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    // This function is mandatory
    void simulateOneTimestep()
    {
        //Declare local variables
                   double p1, q1, c1, Zc1, p2, q2, c2, Zc2, p3, q3, c3, Zc3, t4, a4, w4, c4, Zx4;
                   double qIdeal, qHp, qLp, qExt, cHp, cLp, cExt, ZcHp, ZcLp, ZcExt, cli, cle, j, b, dp, n, num, eps;
                   bool cav;
                   cav = false;
                   dp = (mDp);
				   j = (mJ);
				   b = mViscFric;
				   cli = mIntLeak;
				   cle = mExtLeak;
				   
                   //Get variable values from nodes
                   c1 = (*mpND_c1);
                   Zc1 = (*mpND_Zc1);
                   c2 = (*mpND_c2);
                   Zc2 = (*mpND_Zc2);
                   c3 = (*mpND_c3);
                   Zc3 = (*mpND_Zc3);
				   c4 = (*mpND_c4);
				   Zx4 = (*mpND_Zx4);
				   eps = (*mpEps);
								
					num = -pi/2*(2*c4*pi-c3*cle*dp*Zc1+2*c4*cle*pi*Zc1+2*c4*cli*pi*Zc1+
								c3*cle*dp*Zc2+2*c4*cle*pi*Zc2+2*c4*cli*pi*Zc2+
								2*c4*cle*cle*pi*Zc1*Zc2+4*c4*cle*cli*pi*Zc1*Zc2+
								4*c4*cle*pi*Zc3+2*c4*cle*cle*pi*Zc1*Zc3+
								4*c4*cle*cli*pi*Zc1*Zc3+2*c4*cle*cle*pi*Zc2*Zc3+
								4*c4*cle*cli*pi*Zc2*Zc3+c2*dp*(1+cle*Zc1+2*cle*Zc3)-
								c1*(dp+cle*dp*(Zc2+2*Zc3)));
								
					mDen[0] = 15*dp*dp*Zc1+15*dp*dp*Zc2+30*cle*dp*dp*Zc1*Zc2+
								30*cle*dp*dp*Zc1*Zc3+30*cle*dp*dp*Zc2*Zc3+
								b*pi*pi*(1+cli*Zc1+cli*Zc2+cle*cle*(Zc2*Zc3+Zc1*(Zc2+Zc3))*+
								cle*(Zc1+Zc2+2*Zc3+2*cli*Zc2*Zc3+
								2*cli*Zc1*(Zc2+Zc3)))+pi*pi*Zx4+cle*pi*pi*Zc1*Zx4+
								cli*pi*pi*Zc1*Zx4+cle*pi*pi*Zc2*Zx4+cli*pi*pi*Zc2*Zx4+
								cle*cle*pi*pi*Zc1*Zc2*Zx4+2*cle*cli*pi*pi*Zc1*Zc2*Zx4+
								2*cle*pi*pi*Zc3*Zx4+cle*cle*pi*pi*Zc1*Zc3*Zx4+
								2*cle*cli*pi*pi*Zc1*Zc3*Zx4+cle*cle*pi*pi*Zc2*Zc3*Zx4+
								2*cle*cli*pi*pi*Zc2*Zc3*Zx4;
								
					mDen[1] = j*pi*pi*(1+cli*Zc1+cli*Zc2+cle*cle*(Zc2*Zc3+Zc1*(Zc2+Zc3))+
								cle*(Zc1+Zc2+2*Zc3+2*cli*Zc2*Zc3+2*cli*Zc1*(Zc2+Zc3)));
								
					//No leakage model for test	
					num = -pi*(-c1*dp + c2*dp + 2*c4*pi)/2;
					mDen[0] = b*pi*pi + 15*dp*dp*Zc1 + 15*dp*dp*Zc2 + pi*pi*Zx4;
					mDen[1] = j*pi*pi;

					mTF.setDen(mDen);
					w4 = mTF.update(num);
					a4 = mIntegrator.update(w4);
					t4 = c4 + Zx4*w4;

				// flow calculations
					n = w4*60/(2*pi);
					qIdeal = eps*dp*n/60.0;				   
				   
					q2 = (c1*cli + qIdeal + cle*qIdeal*Zc1 + c3*cle*(1 + cle*Zc1 + 2*cli*Zc1) +
							 c1*cle*cle*Zc3 + 2*c1*cle*cli*Zc3 + 2*cle*qIdeal*Zc3 - 
							 c2*(cle + cli + cle*cle*(Zc1 + Zc3) + 2*cle*cli*(Zc1 + Zc3)))/(1 + 
							 cli*(Zc1 + Zc2) + cle*cle*(Zc2*Zc3 + Zc1*(Zc2 + Zc3)) + 
							 cle*(Zc1 + Zc2 + 2*Zc3 + 2*cli*Zc2*Zc3 + 2*cli*Zc1*(Zc2 + Zc3)));
							
					q1 = (c3*cle + c2*cli - qIdeal + cli*q2*Zc2 + c2*cle*cle*Zc3 + 
							 2*c2*cle*cli*Zc3 - 2*cle*qIdeal*Zc3 + cle*cle*q2*Zc2*Zc3 + 
							 2*cle*cli*q2*Zc2*Zc3 - 
							 c1*(cle + cli + cle*cle*Zc3 + 2*cle*cli*Zc3))/(1 + cli*Zc1 + 
							 cle*cle*Zc1*Zc3 + cle*(Zc1 + 2*Zc3 + 2*cli*Zc1*Zc3));
							
					q3 = (cle*(c1 + c2 - 2*c3 + q1*Zc1 + q2*Zc2))/(1 + 2*cle*Zc3);				
				   
				   // pressure calculations
				   p1 = c1 + Zc1*q1;
                   p2 = c2 + Zc2*q2;                   
				   p3 = c3 + Zc3*q3;
				   
                   /* Cavitation Check */
                   if (p1 < 0.0)
                   {
                       c1 = 0.0;
                       Zc1 = 0.0;
                       cav = true;
                   }
                   if (p2 < 0.0)
                   {
                       c2 = 0.0;
                       Zc2 = 0.0;
                       cav = true;
                   }
                   if (cav)
                   {
                       q2 = (c1*cli + qIdeal + cle*qIdeal*Zc1 + c3*cle*(1 + cle*Zc1 + 2*cli*Zc1) +
							 c1*cle*cle*Zc3 + 2*c1*cle*cli*Zc3 + 2*cle*qIdeal*Zc3 - 
							 c2*(cle + cli + cle*cle*(Zc1 + Zc3) + 2*cle*cli*(Zc1 + Zc3)))/(1 + 
							 cli*(Zc1 + Zc2) + cle*cle*(Zc2*Zc3 + Zc1*(Zc2 + Zc3)) + 
							 cle*(Zc1 + Zc2 + 2*Zc3 + 2*cli*Zc2*Zc3 + 2*cli*Zc1*(Zc2 + Zc3)));
							
						q1 = (c3*cle + c2*cli - qIdeal + cli*q2*Zc2 + c2*cle*cle*Zc3 + 
							 2*c2*cle*cli*Zc3 - 2*cle*qIdeal*Zc3 + cle*cle*q2*Zc2*Zc3 + 
							 2*cle*cli*q2*Zc2*Zc3 - 
							 c1*(cle + cli + cle*cle*Zc3 + 2*cle*cli*Zc3))/(1 + cli*Zc1 + 
							 cle*cle*Zc1*Zc3 + cle*(Zc1 + 2*Zc3 + 2*cli*Zc1*Zc3));
					q3 = (cle*(c1 + c2 - 2*c3 + q1*Zc1 + q2*Zc2))/(1 + 2*cle*Zc3);	
                       p1 = c1 + Zc1 * q1;
                       p2 = c2 + Zc2 * q2;
					   p3 = c3 + Zc3 * q3;
                       if (p1 <= 0.0)
                       {
                           p1 = 0.0;
                           q2 = std::min(q2, 0.0);
                           p2 = c2;
                       }
                       if (p2 <= 0.0)
                       {
                           p2 = 0.0;
                           q2 = std::max(q2, 0.0);
                           p1 = c1;
                       }
                    }
					
                   //Write new values to nodes
					(*mpND_p1) = p1;
					(*mpND_q1) = q1;
					(*mpND_p2) = p2;
					(*mpND_q2) = q2;
					(*mpND_p3) = p3;
					(*mpND_q3) = q3;	
					(*mpND_t4) = t4;
					(*mpND_a4) = a4;
					(*mpND_w4) = w4;
		   
	}
    void finalize()
    {
        //WRITE YOUR FINALIZE CODE HERE (OPTIONAL)
    }
    void deconfigure()
    {
        //WRITE YOUR DECONFIGURATION CODE HERE (OPTIONAL)
    }
};
}
#endif // VARIABLEMACHINEW_H