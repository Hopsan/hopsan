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
    HydraulicPressureSource psourceL("ps_left_side", 10e5);
    HydraulicLaminarOrifice orificeL("orifice_left_side", 1e-12);
    HydraulicVolume volumeC("volume_center");
    //ComponentTLMlossless volumeC("volume_center");
    HydraulicLaminarOrifice orificeR("orifice_right_side", 1e-12);
    HydraulicPressureSource psourceR("ps_right_side", 0e5);

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
//	Delay d1(.15, .1); //delay .15 with sampletime .1
	Delay d1(2); // delay 2 steps
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
    HydraulicFlowSourceQ qsourceL(    "qs_left_side",  1.0);
    HydraulicTLMlossless lineC(       "line_center",   3.0, 0.2);
    HydraulicPressureSourceQ psourceR("ps_right_side", 1.0);

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
    HydraulicFlowSourceQ qsourceL("qs_left_side",  1.0);
    HydraulicTLMRlineR lineL("line_left",     Zc, T/4.0, R/8.0, 0.0);
    HydraulicLaminarOrifice orificeL("orifice_L", 4.0*R);
    HydraulicTLMRlineR lineLC("line_lcenter", Zc, T/4.0, 0.0, 0.0);
    HydraulicLaminarOrifice orificeC("orifice_C", 4.0*R);
    HydraulicTLMRlineR lineRC("line_rcenter", Zc, T/4.0, 0.0, 0.0);
    HydraulicLaminarOrifice orificeR("orifice_R", 4.0*R);
    HydraulicTLMRlineR lineR("line_right",    Zc, T/4.0, 0.0, R/8.0);
    HydraulicPressureSourceQ psourceR("ps_right_side", 1.0);

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
    HydraulicFlowSourceQ qsourceL(   "qs_left_side",       1.0);
    HydraulicTLMlossless lineC(      "line_center",        1.0, 0.1, 0.0);
    HydraulicLaminarOrifice orificeR(       "orifice_right_side", 3.0);
    HydraulicPressureSource psourceR("ps_right_side",      1.0);

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
    //ComponentPressureSource psourceL("ps_left_side", 10e5);
    //ComponentOrifice orificeL("orifice_left_side", 1e-12);

    #ifdef WIN32
    Hopsan.externalLoader.load("./libHydraulic.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libHydraulic.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libHydraulic.so");
    #endif

    cout << "afterload" << endl;

    Component* psourceL = Hopsan.CreateComponent("HydraulicPressureSource");
    Component* orificeL = Hopsan.CreateComponent("HydraulicOrifice");
    Component* volumeC = Hopsan.CreateComponent("HydraulicVolume");
    Component* orificeR = Hopsan.CreateComponent("HydraulicOrifice");
    Component* psourceR = Hopsan.CreateComponent("HydraulicPressureSource");
    psourceL->setParameter("P", 10e5);
    orificeR->setName("right orifice");
    orificeR->setParameter("Kc", 1e-12);
    psourceR->setParameter("P", 0e5);


    //ComponentOrifice orificeR("orifice_right_side", 1e-12);
    //ComponentPressureSource psourceR("ps_right_side", 0e5);

    //Add components
    simulationmodel.addComponent(*psourceL);
    simulationmodel.addComponent(*orificeL);
    simulationmodel.addComponent(*volumeC);
    simulationmodel.addComponent(*orificeR);
    simulationmodel.addComponent(*psourceR);
    //Connect components
    simulationmodel.connect(*psourceL, "P1", *orificeL, "P1");
    simulationmodel.connect(*orificeL, "P2", *volumeC, "P1");
    simulationmodel.connect(*volumeC, "P2", *orificeR, "P1");
    simulationmodel.connect(*orificeR, "P2", *psourceR, "P1");

    //list some stuff
    orificeR->listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 100);
    simulationmodel.simulate(0,100);

    //Test write to file
    volumeC->getPort("P1").getNode().saveLogData("output.txt");

    cout << "test_external_lib() Done!" << endl;

}


