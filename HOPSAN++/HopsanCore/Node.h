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

//! @file   Node.h
//! @author FluMeS
//! @date   2009-12-20
//! @brief Contains Node base classes
//!
//$Id$

#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <vector>
#include <string>
#include "CoreUtilities/ClassFactory.hpp"
#include "win32dll.h"

namespace hopsan {

    typedef std::string NodeTypeT;

    //Forward Declarations
    class Port;
    class Component;
    class ComponentSystem;
    class ConnectionAssistant;
    class HopsanEssentials;

    class DLLIMPORTEXPORT Node
    {
        friend class Port;
        friend class PowerPort;
        friend class WritePort;
        friend class Component;
        friend class ComponentSystem;
        friend class ConnectionAssistant;
        friend class HopsanEssentials;

    public:
        //The user should never bother about Nodes
        void logData(const double time);  //Public because simulation threads must be able to log data
        void setData(const size_t data_type, const double data);
        Component *getWritePortComponentPtr();

    protected:
        //Protected member functions
        Node(size_t datalength);
        const NodeTypeT getNodeType() const;

        enum PLOTORNOT {PLOT, NOPLOT};

        void copyNodeVariables(Node *pNode);
        virtual void setSpecialStartValues(Node *pNode);

        void setLogSettingsNSamples(int nSamples, double start, double stop, double sampletime);
        void setLogSettingsSkipFactor(double factor, double start, double stop, double sampletime);
        void setLogSettingsSampleTime(double log_dt, double start, double stop, double sampletime);
        bool preAllocateLogSpace();
        void saveLogData(std::string filename);

        //void setData is now public!
        const double getData(const size_t data_type) const;
        double *getDataPtr(const size_t data_type);

        void setDataCharacteristics(size_t id, std::string name, std::string unit, Node::PLOTORNOT plotBehaviour = Node::PLOT);
        void getDataNameAndUnit(const size_t id, std::string &rName, std::string &rUnit);
        int getDataIdFromName(const std::string name);
        void getDataNamesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rUnits, bool getAll=false);
        void getDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<double> &rValues, std::vector<std::string> &rUnits, bool getAll=false);
        bool setDataValuesByNames(std::vector<std::string> names, std::vector<double> values);
        int getNumberOfPortsByType(int type);

        ComponentSystem *getOwnerSystem();

        //Protected member variables
        std::vector<double> mDataVector;
        std::vector<Port*> mPortPtrs;

        //WAS: Private member variables, be made them protected to get access in inherented classes
        std::string mName;
        std::vector<std::string> mDataNames;
        std::vector<std::string> mDataUnits;
        std::vector<Node::PLOTORNOT> mPlotBehaviour;
        ComponentSystem *mpOwnerSystem;

    private:
        //Private member fuctions
        void setPort(Port *pPort);
        void removePort(Port *pPort);
        bool isConnectedToPort(Port *pPort);
        void enableLog();
        void disableLog();

        //Private member variables
        NodeTypeT mNodeType;

        //Log specific variables
        std::vector<double> mTimeStorage;
        std::vector<std::vector<double> > mDataStorage;
        bool mLogSpaceAllocated;
        bool mDoLog;
        double mLogTimeDt;
        double mLastLogTime;
        size_t mLogSlots;
        size_t mLogCtr;
    };

    typedef ClassFactory<NodeTypeT, Node> NodeFactory;
}

#endif // NODE_H_INCLUDED
