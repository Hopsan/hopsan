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

#include "ModelUtilities.h"

#include "HopsanCore.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/HmfLoader.h"

#include <assert.h>
#include <algorithm>
#include <vector>

#ifndef DEFAULT_LIBRARY_ROOT
#define DEFAULT_LIBRARY_ROOT "../componentLibraries/defaultLibrary"
#endif

#ifndef TEST_DATA_ROOT
#define TEST_DATA_ROOT "../UnitTests/HopsanCoreTests/SimulationTest/"
#endif

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE SHAREDLIB_PREFIX "defaultcomponentlibrary" HOPSAN_DEBUG_POSTFIX "." SHAREDLIB_SUFFIX
const std::string defaultLibraryFilePath = DEFAULT_LIBRARY_ROOT "/" DEFAULTLIBFILE;
#else
const std::string defaultLibraryFilePath = "";
#endif

using namespace hopsan;

Q_DECLARE_METATYPE(HString);
Q_DECLARE_METATYPE(std::string);
Q_DECLARE_METATYPE(std::vector<std::string>);
Q_DECLARE_METATYPE(std::vector<HString>);

class HopsanCLITest : public QObject
{
    Q_OBJECT

private:
    HopsanEssentials mHopsanCore;
    ComponentSystem *mpSystemFromFile = nullptr;

private slots:
    void init() {
        bool did_load = mHopsanCore.loadExternalComponentLib(defaultLibraryFilePath.c_str());
        QVERIFY2(did_load, qPrintable(QString("Could not load default component library: ")+QString::fromStdString(defaultLibraryFilePath)));

        double startT, stopT;
        mpSystemFromFile = mHopsanCore.loadHMFModelFile(TEST_DATA_ROOT "unittestmodel.hmf",startT,stopT);
        QVERIFY2(mpSystemFromFile, "Could not load system from " TEST_DATA_ROOT "unittestmodel.hmf");

        SimulationHandler simuhandler;
        stopT = startT+0.1;
        bool isOK = simuhandler.initializeSystem(startT, stopT, mpSystemFromFile);
        QVERIFY2(isOK, "Initialization of model failed");
        isOK = simuhandler.simulateSystem(startT, startT, 1, mpSystemFromFile);
        QVERIFY2(isOK, "Simulation of model failed");
        simuhandler.finalizeSystem(mpSystemFromFile);
    }

    void cleanup() {
        mHopsanCore.removeComponent(mpSystemFromFile);
    }

    void testResultExport() {

        QFETCH(std::vector<std::string>, includeFilter);
        QFETCH(size_t, expectedNumberOfSavedVariables);
        QFETCH(std::vector<HString>, expectedVariables);

        std::vector<HString> savedVariables;
        auto saveTime = [&savedVariables] (ComponentSystem* pSystem) {
            savedVariables.push_back(generateFullSubSystemHierarchyName(pSystem, ".")+"Time");
        };

        auto saveVariable = [&savedVariables] (const ComponentSystem* pSystem, const Component* pComponent, const Port* pPort, size_t variableIndex) {
            const auto &varName = pPort->getNodeDataDescription(variableIndex)->name;
            savedVariables.push_back(generateFullSubSystemHierarchyName(pSystem,".")+pComponent->getName()+"."+pPort->getName()+"."+varName);
        };

        saveResultsTo(mpSystemFromFile, includeFilter, saveTime, saveVariable);

        QCOMPARE(savedVariables.size(), expectedNumberOfSavedVariables);

        for(const auto& eachExpected : expectedVariables) {
            QVERIFY2(contains(savedVariables, eachExpected), ("Saved variables did not contain exepected: "+eachExpected).c_str());
        }
    }

    void testResultExport_data() {

        QTest::addColumn<std::vector<std::string>>("includeFilter");
        QTest::addColumn<size_t>("expectedNumberOfSavedVariables");
        QTest::addColumn<std::vector<HString>>("expectedVariables");

        std::vector<std::string> includeFilter;
        std::vector<HString> expectedVariables;
        size_t expectedNumVariables;


        includeFilter = {"TestGain#out"};
        expectedVariables = {"TestGain.out.Value", "Time"};
        expectedNumVariables = expectedVariables.size();
        QTest::newRow("0") << includeFilter  << expectedNumVariables << expectedVariables;

        includeFilter = {"TestGain#out#Value"};
        expectedVariables = {"TestGain.out.Value", "Time"};
        expectedNumVariables = expectedVariables.size();
        QTest::newRow("0.1") << includeFilter  << expectedNumVariables << expectedVariables;

        includeFilter = {"TestGain#out", "TestOrifice1#P1"};
        expectedVariables = {"TestGain.out.Value", "TestOrifice1.P1.Pressure", "TestOrifice1.P1.Flow", "TestOrifice1.P1.CharImpedance",
                             "TestOrifice1.P1.WaveVariable", "TestOrifice1.P1.HeatFlow", "TestOrifice1.P1.Temperature", "Time"};
        expectedNumVariables = expectedVariables.size();
        QTest::newRow("1") << includeFilter  << expectedNumVariables << expectedVariables;

        includeFilter = {"TestGain#out", "TestOrifice1#P1#Pressure"};
        expectedVariables = {"TestGain.out.Value", "TestOrifice1.P1.Pressure", "Time"};
        expectedNumVariables = expectedVariables.size();
        QTest::newRow("3") << includeFilter  << expectedNumVariables << expectedVariables;

        includeFilter = {"Subsystem$Gain#out", "TestGain#out", "TestOrifice1#P1#Pressure"};
        expectedVariables = {"Subsystem.Gain.out.Value", "TestGain.out.Value", "TestOrifice1.P1.Pressure", "Time", "Subsystem.Time"};
        expectedNumVariables = expectedVariables.size();
        QTest::newRow("4") << includeFilter  << expectedNumVariables << expectedVariables;

        includeFilter = {"Subsystem$Gain#out"};
        expectedVariables = {"Subsystem.Gain.out.Value", "Subsystem.Time"};
        expectedNumVariables = expectedVariables.size();
        QTest::newRow("5") << includeFilter  << expectedNumVariables << expectedVariables;

        // A system port is logged with the parent systems time vector, so the internal subsystem time vector should not be exported
        includeFilter = {"Subsystem#SubPortOut"};
        expectedVariables = {"Subsystem.SubPortOut.Value", "Time"};
        expectedNumVariables = expectedVariables.size();
        QTest::newRow("6") << includeFilter  << expectedNumVariables << expectedVariables;
    }


};

QTEST_APPLESS_MAIN(HopsanCLITest)

#include "tst_hopsancli.moc"