void test_fixed_pump()
{
    HopsanEssentials Hopsan;

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");

    //Create other components

    #ifdef WIN32
    Hopsan.externalLoader.load("./libHydraulic.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libHydraulic.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libHydraulic.so");
    #endif

    cout << "afterload" << endl;

    Component* psourceL = Hopsan.CreateComponent("HydraulicPressureSource");
    Component* pump = Hopsan.CreateComponent("HydraulicFixedDisplacementPump");
    Component* volumeC = Hopsan.CreateComponent("HydraulicVolume");
    Component* psourceR = Hopsan.CreateComponent("HydraulicPressureSourceQ");

    psourceL->setParameter("P", 10e5);
    //pump->setParameter("Kcp", 1e-7);
    psourceR->setParameter("P", 10e5);

    //Add components
    simulationmodel.addComponent(*psourceL);
    simulationmodel.addComponent(*pump);
    simulationmodel.addComponent(*volumeC);
    simulationmodel.addComponent(*psourceR);

    //Connect components
    simulationmodel.connect(*psourceL, "P1", *pump, "P1");
    simulationmodel.connect(*pump, "P2", *volumeC, "P1");
    simulationmodel.connect(*volumeC, "P2", *psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 100);
    simulationmodel.simulate(0,100);

    //Test write to file
    pump->getPort("P2").getNode().saveLogData("output.txt");

    cout << "test_fixed_pump() Done!" << endl;

}

void test_variable_pump()
{
    HopsanEssentials Hopsan;

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");

    //Create other components

    HydraulicPressureSource psourceL("ps_left_side", 1e5);
    HydraulicVariableDisplacementPump pump("VariableDisplacementPump");
    SignalSource eps("Swivel Angle", 1.0);
    HydraulicVolume volumeC("Volume");
    HydraulicPressureSourceQ psourceR("ps_right_side");

    //psourceL.setParameter("P", 1e5);
    //pump->setParameter("Kcp", 1e-7);
    eps.setParameter("Value", 0.5);
    psourceR.setParameter("P", 10e5);

    //Add components
    simulationmodel.addComponent(psourceL);
    simulationmodel.addComponent(pump);
    simulationmodel.addComponent(eps);
    simulationmodel.addComponent(volumeC);
    simulationmodel.addComponent(psourceR);

    //Connect components
    simulationmodel.connect(psourceL, "P1", pump, "P1");
    simulationmodel.connect(eps, "out", pump, "in");
    simulationmodel.connect(pump, "P2", volumeC, "P1");
    simulationmodel.connect(volumeC, "P2", psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 100);
    simulationmodel.simulate(0,100);

    //Test write to file
    pump.getPort("P2").getNode().saveLogData("output.txt");

    cout << "test_variable_pump() Done!" << endl;

}


//void testSignal()
//{
//	/*   Exempelsystem:
//
//	 2   |\ 3
//	 o===| >===o
//	     |/
//    */
//
//    HopsanEssentials Hopsan;
//
//    #ifdef WIN32
//    Hopsan.externalLoader.load("./libHydraulic.dll");
//    #elif defined MAC
//    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libHydraulic.dylib");
//    #else
//    Hopsan.externalLoader.load("./bin/Debug/libHydraulic.so");
//    #endif
//
//    cout << "afterload" << endl;
//
//	ComponentSystem simulationmodel("simulationmodel");
//    //Create other components
//    SignalSource sourceL("source_left", 1.0);
//    SignalGain gainC("gain_center", 1.0);
//    SignalSink sinkR("sink_right");
//
//    //Add components
//    simulationmodel.addComponent(sourceL);
//    simulationmodel.addComponent(gainC);
//    simulationmodel.addComponent(sinkR);
//
//    //Connect components
//    simulationmodel.connect(sourceL, "out", gainC, "in");
//    simulationmodel.connect(gainC, "out", sinkR, "in");
//
//    //List and set parameters
//    sourceL.listParametersConsole();
//    gainC.listParametersConsole();
//    sourceL.setParameter("Value", 2.0);
//    gainC.setParameter("Gain", 3.0);
//    sourceL.listParametersConsole();
//    gainC.listParametersConsole();
//
//    //Run simulation
//    simulationmodel.preAllocateLogSpace(0.0, 1.0);
//
//    simulationmodel.simulate(0.0, 1.0);
//
//    //Test write to file
//    sinkR.getPort("in").getNode().saveLogData("output.txt");
//
//	//Finished
//    cout << "testSignal() Done!" << endl;
//}


