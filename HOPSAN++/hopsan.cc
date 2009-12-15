#include "Components.h"
#include "Nodes.h"
#include <iostream>
//#include <typeinfo>
#include "PressureSource.hpp"
#include "Orifice.hpp"
#include "Volume.hpp"

#include <time.h>


double CalcTimeDiff(const timespec &time_now, const timespec &time_last)
{
    return (double)(time_now.tv_sec - time_last.tv_sec) + ( (double)(time_now.tv_nsec - time_last.tv_nsec) )/1000000000.0;
}

int main()
{
    /* // Static orifice test
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
    */

    timespec t1,t2;
    clock_gettime(CLOCK_REALTIME,&t1);

    // Test with a volume
    //   This example ~20 times faster than Python.

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentPressureSource psourceL("ps_left_side", 10e5);
    ComponentOrifice orificeL("orifice_left_side", 1e-12);
    ComponentVolume volumeC("volume_center");
    ComponentOrifice orificeR("orifice_right_side", 1e-12);
    ComponentPressureSource psourceR("ps_right_side", 0e5);


    //Add components
    simulationmodel.addComponent(psourceL);
    simulationmodel.addComponent(orificeL);
    simulationmodel.addComponent(volumeC);
    simulationmodel.addComponent(orificeR);
    simulationmodel.addComponent(psourceR);
    //Connect components
    simulationmodel.connect(psourceL, psourceL.P1, orificeL, orificeL.P1);
    simulationmodel.connect(orificeL, orificeL.P2, volumeC, volumeC.P1);
    simulationmodel.connect(volumeC, volumeC.P2, orificeR, orificeR.P1);
    simulationmodel.connect(orificeR, orificeR.P2, psourceR, psourceR.P1);

    //Run simulation
    simulationmodel.simulate(0,10);
    clock_gettime(CLOCK_REALTIME,&t2);
    cout << "Time: " << CalcTimeDiff(t2,t1) << endl;

    //Test write to file
    volumeC.getPort(volumeC.P1).getNode().saveLogData("volumeC_P1.txt");
    cout << "HOPSAN++ Done!" << endl;

    return 0;
}
