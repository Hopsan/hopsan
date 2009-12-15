#include "Components.h"
#include "Nodes.h"
#include <iostream>
//#include <typeinfo>
#include "PressureSource.hpp"
#include "Orifice.hpp"
#include "Volume.hpp"

int main()
{
    NodeHydraulic myNode;
    myNode.setData(NodeHydraulic::PRESSURE, 8);
    cout << "p1: " << myNode.getData(NodeHydraulic::PRESSURE) << endl;
    myNode.setData(NodeHydraulic::PRESSURE, 2);
    cout << "p2: " << myNode.getData(myNode.PRESSURE) << endl;
    myNode.setData(myNode.PRESSURE, 5);
    cout << "p3: " << myNode.getData(NodeMech::FORCE) << endl;

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentPressureSource psourceL("ps_left_side", 10e5);
    ComponentPressureSource psourceR("ps_right_side", 0e5);
    ComponentOrifice orificeC("orifice_center", 1e-12);

    ComponentVolume volumeL("volume_left_side");


    //Add components
    simulationmodel.addComponent(psourceL);
    simulationmodel.addComponent(psourceR);
    simulationmodel.addComponent(orificeC);
    //Connect components
    simulationmodel.connect(psourceL, psourceL.P1, orificeC, orificeC.P1);
    simulationmodel.connect(orificeC, orificeC.P2, psourceR, psourceR.P1);

    //Run simulation
    simulationmodel.simulate(0,1);

    //Test write to file
    orificeC.getPort(orificeC.P1).getNode().saveLogData("orificeC_P1.txt");
    cout << "HOPSAN++ Done!" << endl;
    return 0;
}
