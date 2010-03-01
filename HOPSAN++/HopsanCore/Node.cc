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
Node::Node()
{
    mNodeType = "Node";
    mDataVector.clear();
    mDataStorage.clear();
    mTimeStorage.clear();
    mPortPtrs.clear();
    mLogSpaceAllocated = false;
    mLogCtr = 0;
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

void Node::preAllocateLogSpace(const size_t nSlots)
{
    size_t data_size = mDataVector.size();
    mTimeStorage.resize(nSlots);
    mDataStorage.resize(nSlots, vector<double>(data_size));
    //mDataStorage.reserve(nSlots);
//    vector<vector<double> >::iterator it;
//    for (it=mDataStorage.begin(); it!=mDataStorage.end(); ++it)
//    {
//        it->reserve(data_size);
//    }
    cout << "requestedSize: " << nSlots << " " << data_size << " Capacities: " << mTimeStorage.capacity() << " " << mDataStorage.capacity() << " " << mDataStorage[1].capacity() << " Size: " << mTimeStorage.size() << " " << mDataStorage.size() << " " << mDataStorage[1].size() << endl;
    mLogSpaceAllocated = true;
}

//! Copy current data vector into log storage, also adds current time
void Node::logData(const double time)
{
    if (mLogSpaceAllocated)
    {
        //cout << "mLogCtr: " << mLogCtr << endl;
        ///TODO: this if check should not be needed if everything else is working
        if (mLogCtr < mTimeStorage.size())
        {
            mTimeStorage[mLogCtr] = time;
            mDataStorage[mLogCtr] = mDataVector;
        }
        ++mLogCtr;
    }
    else
    {
        ///TODO: for now always append
        mTimeStorage.push_back(time);
        mDataStorage.push_back(mDataVector);
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
    mPortPtrs.push_back(pPort);
}

void Node::removePort(Port *pPort)
{
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if (*it == pPort)
        {
            mPortPtrs.erase(it);
            break;
        }
    }
    //! @todo some notification if you try to remove something that does not exist (can not check it==mPortPtrs.end() ) this check can be OK after an successfull erase
}

bool Node::connectedToPort(Port *pPort)
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

NodeFactory gCoreNodeFactory;
DLLIMPORTEXPORT NodeFactory* getCoreNodeFactoryPtr()
{
    return &gCoreNodeFactory;
}
