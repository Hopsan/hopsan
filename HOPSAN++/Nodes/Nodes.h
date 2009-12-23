#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include <vector>
#include <string>
#include <map>
using namespace std;

class Node
{
public:
    Node();
    //string &getName();
    string &getNodeType();

    void setData(const size_t data_type, double data);
    double getData(const size_t data_type);
    double &getDataRef(const size_t data_type);

    void preAllocateLogSpace(const size_t nSlots);
    void logData(const double time);
    void saveLogData(string filename);

protected:

    string mNodeType;
    vector<double> mDataVector;

private:
    string mName;
    vector<double> mTimeStorage;
    vector<vector<double> > mDataStorage;
    bool mLogSpaceAllocated;
    size_t mLogCtr;

};


template <typename _Key, typename _Base, typename _Predicator = std::less<_Key> >
class CClassFactory
{
public:
    CClassFactory() {};
    ~CClassFactory() {};

    typedef _Base* (*CreatorFunction) (void);
    typedef std::map<_Key, CreatorFunction, _Predicator> _mapFactory;

    // called at the beginning of execution to register creation functions
    // used later to create class instances
    static _Key RegisterCreatorFunction(_Key idKey, CreatorFunction classCreator)
    {
        get_mapFactory()->insert(std::pair<_Key, CreatorFunction>(idKey, classCreator));
        return idKey;
    }

    // tries to create instance based on the key
    // using creator function (if provided)
    static _Base* CreateInstance(_Key idKey)
    {
        typename _mapFactory::iterator it = get_mapFactory()->find(idKey);
        if (it != get_mapFactory()->end())
        {
            if (it->second)
            {
                return it->second();
            }
        }
        return NULL;
    }

protected:
    // map where the construction info is stored
    // to prevent inserting into map before initialisation takes place
    // place it into static function as static member,
    // so it will be initialised only once - at first call

    static _mapFactory * get_mapFactory()
    {
        static _mapFactory m_sMapFactory;
        return &m_sMapFactory;
    }
};


class NodeHydraulic :public Node
{
    static Node* CreatorFunction() {return new NodeHydraulic;}
    static string iDummyId;

public:
    enum {MASSFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
    NodeHydraulic() : Node()
    {
        mNodeType = "NodeHydraulic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};


class NodeMechanic :public Node
{
    static Node* CreatorFunction() {return new NodeMechanic;}
    static string iDummyId;

public:
    enum {VELOCITY, FORCE, DATALENGTH};
    NodeMechanic() : Node()
    {
        mNodeType = "NodeMechanic";
        mDataVector.resize(DATALENGTH,0.0);
    }
};

#endif // NODES_H_INCLUDED
