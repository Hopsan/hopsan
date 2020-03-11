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
Q_DECLARE_METATYPE(HString)
Q_DECLARE_METATYPE(Component*)
Q_DECLARE_METATYPE(Port*)
Q_DECLARE_METATYPE(Node*)


class SimulationTests : public QObject
{
    Q_OBJECT

private:

    void getComponent(const HString& fullCompName, Component** ppComponent) {
        HVector<HString> nameParts = fullCompName.split('.');
        QVERIFY(nameParts.size() >= 1);

        // Seek into subsystems
        ComponentSystem* pEvalSystem = mpSystemFromFile;
        Component* pComp = nullptr;
        HVector<HString> compNameParts = nameParts.first().split('$');
        for (size_t i=0; i < compNameParts.size(); ++i) {
            pComp = pEvalSystem->getSubComponent(compNameParts[i]);
            if (pComp->isComponentSystem()) {
                pEvalSystem = dynamic_cast<ComponentSystem*>(pComp);
            }
        }
        QVERIFY(pComp);
        *ppComponent = pComp;
    }

    void getSystem(const HString& fullSystemCompName, ComponentSystem** ppSystem) {
        if (fullSystemCompName.empty()) {
            return;
        }
        Component* pComp = nullptr;
        getComponent(fullSystemCompName, &pComp);
        QVERIFY(pComp->isComponentSystem());
        *ppSystem = dynamic_cast<ComponentSystem*>(pComp);
    }

    void getPort(const HString& fullPortName, Port** ppPort) {
        Component* pComp;
        getComponent(fullPortName, &pComp);

        HVector<HString> parts = fullPortName.split('.');
        QVERIFY(parts.size() >= 2);

        Port* pPort = pComp->getPort(parts.last());
        QVERIFY(pPort);
        *ppPort = pPort;
    }


    HopsanEssentials mHopsanCore;

    ComponentSystem *mpSystemFromFile = nullptr;


private slots:
    void init() {
        bool did_load = mHopsanCore.loadExternalComponentLib(defaultLibraryFilePath.c_str());
        QVERIFY2(did_load, qPrintable(QString("Could not load default component library: ")+QString::fromStdString(defaultLibraryFilePath)));

        double startT, stopT;
        mpSystemFromFile = mHopsanCore.loadHMFModelFile(TEST_DATA_ROOT "unittestmodel.hmf",startT,stopT);
        QVERIFY2(mpSystemFromFile, "Could not load system from " TEST_DATA_ROOT "unittestmodel.hmf");
    }

    void cleanup() {
        mHopsanCore.removeComponent(mpSystemFromFile);
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
        QFETCH(QString, typeName);
        Node *pNode = mHopsanCore.createNode(qPrintable(typeName));
        bool ok = pNode;
        QVERIFY2(ok, "Failed to create node!");
        mHopsanCore.removeNode(pNode);
    }

    void HopsanCore_Create_Node_data()
    {
        QTest::addColumn<QString>("typeName");
        QTest::newRow("0") << "NodeSignal";
        QTest::newRow("1") << "NodeHydraulic";
        QTest::newRow("2") << "NodePneumatic";
        QTest::newRow("3") << "NodeElectric";
        QTest::newRow("4") << "NodeMechanic";
        QTest::newRow("5") << "NodeMechanicRotational";
        QTest::newRow("6") << "NodePetriNet";
    }

    void Load_Component()
    {
        QFETCH(QString, name);
        QVERIFY2(mpSystemFromFile->haveSubComponent(qPrintable(name)), "Failed to load sub component!");
    }

    void Load_Component_data()
    {
        QTest::addColumn<QString>("name");
        QTest::newRow("2") << "TestStep";
        QTest::newRow("3") << "TestGain";
    }

    void Load_Connect()
    {
        QFETCH(QString, port1);
        QFETCH(QString, port2);
        Port *pPort1, *pPort2;
        getPort(qPrintable(port1), &pPort1);
        getPort(qPrintable(port2), &pPort2);
        QVERIFY2(pPort1->isConnectedTo(pPort2), "Failed to load connection!");
    }

    void Load_Connect_data()
    {
        QTest::addColumn<QString>("port1");
        QTest::addColumn<QString>("port2");

        QTest::newRow("1") << "TestStep.out" << "TestGain.in";
    }

