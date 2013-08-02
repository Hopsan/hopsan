#include <QtTest>
#include <QtTest>
#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/GeneratorHandler.h"
#include <assert.h>

#ifndef BUILTINDEFAULTCOMPONENTLIB
#ifdef WIN32
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/defaultComponentLibrary.dll"
#else
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/libdefaultComponentLibrary.so"
#endif
#endif

using namespace hopsan;

Q_DECLARE_METATYPE(bool)
Q_DECLARE_METATYPE(ComponentSystem*)
Q_DECLARE_METATYPE(Component*)
Q_DECLARE_METATYPE(Port*)
Q_DECLARE_METATYPE(HString)
Q_DECLARE_METATYPE(Node*)

class SimulationTests : public QObject
{
    Q_OBJECT

public:
    SimulationTests()
    {
        mHopsanCore.loadExternalComponentLib(DEFAULTCOMPONENTLIB);
        const char* xmlStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<hopsanmodelfile hmfversion=\"0.4\" hopsanguiversion=\"0.6.0\" hopsancoreversion=\"0.6.0\">"
                "  <system logsamples=\"2048\" typename=\"Subsystem\" name=\"unittestmodel\">"
                "    <simulationtime stop=\"10\" timestep=\"0.001\" start=\"0\" inherit_timestep=\"true\"/>"
                "    <parameters>"
                "      <parameter unit=\"\" value=\"7\" type=\"double\" name=\"apa\"/>"
                "    </parameters>"
                "    <aliases/>"
                "    <objects>"
                "      <component typename=\"SignalStep\" name=\"TestStep\">"
                "        <parameters>"
                "          <parameter unit=\"-\" value=\"-5\" type=\"double\" name=\"y_0#Value\"/>"
                "          <parameter unit=\"-\" value=\"5\" type=\"double\" name=\"y_A#Value\"/>"
                "          <parameter unit=\"-\" value=\"apa\" type=\"double\" name=\"t_step#Value\"/>"
                "        </parameters>"
                "        <ports>"
                "          <port nodetype=\"NodeSignal\" name=\"out\"/>"
                "        </ports>"
                "      </component>"
                "      <component typename=\"SignalGain\" name=\"TestGain\">"
                "        <parameters>"
                "          <parameter unit=\"-\" value=\"0\" type=\"double\" name=\"in_bottom#Value\"/>"
                "        </parameters>"
                "        <ports>"
                "          <port nodetype=\"NodeSignal\" name=\"in_right\"/>"
                "          <port nodetype=\"NodeSignal\" name=\"in\"/>"
                "          <port nodetype=\"NodeSignal\" name=\"in_bottom\"/>"
                "        </ports>"
                "      </component>"
                "    </objects>"
                "    <connections>"
                "      <connect endport=\"in\" endcomponent=\"TestGain\" startport=\"out\" startcomponent=\"TestStep\"/>"
                "    </connections>"
                "  </system>"
                "</hopsanmodelfile>"
                "";

        mpSystemFromText = mHopsanCore.loadHMFModel(xmlStr);
        double startT, stopT;
        mpSystemFromFile = mHopsanCore.loadHMFModel("../../../Models/unittestmodel.hmf",startT,stopT);
    }

private:
    HopsanEssentials mHopsanCore;
    ComponentSystem *mpSystemFromText;
    ComponentSystem *mpSystemFromFile;

private Q_SLOTS:
    void HopsanCore_Create_Component()
    {
        QFETCH(HString, typeName);
        Component *pComp = mHopsanCore.createComponent(typeName);
        bool ok = pComp;
        QVERIFY2(ok, "Failed to create component!");
        mHopsanCore.removeComponent(pComp);
    }

