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
//! @file   MechanicTranslationalMassWithCoulombFrictionAndMultiPorts.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-07-07
//!
//! @brief Contains a translational mass with coulomb friction and damper using multi-ports (converted from version without multi-ports)
//$Id$

#ifndef MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTIONANDMULTIPORTS_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTIONANDMULTIPORTS_HPP_INCLUDED

#include <sstream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMassWithCoulombFrictionAndMultiPorts : public ComponentQ
    {

    private:
        double m;
        double *mpB, *mpFs, *mpFk, *xMin, *xMax;                                        //Changeable parameters
        double wx, u0, f, be, fe;                                                       //Local Variables
        double mLength;                                                                 //This length is not accessible by the user, it is set from the start values by the c-components in the ends
        std::vector<double> x1, x2, mvStartX1, mvStartX2;
        size_t mNumPorts1, mNumPorts2;
        double mNum[3];
        double mDen[3];
        DoubleIntegratorWithDampingAndCoulombFriction mIntegrator;

        // Ports and node data variables
        Port *mpP1, *mpP2;
        std::vector<double*> mvpP1_f, mvpP1_x, mvpP1_v, mvpP1_me, mvpP1_c, mvpP1_Zx;
        std::vector<double*> mvpP2_f, mvpP2_x, mvpP2_v, mvpP2_me, mvpP2_c, mvpP2_Zx;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMassWithCoulombFrictionAndMultiPorts();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeMechanic");
            mpP2 = addPowerMultiPort("P2", "NodeMechanic");

            //Register changeable parameters to the HOPSAN++ core
            addConstant("m", "Mass", "kg", 100.0, m);

            addInputVariable("b", "Viscous Friction Coefficient", "Ns/m", 10, &mpB);
            addInputVariable("f_s", "Static Friction Force", "N", 50,  &mpFs);
            addInputVariable("f_k", "Kinetic Friction Force", "N", 45,  &mpFk);
            addInputVariable("x_min", "Lower Limit of Position of Port P2", "m", -1.0e+300,  &xMin);
            addInputVariable("x_max", "Upper Limit of Position of Port P2", "m", 1.0e+300,  &xMax);
        }


        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mNumPorts2 = mpP2->getNumPorts();

            mvpP1_f.resize(mNumPorts1);
            mvpP1_x.resize(mNumPorts1);
            mvpP1_v.resize(mNumPorts1);
            mvpP1_me.resize(mNumPorts1);
            mvpP1_c.resize(mNumPorts1);
            mvpP1_Zx.resize(mNumPorts1);

            mvpP2_f.resize(mNumPorts2);
            mvpP2_x.resize(mNumPorts2);
            mvpP2_v.resize(mNumPorts2);
            mvpP2_me.resize(mNumPorts2);
            mvpP2_c.resize(mNumPorts2);
            mvpP2_Zx.resize(mNumPorts2);

            x1.resize(mNumPorts1);
            x2.resize(mNumPorts2);
            mvStartX1.resize(mNumPorts1);
            mvStartX2.resize(mNumPorts2);

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                mvpP1_f[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::FORCE);
                mvpP1_x[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Position);
                mvpP1_v[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Velocity);
                mvpP1_me[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::EquivalentMass);
                mvpP1_c[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::WaveVariable);
                mvpP1_Zx[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::CharImpedance);
            }

            for (size_t i=0; i<mNumPorts2; ++i)
            {
                mvpP2_f[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanic::Force);
                mvpP2_x[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanic::Position);
                mvpP2_v[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanic::Velocity);
                mvpP2_me[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanic::EquivalentMass);
                mvpP2_c[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanic::WaveVariable);
                mvpP2_Zx[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanic::CharImpedance);
            }

            double f1, v1, f2, v2;

            //Initialization
            f1 = 0;
            v1 = (*mvpP1_v[0]);
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                f1 += (*mvpP1_f[i]);
                x1[i] = (*mvpP1_x[i]);
                mvStartX1[i]= (*mvpP1_x[i]);
                if(v1 != (*mvpP1_v[i]))
                {
                    addErrorMessage("Velocities in multiport does not match, {"+getName()+"::"+mpP1->getName());
                    stopSimulation();
                }
            }

            f2 = 0;
            v2 = (*mvpP2_v[0]);
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                f2 += (*mvpP2_f[i]);
                x2[i] = (*mvpP2_x[i]);
                mvStartX2[i] = (*mvpP2_x[i]);
                if(v2 != (*mvpP2_v[i]))
                {
                    addWarningMessage("Velocities in multiport does not match, {"+getName()+"::"+mpP2->getName());
                    stopSimulation();
                }
            }

            //Initialize integrator
            mIntegrator.initialize(mTimestep, 0, (*mpFs)/m, (*mpFk)/m, 0, x2[0], v2);

            //Print debug message if velocities do not match
            if(v1 != -v2)
            {
                addWarningMessage("Start velocities does not match, {"+getName()+"::"+mpP1->getName()+
                                  "} and {"+getName()+"::"+mpP2->getName()+"}.");
                stopSimulation();
            }

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpP1_me[i]) = m;
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                (*mvpP2_me[i]) = m;
            }
        }


        void simulateOneTimestep()
        {
            double v1, c1, Zx1, v2, c2, Zx2;

            //Get variable values from nodes
            c1 = 0;
            Zx1 = 0;
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                c1 += (*mvpP1_c[i]);
                Zx1 += (*mvpP1_Zx[i]);
            }

            c2 = 0;
            Zx2 = 0;
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                c2 += (*mvpP2_c[i]);
                Zx2 += (*mvpP2_Zx[i]);
            }

            mIntegrator.setFriction((*mpFs)/m, (*mpFk)/m);

            mIntegrator.setDamping(((*mpB)+Zx1+Zx2) / m * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/m);
            v2 = mIntegrator.valueFirst();
            double x_nom = mIntegrator.valueSecond();

            if(x_nom<(*xMin))
            {
                x_nom=(*xMin);
                v2=std::max(0.0, v2);
                mIntegrator.initializeValues(0.0, x_nom, v2);
            }
            if(x_nom>(*xMax))
            {
                x_nom=(*xMax);
                v2=std::min(0.0, v2);
                mIntegrator.initializeValues(0.0, x_nom, v2);
            }

            v1 = -v2;

            //Write new values to nodes
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpP1_f[i]) = (*mvpP1_c[i]) + (*mvpP1_Zx[i])*v1;
                (*mvpP1_x[i]) = mvStartX1[i]+mvStartX2[0]-x_nom;
                (*mvpP1_v[i]) = v1;
                (*mvpP1_me[i]) = m;
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                (*mvpP2_f[i]) = (*mvpP2_c[i]) + (*mvpP2_Zx[i])*v2;
                (*mvpP2_x[i]) = mvStartX2[i]-mvStartX2[0]+x_nom;
                (*mvpP2_v[i]) = v2;
                (*mvpP2_me[i]) = m;
            }
        }
    };
}

#endif // MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTIONANDMULTIPORTS_HPP_INCLUDED