void testExternalSignal()
{
	/*   Exempelsystem:

	 2   |\ 3
	 o===| >===o
	     |/
    */

    HopsanEssentials Hopsan;

    #ifdef WIN32
    Hopsan.externalLoader.load("./libSignal.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libSignal.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libSignal.so");
    #endif

    cout << "afterload" << endl;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    Component* sourceL = Hopsan.CreateComponent("SignalSource");
    Component* gainC = Hopsan.CreateComponent("SignalGain");
    Component* sinkR = Hopsan.CreateComponent("SignalSink");

    //Add components
    simulationmodel.addComponent(*sourceL);
    simulationmodel.addComponent(*gainC);
    simulationmodel.addComponent(*sinkR);

    //Connect components
    simulationmodel.connect(*sourceL, "out", *gainC, "in");
    simulationmodel.connect(*gainC, "out", *sinkR, "in");

    //List and set parameters
    sourceL->listParametersConsole();
    gainC->listParametersConsole();
    sourceL->setParameter("Value", 2.0);
    gainC->setParameter("Gain", 3.0);
    sourceL->listParametersConsole();
    gainC->listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 1.0);

    simulationmodel.simulate(0.0, 1.0);

    //Test write to file
    sinkR->getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testSignal() Done!" << endl;
}


void testkarl()
{

    HopsanEssentials Hopsan;
	ComponentSystem simulationmodel("simulationmodel");

#ifdef WIN32
    Hopsan.externalLoader.load("./libHydraulic.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libHydraulic.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libHydraulic.so");
    #endif

//    //Create other components
//    ComponentPressureSource psourceL(   "ps_left_side");
//    ComponentTurbOrifice orificeL(       "orifice_left_side");
//    ComponentVolume volume("volume_left");
//    ComponentTurbOrifice orificeR(       "orifice_right_side");
//    ComponentPressureSource psourceR("ps_right_side");
//
//    //Add components
//    simulationmodel.addComponent(psourceL);
//    simulationmodel.addComponent(orificeL);
//    simulationmodel.addComponent(volume);
//    simulationmodel.addComponent(orificeR);
//    simulationmodel.addComponent(psourceR);
//
//    //Connect components
//    simulationmodel.connect(psourceL, "P1", orificeL,    "P1");
//    simulationmodel.connect(orificeL,    "P2", volume, "P1");
//    simulationmodel.connect(volume, "P2", orificeR, "P1");
//    simulationmodel.connect(orificeR, "P2", psourceR, "P1");
//
//    //List and set parameters
//    psourceL.listParametersConsole();
//    orificeL.listParametersConsole();
//    volume.listParametersConsole();
//    orificeR.listParametersConsole();
//    psourceR.listParametersConsole();
//
//    psourceL.setParameter("P", 1.0e5);
//    psourceR.setParameter("P", 10.0e5);
//    orificeR.setParameter("A",0.00001);
//    volume.setParameter("V", 1.0e-1);
//    psourceL.listParametersConsole();
//    psourceR.listParametersConsole();

    //Create other components
    Component* psource = Hopsan.CreateComponent("HydraulicPressureSource");
    Component* tankT = Hopsan.CreateComponent("HydraulicPressureSource");
    Component* valve = Hopsan.CreateComponent("Hydraulic43Valve");
    Component* volumeA = Hopsan.CreateComponent("HydraulicVolume");
    Component* volumeB = Hopsan.CreateComponent("HydraulicVolume");
    Component* orificeA = Hopsan.CreateComponent("HydraulicLaminarOrifice");
    Component* orificeB = Hopsan.CreateComponent("HydraulicLaminarOrifice");
    Component* tankA = Hopsan.CreateComponent("HydraulicPressureSource");
    Component* tankB = Hopsan.CreateComponent("HydraulicPressureSource");
    Component* sinus = Hopsan.CreateComponent("SignalSineWave");

    //Add components
    simulationmodel.addComponent(*psource);
    simulationmodel.addComponent(*tankT);
    simulationmodel.addComponent(*valve);
    simulationmodel.addComponent(*volumeA);
    simulationmodel.addComponent(*volumeB);
    simulationmodel.addComponent(*orificeA);
    simulationmodel.addComponent(*orificeB);
    simulationmodel.addComponent(*tankA);
    simulationmodel.addComponent(*tankB);
    simulationmodel.addComponent(*sinus);

    //Connect components
    simulationmodel.connect(*psource, "P1", *valve,    "PP");
    simulationmodel.connect(*tankT, "P1", *valve,    "PT");
    simulationmodel.connect(*volumeA, "P1", *valve,    "PA");
    simulationmodel.connect(*volumeB, "P1", *valve,    "PB");
    simulationmodel.connect(*sinus, "out", *valve,    "PX");
    simulationmodel.connect(*volumeA,    "P2", *orificeA, "P1");
    simulationmodel.connect(*volumeB,    "P2", *orificeB, "P1");
    simulationmodel.connect(*orificeA,    "P2", *tankA, "P1");
    simulationmodel.connect(*orificeB,    "P2", *tankB, "P1");

    //List and set parameters
    psource->setParameter("P", 10.0e5);
    tankT->setParameter("P", 1.0e5);
    tankA->setParameter("P", 1.0e5);
    tankB->setParameter("P", 1.0e5);
    volumeA->setParameter("V", 1.0e-1);
    volumeB->setParameter("V", 1.0e-1);
    valve->setParameter("omegah", 10.0);
    valve->setParameter("deltah", 0.6);
    sinus->setParameter("Frequency",0.1);
    sinus->setParameter("StartTime", 0.0);
    sinus->setParameter("Amplitude", 0.001);

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 20.0);

    simulationmodel.simulate(0.0, 20.0);

    //Test write to file
    valve->getPort("PA").getNode().saveLogData("output2.txt");
    sinus->getPort("out").getNode().saveLogData("output.txt");

	//Finished
    cout << "testkarl() Done!" << endl;
}


