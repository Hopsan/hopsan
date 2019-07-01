/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#include <QtTest>
#include "HopsanEssentials.h"
#include "HopsanCoreMacros.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/HmfLoader.h"
#include <assert.h>

#define DEFAULTLIBPATH "../componentLibraries/defaultLibrary"

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE TO_STR(DLL_PREFIX) "defaultcomponentlibrary" TO_STR(DEBUG_EXT) TO_STR(DLL_EXT)
const std::string defaultLibraryFilePath = DEFAULTLIBPATH "/" DEFAULTLIBFILE;
#else
const std::string defaultLibraryFilePath = "";
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
        if (!defaultLibraryFilePath.empty())
        {
            mHopsanCore.loadExternalComponentLib(defaultLibraryFilePath.c_str());
        }
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

        double startT, stopT;
        mpSystemFromText = mHopsanCore.loadHMFModel(xmlStr, startT, stopT);
        mpSystemFromFile = mHopsanCore.loadHMFModelFile("../Models/unittestmodel.hmf",startT,stopT);
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
        QTest::newRow("72") << HString("PetriNetInterfaceC");
        QTest::newRow("73") << HString("PetriNetInterfaceQ");
        QTest::newRow("74") << HString("SignalInputInterface");
        QTest::newRow("75") << HString("SignalOutputInterface");
        QTest::newRow("76") << HString("SignalAnimationSwitch");
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
        QTest::newRow("6") << HString("NodePetriNet");
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

        pComp = mHopsanCore.createComponent("SignalSink");
        QTest::newRow("2") << pComp->getPort("in") << true;

        pComp = mHopsanCore.createComponent("HydraulicCylinderC");
        QTest::newRow("3") << pComp->getPort("P1") << true;
        QTest::newRow("4") << pComp->getPort("P2") << true;
        QTest::newRow("5") << pComp->getPort("P3") << false;

        pComp = mHopsanCore.createComponent("HydraulicLaminarOrifice");
        QTest::newRow("6") << pComp->getPort("P1") << false;

        pComp = mHopsanCore.createComponent("MechanicMultiPortTranslationalMass");
        QTest::newRow("7") << pComp->getPort("P1") << true;
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

    void Version_Utilities()
    {
        QFETCH(int, retVal);
        QFETCH(int, ans);
        QVERIFY2(retVal == ans, "Version utility function returned wrong answer.");
    }

    void Version_Utilities_data()
    {
        QTest::addColumn<int>("retVal");
        QTest::addColumn<int>("ans");
        QTest::newRow("0") << getEpochVersion("0.6.7") << 0;
        QTest::newRow("1") << getMajorVersion("0.6.7") << 6;
        QTest::newRow("2") << getMinorVersion("0.6.7") << 7;
        //QTest::newRow("3") << getRevisionNumber("0.6.7") << -1;
        QTest::newRow("4") << getEpochVersion("0.6.x_r7236") << 0;
        QTest::newRow("5") << getMajorVersion("0.6.x_r7236") << 6;
        QTest::newRow("6") << getMinorVersion("0.6.x_r7236") << -1;
        //QTest::newRow("7") << getRevisionNumber("0.6.x_r7236") << 7236;
    }
};

QTEST_APPLESS_MAIN(SimulationTests)

#include "tst_simulationtest.moc"
