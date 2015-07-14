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

inline double readSignal(double *pNodeData)
{
    return *pNodeData;
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

class HydraulicNodeDataValueStructT
{
public:
    double q;
    double p;
    double c;
    double Zc;
};

class HydraulicNodeDataPointerStructT
{
public:
    double *pQ;
    double *pP;
    double *pC;
    double *pZc;

    inline double q() const
    {
        return *pQ;
    }

    inline double p() const
    {
        return *pP;
    }

    inline double c() const
    {
        return *pC;
    }

    inline double Zc() const
    {
        return *pZc;
    }

    inline double &rq()
    {
        return *pQ;
    }

    inline double &rp()
    {
        return *pP;
    }

    inline double &rc()
    {
        return *pC;
    }

    inline double &rZc()
    {
        return *pZc;
    }
};

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

inline void getHydraulicPortNodeDataPointers(Port *pPort, HydraulicNodeDataPointerStructT &rPointers)
{
    rPointers.pQ  = pPort->getNodeDataPtr(NodeHydraulic::Flow, 0);
    rPointers.pP  = pPort->getNodeDataPtr(NodeHydraulic::Pressure, 0);
    rPointers.pC  = pPort->getNodeDataPtr(NodeHydraulic::WaveVariable, 0);
    rPointers.pZc = pPort->getNodeDataPtr(NodeHydraulic::CharImpedance, 0);
}

inline void getHydraulicMultiPortValues_pq(Port *pMainPort, const size_t subPortIdx, std::vector<HydraulicNodeDataValueStructT> &rValues)
{
    const std::vector<double> &rData = pMainPort->getNodeDataVector(subPortIdx);
    rValues[subPortIdx].q = rData[NodeHydraulic::Flow];
    rValues[subPortIdx].p = rData[NodeHydraulic::Pressure];
//    rValues[subPortIdx].c = rData[NodeHydraulic::WaveVariable];
//    rValues[subPortIdx].Zc = rData[NodeHydraulic::CharImpedance];
}

inline void getHydraulicMultiPortValues_cZc(Port *pMainPort, const size_t subPortIdx, std::vector<HydraulicNodeDataValueStructT> &rValues)
{
    const std::vector<double> &rData = pMainPort->getNodeDataVector(subPortIdx);
//    rValues[subPortIdx].q = rData[NodeHydraulic::Flow];
//    rValues[subPortIdx].p = rData[NodeHydraulic::Pressure];
    rValues[subPortIdx].c = rData[NodeHydraulic::WaveVariable];
    rValues[subPortIdx].Zc = rData[NodeHydraulic::CharImpedance];
}

inline void readHydraulicMultiPortValues_all(Port *pMainPort, const size_t subPortIdx, std::vector<HydraulicNodeDataValueStructT> &rValues)
{
    const std::vector<double> &rData = pMainPort->getNodeDataVector(subPortIdx);
    rValues[subPortIdx].q = rData[NodeHydraulic::Flow];
    rValues[subPortIdx].p = rData[NodeHydraulic::Pressure];
    rValues[subPortIdx].c = rData[NodeHydraulic::WaveVariable];
    rValues[subPortIdx].Zc = rData[NodeHydraulic::CharImpedance];
}

inline void readHydraulicMultiPortValues_all(Port *pMainPort, std::vector<HydraulicNodeDataValueStructT> &rValues)
{
    for (size_t i=0; i<pMainPort->getNumPorts(); ++i)
    {
        const std::vector<double> &rData = pMainPort->getNodeDataVector(i);
        rValues[i].q = rData[NodeHydraulic::Flow];
        rValues[i].p = rData[NodeHydraulic::Pressure];
        rValues[i].c = rData[NodeHydraulic::WaveVariable];
        rValues[i].Zc = rData[NodeHydraulic::CharImpedance];
    }
}

inline void getHydraulicMultiPortNodeDataPointers(Port *pMainPort, const size_t subPortIdx, HydraulicNodeDataPointerStructT &rPointers)
{
    rPointers.pQ  = pMainPort->getNodeDataPtr(NodeHydraulic::Flow, subPortIdx);
    rPointers.pP  = pMainPort->getNodeDataPtr(NodeHydraulic::Pressure, subPortIdx);
    rPointers.pC  = pMainPort->getNodeDataPtr(NodeHydraulic::WaveVariable, subPortIdx);
    rPointers.pZc = pMainPort->getNodeDataPtr(NodeHydraulic::CharImpedance, subPortIdx);
}

inline void getHydraulicMultiPortNodeDataPointers(Port *pMainPort, std::vector<HydraulicNodeDataPointerStructT> &rPointers)
{
    // nsp = numSubPorts
    // spi = subPortIndex
    const size_t nsp = pMainPort->getNumPorts();
    rPointers.resize(nsp);
    for (size_t spi=0; spi<nsp; ++spi)
    {
        getHydraulicMultiPortNodeDataPointers(pMainPort, spi, rPointers[spi]);
    }
}

inline void writeHydraulicPort_pq(Port *pPort, const double p, const double q)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeHydraulic::Flow] = q;
    rData[NodeHydraulic::Pressure] = p;
}

inline void writeHydraulicMultiPort_pq(Port *pPort, const size_t subPortIdx, const double p, const double q)
{
    std::vector<double> &rData = pPort->getNodeDataVector(subPortIdx);
    rData[NodeHydraulic::Flow] = q;
    rData[NodeHydraulic::Pressure] = p;
}

inline void writeHydraulicPort_cZc(Port *pPort, const double c, const double Zc)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    rData[NodeHydraulic::WaveVariable] = c;
    rData[NodeHydraulic::CharImpedance] = Zc;
}

inline void writeHydraulicMultiPort_cZc(Port *pPort, const size_t subPortIdx, const double c, const double Zc)
{
    std::vector<double> &rData = pPort->getNodeDataVector(subPortIdx);
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

class MechanicNodeDataPointerStructT
{
public:
    double *pV;
    double *pF;
    double *pX;
    double *pC;
    double *pZc;
    double *pMe;

    inline double v() const
    {
        return *pV;
    }

    inline double f() const
    {
        return *pF;
    }

    inline double x() const
    {
        return *pX;
    }

    inline double c() const
    {
        return *pC;
    }

    inline double Zc() const
    {
        return *pZc;
    }

    inline double me() const
    {
        return *pMe;
    }

    inline double &rv()
    {
        return *pV;
    }

    inline double &rf()
    {
        return *pF;
    }

    inline double &rx()
    {
        return *pX;
    }

    inline double &rc()
    {
        return *pC;
    }

    inline double &rZc()
    {
        return *pZc;
    }

    inline double &rMe()
    {
        return *pMe;
    }
};

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

inline void getMechanicPortNodeDataPointers(Port *pPort, MechanicNodeDataPointerStructT &rPointers)
{
    rPointers.pV   = pPort->getNodeDataPtr(NodeMechanic::Velocity, 0);
    rPointers.pF   = pPort->getNodeDataPtr(NodeMechanic::Force, 0);
    rPointers.pX   = pPort->getNodeDataPtr(NodeMechanic::Position, 0);
    rPointers.pC   = pPort->getNodeDataPtr(NodeMechanic::WaveVariable, 0);
    rPointers.pZc  = pPort->getNodeDataPtr(NodeMechanic::CharImpedance, 0);
    rPointers.pMe  = pPort->getNodeDataPtr(NodeMechanic::EquivalentMass, 0);
}

}

#endif // NODERWHELPFUNCS_HPP

