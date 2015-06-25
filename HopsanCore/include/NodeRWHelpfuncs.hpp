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

inline double readSignalPort(Port *pPort)
{
    return pPort->readNode(NodeSignal::Value);
}

inline void readSignalPort(Port *pPort, double &y)
{
    y = pPort->readNode(NodeSignal::Value);
}

inline void writeSignalPort(Port *pPort, const double y)
{
    pPort->writeNode(NodeSignal::Value, y);
}

inline void readHydraulicPort_pq(Port *pPort, double &p, double &q)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    q = rData[NodeHydraulic::Flow];
    p = rData[NodeHydraulic::Pressure];
}

inline void readHydraulicPort_cZc(Port *pPort, double &c, double &Zc)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    c = rData[NodeHydraulic::WaveVariable];
    Zc = rData[NodeHydraulic::CharImpedance];
}

inline void readHydraulicPort_all(Port *pPort, double &p, double &q, double &c, double &Zc)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
    q = rData[NodeHydraulic::Flow];
    p = rData[NodeHydraulic::Pressure];
    c = rData[NodeHydraulic::WaveVariable];
    Zc = rData[NodeHydraulic::CharImpedance];
}

inline void readHydraulicPort_all(Port *pPort, HydraulicNodeDataValueStructT &rValues)
{
    std::vector<double> &rData = pPort->getNodeDataVector();
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

}

#endif // NODERWHELPFUNCS_HPP

