#include "Components.h"
#include "Nodes.h"
#include <iostream>
//#include <typeinfo>

int main()
{
    NodeHydraulic myNode("mynode");
    myNode.setData(NodeHydraulic::PRESSURE, 8);
    cout << "p1: " << myNode.getData(NodeHydraulic::PRESSURE) << endl;
    myNode.setData(NodeHydraulic::PRESSURE, 2);
    cout << "p2: " << myNode.getData(myNode.PRESSURE) << endl;
    myNode.setData(myNode.PRESSURE, 5);
    cout << "p3: " << myNode.getData(NodeMech::FORCE) << endl;


    //cout << "type: " << typeid(myNode).name() << endl;


    cout << "Hello HOPSAN" << endl;
    return 0;
}
