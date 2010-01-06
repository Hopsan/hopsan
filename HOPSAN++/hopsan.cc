#include "HopsanCore.h"
#include "TicToc.h"
#include "CoreUtilities/Delay.h"

void test1()
{
    HopsanEssentials Hopsan;
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
    simulationmodel.preAllocateLogSpace(0, 10);
    prealloctimer.TocPrint();

    TicToc simutimer("simutimer");
    simulationmodel.simulate(0,10);
    simutimer.TocPrint();

    totaltimer.TocPrint();

    //Test write to file
    TicToc filewritetimer("filewritetimer");
    volumeC.getPort("P1").getNode().saveLogData("output.txt");
    filewritetimer.TocPrint();
    cout << "test1() Done!" << endl;
}


void test2() //Test of the Delay utillity class
{
	Delay d1(.15, .1); //delay .15 with sampletime .1
	for (int i=0; i < 11; ++i) {
		cout << "Value: " << i << "    Delayed value: " << d1.value() << endl;
		d1.update(i);
	}
}


void testTLM()
{
    HopsanEssentials Hopsan;
	/*   Exempelsystem:

	   q       T, Zc
	 ------>o=========o p

	 */
	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentFlowSourceQ qsourceL(    "qs_left_side",  1.0);
    ComponentTLMlossless lineC(       "line_center",   3.0, 0.2);
    ComponentPressureSourceQ psourceR("ps_right_side", 1.0);

    //Add components
    simulationmodel.addComponent(qsourceL);
    simulationmodel.addComponent(lineC);
    simulationmodel.addComponent(psourceR);

    //List and set parameters
    lineC.listParametersConsole();
    lineC.setParameter("Zc",  3.0);
    lineC.setParameter("TD", 0.1);
    lineC.listParametersConsole();

    //Connect components
    simulationmodel.connect(qsourceL, "P1", lineC, "P1");
    simulationmodel.connect(lineC, "P2", psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 1.0);

    simulationmodel.simulate(0.0, 1.0);

    //Test write to file
    lineC.getPort("P1").getNode().saveLogData("output.txt");

	//Finished
    cout << "testTLM Done!" << endl;

}


void testTLMlumped()
{
    HopsanEssentials Hopsan;
	/*   Exempelsystem:

	   q         T, Zc
             v  v  v  v  v
	 ------>o=============o p
             ^  ^  ^  ^  ^
            R/8 R/4 ... R/8

	 */
    double R  = 1.0;
    double Zc = 3.0;
    double T  = 0.1;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentFlowSourceQ qsourceL("qs_left_side",  1.0);
    ComponentTLMRlineR lineL("line_left",     Zc, T/4.0, R/8.0, 0.0);
    ComponentOrifice orificeL("orifice_L", 4.0*R);
    ComponentTLMRlineR lineLC("line_lcenter", Zc, T/4.0, 0.0, 0.0);
    ComponentOrifice orificeC("orifice_C", 4.0*R);
    ComponentTLMRlineR lineRC("line_rcenter", Zc, T/4.0, 0.0, 0.0);
    ComponentOrifice orificeR("orifice_R", 4.0*R);
    ComponentTLMRlineR lineR("line_right",    Zc, T/4.0, 0.0, R/8.0);
    ComponentPressureSourceQ psourceR("ps_right_side", 1.0);

    //Add components
    simulationmodel.addComponent(qsourceL);
    simulationmodel.addComponent(lineL);
    simulationmodel.addComponent(orificeL);
    simulationmodel.addComponent(lineLC);
    simulationmodel.addComponent(orificeC);
    simulationmodel.addComponent(lineRC);
    simulationmodel.addComponent(orificeR);
    simulationmodel.addComponent(lineR);
    simulationmodel.addComponent(psourceR);

    //List and set parameters
    lineLC.listParametersConsole();

    //Connect components
    simulationmodel.connect(qsourceL, "P1", lineL, "P1");
    simulationmodel.connect(lineL, "P2", orificeL, "P1");
    simulationmodel.connect(orificeL, "P2", lineLC, "P1");
    simulationmodel.connect(lineLC, "P2", orificeC, "P1");
    simulationmodel.connect(orificeC, "P2", lineRC, "P1");
    simulationmodel.connect(lineRC, "P2", orificeR, "P1");
    simulationmodel.connect(orificeR, "P2", lineR, "P1");
    simulationmodel.connect(lineR, "P2", psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 2.0);

    simulationmodel.simulate(0.0, 2.0);

    //Test write to file
    lineL.getPort("P1").getNode().saveLogData("output.txt");

	//Finished
    cout << "testTLMlumped() Done!" << endl;

}