void testExternalSignalStep()
{
	/*   Exempelsystem:

	 2   |\ 3
	 o===| >===o
	     |/
    */

    HopsanEssentials Hopsan;

    #ifdef WIN32
    Hopsan.externalLoader.load("./libSignal.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libSignal.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libSignal.so");
    #endif

    cout << "afterload" << endl;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    Component* stepL = Hopsan.CreateComponent("SignalStep");
    Component* gainC = Hopsan.CreateComponent("SignalGain");
    Component* sinkR = Hopsan.CreateComponent("SignalSink");

    //Add components
    simulationmodel.addComponent(*stepL);
    simulationmodel.addComponent(*gainC);
    simulationmodel.addComponent(*sinkR);

    //Connect components
    simulationmodel.connect(*stepL, "out", *gainC, "in");
    simulationmodel.connect(*gainC, "out", *sinkR, "in");

    //List and set parameters
    stepL->listParametersConsole();
    gainC->listParametersConsole();
    //sourceL->setParameter("Value", 2.0);
    gainC->setParameter("Gain", 3.0);
    gainC->listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 2.0);

    simulationmodel.simulate(0.0, 2.0);

    //Test write to file
    sinkR->getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testExternalSignalStep() Done!" << endl;
}


void testExternalSineWave()
{
	/*   Exempelsystem:

	 2   |\ 3
	 o===| >===o
	     |/
    */

    HopsanEssentials Hopsan;

    #ifdef WIN32
    Hopsan.externalLoader.load("./libSignal.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libSignal.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libSignal.so");
    #endif

    cout << "afterload" << endl;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    Component* sineL = Hopsan.CreateComponent("SignalSineWave");
    Component* gainC = Hopsan.CreateComponent("SignalGain");
    Component* sinkR = Hopsan.CreateComponent("SignalSink");

    //Add components
    simulationmodel.addComponent(*sineL);
    simulationmodel.addComponent(*gainC);
    simulationmodel.addComponent(*sinkR);

    //Connect components
    simulationmodel.connect(*sineL, "out", *gainC, "in");
    simulationmodel.connect(*gainC, "out", *sinkR, "in");

    //List and set parameters
    sineL->listParametersConsole();
    gainC->listParametersConsole();
    sineL->setParameter("StartTime", 1.0);
    sineL->setParameter("Frequency", 2.0);
    sineL->setParameter("Amplitude", 5);
    //sineL->setParameter("Offset", 0.5);
    //gainC->setParameter("Gain", 3.0);
    gainC->listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 10.0);

    simulationmodel.simulate(0.0, 10.0);

    //Test write to file
    sinkR->getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testExternalSineWave() Done!" << endl;
}