    void Load_Parameter()
    {
        QFETCH(QString, compName);
        QFETCH(QString, paramName);
        QFETCH(QString, expectedParamValue);
        HString hCompName(qPrintable(compName));
        HString hParamName(qPrintable(paramName));
        HString hExpectedParamValue(qPrintable(expectedParamValue));
        Component* pComp;
        getComponent(hCompName, &pComp);

        HString actualParamVal;
        pComp->getParameterValue(hParamName, actualParamVal);
        QVERIFY2(actualParamVal.compare(hExpectedParamValue), "Failed to load sub component parameter value!");

    }

    void Load_Parameter_data()
    {
        QTest::addColumn<QString>("compName");
        QTest::addColumn<QString>("paramName");
        QTest::addColumn<QString>("expectedParamValue");
        QTest::newRow("1") << "TestStep" << "y_0#Value" << "-5";
        QTest::newRow("2") << "TestStep" << "y_A#Value" << "5";
        QTest::newRow("3") << "TestStep" << "t_step#Value" << "apa";
    }

    void System_Set_Parameter()
    {
        QFETCH(HString, subSystemName);
        QFETCH(HString, paramName);
        QFETCH(HString, paramType);
        QFETCH(HString, paramValue);
        QFETCH(HString, expectedEvaluatedParamValue);

        ComponentSystem* pEvalSystem = mpSystemFromFile;
        getSystem(subSystemName, &pEvalSystem);

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
        getSystem(subSystemName, &pEvalSystem);

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
          QCOMPARE(expectedDValue, actualDValue);
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
        getSystem(subSystemName, &pEvalSystem);

        QVERIFY(pEvalSystem->hasParameter(paramName) == expectHasParameter);
        HString actualValue;
        pEvalSystem->getParameterValue(paramName, actualValue);
        QVERIFY(actualValue.compare(expectedParamValue));
        bool evalOK = pEvalSystem->evaluateParameter(paramName, actualValue, "integer");
        QVERIFY(evalOK == expectEvalOK);
        bool numEvelOK;
        auto actualIValue = static_cast<int>(actualValue.toLongInt(&numEvelOK));
        QVERIFY(numEvelOK == true);
        QCOMPARE(expectedIValue, actualIValue);
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
        getSystem(subSystemName, &pEvalSystem);

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
        getSystem(subSystemName, &pEvalSystem);

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
        getSystem(subSystemName, &pEvalSystem);

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

        // NumHop can use system parameters in this system directly,
        evalSystem = mainsystem;
        script = "TestConstant.y = self.apa";
        expectEvalOK = true;
        expectedValue = "7";
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
        // This does not work, because you cant give them the same name exatly, in the system case # separator is OK but in the component name, the separator must be . for some reson
        // Under the hood . and # should be treeted equally, but this is not the case, but this is an edge-case
        //! @todo Remove possibility to name system paremters wiht # in the name
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
        // Test the same thing but explicitly add .Value for assignment and getting the value
        evalSystem = subsystem;
        script = "TestConstant.y.Value = NumhopSetValue.y.Value";
        expectEvalOK = true;
        expectedValue = "6";
        QTest::newRow("7") << evalSystem << script << expectEvalOK << expectedValue;

        // Test assigning same value using explicit .Value
        evalSystem = subsystem;
        script = "TestConstant.y.Value = TestConstant.y";
        expectEvalOK = true;
        expectedValue = "1";
        QTest::newRow("8") << evalSystem << script << expectEvalOK << expectedValue;
        evalSystem = subsystem;
        script = "TestConstant.y = TestConstant.y.Value";
        expectEvalOK = true;
        expectedValue = "1";
        QTest::newRow("9") << evalSystem << script << expectEvalOK << expectedValue;

        // Test assign and read Powerport start value
        evalSystem = mainsystem;
        script = "TestPressureSource.p = 5; TestTank.P1.Flow = TestPressureSource.p; TestConstant.y = TestTank.P1.Flow ";
        expectEvalOK = true;
        expectedValue = "5";
        QTest::newRow("10") << evalSystem << script << expectEvalOK << expectedValue;

        // Test self variable assign
        evalSystem = mainsystem;
        script = "self.apa = 5; TestConstant.y = self.apa ";
        expectEvalOK = true;
        expectedValue = "5";
        QTest::newRow("11") << evalSystem << script << expectEvalOK << expectedValue;

        // Test numhop assign with incomplete name (in this case a local variable self will be created)
        evalSystem = mainsystem;
        script = "self = 123; TestConstant.y = self";
        expectEvalOK = true;
        expectedValue = "123";
        QTest::newRow("12") << evalSystem << script << expectEvalOK << expectedValue;

        // Test numhop assign with incomplete name, parameter name missing
        script = "self. = 123";
        expectEvalOK = false;
        expectedValue = "-";
        QTest::newRow("13") << evalSystem << script << expectEvalOK << expectedValue;
    }

