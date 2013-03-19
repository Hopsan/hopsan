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
//! @file   MechanicTranslationalMassWithCoulumbFriction.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-05
//!
//! @brief Contains a translational mass with coulumb friction and damper
//$Id$

#ifndef MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTION_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTION_HPP_INCLUDED

#include <sstream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMassWithCoulombFriction : public ComponentQ
    {

    private:
        double m, B, fs, fk, xMin, xMax;                                                                        //Changeable parameters
        double wx, u0, f, be, fe;                                                                              //Local Variables
        double mLength;                                                                                     //This length is not accesible by the user, it is set from the start values by the c-components in the ends
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_me1;    //Node data pointers
        double *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2, *mpND_me2;
        double *mpND_fs, *mpND_fk;
        double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;                                                    //Node data variables
        //DoubleIntegratorWithDamping mIntegrator;                                                            //External functions
        double mNum[3];
        double mDen[3];
        DoubleIntegratorWithDampingAndCoulombFriction mIntegrator;
//        SecondOrderFilter mFilter;
//        Integrator mInt;
        Port *mpP1, *mpP2, *mpPfs, *mpPfk;                                                                                  //Ports

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMassWithCoulombFriction();
        }

        void configure()
        {
            //Set member attributes
            m = 100.0;
            B = 10;
            fs = 50;
            fk = 45;
            xMin = 0;
            xMax = 1;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");
            mpPfs = addReadPort("Pfs", "NodeSignal", Port::NotRequired);
            mpPfk = addReadPort("Pfk", "NodeSignal", Port::NotRequired);

            //Register changable parameters to the HOPSAN++ core
            registerParameter("m", "Mass", "[kg]", m);
            registerParameter("b", "Viscous Friction Coefficient", "[Ns/m]", B);
            registerParameter("f_s", "Static Friction Force", "[N]",  fs);
            registerParameter("f_k", "Kinetic Friction Force", "[N]",  fk);
            registerParameter("x_min", "Lower Limit of Position", "[m]",  xMin);
            registerParameter("x_max", "Upper Limit of Position", "[m]",  xMax);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);

            mpND_f2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);
            mpND_me2 = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);

            mpND_fs = getSafeNodeDataPtr(mpPfs, NodeSignal::Value, fs);
            mpND_fk = getSafeNodeDataPtr(mpPfk, NodeSignal::Value, fk);

            f1 = (*mpND_f1);
            x1 = (*mpND_x1);
            v1 = (*mpND_v1);
            c1 = (*mpND_c1);
            x2 = (*mpND_x2);
            c2 = (*mpND_c2);

            mLength = x1+x2;

//            //Initialize integrator
//            mNum[0] = 0.0;
//            mNum[1] = 1.0;
//            mNum[2] = 0.0;
//            mDen[0] = 0;
//            mDen[1] = b;
//            mDen[2] = m;
//            mFilter.initialize(mTimestep, mNum, mDen, -f1, -v1);
//            mInt.initialize(mTimestep, -v1, -x1+mLength);
            mIntegrator.initialize(mTimestep, 0, m, fs, fk, 0, 0, 0);

            (*mpND_me1) = m;
            (*mpND_me2) = m;

            //Print debug message if start velocities doe not match
            if((*mpND_v1) != -(*mpND_v2))
            {
                std::stringstream ss;
                ss << "Start velocities does not match, {" << getName() << "::" << mpP1->getName() <<
                        "} and {" << getName() << "::" << mpP2->getName() << "}.";
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
            fs = (*mpND_fs);
            fk = (*mpND_fk);

            mIntegrator.setFriction(fs, fk);

            mIntegrator.setDamping((B+Zx1+Zx2) / m * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/m);
            v2 = mIntegrator.valueFirst();
            x2 = mIntegrator.valueSecond();

            if(x2<xMin)
            {
                x2=xMin;
                v2=std::max(0.0, v2);
                mIntegrator.initializeValues(0.0, x2, v2);
            }
            if(x2>xMax)
            {
                x2=xMax;
                v2=std::min(0.0, v2);
                mIntegrator.initializeValues(0.0, x2, v2);
            }

            v1 = -v2;
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

#endif // MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTION_HPP_INCLUDED

