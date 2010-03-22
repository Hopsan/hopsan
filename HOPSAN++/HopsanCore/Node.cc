//!
//! @file   Node.cc
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Node base classes
//!
//$Id$

//! @defgroup Nodes Nodes
//! @class Node
//! @brief The Node base class
//! @ingroup Nodes

#include <fstream>
#include <cassert>
#include <iostream>
#include "Node.h"

//! Node base class constructor
Node::Node(size_t datalength)
{
    mNodeType = "Node";
    mDataVector.clear();
    mDataStorage.clear();
    mTimeStorage.clear();
    mPortPtrs.clear();

    //Resize
    mDataVector.resize(datalength,0.0);
    mDataNames.resize(datalength,"");
    mDataUnits.resize(datalength,"");

    mLogSpaceAllocated = false;
    mLogCtr = 0;

    //Default log node allways
    mDoLog = true;
    mLogTimeDt = 0.0;
    mLastLogTime = -10e10; //! @todo Find better value like -inf or nearby
}

//!
//! @brief returns the node type
//!
NodeTypeT &Node::getNodeType()
{
    return mNodeType;
}

//!
//! @brief set data in node
//! @param [in] data_type Identifier for the typ of node data to set
//! @param [in] data The data value
//!
void Node::setData(const size_t data_type, double data)
{
    mDataVector[data_type] = data;
}

//!
//! @brief get data from node
//! @param [in] data_type Identifier for the typ of node data to set
//! @return The data value
//!
double Node::getData(const size_t data_type)
{
    return mDataVector[data_type];
}

double &Node::getDataRef(const size_t data_type)
{
    return mDataVector[data_type];
}

//! Set data name and unit for a specified data variable
//! @param [in] id This is the ENUM data id
//! @param [in,out] name The variable name
//! @param [in,out] unit The variable unit
void Node::setDataNameAndUnit(size_t id, string name, string unit)
{
    mDataNames[id] = name;
    mDataUnits[id] = unit;
}

//! Get a specific data name
//! @param [in] id This is the ENUM data id
string Node::getDataName(size_t id)
{
    return mDataNames[id];
}

//! Get a specific data unit
//! @param [in] id This is the ENUM data id
string Node::getDataUnit(size_t id)
{
    return mDataUnits[id];
}


//! Get all data names and units
//! @param [in,out] rNames This vector will contain the names
//! @param [in,out] rUnits This vector will contain the units
void Node::getDataNamesAndUnits(vector<string> &rNames, vector<string> &rUnits)
{
    rNames = mDataNames;
    rUnits = mDataUnits;
}

//! This function will set log data slots for preallocation and logDt based on the number of samples that should be loged
void Node::setLogSettingsNSamples(int nSamples, double start, double stop, double sampletime)
{
    mLogSlots = nSamples;
    mLogTimeDt = (stop - start) / (double)nSamples;
    mLastLogTime = start-mLogTimeDt;
    //mLogTimeDt -= sampletime/2.0; //This is needed to avoid rounding problems in = comparison
    //! @todo Maybe round to neerest nice time number
}

void Node::setLogSettingsSkipFactor(double factor, double start, double stop,  double sampletime)
{
    //! @todo make sure factor is not less then 1.0
    //! @todo maybe only use integer factors
    mLogTimeDt = sampletime * factor;
    mLastLogTime = start-mLogTimeDt;
    mLogSlots = (size_t)((stop-start)/mLogTimeDt+0.5); //Round to nearest

    //mLogTimeDt -= sampletime/2.0; //This is needed to avoid rounding problems in = comparison
    //! @todo Maybe round to neerest nice time number
}

void Node::setLogSettingsSampleTime(double log_dt, double start, double stop,  double sampletime)
{
    //! @todo make sure that we dont have log_dt lower than sampletime ( we cant log more then we calc
    mLogTimeDt = log_dt;
    mLastLogTime = start-mLogTimeDt;
    mLogSlots = (size_t)((stop-start)/log_dt+0.5); //Round to nearest

    //mLogTimeDt -= sampletime/2.0; //This is needed to avoid rounding problems in = comparison
    //! @todo Maybe round to neerest nice time number
}