    void System_Add_And_Remove_Component()
    {
        Component *pComp = mHopsanCore.createComponent("SignalSink");
        pComp->setName("AddComponentTest");
        mpSystemFromFile->addComponent(pComp);
        QVERIFY2(mpSystemFromFile->haveSubComponent("AddComponentTest"), "Failed to add component to system.");
        mpSystemFromFile->removeSubComponent("AddComponentTest");
        QVERIFY2(!mpSystemFromFile->haveSubComponent("AddComponentTest"), "Failed to remove component from system.");
        mHopsanCore.removeComponent(pComp);
    }

    void System_Rename_Component()
    {
        QFETCH(HString, compname);
        QFETCH(HString, new_compname);

        Component *pComp = mpSystemFromFile->getSubComponent(compname);
        mpSystemFromFile->renameSubComponent(compname, new_compname);
        QVERIFY2(pComp->getName().compare(new_compname), "Failed to rename sub component.");
        mpSystemFromFile->renameSubComponent(new_compname, compname);
        QVERIFY2(pComp->getName().compare(compname), "Failed to rename sub component back.");
    }

    void System_Rename_Component_data()
    {
        QTest::addColumn<HString>("compname");
        QTest::addColumn<HString>("new_compname");
        QTest::newRow("0") << HString("TestStep") << HString("NewName");
    }

    void System_Disconnect_Connect()
    {
        QFETCH(QString, fullPortName1);
        QFETCH(QString, fullPortName2);

        Port *pPort1 = nullptr, *pPort2 = nullptr;
        getPort(qPrintable(fullPortName1), &pPort1);
        getPort(qPrintable(fullPortName2), &pPort2);

        HString comp1 = qPrintable(fullPortName1.split(".")[0]);
        HString port1 = qPrintable(fullPortName1.split(".")[1]);
        HString comp2 = qPrintable(fullPortName2.split(".")[0]);
        HString port2 = qPrintable(fullPortName2.split(".")[1]);

        mpSystemFromFile->disconnect(comp1,port1,comp2,port2);
        QVERIFY2(!pPort1->isConnectedTo(pPort2), "Failed to disconnect ports.");
        mpSystemFromFile->connect(comp1,port1,comp2,port2);
        QVERIFY2(pPort1->isConnectedTo(pPort2), "Failed to disconnect ports.");
    }

    void System_Disconnect_Connect_data()
    {
        QTest::addColumn<QString>("fullPortName1");
        QTest::addColumn<QString>("fullPortName2");
        QTest::newRow("0") << "TestStep.out" << "TestGain.in";
    }

    void System_Get_Set_Timestep()
    {
        QFETCH(double, timestep);

        mpSystemFromFile->setDesiredTimestep(timestep);
        QCOMPARE(mpSystemFromFile->getDesiredTimeStep(), timestep);
    }

    void System_Get_Set_Timestep_data()
    {
        QTest::addColumn<double>("timestep");
        QTest::newRow("0") << 0.005;
    }

    void System_Inherit_Timestep()
    {
        QFETCH(bool, inheritTimestep);

        mpSystemFromFile->setInheritTimestep(inheritTimestep);
        QVERIFY2(mpSystemFromFile->doesInheritTimestep() == inheritTimestep, "Failed to set inherit time step.");
    }

    void System_Inherit_Timestep_data()
    {
        QTest::addColumn<bool>("inheritTimestep");
        QTest::newRow("true") << true;
        QTest::newRow("false") << false;
    }

    void System_Num_Log_Samples()
    {
        //! @todo Also test his by running a simulation
        QFETCH(size_t, numLogsamples);

        mpSystemFromFile->setNumLogSamples(numLogsamples);
        QVERIFY2(mpSystemFromFile->getNumLogSamples() == numLogsamples, "Failed to get or set number of log samples.");
    }

    void System_Num_Log_Samples_data()
    {
        QTest::addColumn<size_t>("numLogsamples");
        QTest::newRow("0") << static_cast<size_t>(1337);
    }

