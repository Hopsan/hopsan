#include "Nodes.h"
#include <fstream>
#include <cassert>
#include <iostream>

//Node constructor
Node::Node()
{
    //mName = name;
    mNodeType = "Node";
    mDataVector.clear();
}

//string &Node::getName()
//{
//    return mName;
//}

string &Node::getNodeType()
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

void Node::logData(const double time)
{
    //Check if vectors are large enough, else alocate
    //if

    ///TODO: for now always append
    mTimeStorage.push_back(time);
    mDataStorage.push_back(mDataVector);
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

////Hydraulic Node constructor
//NodeFluid::NodeFluid() : Node()
//{
//    mNodeType = "NodeFluid";
//    mDataVector.resize(3);
//}

//Hydraulic Node constructor
NodeHydraulic::NodeHydraulic() : Node()
{
    mNodeType = "NodeHydraulic";
    mDataVector.resize(DATALENGTH,0.0);
}

//Mechanic Node constructor
NodeMech::NodeMech() : Node()
{
    mNodeType = "NodeMech";
    mDataVector.resize(DATALENGTH,0.0);
}
