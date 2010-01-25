//!
//! @file   Node.cc
//! @author <FluMeS>
//! @date   2009-12-20
//!
//! @brief Contains Node base classes
//!
//$Id$

#include <fstream>
#include <cassert>
#include <iostream>
#include "Node.h"

//Node constructor
Node::Node()
{
    mNodeType = "Node";
    mDataVector.clear();
    mDataStorage.clear();
    mTimeStorage.clear();
    mPortPtrs.clear();
    mTransparentPortPtrs.clear();
    mLogSpaceAllocated = false;
    mLogCtr = 0;
}

NodeTypeT &Node::getNodeType()
{
    return mNodeType;
}

void Node::setData(const size_t data_type, double data)
{
    mDataVector[data_type] = data;
}

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

void Node::logData(const double time)
{
    //Check if vectors are large enough, else alocate
    //if

    if (mLogSpaceAllocated)
    {
        //cout << "mLogCtr: " << mLogCtr << endl;
        ///TODO: this if check should not be needed if everything else is working
        if (mLogCtr < mTimeStorage.size())
        {
            mTimeStorage[mLogCtr] = time;
            mDataStorage[mLogCtr] = mDataVector;
            //mDataStorage.push_back(mDataVector);
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

void Node::setTransparentPort(Port *pPort)
{
    mTransparentPortPtrs.push_back(pPort);
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