    void System_Initialize()
    {
        mpSystemFromFile->setDesiredTimestep(0.001);
        mpSystemFromFile->setNumLogSamples(1024);
        QVERIFY2(mpSystemFromFile->initialize(0,10), "Failed to initialize system!");
    }

    void System_Simulate()
    {
        QVERIFY(mpSystemFromFile->initialize(0, 10.0));
        mpSystemFromFile->simulate(10.0);
        QVERIFY2(mpSystemFromFile->getLogTimeVector()->size() == 2048, "Failed to simulate system!");
        QVERIFY2(mpSystemFromFile->getNumActuallyLoggedSamples() == 2048, "Failed to simulate system!");
    }

    void System_Simulate_Multicore()
    {
        QVERIFY(mpSystemFromFile->initialize(0, 10.0));
        mpSystemFromFile->simulateMultiThreaded(0, 10.0);
        QVERIFY2(mpSystemFromFile->getLogTimeVector()->size() == 2048, "Failed to simulate system!");
        QVERIFY2(mpSystemFromFile->getNumActuallyLoggedSamples() == 2048, "Failed to simulate system!");

        std::vector<double> multiResults1 = mpSystemFromFile->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(0);
        std::vector<double> multiResults2 = mpSystemFromFile->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(511);
        std::vector<double> multiResults3 = mpSystemFromFile->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(1023);
        mpSystemFromFile->simulate(10.0);
        std::vector<double> singleResults1 = mpSystemFromFile->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(0);
        std::vector<double> singleResults2 = mpSystemFromFile->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(511);
        std::vector<double> singleResults3 = mpSystemFromFile->getSubComponent("TestStep")->getPort("out")->getLogDataVectorPtr()->at(1023);
        QVERIFY2(multiResults1 == singleResults1, "Single-threaded and multi-threaded simulation gave different results!");
        QVERIFY2(multiResults2 == singleResults2, "Single-threaded and multi-threaded simulation gave different results!");
        QVERIFY2(multiResults3 == singleResults3, "Single-threaded and multi-threaded simulation gave different results!");
    }

    void Component_Set_Parameter()
    {
        QFETCH(QString, compName);
        QFETCH(QString, parName);
        QFETCH(QString, parVal);
        QFETCH(bool, expectSuccess);
        QFETCH(QString, expectedEvaluatedParameterValue);

        Component* pComp = nullptr;
        getComponent(qPrintable(compName), &pComp);

        const HString hParName = qPrintable(parName);
        const HString hParVal = qPrintable(parVal);

        HString previousValue, actualValue;
        pComp->getParameterValue(hParName, previousValue);

        pComp->setParameterValue(hParName, hParVal);

        pComp->getParameterValue(hParName, actualValue);
        bool hasChangedParameterValue = actualValue.compare(hParVal);
        QVERIFY(hasChangedParameterValue == expectSuccess);
        if (!hasChangedParameterValue) {
            QVERIFY(actualValue == previousValue);
        }
        else {
            HString evaluatedValue;
            pComp->evaluateParameter(hParName, evaluatedValue, "double");
            bool convertOK;
            QCOMPARE(evaluatedValue.toDouble(&convertOK), expectedEvaluatedParameterValue.toDouble());
            QVERIFY(convertOK);
        }
    }

