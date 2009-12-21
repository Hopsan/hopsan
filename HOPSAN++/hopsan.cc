#include "Components.h"
#include "Nodes.h"
#include <iostream>
//#include <typeinfo>
#include "PressureSource.hpp"
#include "Orifice.hpp"
#include "Volume.hpp"
#include "TLMlossless.hpp"
#include "PressureSourceQ.hpp"
#include "FlowSourceQ.hpp"
#include "TicToc.h"
#include "Delay.h"
#include <math.h>

void test1()
{
    TicToc totaltimer("totaltimer");

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentPressureSource psourceL("ps_left_side", 10e5);
    ComponentOrifice orificeL("orifice_left_side", 1e-12);
    ComponentVolume volumeC("volume_center");
    //ComponentTLMlossless volumeC("volume_center");
    ComponentOrifice orificeR("orifice_right_side", 1e-12);
    ComponentPressureSource psourceR("ps_right_side", 0e5);

    //Add components
    simulationmodel.addComponent(psourceL);
    simulationmodel.addComponent(orificeL);
    simulationmodel.addComponent(volumeC);
    simulationmodel.addComponent(orificeR);
    simulationmodel.addComponent(psourceR);
    //Connect components
    simulationmodel.connect(psourceL, "P1", orificeL, "P1");
    simulationmodel.connect(orificeL, "P2", volumeC, "P1");
    simulationmodel.connect(volumeC, "P2", orificeR, "P1");
    simulationmodel.connect(orificeR, "P2", psourceR, "P1");

    //Run simulation
    TicToc prealloctimer("prealloctimer");
    simulationmodel.preAllocateLogSpace(0, 100);
    prealloctimer.TocPrint();

    TicToc simutimer("simutimer");
    simulationmodel.simulate(0,100);
    simutimer.TocPrint();

    totaltimer.TocPrint();

    //Test write to file
    TicToc filewritetimer("filewritetimer");
    volumeC.getPort("P1").getNode().saveLogData("volumeC_P1.txt");
    filewritetimer.TocPrint();
    cout << "HOPSAN++ Done!" << endl;
}


void test2()
{
	Delay d1(.15, .1); //delay .15 with sampletime .1
	for (int i=0; i < 11; ++i) {
		cout << "Value: " << i << "    Delayed value: " << d1.value() << endl;
		d1.update(i);
	}
}


void testTLM()
{
	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentFlowSourceQ qsourceL("qs_left_side", 1);
    ComponentTLMlossless lineC("line_center", 3, .2);
    ComponentPressureSourceQ psourceR("ps_right_side", 1);

    //Add components
    simulationmodel.addComponent(qsourceL);
    simulationmodel.addComponent(lineC);
    simulationmodel.addComponent(psourceR);

    //Connect components
    simulationmodel.connect(qsourceL, "P1", lineC, "P1");
    simulationmodel.connect(lineC, "P2", psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 1.0);

    simulationmodel.simulate(0, 1.0);

    //Test write to file
    lineC.getPort("P1").getNode().saveLogData("output.txt");

	//Finished
    cout << "HOPSAN++ Done!" << endl;

}


void test3()
{
	/*   Exempelsystem:
					  Kc
	   q       T, Zc  v
	 ------>o=========----o p
	                  ^
	 */
	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentFlowSourceQ qsourceL("qs_left_side", 1.0);
    ComponentTLMlossless lineC("line_center", 1.0, .1);
    ComponentOrifice orificeR("orifice_right_side", 3.0);
    ComponentPressureSource psourceR("ps_right_side", 1.0);

    //Add components
    simulationmodel.addComponent(qsourceL);
    simulationmodel.addComponent(lineC);
    simulationmodel.addComponent(orificeR);
    simulationmodel.addComponent(psourceR);

    //Connect components
    simulationmodel.connect(qsourceL, "P1", lineC, "P1");
    simulationmodel.connect(lineC, "P2", orificeR, "P1");
    simulationmodel.connect(orificeR, "P2", psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 1.0);

    simulationmodel.simulate(0, 1.0);

    //Test write to file
    lineC.getPort("P1").getNode().saveLogData("output.txt");
    lineC.getPort("P2").getNode().saveLogData("output2.txt");

	//Finished
    cout << "HOPSAN++ Done!" << endl;

}


int main()
{
    test3();


    return 0;
}
