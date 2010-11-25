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

    class Port;

    class DLLIMPORTEXPORT Node
    {
        friend class Port;
        friend class Component;
        friend class ComponentSystem;

    public:
        //The user should never bother about Nodes
        void logData(const double time);  //Public because simlation threads must be able to log data

    protected:
        //Protected member functions
        Node(size_t datalength);
        NodeTypeT &getNodeType();

        enum PLOTORNOT {PLOT, NOPLOT};

        void copyNodeVariables(Node *pNode);

        void setLogSettingsNSamples(size_t nSamples, double start, double stop, double sampletime);
        void setLogSettingsSkipFactor(double factor, double start, double stop, double sampletime);
        void setLogSettingsSampleTime(double log_dt, double start, double stop, double sampletime);
        //void preAllocateLogSpace(const size_t nSlots);
        void preAllocateLogSpace();
        void saveLogData(std::string filename);

        void setData(const size_t &data_type, const double &data);
        double getData(const size_t data_type);
        double &getDataRef(const size_t data_type);
        double *getDataPtr(const size_t data_type);

        void setDataNameAndUnit(size_t id, std::string name, std::string unit, Node::PLOTORNOT plotBehaviour = Node::PLOT);
        std::string getDataName(size_t id);
        std::string getDataUnit(size_t id);
        int getDataIdFromName(const std::string name);
        void getDataNamesAndUnits(std::vector<std::string> &rNames, std::vector<std::string> &rUnits);
        void getDataNamesValuesAndUnits(std::vector<std::string> &rNames, std::vector<double> &rValues, std::vector<std::string> &rUnits);
        void setDataValuesByNames(std::vector<std::string> names, std::vector<double> values);
        int getNumberOfPortsByType(int type);

        //Protected member variables
        NodeTypeT mNodeType;
        std::vector<double> mDataVector;
        std::vector<Port*> mPortPtrs;

    private:
        //Private member fuctions
        void setPort(Port *pPort);
        void removePort(Port *pPort);
        bool isConnectedToPort(Port *pPort);
        void enableLog();
        void disableLog();

        //Private member variables
        std::string mName;
        std::vector<std::string> mDataNames;
        std::vector<std::string> mDataUnits;
        std::vector<Node::PLOTORNOT> mPlotBehaviour;

        //Log specific
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
    extern NodeFactory gCoreNodeFactory;
    DLLIMPORTEXPORT NodeFactory* getCoreNodeFactoryPtr();
}

#endif // NODE_H_INCLUDED