    void HopsanCore_Create_Component_data()
    {
        QTest::addColumn<HString>("typeName");
        QTest::newRow("0") << HString("SignalSink");
        QTest::newRow("1") << HString("SignalAnimationGauge");
        QTest::newRow("2") << HString("SignalDisplay");
        QTest::newRow("3") << HString("SignalAnimationSlider");
        QTest::newRow("4") << HString("SignalAdd");
        QTest::newRow("5") << HString("SignalSubtract");
        QTest::newRow("6") << HString("SignalMultiply");
        QTest::newRow("7") << HString("SignalDivide");
        QTest::newRow("8") << HString("SignalSin");
        QTest::newRow("9") << HString("SignalCos");
        QTest::newRow("10") << HString("SignalTan");
        QTest::newRow("11") << HString("SignalSign");
        QTest::newRow("12") << HString("SignalSub");
        QTest::newRow("13") << HString("SignalPower");
        QTest::newRow("14") << HString("SignalGain");
        QTest::newRow("15") << HString("SignalAbsoluteValue");
        QTest::newRow("16") << HString("SignalMin");
        QTest::newRow("17") << HString("SignalMax");
        QTest::newRow("18") << HString("SignalSum");
        QTest::newRow("19") << HString("SignalSquare");
        QTest::newRow("20") << HString("SignalPIlead");
        QTest::newRow("21") << HString("SignalPID");
        QTest::newRow("22") << HString("SignalLP1Filter");
        QTest::newRow("23") << HString("SignalLP2Filter");
        QTest::newRow("24") << HString("SignalHP1Filter");
        QTest::newRow("25") << HString("SignalHP2Filter");
        QTest::newRow("26") << HString("SignalFirstOrderFilter");
        QTest::newRow("27") << HString("SignalFirstOrderTransferFunction");
        QTest::newRow("28") << HString("SignalSecondOrderFilter");
        QTest::newRow("29") << HString("SignalSecondOrderTransferFunction");
        QTest::newRow("30") << HString("SignalIntegrator2");
        QTest::newRow("31") << HString("SignalAnd");
        QTest::newRow("32") << HString("SignalOr");
        QTest::newRow("33") << HString("SignalXor");
        QTest::newRow("34") << HString("SignalStopSimulation");
        QTest::newRow("35") << HString("SignalGreaterThan");
        QTest::newRow("36") << HString("SignalSmallerThan");
        QTest::newRow("37") << HString("SignalSRlatch");
        QTest::newRow("38") << HString("SignalAdditiveNoise");
        QTest::newRow("39") << HString("SignalSaturation");
        QTest::newRow("40") << HString("SignalDeadZone");
        QTest::newRow("41") << HString("SignalHysteresis");
        QTest::newRow("42") << HString("SignalLookUpTable2D");
        QTest::newRow("43") << HString("SignalUnitDelay");
        QTest::newRow("44") << HString("SignalVariableTimeDelay");
        QTest::newRow("45") << HString("SignalTimeDelay");
        QTest::newRow("46") << HString("SignalDualRoute");
        QTest::newRow("47") << HString("SignalTripleRoute");
        QTest::newRow("48") << HString("SignalQuadRoute");
        QTest::newRow("49") << HString("SignalSink");
        QTest::newRow("50") << HString("SignalStep");
        QTest::newRow("51") << HString("SignalSineWave");
        QTest::newRow("52") << HString("SignalSquareWave");
        QTest::newRow("53") << HString("SignalRamp");
        QTest::newRow("54") << HString("SignalConstant");
        QTest::newRow("55") << HString("SignalPulse");
        QTest::newRow("56") << HString("SignalPulseWave");
        QTest::newRow("57") << HString("SignalNoiseGenerator");
        QTest::newRow("58") << HString("SignalStaircase");
        QTest::newRow("59") << HString("SignalSoftStep");
        QTest::newRow("60") << HString("SignalStepExponentialDelay");
        QTest::newRow("61") << HString("SignalTime");

        QTest::newRow("62") << HString("MechanicInterfaceC");
        QTest::newRow("63") << HString("MechanicInterfaceQ");
        QTest::newRow("64") << HString("MechanicRotationalInterfaceC");
        QTest::newRow("65") << HString("MechanicRotationalInterfaceQ");
        QTest::newRow("66") << HString("HydraulicInterfaceC");
        QTest::newRow("67") << HString("HydraulicInterfaceQ");
        QTest::newRow("68") << HString("PneumaticInterfaceC");
        QTest::newRow("69") << HString("PneumaticInterfaceQ");
        QTest::newRow("70") << HString("ElectricInterfaceC");
        QTest::newRow("71") << HString("ElectricInterfaceQ");
        QTest::newRow("72") << HString("SignalInputInterface");
        QTest::newRow("73") << HString("SignalOutputInterface");

        //! @todo Add all non-signal and non-connectivity components here
    }

    void HopsanCore_Create_Node()
    {
        QFETCH(HString, typeName);
        Node *pNode = mHopsanCore.createNode(typeName);
        bool ok = pNode;
        QVERIFY2(ok, "Failed to create node!");
        mHopsanCore.removeNode(pNode);
    }