void testSineWave()
{
	/*   Exempelsystem:

	 +-----+   +------+
	 + sin +===+ gain +===o
	 +-----+   +------+

    */

    HopsanEssentials Hopsan;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    SignalSineWave sineL("SineWave");
    SignalGain gainC("Gain");
    SignalSink sinkR("Sink");

    //Add components
    simulationmodel.addComponent(sineL);
    simulationmodel.addComponent(gainC);
    simulationmodel.addComponent(sinkR);

    //Connect components
    simulationmodel.connect(sineL, "out", gainC, "in");
    simulationmodel.connect(gainC, "out", sinkR, "in");

    //List and set parameters
    sineL.listParametersConsole();
    gainC.listParametersConsole();
    sineL.setParameter("StartTime", 1.0);
    sineL.setParameter("Frequency", 2.0);
    sineL.setParameter("Amplitude", 5);
    //sineL.setParameter("Offset", 0.5);
    //gainC.setParameter("Gain", 3.0);
    gainC.listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 10.0);

    simulationmodel.simulate(0.0, 10.0);

    //Test write to file
    sinkR.getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testSineWave() Done!" << endl;
}

void testMicke()
{
    HopsanEssentials Hopsan;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    SignalSineWave sine("SineWave");
    SignalDeadZone deadzone("DeadZone");
    SignalSink sink("Sink");

    //Add components
    simulationmodel.addComponent(sine);
    simulationmodel.addComponent(deadzone);
    simulationmodel.addComponent(sink);

    //Connect components
    simulationmodel.connect(sine, "out", deadzone, "in");
    simulationmodel.connect(deadzone, "out", sink, "in");

    //List and set parameters
    //sine.listParametersConsole();
    sine.setParameter("StartTime", 1.0);
    sine.setParameter("Frequency", 2.0);
    sine.setParameter("Amplitude", 5);
    deadzone.setParameter("StartDead", -2.0);
    deadzone.setParameter("EndDead", 2.0);

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 10.0);

    simulationmodel.simulate(0.0, 10.0);

    //Test write to file
    sink.getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testMicke() Done!" << endl;
}