    void Component_Set_Parameter_data()
    {
        QTest::addColumn<QString>("compName");
        QTest::addColumn<QString>("parName");
        QTest::addColumn<QString>("parVal");
        QTest::addColumn<bool>("expectSuccess");
        QTest::addColumn<QString>("expectedEvaluatedParameterValue");

        // Test set ordinary value, and system parameter names
        QTest::newRow("0") << "TestStep" << "y_0#Value" << "4" << true << "4";
        QTest::newRow("1") << "TestStep" << "y_0#Value" << "apa" << true << "7";
        QTest::newRow("2") << "TestStep" << "y_0#Value" << "sysparThatDoesNotExist" << false << "-";
        // Test setting signal value with and without explicitly using .Value
        QTest::newRow("3") << "TestGain" << "k#Value" << "self.in.Value" << true << "0";
        QTest::newRow("4") << "TestGain" << "k#Value" << "self.in" << true << "0";
        // Expect failure of self assignment when setting signal value with and without explicitly using .Value
        QTest::newRow("5") << "TestGain" << "k#Value" << "self.k.Value" << false << "-";
        QTest::newRow("6") << "TestGain" << "k#Value" << "self.k" << false << "-";
        // Test settings start values in Hydraulic TLM port
        QTest::newRow("7") << "TestVolume" << "P1#Pressure" << "42" << true << "42";
        QTest::newRow("8") << "TestVolume" << "P1#Pressure" << "apa" << true << "7";
        QTest::newRow("9") << "TestVolume" << "P1#Pressure" << "sysparThatDoesNotExist" << false << "-";
        QTest::newRow("10") << "TestVolume" << "P1#Pressure" << "self.P2.Flow" << true << "0.001";
        QTest::newRow("11") << "TestVolume" << "P1#Pressure" << "self.P1.Pressure" << false << "-";
        QTest::newRow("12") << "TestVolume" << "P1#Pressure" << "self.P2" << false << "-";
        QTest::newRow("13") << "TestVolume" << "P1#Pressure" << "self.alpha" << true << "0.1";
        QTest::newRow("14") << "TestVolume" << "P1#Pressure" << "self.alpha.Value" << true << "0.1";
        QTest::newRow("15") << "TestVolume" << "P1#Pressure" << "self.alpha.DataTheDoesNotExist" << false << "-";
        QTest::newRow("16") << "TestVolume" << "P1#Pressure" << "self.P2.DataTheDoesNotExist" << false << "-";
        // Test that looping parameter is not working
        QTest::newRow("16") << "TestVolume" << "P2#Pressure" << "self.P1.Pressure" << false << "-";
        // Ensure that we can set a "multi-line" script in an expression
        QTest::newRow("17") << "TestVolume" << "P2#Pressure" << "a=5; b=a*2; b" << true << "10";
        // Ensure that we can assign an other parameter in an expression, this is really odd, but I see no point in preventing it
        QTest::newRow("18") << "TestVolume" << "P2#Pressure" << "self.V = 5" << true << "5";
        QTest::newRow("19") << "TestVolume" << "P2#Pressure" << "self.V = 5; 6" << true << "6";
        QTest::newRow("20") << "TestVolume" << "P2#Pressure" << "6; self.V = 4" << true << "4";
        QTest::newRow("21") << "TestVolume" << "P2#Pressure" << "self.alpha.Value = 4" << true << "4";
        QTest::newRow("22") << "TestVolume" << "P2#Pressure" << "self.alpha = 4" << true << "4";
    }

    void Component_Get_Name()
    {
        QFETCH(QString, compName);
        HString hCompName = qPrintable(compName);

        Component* pComp = mpSystemFromFile->getSubComponent(hCompName);
        QVERIFY(pComp);
        QVERIFY2(pComp->getName().compare(hCompName), "Failed to get component name.");
    }

    void Component_Get_Name_data()
    {
        QTest::addColumn<QString>("compName");
        QTest::newRow("0") << "TestStep";
        QTest::newRow("1") << "TestGain";
    }

    void Component_Get_Typename()
    {
        QFETCH(QString, compName);
        QFETCH(QString, expectedTypeName);
        Component* pComp = mpSystemFromFile->getSubComponent(qPrintable(compName));
        QVERIFY(pComp);
        QVERIFY2(pComp->getTypeName().compare(qPrintable(expectedTypeName)), "Failed to get component type name.");
    }

    void Component_Get_Typename_data()
    {
        QTest::addColumn<QString>("compName");
        QTest::addColumn<QString>("expectedTypeName");
        QTest::newRow("0") << "TestStep" << "SignalStep";
        QTest::newRow("1") << "TestGain" << "SignalGain";
    }

    void Component_Has_Parameter()
    {
        QFETCH(QString, compName);
        QFETCH(QString, parName);
        QFETCH(bool, expectHasParameter);

        Component* pComp = mpSystemFromFile->getSubComponent(qPrintable(compName));
        QVERIFY(pComp);
        QVERIFY(pComp->hasParameter(qPrintable(parName)) == expectHasParameter);
    }

    void Component_Has_Parameter_data()
    {
        QTest::addColumn<QString>("compName");
        QTest::addColumn<QString>("parName");
        QTest::addColumn<bool>("expectHasParameter");
        QTest::newRow("0") << "TestStep" << "y_0#Value" << true;
        QTest::newRow("1") << "TestStep" << "y_A#Value" << true;
        QTest::newRow("2") << "TestStep" << "t_step#Value"  << true;
        QTest::newRow("3") << "TestStep" << "t_step#Value2"  << false;
    }

