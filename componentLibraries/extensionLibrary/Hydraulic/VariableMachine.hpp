/*-----------------------------------------------------------------------------

//! @file   VariableMachine.hpp
//! @author Samuel Kärnell <samuel.karnell@liu.se>
//! @date   2021-01-07

-----------------------------------------------------------------------------*/
// Header guard to avoid inclusion of the same code twice
// Every hpp file in your library need to have its own UNIQUE header guard
#ifndef VARIABLEMACHINE_H
#define VARIABLEMACHINE_H
// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>
// Put your component class inside the hopsan namespace (optional)
namespace hopsan {
// Define a new Class that inherits from ComponentC, ComponentQ or ComponentSignal
// This depends on the type of component you want to create, a C, Q or signal component
class VariableMachine : public ComponentQ
{
private:
    double *mpN, *mpT, *mpEps;
    double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2,*mpND_p3, *mpND_q3, *mpND_c3, *mpND_Zc3;
	double mViscFric, mExtLeak, mIntLeak, mDp;
    Port *mpP1, *mpP2, *mpP3;
public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    // This static function is mandatory
    static Component *Creator()
    {
        return new VariableMachine();
    }
    // The Configure function that is run ONCE when a new object of the class is created
    // Use this function to set initial member variable values and to register Ports, Parameters and Startvalues
    // This function is mandatory
    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");
		mpP3 = addPowerPort("P3", "NodeHydraulic");
        addOutputVariable("torque", "Torque (inertia is neglected)", "", 0.0, &mpT);
        addInputVariable("n_p", "Angular Velocity", "rpm", 1500.0, &mpN);
		addInputVariable("eps", "Displacement setting", "-", 1, &mpEps);
        addConstant("D_p", "Displacement", "m^3/rev", 60e-6, mDp);
		addConstant("viscFric", "Viscous friction", "Nms/rad", 0.1, mViscFric);
		addConstant("intLeak", "Internal leakage coefficient", "(m^3/s)/Pa", 1e-012, mIntLeak);
		addConstant("extLeak", "External leakage coefficient", "(m^3/s)/Pa", 1e-012, mExtLeak);
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
    }
    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    // This function is mandatory
    void simulateOneTimestep()
    {
        //Declare local variables
                   double p1, q1, c1, Zc1, p2, q2, c2, Zc2, p3, q3, c3, Zc3;
                   double qIdeal, dp, n, tIdeal, t, cli, cle, eps;
                   bool cav;
                   cav = false;
				   
                   dp = (mDp);
				   eps = (*mpEps);
				   n = (*mpN);
				   cli = mIntLeak;
				   cle = mExtLeak;

                   c1 = (*mpND_c1);
                   Zc1 = (*mpND_Zc1);
                   c2 = (*mpND_c2);
                   Zc2 = (*mpND_Zc2);
                   c3 = (*mpND_c3);
                   Zc3 = (*mpND_Zc3);		   			   

				// flow calculations
					limitValue(eps, -1, 1);
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
				   				   
					// torque calculations
					tIdeal = -dp*(p2 - p1)/(2*pi)*sign(n); //inertia is neglected
					t = tIdeal - mViscFric*n*2*pi/60;
					
					// pressure calculations
					p1 = c1 + Zc1 * q1;
					p2 = c2 + Zc2 * q2;
					p3 = c3 + Zc3 * q3;
								   
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
					(*mpT) = t;
				   
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
#endif // VARIABLEMACHINE_H