void testExternalSquareWave()
{
	/*   Exempelsystem:

	 2   |\ 3
	 o===| >===oComponentExternalSquareWave
	     |/
    */

    HopsanEssentials Hopsan;

    #ifdef WIN32
    Hopsan.externalLoader.load("./libSignal.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libSignal.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libSignal.so");
    #endif

    cout << "afterload" << endl;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    Component* squareL = Hopsan.CreateComponent("SignalSquareWave");
    Component* gainC = Hopsan.CreateComponent("SignalGain");
    Component* sinkR = Hopsan.CreateComponent("SignalSink");

    //Add components
    simulationmodel.addComponent(*squareL);
    simulationmodel.addComponent(*gainC);
    simulationmodel.addComponent(*sinkR);

    //Connect components
    simulationmodel.connect(*squareL, "out", *gainC, "in");
    simulationmodel.connect(*gainC, "out", *sinkR, "in");

    //List and set parameters
    squareL->listParametersConsole();
    gainC->listParametersConsole();
    squareL->setParameter("StartTime", 1.0);
    squareL->setParameter("Frequency", 2.0);
    squareL->setParameter("Amplitude", 5);
    squareL->setParameter("BaseValue", 2);
    squareL->listParametersConsole();
    //gainC->setParameter("Gain", 3.0);
    gainC->listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 10.0);

    simulationmodel.simulate(0.0, 10.0);

    //Test write to file
    sinkR->getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testExternalSquareWave() Done!" << endl;
}


void testExternalRamp()
{
	/*   Exempelsystem:

	 2   |\ 3
	 o===| >===oComponentExternalSquareWave
	     |/
    */

    HopsanEssentials Hopsan;

    #ifdef WIN32
    Hopsan.externalLoader.load("./libSignal.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libSignal.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libSignal.so");
    #endif

    cout << "afterload" << endl;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    Component* rampL = Hopsan.CreateComponent("SignalRamp");
    Component* gainC = Hopsan.CreateComponent("SignalGain");
    Component* sinkR = Hopsan.CreateComponent("SignalSink");

    //Add components
    simulationmodel.addComponent(*rampL);
    simulationmodel.addComponent(*gainC);
    simulationmodel.addComponent(*sinkR);

    //Connect components
    simulationmodel.connect(*rampL, "out", *gainC, "in");
    simulationmodel.connect(*gainC, "out", *sinkR, "in");

    //List and set parameters
    rampL->listParametersConsole();
    gainC->listParametersConsole();
    rampL->setParameter("BaseValue", 1.0);
    rampL->setParameter("Amplitude", 1.0);
    rampL->setParameter("StartTime", 1.0);
    rampL->setParameter("StopTime", 2.0);
    rampL->listParametersConsole();
    //gainC->setParameter("Gain", 3.0);
    gainC->listParametersConsole();

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 10.0);

    simulationmodel.simulate(0.0, 10.0);

    //Test write to file
    sinkR->getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testExternalRamp() Done!" << endl;
}


void test_signals_and_hydraulics()
{
    HopsanEssentials Hopsan;

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");

    //Create other components

    #ifdef WIN32
    Hopsan.externalLoader.load("./libHydraulic.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libHydraulic.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libHydraulic.so");
    #endif

    #ifdef WIN32
    Hopsan.externalLoader.load("./libSignal.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libSignal.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libSignal.so");
    #endif

    cout << "afterload" << endl;

    Component* rampL = Hopsan.CreateComponent("SignalRamp");
    Component* psourceL = Hopsan.CreateComponent("HydraulicPressureSource");
    Component* orifice = Hopsan.CreateComponent("HydraulicLaminarOrifice");
    Component* psourceR = Hopsan.CreateComponent("HydraulicPressureSource");

    rampL->setParameter("BaseValue", 100000);
    rampL->setParameter("Amplitude", 1000000);
    rampL->setParameter("StartTime", 1.0);
    rampL->setParameter("StopTime", 2.0);
    rampL->listParametersConsole();
    psourceL->setParameter("P", 20e5);
    //pump->setParameter("Kcp", 1e-7);
    psourceR->setParameter("P", 10e5);

    //Add components
    simulationmodel.addComponent(*rampL);
    simulationmodel.addComponent(*psourceL);
    simulationmodel.addComponent(*orifice);
    simulationmodel.addComponent(*psourceR);

    //Connect components
    simulationmodel.connect(*rampL, "out", *psourceL, "in");
    simulationmodel.connect(*psourceL, "P1", *orifice, "P1");
    simulationmodel.connect(*orifice, "P2", *psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 10);
    simulationmodel.simulate(0,10);

    //Test write to file
    orifice->getPort("P1").getNode().saveLogData("output.txt");

    cout << "test_signals_and_hydraulics() Done!" << endl;

}


