#include "Components.h"
#include "Nodes.h"
#include <iostream>

int main()
{
    HydraulicNode myNode("mynode");
    myNode.setData(HydraulicNode::PRESSURE, 8);
    cout << "p1: " << myNode.getData(HydraulicNode::PRESSURE) << endl;
    myNode.setData(HydraulicNode::PRESSURE, 2);
    cout << "p2: " << myNode.getData(myNode.PRESSURE) << endl;
    myNode.setData(myNode.PRESSURE, 5);
    cout << "p3: " << myNode.getData(MechNode::FORCE) << endl;


    cout << "Hello HOPSAN" << endl;
    return 0;
}
