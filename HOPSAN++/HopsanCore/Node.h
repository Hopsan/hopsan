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
#include "CoreUtilities/ClassFactory.h"
#include "win32dll.h"

using namespace std;

typedef string NodeTypeT;

class Port; //forward declaration

class DLLIMPORTEXPORT Node
{
    friend class Port;
    friend class Component;
    friend class ComponentSystem;

public:
    //The user should never bother about Nodes

protected:
    //Protected member functions
    Node(size_t datalength);
    NodeTypeT &getNodeType();

    void setLogSettingsNSamples(size_t nSamples, double start, double stop, double sampletime);
    void setLogSettingsSkipFactor(double factor, double start, double stop, double sampletime);
    void setLogSettingsSampleTime(double log_dt, double start, double stop, double sampletime);
    //void preAllocateLogSpace(const size_t nSlots);
    void preAllocateLogSpace();
    void logData(const double time);
    void saveLogData(string filename);

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);
    double &getDataRef(const size_t data_type);

    void setDataNameAndUnit(size_t id, string name, string unit);
    string getDataName(size_t id);
    string getDataUnit(size_t id);
    int getDataIdFromName(const string name);
    void getDataNamesAndUnits(vector<string> &rNames, vector<string> &rUnits);

    //Protected member variables
    NodeTypeT mNodeType;
    vector<double> mDataVector;
    vector<Port*> mPortPtrs;

private:
    //Private member fuctions
    void setPort(Port *pPort);
    void removePort(Port *pPort);
    bool isConnectedToPort(Port *pPort);
    void enableLog();
    void disableLog();

    //Private member variables
    string mName;
    vector<string> mDataNames;
    vector<string> mDataUnits;
    
    //Log specific
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;
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

#endif // NODE_H_INCLUDED