    void HopsanCore_Create_Node_data()
    {
        QTest::addColumn<HString>("typeName");
        QTest::newRow("0") << HString("NodeSignal");
        QTest::newRow("1") << HString("NodeHydraulic");
        QTest::newRow("2") << HString("NodePneumatic");
        QTest::newRow("3") << HString("NodeElectric");
        QTest::newRow("4") << HString("NodeMechanic");
        QTest::newRow("5") << HString("NodeMechanicRotational");
    }

    void Load_System()
    {
        QFETCH(ComponentSystem*, system);
        QVERIFY2(system != 0,"Failed to load system!");
    }

    void Load_System_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void Load_System_Parameter()
    {
        QFETCH(ComponentSystem*, system);
        QVERIFY2(system->hasParameter("apa"), "Failed to load system parameter!");
        QVERIFY2(system->getParameter("apa")->getValue().compare("7"), "Failed to load system parameter value!");
    }

    void Load_System_Parameter_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void Load_Component()
    {
        QFETCH(ComponentSystem*, system);
        QFETCH(HString, name);
        QVERIFY2(system->haveSubComponent(name), "Failed to load sub component!");
    }

    void Load_Component_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::addColumn<HString>("name");
        QTest::newRow("0") << mpSystemFromText << HString("TestStep");
        QTest::newRow("1") << mpSystemFromText << HString("TestGain");
        QTest::newRow("2") << mpSystemFromFile << HString("TestStep");
        QTest::newRow("3") << mpSystemFromFile << HString("TestGain");
    }

    void Load_Connect()
    {
        QFETCH(ComponentSystem*, system);
        Component *pStep = system->getSubComponent("TestStep");
        Component *pScope = system->getSubComponent("TestGain");
        QVERIFY2(pStep->getPort("out")->isConnectedTo(pScope->getPort("in")), "Failed to load connection!");
    }

    void Load_Connect_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void Load_Parameter()
    {
        QFETCH(Component*, comp);
        HString parVal;
        comp->getParameterValue("y_0#Value", parVal);
        QVERIFY2(parVal.compare("-5"), "Failed to load sub component parameter value!");
        comp->getParameterValue("y_A#Value", parVal);
        QVERIFY2(parVal.compare("5"),"Failed to load sub component parameter value!");
        comp->getParameterValue("t_step#Value", parVal);
        QVERIFY2(parVal.compare("apa"), "Failed to load sub component parameter value (system parameter)!");
    }

    void Load_Parameter_data()
    {
        QTest::addColumn<Component*>("comp");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep");
        QTest::newRow("1") << mpSystemFromFile->getSubComponent("TestStep");
    }

    void System_Set_Parameter()
    {
        QFETCH(ComponentSystem*, system);

        system->setParameterValue("apa", "12");
        HString value;
        system->getParameterValue("apa", value);
        QVERIFY2(value.compare("12"), "Failed to set system parameter.");
    }

    void System_Set_Parameter_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void System_Add_And_Remove_Component()
    {
        QFETCH(ComponentSystem*, system);
        Component *pComp = mHopsanCore.createComponent("SignalSink");
        pComp->setName("AddComponentTest");
        system->addComponent(pComp);
        QVERIFY2(system->haveSubComponent("AddComponentTest"), "Failed to add component to system.");
        system->removeSubComponent("AddComponentTest");
        QVERIFY2(!system->haveSubComponent("AddComponentTest"), "Failed to remove component from system.");
        mHopsanCore.removeComponent(pComp);
    }

    void System_Add_And_Remove_Component_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void System_Rename_Component()
    {
        QFETCH(ComponentSystem*, system);
        QFETCH(HString, comp);

        Component *pComp = system->getSubComponent(comp);
        system->renameSubComponent(comp, "gris");
        QVERIFY2(pComp->getName().compare("gris"), "Failed to rename sub component.");
        system->renameSubComponent("gris", comp);
        QVERIFY2(pComp->getName().compare(comp), "Failed to rename sub component.");
    }

    void System_Rename_Component_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::addColumn<HString>("comp");
        QTest::newRow("0") << mpSystemFromText << HString("TestStep");
    }

    void System_Disconnect_Connect()
    {
        QFETCH(ComponentSystem*, system);
        QFETCH(HString, comp1);
        QFETCH(HString, port1);
        QFETCH(HString, comp2);
        QFETCH(HString, port2);

        system->disconnect(comp1,port1,comp2,port2);
        QVERIFY2(!system->getSubComponent(comp1)->getPort(port1)->isConnectedTo(system->getSubComponent(comp2)->getPort(port2)), "Failed to disconnect ports.");
        system->connect(comp1,port1,comp2,port2);
        QVERIFY2(system->getSubComponent(comp1)->getPort(port1)->isConnectedTo(system->getSubComponent(comp2)->getPort(port2)), "Failed to disconnect ports.");
    }

    void System_Disconnect_Connect_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::addColumn<HString>("comp1");
        QTest::addColumn<HString>("port1");
        QTest::addColumn<HString>("comp2");
        QTest::addColumn<HString>("port2");
        QTest::newRow("0") << mpSystemFromText << HString("TestStep") << HString("out") << HString("TestGain") << HString("in");
    }

    void System_Get_Set_Timestep()
    {
        QFETCH(ComponentSystem*, system);

        system->setDesiredTimestep(0.005);
        QVERIFY2(system->getDesiredTimeStep() == 0.005, "Failed to get or set time step.");
    }

    void System_Get_Set_Timestep_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
    }

    void System_Inherit_Timestep()
    {
        QFETCH(ComponentSystem*, system);

        system->setInheritTimestep(true);
        QVERIFY2(system->doesInheritTimestep(), "Failed to set inherit time step.");
        system->setInheritTimestep(false);
    }

    void System_Inherit_Timestep_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
    }

    void System_Num_Log_Samples()
    {
        //! @todo Also test his by running a simulation
        QFETCH(ComponentSystem*, system);

        system->setNumLogSamples(1337);
        QVERIFY2(system->getNumLogSamples() == 1337, "Failed to get or set number of log samples.");
    }

    void System_Num_Log_Samples_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
    }

    void System_Initialize()
    {
        QFETCH(ComponentSystem*, system);
        system->setDesiredTimestep(0.001);
        system->setNumLogSamples(1024);
        QVERIFY2(system->initialize(0,10), "Failed to initialize system!");
    }

    void System_Initialize_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void System_Simulate()
    {
        QFETCH(ComponentSystem*, system);
        system->simulate(10.0);
        QVERIFY2(system->getLogTimeVector()->size() == 1024, "Failed to simulate system!");
        QVERIFY2(system->getNumActuallyLoggedSamples() == 1024, "Failed to simulate system!");
    }

    void System_Simulate_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void System_Simulate_Multicore()
    {
        QFETCH(ComponentSystem*, system);
        system->simulateMultiThreaded(0, 10.0);
        QVERIFY2(system->getLogTimeVector()->size() == 1024, "Failed to simulate system!");
        QVERIFY2(system->getNumActuallyLoggedSamples() == 1024, "Failed to simulate system!");

        std::vector<double> multiResults1 = system->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(0);
        std::vector<double> multiResults2 = system->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(511);
        std::vector<double> multiResults3 = system->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(1023);
        system->simulate(10.0);
        std::vector<double> singleResults1 = system->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(0);
        std::vector<double> singleResults2 = system->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(511);
        std::vector<double> singleResults3 = system->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(1023);
        QVERIFY2(multiResults1 == singleResults1, "Single-threaded and multi-threaded simulation gave different results!");
        QVERIFY2(multiResults2 == singleResults2, "Single-threaded and multi-threaded simulation gave different results!");
        QVERIFY2(multiResults3 == singleResults3, "Single-threaded and multi-threaded simulation gave different results!");
    }

    void System_Simulate_Multicore_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void Component_Set_Parameter()
    {
        QFETCH(Component*, comp);
        QFETCH(HString, parName);
        QFETCH(HString, parVal);

        HString value;
        comp->setParameterValue(parName, parVal);
        comp->getParameterValue(parName, value);
        QVERIFY2(value.compare(parVal), "Failed to set parameter value.");
    }

    void Component_Set_Parameter_data()
    {
        QTest::addColumn<Component*>("comp");
        QTest::addColumn<HString>("parName");
        QTest::addColumn<HString>("parVal");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep") << HString("y_0#Value") << HString("4");
        QTest::newRow("1") << mpSystemFromText->getSubComponent("TestStep") << HString("y_0#Value") << HString("apa");
    }

    void Component_Get_Name()
    {
        QFETCH(Component*, comp);
        QFETCH(HString, name);

        QVERIFY2(comp->getName().compare(name), "Failed to get component name.");
    }

    void Component_Get_Name_data()
    {
        QTest::addColumn<Component*>("comp");
        QTest::addColumn<HString>("name");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep") << HString("TestStep");
        QTest::newRow("1") << mpSystemFromText->getSubComponent("TestGain") << HString("TestGain");
    }

    void Component_Get_Typename()
    {
        QFETCH(Component*, comp);
        QFETCH(HString, typeName);
        QVERIFY2(comp->getTypeName().compare(typeName), "Failed to get component type name.");
    }

    void Component_Get_Typename_data()
    {
        QTest::addColumn<Component*>("comp");
        QTest::addColumn<HString>("typeName");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep") << HString("SignalStep");
        QTest::newRow("1") << mpSystemFromText->getSubComponent("TestGain") << HString("SignalGain");
    }

    void Component_Has_Parameter()
    {
        QFETCH(Component*, comp);
        QFETCH(HString, parName);
        QVERIFY2(comp->hasParameter(parName), "Failed in hasParameter()!");
    }

    void Component_Has_Parameter_data()
    {
        QTest::addColumn<Component*>("comp");
        QTest::addColumn<HString>("parName");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep") << HString("y_0#Value");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep") << HString("y_A#Value");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep") << HString("t_step#Value");
    }

    void Component_Get_CQS()
    {
        QFETCH(Component*, comp);
        QFETCH(HString, type);
        QVERIFY2(comp->getTypeCQSString().compare(type), "Failed to get CQS type!");
        mHopsanCore.removeComponent(comp);
    }

    void Component_Get_CQS_data()
    {
        QTest::addColumn<Component*>("comp");
        QTest::addColumn<HString>("type");

        Component *pCompS = mHopsanCore.createComponent("SignalStep");
        QTest::newRow("0") << pCompS << HString("S");

        Component *pCompQ = mHopsanCore.createComponent("HydraulicLaminarOrifice");
        QTest::newRow("1") << pCompQ << HString("Q");

        Component *pCompC = mHopsanCore.createComponent("HydraulicCylinderC");
        QTest::newRow("2") << pCompC << HString("C");
    }

    void Port_Get_Name()
    {
        QFETCH(Port*, port);
        QFETCH(HString, name);

        QVERIFY2(port->getName().compare(name), "Failed to get port name.");
    }

    void Port_Get_Name_data()
    {
        QTest::addColumn<Port*>("port");
        QTest::addColumn<HString>("name");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep")->getPort("out") << HString("out");
        QTest::newRow("1") << mpSystemFromText->getSubComponent("TestGain")->getPort("in") << HString("in");
    }

    void Port_Get_Connected_Ports()
    {
        QFETCH(Port*, port);
        QFETCH(Port*, otherport);

        QVERIFY2(port->getConnectedPorts().size() == 1, "Failed, getConnectedPorts return wrong number of ports.");
        QVERIFY2(port->getConnectedPorts()[0] == otherport, "Failed to get connected ports.");
    }

    void Port_Get_Connected_Ports_data()
    {
        QTest::addColumn<Port*>("port");
        QTest::addColumn<Port*>("otherport");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep")->getPort("out") <<
                              mpSystemFromText->getSubComponent("TestGain")->getPort("in");
        QTest::newRow("1") << mpSystemFromText->getSubComponent("TestGain")->getPort("in") <<
                              mpSystemFromText->getSubComponent("TestStep")->getPort("out");
    }

    void Port_Get_Component_Name()
    {
        QFETCH(Port*, port);
        QFETCH(HString, compName);
        QVERIFY2(port->getComponentName().compare(compName), "Failed to get component name in port.");
    }

    void Port_Get_Component_Name_data()
    {
        QTest::addColumn<Port*>("port");
        QTest::addColumn<HString>("compName");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestStep")->getPort("out") << HString("TestStep");
    }

    void Port_Is_Multiport()
    {
        QFETCH(Port*, port);
        QFETCH(bool, multi);
        QVERIFY2(port->isMultiPort() == multi, "Failed: isMultiPort() gave wrong answer.");
    }

    void Port_Is_Multiport_data()
    {
        QTest::addColumn<Port*>("port");
        QTest::addColumn<bool>("multi");
        Component *pComp;

        pComp = mHopsanCore.createComponent("SignalGain");
        QTest::newRow("0") << pComp->getPort("in") << false;
        QTest::newRow("1") << pComp->getPort("out") << false;
        mHopsanCore.removeComponent(pComp);

        pComp = mHopsanCore.createComponent("SignalSink");
        QTest::newRow("2") << pComp->getPort("in") << true;
        mHopsanCore.removeComponent(pComp);

        pComp = mHopsanCore.createComponent("HydraulicCylinderC");
        QTest::newRow("3") << pComp->getPort("P1") << true;
        QTest::newRow("4") << pComp->getPort("P2") << true;
        QTest::newRow("5") << pComp->getPort("P3") << false;
        mHopsanCore.removeComponent(pComp);

        pComp = mHopsanCore.createComponent("HydraulicLaminarOrifice");
        QTest::newRow("6") << pComp->getPort("P1") << false;
        mHopsanCore.removeComponent(pComp);

        pComp = mHopsanCore.createComponent("MechanicMultiPortTranslationalMass");
        QTest::newRow("7") << pComp->getPort("P1") << true;
        mHopsanCore.removeComponent(pComp);
    }

    void Node_Get_Write_Port_Component_Pointer()
    {
        QFETCH(Node*, node);
        QFETCH(Component*, comp);
        QVERIFY2(node->getWritePortComponentPtr() == comp, "Failed to get write port component from node.");
    }

    void Node_Get_Write_Port_Component_Pointer_data()
    {
        QTest::addColumn<Node*>("node");
        QTest::addColumn<Component*>("comp");
        QTest::newRow("0") << mpSystemFromText->getSubComponent("TestGain")->getPort("in")->getNodePtr() << mpSystemFromText->getSubComponent("TestStep");
    }

    void Generator_FMU_Export()
    {
        QFETCH(ComponentSystem*, system);

        QString pwd = QDir::currentPath();

        //Generate FMU
        GeneratorHandler *pHandler = new GeneratorHandler();
        pHandler->callFmuExportGenerator(HString(pwd.toStdString().c_str())+"/fmu/", system, HString(pwd.toStdString().c_str())+"/../../../HopsanCore/include/", HString(pwd.toStdString().c_str())+"/../../../bin/", false);

        QString code = "#include \"ComponentEssentials.h\"\n"
                "namespace hopsan {\n"
                "  class HydraulicLaminarOrifice : public ComponentQ\n"
                "    {\n"
                "      public:\n"
                "        double ko;\n"
                "      private:\n"
                "        int gris;\n"
                "        Integrator katt;\n"
                "      void simulateOneTimestep()\n"
                "      {\n"
                "        gris=ko+5;\n"
                "      }\n"
                "      bool initialize()\n"
                "      {\n"
                "        int x=gris*3;\n"
                "        gris= 3;\n"
                "        ko=5;\n"
                "      }\n"
                "    };\n"
                "}\n";
        QStringList errorMsgs;
        //examineCode(code, errorMsgs);

        //Run FMUChecker
        QStringList args;
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << QDir::currentPath()+"/fmu/unittestmodel_export.fmu";
        QProcess p;
        p.start(QDir::currentPath()+"/../../../ThirdParty/FMUChecker/fmuCheck.win32.exe", args);
        p.waitForReadyRead();
        QString output = p.readAllStandardError();
        QStringList errors = output.split("\n");

        QVERIFY2(errors.contains("\t0 warning(s) and error(s)\r"), "Failed to generate FMU, FMU not acceted by FMUChecker.");
    }

    void Generator_FMU_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        double start, stop;
        removeDir(QDir::currentPath()+"/fmu/");
        QDir().mkpath(QDir::currentPath()+"/fmu/");
        QString path = QDir::currentPath()+"/../../../Models/unittestmodel_export.hmf";
        QFile file(path);
        file.copy(QDir::currentPath()+"/fmu/unittestmodel_export.hmf");
        QTest::newRow("0") << mHopsanCore.loadHMFModel(path.toStdString().c_str(),start,stop);
    }

    void Generator_Simulink_Export()
    {
        QFETCH(ComponentSystem*, system);

        QString pwd = QDir::currentPath();

        //Generate S-function
        GeneratorHandler *pHandler = new GeneratorHandler();
        pHandler->callSimulinkExportGenerator(HString(pwd.toStdString().c_str())+"/simulink/", "unittestmodel_export.hmf", system, false, 0, HString(pwd.toStdString().c_str())+"/../../../HopsanCore/include/", HString(pwd.toStdString().c_str())+"/../../../bin/", false);

        QVERIFY2(QFile::exists(pwd+"/simulink/externalLibs.txt"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanCore.dll"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanCore.exp"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanCore.lib"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanSimulink.cpp"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanSimulinkCompile.m"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanSimulinkPortLabels.m"), "Failed to generate S-function, all files not found.");
    }

    void Generator_Simulink_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        double start, stop;
        QString path = QDir::currentPath()+"/../../../Models/unittestmodel_export.hmf";
        removeDir(QDir::currentPath()+"/simulink/");
        QDir().mkpath(QDir::currentPath()+"/simulink/");
        QFile file(path);
        file.copy(QDir::currentPath()+"/simulink/unittestmodel_export.hmf");
        QTest::newRow("0") << mHopsanCore.loadHMFModel(path.toStdString().c_str(),start,stop);
    }

    void Generator_Labview_Export()
    {
        QFETCH(ComponentSystem*, system);

        QString pwd = QDir::currentPath();
        HString pwdPath = HString(pwd.toStdString().c_str());

        //Generate S-function
        GeneratorHandler *pHandler = new GeneratorHandler();
        pHandler->callLabViewSITGenerator(pwdPath+"/labview/unittestmodel_export.cpp", system, HString(pwd.toStdString().c_str())+"/../../../HopsanCore/include/", HString(pwd.toStdString().c_str())+"/../../../bin/", false);

        QVERIFY2(QFile::exists(pwd+"/labview/codegen.c"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/hopsanrt-wrapper.h"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/HOW_TO_COMPILE.txt"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/model.h"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/SIT_API.h"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/unittestmodel_export.cpp"), "Failed to generate LabVIEW files, all files not found.");

        QDir includeDir(pwd+"/labview/HopsanCore/include");
        QVERIFY2(includeDir.exists() && !includeDir.entryList().isEmpty(), "Failed to generate LabVIEW files: Include files not found.");
        QDir srcDir(pwd+"/labview/HopsanCore/src");
        QVERIFY2(srcDir.exists() && !srcDir.entryList().isEmpty(), "Failed to generate LabVIEW files: Source files not found.");
        QDir dependenciesDir(pwd+"/labview/HopsanCore/Dependencies");
        QVERIFY2(dependenciesDir.exists() && !dependenciesDir.entryList(QDir::AllEntries).isEmpty(), "Failed to generate LabVIEW files: Dependency files not found.");
        QDir libDir(pwd+"/labview/componentLibraries/defaultLibrary");
        QVERIFY2(libDir.exists() && !libDir.entryList().isEmpty(), "Failed to generate LabVIEW files: Default library files not found.");
    }

    void Generator_Labview_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        double start, stop;
        QString path = QDir::currentPath()+"/../../../Models/unittestmodel_export.hmf";
        removeDir(QDir::currentPath()+"/labview/");
        QDir().mkpath(QDir::currentPath()+"/labview/");
        QTest::newRow("0") << mHopsanCore.loadHMFModel(path.toStdString().c_str(),start,stop);
    }

    void Generator_Modelica()
    {
        QFETCH(HString, code);
        QFETCH(HString, name);

        QString pwd = QDir::currentPath();

        //Generate FMU
        GeneratorHandler *pHandler = new GeneratorHandler();
        pHandler->callModelicaGenerator(code, HString(pwd.toStdString().c_str())+"/../../../HopsanCore/include/", HString(pwd.toStdString().c_str())+"/../../../bin/", false, HString(pwd.toStdString().c_str())+"/modelica/", name);

        QVERIFY2(QDir().exists(QString(HString(HString(pwd.toStdString().c_str())+HString("/modelica/")+name+HString(".dll")).c_str())), "Failure! Modelica generator failed to generate dll.");
    }

    void Generator_Modelica_data()
    {
        QTest::addColumn<HString>("code");
        QTest::addColumn<HString>("name");

        const char* moCode = "model MyLaminarOrifice \"Hydraulic Laminar Orifice\"\n"
              "   annotation(hopsanCqsType = \"Q\");\n"
              "   parameter Real Kc(unit=\"-\")=1e-11 \"Pressure-Flow Coefficient\";\n"
              "   NodeHydraulic P1, P2;\n"
              "equation\n"
              "   P2.q = Kc*(P1.p-P2.p);\n"
              "   P1.q = -P2.q;\n"
              "   P1.p = P1.c + P1.Zc*P1.q;\n"
              "   P2.p = P2.c + P2.Zc*P2.q;\n"
              "end LaminarOrifice;\n";

        QTest::newRow("0") << HString(moCode) << HString("MyLaminarOrifice");
    }

    void examineCode(QString code, QStringList &errors)
    {
        QStringList lines = code.split("\n");
        bool includesOk=false;
        bool nameSpaceOk=false;
        bool inClass=false;
        bool inPublic=false;
        bool inPrivate=false;
        bool publicMembers = false;
        bool inFunction=false;
        QString name, cqsType, funcName;
        QStringList memberNames, memberTypes;
        QList<bool> assigned, used, assignedBeforeUse;
        int bal=0;
        QStringList initLines, simLines;
        for(int i=0; i<lines.size(); ++i)
        {
            QString l = lines[i].simplified();

            if(!inClass && l == "#include \"ComponentEssentials.h\"") { includesOk = true; }
            if(!inClass && l.startsWith("namespace hopsan {")) { nameSpaceOk = true; }
            if(l.startsWith("class ") && l.contains(" : public Component"))
            {
                inClass = true;
                name = l.section(" ",1,1);
                cqsType = l.section(" : public Component",1,1);
            }
            if(inClass && "public:") inPublic=true;
            if(inClass && l == "private:") inPrivate=true;
            if(!inFunction && inClass && (inPublic || inPrivate) && l.count(" ") == 1 && l.endsWith(";"))
            {
                memberTypes.append(l.section(" ",0,0));
                memberNames.append(l.section(" ",1,1).remove(";"));
                assigned.append(false);
                used.append(false);
                assignedBeforeUse.append(true);
                if(inPublic) publicMembers = true;
            }
            if((inPublic || inPrivate) && l.count("(") == 1 && l.count(")") == 1)
            {
                inFunction = true;
                funcName = l.section(" ",1,1).section("(",0,0);
            }
            if(l.contains("{"))
            {
                bal = bal+l.count("{");
            }
            if(l.contains("}"))
            {
                bal = bal-l.count("}");
                if(bal==2)
                {
                    inFunction=false;
                    funcName="";
                }
                if(bal==1)
                {
                    inClass = false;
                    inPrivate = false;
                    inPublic = false;
                }
            }
            if(inFunction && funcName == "initialize") initLines.append(l);
            if(inFunction && funcName == "simulateOneTimestep") simLines.append(l);
        }
        initLines.removeAt(0);
        initLines.removeAt(0);
        simLines.removeAt(0);
        simLines.removeAt(0);

        QStringList initAndSimulate = initLines;
        initAndSimulate.append(simLines);

        for(int i=0; i<initAndSimulate.size(); ++i)
        {
            QString l = initAndSimulate[i];

            if(l.count("=") == 1)
            {
                QString name = l.section("=",0,0).trimmed();
                if(memberNames.contains(name))
                    assigned[memberNames.indexOf(name)] = true;
            }
            l.remove(0,l.indexOf("="));
            for(int j=0; j<l.size(); ++j)
            {
                if(!l[j].isLetterOrNumber())
                {
                    l[j] = ' ';
                }
            }
            QStringList splitLine = l.split(" ");
            for(int j=0; j<memberNames.size(); ++j)
            {
                if(splitLine.contains(memberNames[j]))
                {
                    used[j] = true;
                    if(!assigned[j])
                    {
                        assignedBeforeUse[j] = false;
                    }
                }
            }
        }

        if(!includesOk)
            errors.append("WARNING: ComponentEssentials.h is not included!");
        if(!nameSpaceOk)
            errors.append("WARNING: \"hopsan\" namespace is not used!");
        if(publicMembers)
            errors.append("WARNING: Public member variables are not recommended!");
        for(int i=0; i<used.size(); ++i)
        {
            if(!used[i])
                errors.append("WARNING: Unused member variable \""+memberNames[i]+"\"!");
            if(!assignedBeforeUse[i])
                errors.append("WARNING: Member variable \""+memberNames[i]+"\" is used uninitialized!");
        }
    }

    void removeDir(QString path)
    {
        QDir dir;
        dir.setPath(path);
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir())
            {
                removeDir(info.absoluteFilePath());
            }
            else
            {
                QFile::remove(info.absoluteFilePath());
            }
        }
        dir.rmdir(path);
    }
};

QTEST_APPLESS_MAIN(SimulationTests)

#include "tst_simulationtest.moc"
