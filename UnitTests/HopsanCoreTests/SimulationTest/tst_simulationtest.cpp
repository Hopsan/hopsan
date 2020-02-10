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

#if __cplusplus < 201103L
#define nullptr 0
#endif

#include "HopsanEssentials.h"
#include "HopsanCoreVersion.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/HmfLoader.h"

#include <assert.h>
#include <algorithm>

#ifndef DEFAULT_LIBRARY_ROOT
#define DEFAULT_LIBRARY_ROOT "../componentLibraries/defaultLibrary"
#endif

#ifndef TEST_DATA_ROOT
#define TEST_DATA_ROOT "../UnitTests/HopsanCoreTests/SimulationTest/"
#endif

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE TO_STR(SHAREDLIB_PREFIX) "defaultcomponentlibrary" TO_STR(HOPSAN_DEBUG_POSTFIX) "." TO_STR(SHAREDLIB_SUFFIX)
const std::string defaultLibraryFilePath = DEFAULT_LIBRARY_ROOT "/" DEFAULTLIBFILE;
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

private:
    HopsanEssentials mHopsanCore;
    ComponentSystem *mpSystemFromText = nullptr;
    ComponentSystem *mpSystemFromFile = nullptr;

private slots:
    void initTestCase() {
        bool did_load = mHopsanCore.loadExternalComponentLib(defaultLibraryFilePath.c_str());
        QVERIFY2(did_load, qPrintable(QString("Could not load default component library: ")+QString::fromStdString(defaultLibraryFilePath)));

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
        QVERIFY2(mpSystemFromText, "Could not load system from hmf text");
        mpSystemFromFile = mHopsanCore.loadHMFModelFile(TEST_DATA_ROOT "unittestmodel.hmf",startT,stopT);
        QVERIFY2(mpSystemFromText, "Could not load system from " TEST_DATA_ROOT "unittestmodel.hmf");
    }

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
        QFETCH(HString, subSystemName);
        QFETCH(HString, paramName);
        QFETCH(HString, paramType);
        QFETCH(HString, paramValue);
        QFETCH(HString, expectedEvaluatedParamValue);

        ComponentSystem* pEvalSystem = mpSystemFromFile;
        if (!subSystemName.empty()) {
            HVector<HString> nameParts = subSystemName.split('$');
            for (size_t i=0; i < nameParts.size(); ++i) {
                pEvalSystem = pEvalSystem->getSubComponentSystem(nameParts[i]);
            }
        }
        QVERIFY(pEvalSystem != nullptr);

        pEvalSystem->setParameterValue(paramName, paramValue);
        HString actualValue;
        pEvalSystem->getParameterValue(paramName, actualValue);
        QVERIFY2(actualValue.compare(paramValue), (actualValue + " != " + paramValue).c_str());
        pEvalSystem->evaluateParameter(paramName, actualValue, paramType);
        QVERIFY2(actualValue.compare(expectedEvaluatedParamValue), (actualValue+" != "+expectedEvaluatedParamValue).c_str());
    }

    void System_Set_Parameter_data()
    {
        QTest::addColumn<HString>("subSystemName");
        QTest::addColumn<HString>("paramName");
        QTest::addColumn<HString>("paramType");
        QTest::addColumn<HString>("paramValue");
        QTest::addColumn<HString>("expectedEvaluatedParamValue");

        const HString mainsystem = "";
        const HString subsystem = "Subsystem";
        const HString subsubsystem = "Subsystem$Subsubsystem";

        HString evalSystem, paramName, paramType, paramValue, expectedEvaluatedParamValue;

        evalSystem = mainsystem;
        paramName = "apa";
        paramType = "double";
        paramValue = "12";
        expectedEvaluatedParamValue = "12";
        QTest::newRow("0") << evalSystem << paramName << paramType << paramValue << expectedEvaluatedParamValue;

        evalSystem = subsubsystem;
        paramName = "subsub_int_a";
        paramType = "integer";
        paramValue = "-sub_int_a";
        expectedEvaluatedParamValue = "-1";
        QTest::newRow("1") << evalSystem << paramName << paramType << paramValue << expectedEvaluatedParamValue;
        // --- RESET ---
        //! @todo make it so that tests do not pollute each other
        paramValue = "sub_int_a";
        expectedEvaluatedParamValue = "1";
        QTest::newRow("1reset") << evalSystem << paramName << paramType << paramValue << expectedEvaluatedParamValue;
        // ---
    }

    void System_GetAndEval_Parameter()
    {
        QFETCH(HString, subSystemName);
        QFETCH(HString, paramName);
        QFETCH(bool, expectHasParameter);
        QFETCH(bool, expectEvalOK);
        QFETCH(HString, expectedParamValue);
        QFETCH(double, expectedDValue);

        ComponentSystem* pEvalSystem = mpSystemFromFile;
        if (!subSystemName.empty()) {
            HVector<HString> nameParts = subSystemName.split('$');
            for (size_t i=0; i < nameParts.size(); ++i) {
                pEvalSystem = pEvalSystem->getSubComponentSystem(nameParts[i]);
            }
        }
        QVERIFY(pEvalSystem != nullptr);

        QVERIFY(pEvalSystem->hasParameter(paramName) == expectHasParameter);
        HString actualValue;
        pEvalSystem->getParameterValue(paramName, actualValue);
        QVERIFY2(actualValue.compare(expectedParamValue), (actualValue+"!="+expectedParamValue).c_str());
        bool evalOK = pEvalSystem->evaluateParameter(paramName, actualValue, "double");
        QVERIFY(evalOK == expectEvalOK);
        if (evalOK) {
          bool toDoubleOK;
          double actualDValue = actualValue.toDouble(&toDoubleOK);
          QVERIFY(toDoubleOK);
          QVERIFY(expectedDValue == actualDValue);
        }
    }

    void System_GetAndEval_Parameter_data()
    {
        QTest::addColumn<HString>("subSystemName");
        QTest::addColumn<HString>("paramName");
        QTest::addColumn<bool>("expectHasParameter");
        QTest::addColumn<bool>("expectEvalOK");
        QTest::addColumn<HString>("expectedParamValue");
        QTest::addColumn<double>("expectedDValue");

        const HString mainsystem = "";
        const HString subsystem = "Subsystem";
        const HString subsubsystem = "Subsystem$Subsubsystem";

        HString evalSystem, paramName, expectedParamValue;
        double expectedDValue;
        bool expectEvalOK, expectHasParameter;

        evalSystem = subsubsystem;
        paramName = "subsub_a";
        expectHasParameter = true;
        expectedParamValue = "sub_a";
        expectedDValue = 1.0;
        expectEvalOK = true;
        QTest::newRow("0") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        evalSystem = subsystem;
        paramName = "sub_a";
        expectHasParameter = true;
        expectedParamValue = "main_a";
        expectedDValue = 1.0;
        expectEvalOK = true;
        QTest::newRow("1") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        evalSystem = mainsystem;
        paramName = "main_a";
        expectHasParameter = true;
        expectedParamValue = "1";
        expectedDValue = 1.0;
        expectEvalOK = true;
        QTest::newRow("2") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        evalSystem = subsubsystem;
        paramName = "subsub_b";
        expectHasParameter = true;
        expectedParamValue = "sub_b";
        expectedDValue = 2.0;
        expectEvalOK = true;
        QTest::newRow("3") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        evalSystem = subsystem;
        paramName = "sub_b";
        expectHasParameter = true;
        expectedParamValue = "2";
        expectedDValue = 2.0;
        expectEvalOK = true;
        QTest::newRow("4") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        // subsub_c is a system parameter whose value is an expression of the two parent variables
        evalSystem = subsubsystem;
        paramName = "subsub_c";
        expectHasParameter = true;
        expectedParamValue = "sub_a+sub_b";
        expectedDValue = 3.0;
        expectEvalOK = true;
        QTest::newRow("5") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        // subsub_d is a system parameter whose value is an expression of the two other variable in this system
        evalSystem = subsubsystem;
        paramName = "subsub_d";
        expectHasParameter = true;
        expectedParamValue = "self.subsub_a+self.subsub_b";
        expectedDValue = 3.0;
        expectEvalOK = true;
        QTest::newRow("6") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        // subsub_e is a system parameter whose value is an expression of one other variable in this system and one of the grandparent
        evalSystem = subsubsystem;
        paramName = "subsub_e";
        expectHasParameter = true;
        expectedParamValue = "main_a+self.subsub_b";
        expectedDValue = 3.0;
        expectEvalOK = true;
        QTest::newRow("7") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        // main_a does not exist in subsystem and can not be evaluated by this component directly
        evalSystem = subsystem;
        paramName = "main_a";
        expectHasParameter = false;
        expectedParamValue = "";
        expectedDValue = 1.0;
        expectEvalOK = false;
        QTest::newRow("8") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        // main_a does not exist in subsubsystem and can not be evaluated by this component directly
        evalSystem = subsubsystem;
        paramName = "main_a";
        expectHasParameter = false;
        expectedParamValue = "";
        expectedDValue = 1.0;
        expectEvalOK = false;
        QTest::newRow("9") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;

        // Make sure that the correct shadow_param is choosen
        evalSystem = subsystem;
        paramName = "shadow_param";
        expectHasParameter = true;
        expectedParamValue = "2";
        expectedDValue = 2.0;
        expectEvalOK = true;
        QTest::newRow("10") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;
        evalSystem = mainsystem;
        expectedParamValue = "-1";
        expectedDValue = -1;
        QTest::newRow("11") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedDValue;
    }

    void System_GetAndEval_Int_Parameter()
    {
        QFETCH(HString, subSystemName);
        QFETCH(HString, paramName);
        QFETCH(bool, expectHasParameter);
        QFETCH(bool, expectEvalOK);
        QFETCH(HString, expectedParamValue);
        QFETCH(int, expectedIValue);

        ComponentSystem* pEvalSystem = mpSystemFromFile;
        if (!subSystemName.empty()) {
            HVector<HString> nameParts = subSystemName.split('$');
            for (size_t i=0; i < nameParts.size(); ++i) {
                pEvalSystem = pEvalSystem->getSubComponentSystem(nameParts[i]);
            }
        }
        QVERIFY(pEvalSystem != nullptr);

        QVERIFY(pEvalSystem->hasParameter(paramName) == expectHasParameter);
        HString actualValue;
        pEvalSystem->getParameterValue(paramName, actualValue);
        QVERIFY(actualValue.compare(expectedParamValue));
        bool evalOK = pEvalSystem->evaluateParameter(paramName, actualValue, "integer");
        QVERIFY(evalOK == expectEvalOK);
        bool numEvelOK;
        int actualIValue = actualValue.toLongInt(&numEvelOK);
        QVERIFY(numEvelOK == true);
        QVERIFY(expectedIValue == actualIValue);
    }

    void System_GetAndEval_Int_Parameter_data()
    {
        QTest::addColumn<HString>("subSystemName");
        QTest::addColumn<HString>("paramName");
        QTest::addColumn<bool>("expectHasParameter");
        QTest::addColumn<bool>("expectEvalOK");
        QTest::addColumn<HString>("expectedParamValue");
        QTest::addColumn<int>("expectedIValue");

        const HString mainsystem = "";
        const HString subsystem = "Subsystem";
        const HString subsubsystem = "Subsystem$Subsubsystem";

        HString evalSystem, paramName, expectedParamValue;
        int expectedIValue;
        bool expectEvalOK, expectHasParameter;


        // Evaluate integer parameter recursively
        evalSystem = subsystem;
        paramName = "sub_int_a";
        expectHasParameter = true;
        expectedParamValue = "main_int_a";
        expectedIValue = 1;
        expectEvalOK = true;
        QTest::newRow("1") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedIValue;

        evalSystem = subsubsystem;
        paramName = "subsub_int_a";
        expectHasParameter = true;
        expectedParamValue = "sub_int_a";
        expectedIValue = 1;
        expectEvalOK = true;
        QTest::newRow("2") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedIValue;

        evalSystem = subsubsystem;
        paramName = "subsub_int_b";
        expectHasParameter = true;
        expectedParamValue = "sub_int_b";
        expectedIValue = 2;
        expectEvalOK = true;
        QTest::newRow("3") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedIValue;
    }

    void System_GetAndEval_Bool_Parameter()
    {
        QFETCH(HString, subSystemName);
        QFETCH(HString, paramName);
        QFETCH(bool, expectHasParameter);
        QFETCH(bool, expectEvalOK);
        QFETCH(HString, expectedParamValue);
        QFETCH(bool, expectedBValue);

        ComponentSystem* pEvalSystem = mpSystemFromFile;
        if (!subSystemName.empty()) {
            HVector<HString> nameParts = subSystemName.split('$');
            for (size_t i=0; i < nameParts.size(); ++i) {
                pEvalSystem = pEvalSystem->getSubComponentSystem(nameParts[i]);
            }
        }
        QVERIFY(pEvalSystem != nullptr);

        QVERIFY(pEvalSystem->hasParameter(paramName) == expectHasParameter);
        HString actualValue;
        pEvalSystem->getParameterValue(paramName, actualValue);
        QVERIFY(actualValue.compare(expectedParamValue));
        bool evalOK = pEvalSystem->evaluateParameter(paramName, actualValue, "bool");
        QVERIFY(evalOK == expectEvalOK);
        bool boolEvelOK;
        bool actualBValue = actualValue.toBool(&boolEvelOK);
        QVERIFY(boolEvelOK == true);
        QVERIFY(expectedBValue == actualBValue);
    }

    void System_GetAndEval_Bool_Parameter_data()
    {
        QTest::addColumn<HString>("subSystemName");
        QTest::addColumn<HString>("paramName");
        QTest::addColumn<bool>("expectHasParameter");
        QTest::addColumn<bool>("expectEvalOK");
        QTest::addColumn<HString>("expectedParamValue");
        QTest::addColumn<bool>("expectedBValue");

        const HString mainsystem = "";
        const HString subsystem = "Subsystem";
        const HString subsubsystem = "Subsystem$Subsubsystem";

        HString evalSystem, paramName, expectedParamValue;
        bool expectedBValue;
        bool expectEvalOK, expectHasParameter;


        // Evaluate integer parameter recursively
        evalSystem = subsystem;
        paramName = "sub_bool_a";
        expectHasParameter = true;
        expectedParamValue = "main_bool_a";
        expectedBValue = true;
        expectEvalOK = true;
        QTest::newRow("1") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedBValue;

        evalSystem = subsubsystem;
        paramName = "subsub_bool_a";
        expectHasParameter = true;
        expectedParamValue = "sub_bool_a";
        expectedBValue = true;
        expectEvalOK = true;
        QTest::newRow("2") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedBValue;

        evalSystem = subsubsystem;
        paramName = "subsub_bool_b";
        expectHasParameter = true;
        expectedParamValue = "sub_bool_b";
        expectedBValue = false;
        expectEvalOK = true;
        QTest::newRow("3") << evalSystem << paramName << expectHasParameter << expectEvalOK << expectedParamValue << expectedBValue;
    }

    void System_GetAndEval_StringTextblock_Parameter()
    {
        QFETCH(HString, subSystemName);
        QFETCH(HString, paramName);
        QFETCH(HString, paramType);
        QFETCH(bool, expectHasParameter);
        QFETCH(bool, expectEvalOK);
        QFETCH(HString, expectedParamValue);
        QFETCH(HString, expectedEvaluatedParamValue);


        ComponentSystem* pEvalSystem = mpSystemFromFile;
        if (!subSystemName.empty()) {
            HVector<HString> nameParts = subSystemName.split('$');
            for (size_t i=0; i < nameParts.size(); ++i) {
                pEvalSystem = pEvalSystem->getSubComponentSystem(nameParts[i]);
            }
        }
        QVERIFY(pEvalSystem != nullptr);

        QVERIFY(pEvalSystem->hasParameter(paramName) == expectHasParameter);
        HString actualValue;
        pEvalSystem->getParameterValue(paramName, actualValue);
        QVERIFY(actualValue.compare(expectedParamValue));
        bool evalOK = pEvalSystem->evaluateParameter(paramName, actualValue, paramType);
        QVERIFY(evalOK == expectEvalOK);
        QVERIFY(expectedEvaluatedParamValue == actualValue);
    }

    void System_GetAndEval_StringTextblock_Parameter_data()
    {
        QTest::addColumn<HString>("subSystemName");
        QTest::addColumn<HString>("paramName");
        QTest::addColumn<HString>("paramType");
        QTest::addColumn<bool>("expectHasParameter");
        QTest::addColumn<bool>("expectEvalOK");
        QTest::addColumn<HString>("expectedParamValue");
        QTest::addColumn<HString>("expectedEvaluatedParamValue");

        const HString mainsystem = "";
        const HString subsystem = "Subsystem";
        const HString subsubsystem = "Subsystem$Subsubsystem";
        const HString stringtype = "string";
        const HString textblocktype = "textblock";

        HString evalSystem, paramName, paramType, expectedParamValue, expectedEvaluatedParamValue;
        bool expectEvalOK, expectHasParameter;

        evalSystem = subsystem;
        paramName = "sub_string_a";
        paramType = stringtype;
        expectHasParameter = true;
        expectedParamValue = "main_string_a";
        expectedEvaluatedParamValue = "string_a";
        expectEvalOK = true;
        QTest::newRow("1") << evalSystem << paramName << paramType << expectHasParameter << expectEvalOK << expectedParamValue << expectedEvaluatedParamValue;

        evalSystem = subsubsystem;
        paramName = "subsub_string_a";
        paramType = stringtype;
        expectHasParameter = true;
        expectedParamValue = "sub_string_a";
        expectedEvaluatedParamValue = "string_a";
        expectEvalOK = true;
        QTest::newRow("2") << evalSystem << paramName << paramType << expectHasParameter << expectEvalOK << expectedParamValue << expectedEvaluatedParamValue;

        evalSystem = subsubsystem;
        paramName = "subsub_string_b";
        paramType = stringtype;
        expectHasParameter = true;
        expectedParamValue = "sub_string_b";
        expectedEvaluatedParamValue = "string_b";
        expectEvalOK = true;
        QTest::newRow("3") << evalSystem << paramName << paramType << expectHasParameter << expectEvalOK << expectedParamValue << expectedEvaluatedParamValue;

        evalSystem = subsystem;
        paramName = "sub_textblock_a";
        paramType = textblocktype;
        expectHasParameter = true;
        expectedParamValue = "main_textblock_a";
        expectedEvaluatedParamValue = "textblock_a";
        expectEvalOK = true;
        QTest::newRow("4") << evalSystem << paramName << paramType << expectHasParameter << expectEvalOK << expectedParamValue << expectedEvaluatedParamValue;

        evalSystem = subsubsystem;
        paramName = "subsub_textblock_a";
        paramType = textblocktype;
        expectHasParameter = true;
        expectedParamValue = "sub_textblock_a";
        expectedEvaluatedParamValue = "textblock_a";
        expectEvalOK = true;
        QTest::newRow("5") << evalSystem << paramName << paramType << expectHasParameter << expectEvalOK << expectedParamValue << expectedEvaluatedParamValue;

        evalSystem = subsubsystem;
        paramName = "subsub_textblock_b";
        paramType = textblocktype;
        expectHasParameter = true;
        expectedParamValue = "sub_textblock_b";
        expectedEvaluatedParamValue = "textblock_b";
        expectEvalOK = true;
        QTest::newRow("6") << evalSystem << paramName << paramType << expectHasParameter << expectEvalOK << expectedParamValue << expectedEvaluatedParamValue;
    }


    void System_NumHop_GetAndEval_Parameter()
    {
        QFETCH(HString, subSystemName);
        QFETCH(HString, script);
        QFETCH(bool, expectedNumhopEvalOK);
        QFETCH(HString, expectedValue);

        // Initialize to evaulate internal numhop script
        QVERIFY(mpSystemFromFile->initialize(0, 10));

        ComponentSystem* pEvalSystem = mpSystemFromFile;
        if (!subSystemName.empty()) {
            HVector<HString> nameParts = subSystemName.split('$');
            for (size_t i=0; i < nameParts.size(); ++i) {
                pEvalSystem = pEvalSystem->getSubComponentSystem(nameParts[i]);
            }
        }
        QVERIFY(pEvalSystem != nullptr);

        HString eval_output;
        bool numhopEvalOK = pEvalSystem->runNumHopScript(script, true, eval_output);
        QVERIFY2(numhopEvalOK == expectedNumhopEvalOK, eval_output.c_str());
        if (numhopEvalOK) {
            Component* pTestConstant = pEvalSystem->getSubComponent("TestConstant");
            QVERIFY(pTestConstant != nullptr);
            HString value;
            bool evalOK = pTestConstant->evaluateParameter("y#Value", value, "double");
            QVERIFY(evalOK);
            QVERIFY2(value == expectedValue, (value+" != "+expectedValue).c_str());
        }
    }

    void System_NumHop_GetAndEval_Parameter_data()
    {
        QTest::addColumn<HString>("subSystemName");
        QTest::addColumn<HString>("script");
        QTest::addColumn<bool>("expectedNumhopEvalOK");
        QTest::addColumn<HString>("expectedValue");

        const HString mainsystem = "";
        const HString subsystem = "Subsystem";

        HString evalSystem, script, expectedValue;
        bool expectEvalOK;

        HString expected_apa_value;
        mpSystemFromFile->getParameterValue("apa", expected_apa_value);


        // NumHop can use system parameters in this system directly,
        evalSystem = mainsystem;
        script = "TestConstant.y = self.apa";
        expectEvalOK = true;
        expectedValue = expected_apa_value;
        QTest::newRow("0") << evalSystem << script << expectEvalOK << expectedValue;

        // NumHop can use system parameters in this system directly, even if one of them points to a value in the current systems parent system
        evalSystem = subsystem;
        script = "TestConstant.y = self.sub_a + self.sub_b";
        expectEvalOK = true;
        expectedValue = "3";
        QTest::newRow("1") << evalSystem << script << expectEvalOK << expectedValue;

        // NumHop can not use variable values in parent system directly (if this is a system component)
        // This works for ordinary parameter expression evaluation but has not been implemented in the numhop helper for systems, there is a todo note about doing that though
        evalSystem = subsystem;
        script = "TestConstant.y = main_a + sub_b";
        expectEvalOK = false;
        expectedValue = "";
        QTest::newRow("2") << evalSystem << script << expectEvalOK << expectedValue;

        // Evaluate system parameters from a subcomponent (that is a subsystem) together with an ordinary component
        evalSystem = mainsystem;
        script = "TestConstant.y = Subsystem.sub_a + Subsystem.sub_b + TestGain.k";
        expectEvalOK = true;
        expectedValue = "4";
        QTest::newRow("3") << evalSystem << script << expectEvalOK << expectedValue;

        // Evaluate system parameter together with an ordinary component parameter, when the parameters have the same name
        // This does not work, because you can give them the same name exatly, in the system case # separator is OK but in the component name, the separator must be . for some reson
        // Under the hood . and # should be treeted equally, but this is not the case, but this is an edge-case
        evalSystem = subsystem;
        script = "TestConstant.y = Add.in2 + in1#Value";
        expectEvalOK = false;
        expectedValue = "1";
        QTest::newRow("4") << evalSystem << script << expectEvalOK << expectedValue;

        // Evaluate system parameters in current and sub system when they have the same name
        evalSystem = mainsystem;
        script = "TestConstant.y = Subsystem.shadow_param * self.shadow_param";
        expectEvalOK = true;
        expectedValue = "-2";
        QTest::newRow("5") << evalSystem << script << expectEvalOK << expectedValue;

        // Verrify that the embedded numhop script in Subsystem has been run and correctly evaulated
        evalSystem = subsystem;
        script = "TestConstant.y = NumhopSetValue.y";
        expectEvalOK = true;
        expectedValue = "6";
        QTest::newRow("6") << evalSystem << script << expectEvalOK << expectedValue;
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

    void Component_GetAndEval_Parameter()
    {
        QFETCH(HString, compName);
        QFETCH(HString, paramName);
        QFETCH(HString, paramType);
        QFETCH(HString, expectedValue);
        QFETCH(HString, expectedEvaluatedValue);

        HVector<HString> compNameParts = compName.split('$');
        ComponentSystem* pEvalSystem = mpSystemFromFile;
        for (size_t i=0; i < std::max(compNameParts.size(), static_cast<size_t>(1)) - 1; ++i) {
            pEvalSystem = pEvalSystem->getSubComponentSystem(compNameParts[i]);
        }
        Component* pComponent = pEvalSystem->getSubComponent(compNameParts.last());
        QVERIFY2(pComponent != nullptr, "Could not get component");

        QVERIFY(pComponent->hasParameter(paramName));
        HString actualValue;
        pComponent->getParameterValue(paramName, actualValue);
        QVERIFY2(actualValue == expectedValue, (actualValue + " != " + expectedValue).c_str());
        pComponent->evaluateParameter(paramName, actualValue, paramType);
        QVERIFY(actualValue == expectedEvaluatedValue);
    }

    void Component_GetAndEval_Parameter_data()
    {
        QTest::addColumn<HString>("compName");
        QTest::addColumn<HString>("paramName");
        QTest::addColumn<HString>("paramType");
        QTest::addColumn<HString>("expectedValue");
        QTest::addColumn<HString>("expectedEvaluatedValue");

        // When a numhop script is used as a parameter value inside an ordinary component, it can lookup parent system variables (Also ones that depend on valus from parents parent)
        QTest::newRow("0") << HString("Subsystem$Gain") << HString("k#Value") << HString("double") << HString("sub_b+sub_a") << HString("3");
        // When a numhop script is used as a parameter value inside an ordinary component, it can lookup parent system variables and even use parents parent parameters directly
        QTest::newRow("1") << HString("Subsystem$Gain_1") << HString("k#Value") << HString("double") << HString("sub_b+main_a") << HString("3");

        // Make sure that parameter in2 can use self.in1, which is points to a system parameter, in an expression
        QTest::newRow("2") << HString("Subsystem$Subsubsystem$Add") << HString("in1#Value") << HString("double") << HString("subsub_a") << HString("1");
        QTest::newRow("3") << HString("Subsystem$Subsubsystem$Add") << HString("in2#Value") << HString("double") << HString("self.in1.Value+1") << HString("2");

        // Test various combinations of using system parameters taht are expressions
        QTest::newRow("4") << HString("Subsystem$Subsubsystem$Gain_1") << HString("k#Value") << HString("double") << HString("subsub_c") << HString("3");
        QTest::newRow("5") << HString("Subsystem$Subsubsystem$Gain_2") << HString("k#Value") << HString("double") << HString("subsub_d") << HString("3");
        QTest::newRow("6") << HString("Subsystem$Subsubsystem$Gain_3") << HString("k#Value") << HString("double") << HString("subsub_e") << HString("3");

        // Test that the correct in.value is choosen in Add, the one in the component and not the one in the system
        QTest::newRow("7") << HString("Subsystem$Add") << HString("in2#Value") << HString("double") << HString("self.in1.Value") << HString("0");
        QTest::newRow("8") << HString("Subsystem") << HString("in1#Value") << HString("double") << HString("1") << HString("1");

        // Test that the correct shadow_parm is choosen in ShadowParamGain, when that parameter also exists in the grand parent system
        QTest::newRow("9") << HString("Subsystem$ShadowParamGain") << HString("k#Value") << HString("double") << HString("shadow_param") << HString("2");
        QTest::newRow("10") << HString("Subsystem") << HString("shadow_param") << HString("double") << HString("2") << HString("2");

        // Test that int,bool,string,textblock system parameter can be evaluated
        QTest::newRow("11") << HString("Subsystem$Subsubsystem$1DLookupTable") << HString("text") << HString("textblock") << HString("subsub_lookup_textblock") << HString("1, 1\n2, 2");
        QTest::newRow("12") << HString("Subsystem$Subsubsystem$1DLookupTable") << HString("inid") << HString("integer") << HString("subsub_lookup_inid") << HString("0");
        QTest::newRow("13") << HString("Subsystem$Subsubsystem$1DLookupTable") << HString("comment") << HString("string") << HString("subsub_lookup_comment") << HString("#");
        QTest::newRow("14") << HString("Subsystem$Subsubsystem$1DLookupTable") << HString("reload") << HString("bool") << HString("subsub_lookup_reload") << HString("true");
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