void test3()
{
    HopsanEssentials Hopsan;
	/*   Exempelsystem:
					  Kc
	   q       T, Zc  v
	 ------>o=========----o p
	                  ^
	 */
	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentFlowSourceQ qsourceL(   "qs_left_side",       1.0);
    ComponentTLMlossless lineC(      "line_center",        1.0, 0.1, 0.0);
    ComponentOrifice orificeR(       "orifice_right_side", 3.0);
    ComponentPressureSource psourceR("ps_right_side",      1.0);

    //Add components
    simulationmodel.addComponent(qsourceL);
    simulationmodel.addComponent(lineC);
    simulationmodel.addComponent(orificeR);
    simulationmodel.addComponent(psourceR);

    //Connect components
    simulationmodel.connect(qsourceL, "P1", lineC,    "P1");
    simulationmodel.connect(lineC,    "P2", orificeR, "P1");
    simulationmodel.connect(orificeR, "P2", psourceR, "P1");

    //List and set parameters
    qsourceL.listParametersConsole();
    lineC.listParametersConsole();
    orificeR.listParametersConsole();
    psourceR.listParametersConsole();
    lineC.setParameter("Zc", 1.0);
    lineC.setParameter("TD", 0.005);
    qsourceL.listParametersConsole();
    lineC.listParametersConsole();
    orificeR.listParametersConsole();
    psourceR.listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 1.0);

    simulationmodel.simulate(0.0, 1.0);

    //Test write to file
    lineC.getPort("P1").getNode().saveLogData("output.txt");
    lineC.getPort("P2").getNode().saveLogData("output2.txt");

	//Finished
    cout << "test3() Done!" << endl;
}


void test_external_lib()
{
    HopsanEssentials Hopsan;

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentPressureSource psourceL("ps_left_side", 10e5);
    //ComponentOrifice orificeL("orifice_left_side", 1e-12);

    #ifdef WIN32
    Hopsan.externalLoader.load("./libHydraulic.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libHydraulic.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libHydraulic.so");
    #endif

    cout << "afterload" << endl;

    Component* orificeL = Hopsan.getComponentFactoryPtr()->CreateInstance("ComponentExternalOrifice");
    Component* volumeC = Hopsan.getComponentFactoryPtr()->CreateInstance("ComponentExternalVolume");
    Component* orificeR = Hopsan.getComponentFactoryPtr()->CreateInstance("ComponentExternalOrifice");
    orificeR->setName("right orifice");
    orificeR->setParameter("Kc", 1e-12);

    //ComponentOrifice orificeR("orifice_right_side", 1e-12);
    ComponentPressureSource psourceR("ps_right_side", 0e5);

    //Add components
    simulationmodel.addComponent(psourceL);
    simulationmodel.addComponent(*orificeL);
    simulationmodel.addComponent(*volumeC);
    simulationmodel.addComponent(*orificeR);
    simulationmodel.addComponent(psourceR);
    //Connect components
    simulationmodel.connect(psourceL, "P1", *orificeL, "P1");
    simulationmodel.connect(*orificeL, "P2", *volumeC, "P1");
    simulationmodel.connect(*volumeC, "P2", *orificeR, "P1");
    simulationmodel.connect(*orificeR, "P2", psourceR, "P1");

    //list some stuff
    orificeR->listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 100);
    simulationmodel.simulate(0,100);

    //Test write to file
    volumeC->getPort("P1").getNode().saveLogData("output.txt");

    cout << "test_external_lib() Done!" << endl;

}


void testSignal()
{
    HopsanEssentials Hopsan;
	/*   Exempelsystem:
					  Kc
	   q       T, Zc  v
	 ------>o=========----o p
	                  ^
	 */
	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    ComponentSource sourceL("source_left", 1.0);
    ComponentGain gainC("gain_center", 1.0);
    ComponentSink sinkR("sink_right");

    //Add components
    simulationmodel.addComponent(sourceL);
    simulationmodel.addComponent(gainC);
    simulationmodel.addComponent(sinkR);

    //Connect components
    simulationmodel.connect(sourceL, "out", gainC, "in");
    simulationmodel.connect(gainC, "out", sinkR, "in");

    //List and set parameters
    sourceL.listParametersConsole();
    gainC.listParametersConsole();
    sourceL.setParameter("Value", 2.0);
    gainC.setParameter("Gain", 3.0);
    sourceL.listParametersConsole();
    gainC.listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 1.0);

    simulationmodel.simulate(0.0, 1.0);

    //Test write to file
    sinkR.getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testSignal() Done!" << endl;
}


int main()
{
    test1();
    test_external_lib();
    testSignal();
    return 0;
}
