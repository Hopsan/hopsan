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
//! @file   NodeRWHelpfuncs.h
//! @author peter.nordin@liu.se
//! @date   2015-06-25
//! @brief Contains help functions for more convenient read and write access to nodes
//!
//$Id$

#ifndef NODERWHELPFUNCS_HPP
#define NODERWHELPFUNCS_HPP

#include "Nodes.h"
#include "Port.h"

namespace hopsan {

// ---------------------------------------------------------------------
// Signal Node Access (input output variables)
// ---------------------------------------------------------------------

inline double readSignalPort(Port *pPort)
{
    return pPort->readNode(NodeSignal::Value);
}

inline double readInputVariable(Port *pPort)
{
    return readSignalPort(pPort);
}

//inline double readOutputVariable(Port *pPort)
//{
//    return readSignalPort(pPort);
//}

inline void readSignalPort(Port *pPort, double &y)
{
    y = pPort->readNode(NodeSignal::Value);
}

inline void readInputVariable(Port *pPort, double &y)
{
    readSignalPort(pPort, y);
}

inline void writeSignalPort(Port *pPort, const double y)
{
    pPort->writeNode(NodeSignal::Value, y);
}

inline void writeOutputVariable(Port *pPort, const double y)
{
    writeSignalPort(pPort, y);
}

//inline double writeInputVariable(Port *pPort)
//{
//    return writeSignalPort(pPort);
//}



// ---------------------------------------------------------------------
// Hydraulic Node Access
// ---------------------------------------------------------------------

typedef struct
{
    double q;
    double p;
    double c;
    double Zc;
} HydraulicNodeDataValueStructT;

inline void readHydraulicPort_pq(Port *pPort, double &p, double &q)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    q = rData[NodeHydraulic::Flow];
    p = rData[NodeHydraulic::Pressure];
}

inline void readHydraulicPort_cZc(Port *pPort, double &c, double &Zc)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    c = rData[NodeHydraulic::WaveVariable];
    Zc = rData[NodeHydraulic::CharImpedance];
}

inline void readHydraulicPort_all(Port *pPort, double &p, double &q, double &c, double &Zc)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    q = rData[NodeHydraulic::Flow];
    p = rData[NodeHydraulic::Pressure];
    c = rData[NodeHydraulic::WaveVariable];
    Zc = rData[NodeHydraulic::CharImpedance];
}

inline void readHydraulicPort_all(Port *pPort, HydraulicNodeDataValueStructT &rValues)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    rValues.q = rData[NodeHydraulic::Flow];
    rValues.p = rData[NodeHydraulic::Pressure];
    rValues.c = rData[NodeHydraulic::WaveVariable];
    rValues.Zc = rData[NodeHydraulic::CharImpedance];
}

inline void writeHydraulicPort_pq(Port *pPort, const double p, const double q)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeHydraulic::Flow] = q;
    rData[NodeHydraulic::Pressure] = p;
}

inline void writeHydraulicPort_cZc(Port *pPort, const double c, const double Zc)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeHydraulic::WaveVariable] = c;
    rData[NodeHydraulic::CharImpedance] = Zc;
}

inline void writeHydraulicPort_all(Port *pPort, const double p, const double q, const double c, const double Zc)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeHydraulic::Flow] = q;
    rData[NodeHydraulic::Pressure] = p;
    rData[NodeHydraulic::WaveVariable] = c;
    rData[NodeHydraulic::CharImpedance] = Zc;
}

inline void writeHydraulicPort_all(Port *pPort, const HydraulicNodeDataValueStructT &rValues)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeHydraulic::Flow] = rValues.q;
    rData[NodeHydraulic::Pressure] = rValues.p;
    rData[NodeHydraulic::WaveVariable] = rValues.c;
    rData[NodeHydraulic::CharImpedance] = rValues.Zc;
}



// ---------------------------------------------------------------------
// Mechanic Node Access
// ---------------------------------------------------------------------

typedef struct
{
    double v;
    double f;
    double x;
    double c;
    double Zc;
    double me;
} MechanicNodeDataValueStructT;

inline void readMechanicPort_vfx(Port *pPort, double &v, double &f, double &x)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    v = rData[NodeMechanic::Velocity];
    f = rData[NodeMechanic::Force];
    x = rData[NodeMechanic::Position];
}

inline void readMechanicPort_cZc(Port *pPort, double &c, double &Zc)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    c = rData[NodeMechanic::WaveVariable];
    Zc = rData[NodeMechanic::CharImpedance];
}

inline void readMechanicPort_all(Port *pPort, double &v, double &f, double &x, double &c, double &Zc, double &me)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    v = rData[NodeMechanic::Velocity];
    f = rData[NodeMechanic::Force];
    x = rData[NodeMechanic::Position];
    c = rData[NodeMechanic::WaveVariable];
    Zc = rData[NodeMechanic::CharImpedance];
    me = rData[NodeMechanic::EquivalentMass];
}

inline void readMechanicPort_all(Port *pPort, MechanicNodeDataValueStructT &rValues)
{
    const std::vector<double> &rData = pPort->getNodeDataVector();
    rValues.v = rData[NodeMechanic::Velocity];
    rValues.f = rData[NodeMechanic::Force];
    rValues.x = rData[NodeMechanic::Position];
    rValues.c = rData[NodeMechanic::WaveVariable];
    rValues.Zc = rData[NodeMechanic::CharImpedance];
    rValues.me = rData[NodeMechanic::EquivalentMass];
}

inline void writeMechanicPort_vfx(Port *pPort, const double v, const double f, const double x)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeMechanic::Velocity] = v;
    rData[NodeMechanic::Force] = f;
    rData[NodeMechanic::Position] = x;
}

inline void writeMechanicPort_cZc(Port *pPort, const double c, const double Zc)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeMechanic::WaveVariable] = c;
    rData[NodeMechanic::CharImpedance] = Zc;
}

inline void writeMechanicPort_all(Port *pPort, const double v, const double f, const double x, const double c, const double Zc, const double me)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeMechanic::Velocity] = v;
    rData[NodeMechanic::Force] = f;
    rData[NodeMechanic::Position] = x;
    rData[NodeMechanic::WaveVariable] = c;
    rData[NodeMechanic::CharImpedance] = Zc;
    rData[NodeMechanic::EquivalentMass] = me;
}

inline void writeMechanicPort_all(Port *pPort, const MechanicNodeDataValueStructT &rValues)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeMechanic::Velocity] = rValues.v;
    rData[NodeMechanic::Force] = rValues.f;
    rData[NodeMechanic::Position] = rValues.x;
    rData[NodeMechanic::WaveVariable] = rValues.c;
    rData[NodeMechanic::CharImpedance] = rValues.Zc;
    rData[NodeMechanic::EquivalentMass] = rValues.me;
}

}

#endif // NODERWHELPFUNCS_HPP