void testArithmetics()
{
	/*   Exempelsystem:

	 2   |\ 3
	 o===| >===oComponentExternalSquareWave
	     |/
    */

    HopsanEssentials Hopsan;

    #ifdef WIN32
    Hopsan.externalLoader.load("./libSignal.dll");
    #elif defined MAC
    Hopsan.externalLoader.load("/Users/bjoer37/svn/HOPSAN++/bin/Debug/libSignal.dylib");
    #else
    Hopsan.externalLoader.load("./bin/Debug/libSignal.so");
    #endif

    cout << "afterload" << endl;

	ComponentSystem simulationmodel("simulationmodel");
    //Create other components
    Component* source1 = Hopsan.CreateComponent("SignalSource");
    //Component* source2 = Hopsan.CreateComponent("SignalSource");
    Component* divide = Hopsan.CreateComponent("SignalDivide");
    Component* sink = Hopsan.CreateComponent("SignalSink");

    //Add components
    simulationmodel.addComponent(*source1);
    //simulationmodel.addComponent(*source2);
    simulationmodel.addComponent(*divide);
    simulationmodel.addComponent(*sink);

    //Connect components
    simulationmodel.connect(*source1, "out", *divide, "in2");
    //simulationmodel.connect(*source2, "out", *divide, "in2");
    simulationmodel.connect(*divide, "out", *sink, "in");

    //List and set parameters
    source1->setParameter("Value", 1.0);
    //source2->setParameter("Value", 3.0);

    //Run simulation
    simulationmodel.preAllocateLogSpace(0.0, 10.0);

    simulationmodel.simulate(0.0, 10.0);

    //Test write to file
    sink->getPort("in").getNode().saveLogData("output.txt");

	//Finished
    cout << "testArithmetics() Done!" << endl;
}

void testCheckValve()
{
    HopsanEssentials Hopsan;

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");

    //Create other components

    HydraulicPressureSource psourceL("ps_left_side", 1e7);
    HydraulicCheckValve checkValve("CheckValve");
    HydraulicVolume volumeC("Volume");
    HydraulicPressureSourceQ psourceR("ps_right_side");

    //psourceL.setParameter("P", 1e5);
    //pump->setParameter("Kcp", 1e-7);
    psourceR.setParameter("P", 10e5);

    //Add components
    simulationmodel.addComponent(psourceL);
    simulationmodel.addComponent(checkValve);
    simulationmodel.addComponent(volumeC);
    simulationmodel.addComponent(psourceR);

    //Connect components
    simulationmodel.connect(psourceL, "P1", checkValve, "P2");
    simulationmodel.connect(checkValve, "P1", volumeC, "P1");
    simulationmodel.connect(volumeC, "P2", psourceR, "P1");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0, 100);
    simulationmodel.simulate(0,100);

    //Test write to file
    volumeC.getPort("P2").getNode().saveLogData("output.txt");

    cout << "test_checkvalve() Done!" << endl;

}