    void Component_GetAndEval_Parameter()
    {
        QFETCH(HString, compName);
        QFETCH(HString, paramName);
        QFETCH(HString, paramType);
        QFETCH(HString, expectedValue);
        QFETCH(HString, expectedEvaluatedValue);

        Component* pComponent = nullptr;
        getComponent(compName, &pComponent);

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

        // Test that int,bool,string,textblock system parameter can be recursively evaluated
        QTest::newRow("15") << HString("Subsystem$Subsubsystem$1DLookupTable_2") << HString("text") << HString("textblock") << HString("main_lookup_textblock") << HString("1, 1\n2, 2");
        QTest::newRow("16") << HString("Subsystem$Subsubsystem$1DLookupTable_2") << HString("inid") << HString("integer") << HString("main_lookup_inid") << HString("1");
        QTest::newRow("17") << HString("Subsystem$Subsubsystem$1DLookupTable_2") << HString("comment") << HString("string") << HString("main_lookup_comment") << HString("#");
        QTest::newRow("18") << HString("Subsystem$Subsubsystem$1DLookupTable_2") << HString("reload") << HString("bool") << HString("main_lookup_reload") << HString("true");

        // Test that int and string parameter can be self. evaluated
        //! @todo need test for textblock and bool as well, but hav no componetns with two of those (may need to write a test component)
        QTest::newRow("19") << HString("Subsystem$Subsubsystem$1DLookupTable_1") << HString("outid") << HString("integer") << HString("self.numlineskip") << HString("1");
        QTest::newRow("20") << HString("Subsystem$Subsubsystem$1DLookupTable_1") << HString("comment") << HString("string") << HString("self.filename") << HString("K");
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
        QFETCH(QString, compName);
        QFETCH(QString, portName);

        Component* pComp = mpSystemFromFile->getSubComponent(qPrintable(compName));
        QVERIFY(pComp);
        Port* pPort = pComp->getPort(qPrintable(portName));
        QVERIFY(pPort);

        QVERIFY2(pPort->getName().compare(qPrintable(portName)), "Failed to get port name.");
    }

    void Port_Get_Name_data()
    {
        QTest::addColumn<QString>("compName");
        QTest::addColumn<QString>("portName");
        QTest::newRow("0") << "TestStep" << "out";
        QTest::newRow("1") << "TestGain" << "in";
    }

    void Port_Get_Connected_Ports()
    {
        QFETCH(QString, fullPortName1);
        QFETCH(QString, fullPortName2);

        Port *pPort1 = nullptr, *pPort2 = nullptr;
        getPort(qPrintable(fullPortName1), &pPort1);
        getPort(qPrintable(fullPortName2), &pPort2);

        QVERIFY2(pPort1->getConnectedPorts().size() == 1, "Failed, getConnectedPorts return wrong number of ports.");
        QVERIFY2(pPort1->getConnectedPorts()[0] == pPort2, "Failed to get connected ports.");
    }

    void Port_Get_Connected_Ports_data()
    {
        QTest::addColumn<QString>("fullPortName1");
        QTest::addColumn<QString>("fullPortName2");
        QTest::newRow("0") << "TestStep.out" << "TestGain.in";
        QTest::newRow("1") << "TestGain.in"  << "TestStep.out";
    }

    void Port_Get_Component_Name()
    {
        QFETCH(QString, compName);
        QFETCH(QString, portName);

        Port* pPort = nullptr;
        getPort(qPrintable(compName+"."+portName), &pPort);

        QVERIFY2(pPort->getComponentName().compare(qPrintable(compName)), "Failed to get component name in port.");
    }

    void Port_Get_Component_Name_data()
    {
        QTest::addColumn<QString>("compName");
        QTest::addColumn<QString>("portName");
        QTest::newRow("0") << "TestStep" << "out";
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
        Component *pComp = nullptr;

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
        QFETCH(QString, fullPortName);
        QFETCH(QString, fullCompName);

        Port *pPort = nullptr;
        getPort(qPrintable(fullPortName), &pPort);
        Component* pWriteComp = pPort->getNodePtr()->getWritePortComponentPtr();

        Component *pComp = nullptr;
        getComponent(qPrintable(fullCompName), &pComp);

        QVERIFY2(pWriteComp == pComp, "Failed to get write port component from node.");
    }

    void Node_Get_Write_Port_Component_Pointer_data()
    {
        QTest::addColumn<QString>("fullPortName");
        QTest::addColumn<QString>("fullCompName");
        QTest::newRow("0") << "TestGain.in" << "TestStep";
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