//void Node::preAllocateLogSpace(const size_t nSlots)
//{
//    size_t data_size = mDataVector.size();
//    mTimeStorage.resize(nSlots);
//    mDataStorage.resize(nSlots, vector<double>(data_size));
//
//    cout << "requestedSize: " << nSlots << " " << data_size << " Capacities: " << mTimeStorage.capacity() << " " << mDataStorage.capacity() << " " << mDataStorage[1].capacity() << " Size: " << mTimeStorage.size() << " " << mDataStorage.size() << " " << mDataStorage[1].size() << endl;
//    mLogSpaceAllocated = true;
//
//    //Make sure the ctr is 0 if we simulate teh same model several times in a row
//    mLogCtr = 0;
//}

void Node::preAllocateLogSpace()
{
    size_t data_size = mDataVector.size();
    mTimeStorage.resize(mLogSlots);
    mDataStorage.resize(mLogSlots, vector<double>(data_size));

    cout << "requestedSize: " << mLogSlots << " " << data_size << " Capacities: " << mTimeStorage.capacity() << " " << mDataStorage.capacity() << " " << mDataStorage[1].capacity() << " Size: " << mTimeStorage.size() << " " << mDataStorage.size() << " " << mDataStorage[1].size() << endl;
    mLogSpaceAllocated = true;

    //Make sure the ctr is 0 if we simulate teh same model several times in a row
    mLogCtr = 0;
}

//! Copy current data vector into log storage, also adds current time
void Node::logData(const double time)
{
    if (mDoLog)
    {
        //! @todo Danger comparing doubles
        //! @todo Mayb can use mLogTimeDt = -1 (or similar) instead of bool to avoid extra comparison
        //! @todo is this correct, Subtract a tenth of logDt to avoid numerical problem with double >= double
        if (time >= mLastLogTime+mLogTimeDt-mLogTimeDt/10.0)
        {
            if (mLogSpaceAllocated)
            {
                //cout << "mLogCtr: " << mLogCtr << endl;
                //! @todo this if check should not be needed if everything else is working
                if (mLogCtr < mTimeStorage.size())
                {
                    mTimeStorage[mLogCtr] = time;
                    mDataStorage[mLogCtr] = mDataVector;
                }
                ++mLogCtr;
            }
            else
            {
                //! @todo for now always append
                mTimeStorage.push_back(time);
                mDataStorage.push_back(mDataVector);
            }
            //mLastLogTime = time;
            mLastLogTime = mLastLogTime+mLogTimeDt; //Cant use time directly as this may mean that not all log slots will be filled
        }
    }
}

void Node::saveLogData(string filename)
{
    ofstream out_file;
    out_file.open(filename.c_str());

    if (out_file.good())
    {
        assert(mTimeStorage.size() == mDataStorage.size());
        //First write HEADER containing node type
        out_file << mNodeType << endl;
        //Write log data to file
        for (size_t row=0; row<mTimeStorage.size(); ++row)
        {
            out_file << mTimeStorage[row];
            for (size_t datacol=0; datacol<mDataVector.size(); ++datacol)
            {
                out_file << " " << mDataStorage[row][datacol];
            }
            out_file << endl;
        }
        out_file.close();
        cout << "Done! Saving node data to file" << endl;
    }
    else
    {
        cout << "Warning! Could not open out file for writing" << endl;
    }
}

void Node::setPort(Port *pPort)
{
    //Prevent duplicate port registration that can happen if oter code is not doing what it is suposed to
    //The other code (connect) will be easier to write if we handle this in here though
    bool found = false;
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if (*it == pPort)
        {
            found = true;
            break;
            cout << "Warning: you are trying to add a Port that does already exist in this node  (does nothing)" << endl;
        }
    }

    if (!found)
    {
        mPortPtrs.push_back(pPort);
    }
}

void Node::removePort(Port *pPort)
{
    bool found = false;
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if (*it == pPort)
        {
            mPortPtrs.erase(it);
            found = true;
            break;
        }
    }

    if (!found)
    {
        cout << "Warning: you are trying to remove a Port that does not exist in this node  (does nothing)" << endl;
    }
}

bool Node::isConnectedToPort(Port *pPort)
{
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if (*it == pPort)
        {
            return true;
        }
    }
    return false;
}

void Node::enableLog()
{
    mDoLog = true;
}

void Node::disableLog()
{
    mDoLog = false;
}

NodeFactory gCoreNodeFactory;
DLLIMPORTEXPORT NodeFactory* getCoreNodeFactoryPtr()
{
    return &gCoreNodeFactory;
}