void testMechanic()
{
    HopsanEssentials Hopsan;

    ComponentSystem simulationmodel("simulationmodel");

    Component* force = Hopsan.CreateComponent("SignalSource");
    Component* ftrans = Hopsan.CreateComponent("MechanicForceTransformer");
    Component* mass = Hopsan.CreateComponent("MechanicTranslationalMass");
    Component* spring = Hopsan.CreateComponent("MechanicTranslationalSpring");
    Component* vtrans = Hopsan.CreateComponent("MechanicVelocityTransformer");
    Component* velocity = Hopsan.CreateComponent("SignalStep");
    Component* filter = Hopsan.CreateComponent("SignalLP1Filter");

    simulationmodel.addComponent(*force);
    simulationmodel.addComponent(*ftrans);
    simulationmodel.addComponent(*mass);
    simulationmodel.addComponent(*spring);
    simulationmodel.addComponent(*vtrans);
    simulationmodel.addComponent(*velocity);
    simulationmodel.addComponent(*filter);

    simulationmodel.connect(*force,"out", *ftrans, "in");
    simulationmodel.connect(*ftrans, "out", *mass, "P1");
    simulationmodel.connect(*mass, "P2", *spring, "P1");
    simulationmodel.connect(*spring, "P2", *vtrans, "out");
    simulationmodel.connect(*vtrans, "in", *filter, "out");
    simulationmodel.connect(*filter, "in", *velocity, "out");

    force->setParameter("Value", 0.0);
    velocity->setParameter("StepSize", 1.0);
    spring->setParameter("k", 1.0e2);
    filter->setParameter("Frequency", 10);

    simulationmodel.preAllocateLogSpace(0.0, 10.0);
    simulationmodel.simulate(0.0, 10.0);

    //Write to file
    spring->getPort("P2").getNode().saveLogData("output.txt");
    filter->getPort("out").getNode().saveLogData("output2.txt");

	//Finished
    cout << "testMechanic() Done!" << endl;

}


void testPressureControlledValve()
{
    HopsanEssentials Hopsan;

    //Create master component
    ComponentSystem simulationmodel("simulationmodel");

    //Create other components

    HydraulicPressureSource psource1("ps1", 2.0e6);
    HydraulicPressureSource psource2("ps1", 1.0e5);
    HydraulicPressureSource psource_open("ps1", 1.0e5);
    HydraulicPressureSource psource_close("ps1", 1.0e5);
    HydraulicPressureControlledValve pValve("pValve");
    SignalRamp ramp("ramp");

    ramp.setParameter("BaseValue", 0.0);
    ramp.setParameter("Amplitude", 2000000.0);
    ramp.setParameter("StartTime", 1.0);
    ramp.setParameter("StopTime", 2.0);
    ramp.listParametersConsole();
    pValve.setParameter("pref", 0.0);
    pValve.listParametersConsole();


    //Add components
    simulationmodel.addComponent(psource1);
    simulationmodel.addComponent(psource2);
    simulationmodel.addComponent(psource_open);
    simulationmodel.addComponent(psource_close);
    simulationmodel.addComponent(pValve);
    simulationmodel.addComponent(ramp);

    //Connect components
    simulationmodel.connect(psource1, "P1", pValve, "P1");
    simulationmodel.connect(psource2, "P1", pValve, "P2");
    simulationmodel.connect(psource_open, "P1", pValve, "P_OPEN");
    simulationmodel.connect(psource_close, "P1", pValve, "P_CLOSE");
    simulationmodel.connect(ramp, "out", psource_open, "in");

    //Run simulation
    simulationmodel.preAllocateLogSpace(0,3);
    simulationmodel.simulate(0,3);

    //Test write to file
    pValve.getPort("P2").getNode().saveLogData("output.txt");

    cout << "test_pvalve() Done!" << endl;

}


int main()
{

    //testArithmetics();

    //test_signals_and_hydraulics();

    //test1();

    //cfact_ptr->RegisterCreatorFunction("ComponentExternalSink", ComponentExternalSink::Creator);
    //test_external_lib();


    //testExternalRamp();


    //testkarl();


    //test2();


    //test_fixed_pump();


    //test_variable_pump();


    //testMicke();


    //testSignal();


    //testSineWave();

    testPressureControlledValve();

    //testMechanic();

    //testCheckValve();


    return 0;
}
