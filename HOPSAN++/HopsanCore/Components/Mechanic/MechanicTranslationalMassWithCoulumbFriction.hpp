//!
//! @file   MechanicTranslationalMassWithCoulumbFriction.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-05
//!
//! @brief Contains a translational mass with coulumb friction and damper
//!
//$Id$

#ifndef MECHANICTRANSLATIONALMASSWITHCOULUMBFRICTION_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASSWITHCOULUMBFRICTION_HPP_INCLUDED

#include <sstream>

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMassWithCoulumbFriction : public ComponentQ
    {

    private:
        double m, b, fs, fk, xmin, xmax;                                                                        //Changeable parameters
        double wx, u0, f, be, fe;                                                                              //Local Variables
        double mLength;                                                                                     //This length is not accesible by the user, it is set from the start values by the c-components in the ends
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2;  //Node data pointers
        double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;                                                    //Node data variables
        //DoubleIntegratorWithDamping mIntegrator;                                                            //External functions
        double mNum[3];
        double mDen[3];
        SecondOrderFilter mFilter;
        Integrator mInt;
        Port *mpP1, *mpP2;                                                                                  //Ports

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMassWithCoulumbFriction("TranslationalMassWithCoulumbFriction");
        }

        MechanicTranslationalMassWithCoulumbFriction(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            m = 100.0;
            b = 10;
            fs = 50;
            fk = 45;
            xmin = 0;
            xmax = 1;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("m", "Mass", "[kg]", m);
            registerParameter("b", "Viscous Friction Coefficient", "[Ns/m]", b);
            registerParameter("f_s", "Static Friction Force", "[N]",  fs);
            registerParameter("f_k", "Kinetic Friction Force", "[N]",  fk);
            registerParameter("x_min", "Lower Limit of Position", "[m]",  xmin);
            registerParameter("x_max", "Upper Limit of Position", "[m]",  xmax);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);

            mpND_f2 = getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::POSITION);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);

            f1 = (*mpND_f1);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);
            c1 = (*mpND_c1);
            x2 = (*mpND_x2);
            c2 = (*mpND_c2);

            mLength = x1+x2;

            //Initialize integrator
            mNum[0] = 0.0;
            mNum[1] = 1.0;
            mNum[2] = 0.0;
            mDen[0] = m;
            mDen[1] = b;
            mDen[2] = 0;
            mFilter.initialize(mTimestep, mNum, mDen, -f1, -v1);
            mInt.initialize(mTimestep, -v1, -x1+mLength);

            //Print debug message if start velocities doe not match
            if(mpP1->readNode(NodeMechanic::VELOCITY) != -mpP2->readNode(NodeMechanic::VELOCITY))
            {
                std::stringstream ss;
                ss << "Start velocities does not match, {" << getName() << "::" << mpP1->getPortName() <<
                        "} and {" << getName() << "::" << mpP2->getPortName() << "}.";
                this->addDebugMessage(ss.str());
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            x1 = (*mpND_x1);
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            x2 = (*mpND_x2);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations
            // f = external forces
            // fs = static friction force
            // fk = kinetic friction force
            // fe = effective friction force
            mDen[1] = b+Zx1+Zx2;
            mFilter.setDen(mDen);
            f = c1 - c2;
            if(f > -fs && f < fs)       //External forces are smaller than coulumb friction, so mass is not moving
            {
                fe = f;
            }
            else if(f > fs)            //External forces are positive and larger than coulumb friction
            {
                fe = fk;
            }
            else if(f < -fs)           //External forces are negative and larger than coulumb friction
            {
                fe = -fk;
            }
            v2 = mFilter.update(f-fe);
            v1 = -v2;
            x2 = mInt.update(v2);
            if(x2>xmax)
            {
                x2=xmax;
                v2=0;
                v1=0;
            }
            else if(x2<xmin)
            {
                x2=xmin;
                v2=0;
                v1=0;
            }
            x1 = -x2 + mLength;
            f1 = c1 + Zx1*v1;
            f2 = c2 + Zx2*v2;



            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_f2) = f2;
            (*mpND_x2) = x2;
            (*mpND_v2) = v2;
        }
    };
}

#endif // MECHANICTRANSLATIONALMASSWITHCOULUMBFRICTION_HPP_INCLUDED

