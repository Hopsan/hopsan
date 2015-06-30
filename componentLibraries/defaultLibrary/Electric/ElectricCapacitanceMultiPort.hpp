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
//! @file   ElectricCapacitanceMultiPort.hpp
//! @author Petter Krus 
//! @date   2012-01-30
//! based on ElectricCapacitanceMultiPort.hpp by Björn Eriksson <bjorn.eriksson@liu.se>
//! @brief Contains a Electric Volume Component
//!

#ifndef ELECTRICCAPACITANCEMULTIPORT_HPP_INCLUDED
#define ELECTRICCAPACITANCEMULTIPORT_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>
#include <sstream>
#include <iostream>

namespace hopsan {

    //!
    //! @brief A Electric volume component
    //! @ingroup ElectricComponents
    //!
    class ElectricCapacitanceMultiPort : public ComponentC
    {

    private:
        double Zc;
        double *mpAlpha;
        double capacitance;

        std::vector<double*> mvpN_uel, mvpN_iel, mvpN_cel, mvpN_Zcel;
        std::vector<double> mvp_C0;
        size_t mNumPorts;
        Port *mpPel1;

    public:
        static Component *Creator()
        {
            return new ElectricCapacitanceMultiPort();
        }

        void configure()
        {
            mpPel1 = addPowerMultiPort("Pel1", "NodeElectric");
            addConstant("Capacitance", "Capacitance", "Fa", 0.0001, capacitance);
            addInputVariable("alpha", "Low pass coeficient to dampen standing delayline waves", "-", 0.3, &mpAlpha);
            setDefaultStartValue(mpPel1, NodeElectric::Current, 0.0);
            setDefaultStartValue(mpPel1, NodeElectric::Voltage, 12);
        }


        void initialize()
        {
            double alpha;
            alpha = (*mpAlpha);

            mNumPorts = mpPel1->getNumPorts();
            mvpN_uel.resize(mNumPorts);
            mvpN_iel.resize(mNumPorts);
            mvpN_cel.resize(mNumPorts);
            mvpN_Zcel.resize(mNumPorts);
            mvp_C0.resize(mNumPorts);

            Zc = double(mNumPorts)*mTimestep/(2.0*capacitance)/(1.0-alpha);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpN_uel[i]  = getSafeMultiPortNodeDataPtr(mpPel1, i, NodeElectric::Voltage, 0.0);
                mvpN_iel[i]  = getSafeMultiPortNodeDataPtr(mpPel1, i, NodeElectric::Current, 0.0);
                mvpN_cel[i]  = getSafeMultiPortNodeDataPtr(mpPel1, i, NodeElectric::WaveVariable, 0.0);
                mvpN_Zcel[i] = getSafeMultiPortNodeDataPtr(mpPel1, i, NodeElectric::CharImpedance, 0.0);

                *mvpN_uel[i] = getDefaultStartValue(mpPel1, NodeElectric::Voltage);
                *mvpN_iel[i] = getDefaultStartValue(mpPel1, NodeElectric::Current)/double(mNumPorts);
                *mvpN_cel[i] = getDefaultStartValue(mpPel1, NodeElectric::Voltage);
                *mvpN_Zcel[i] = Zc;
            }
        }


        void simulateOneTimestep()
        {
            double alpha;
            alpha = (*mpAlpha);

            double cTot = 0.0;
            double uAvg;

            for (size_t i=0; i<mNumPorts; ++i)
            {
                cTot += (*mvpN_cel[i]) + 2.0*Zc*(*mvpN_iel[i]);
            }
            uAvg = cTot/double(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvp_C0[i] = uAvg*2.0-(*mvpN_cel[i]) - 2.0*Zc*(*mvpN_iel[i]);
                (*mvpN_cel[i]) = alpha*(*mvpN_cel[i]) + (1.0-alpha)*mvp_C0[i];
                (*mvpN_Zcel[i]) = Zc;
            }
        }


        void finalize()
        {
        }
    };
}

#endif // ELECTRICCAPCITANCEMULTIPORT_HPP_INCLUDED
