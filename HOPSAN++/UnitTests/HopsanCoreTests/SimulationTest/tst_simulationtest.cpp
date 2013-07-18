#include <QtTest>
#include <QtTest>
#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include <assert.h>

#ifndef BUILTINDEFAULTCOMPONENTLIB
#ifdef WIN32
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/defaultComponentLibrary.dll"
#else
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/libdefaultComponentLibrary.so"
#endif
#endif

using namespace hopsan;

Q_DECLARE_METATYPE(bool);
Q_DECLARE_METATYPE(ComponentSystem*);

class SimulationTests : public QObject
{
    Q_OBJECT

public:
    SimulationTests()
    {
        HopsanEssentials pHopsanCore;
        pHopsanCore.loadExternalComponentLib(DEFAULTCOMPONENTLIB);
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
                "      <component typename=\"SignalSink\" name=\"TestScope\">"
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
                "      <connect endport=\"in\" endcomponent=\"TestScope\" startport=\"out\" startcomponent=\"TestStep\"/>"
                "    </connections>"
                "  </system>"
                "</hopsanmodelfile>"
                "";

        mpSystemFromText = pHopsanCore.loadHMFModel(xmlStr);
        double startT, stopT;
        mpSystemFromFile = pHopsanCore.loadHMFModel("../../../Models/unittestmodel.hmf",startT,stopT);
    }

private:
    ComponentSystem *mpSystemFromText;
    ComponentSystem *mpSystemFromFile;

private Q_SLOTS:
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
        QVERIFY2(system->haveSubComponent("TestStep"), "Failed to load sub component!");
        QVERIFY2(system->haveSubComponent("TestScope"), "Failed to load sub component!");
    }

    void Load_Component_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void Load_Connect()
    {
        QFETCH(ComponentSystem*, system);
        Component *pStep = system->getSubComponent("TestStep");
        Component *pScope = system->getSubComponent("TestScope");
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
        QFETCH(ComponentSystem*, system);
        Component *pStep = system->getSubComponent("TestStep");
        HString parVal;
        pStep->getParameterValue("y_0#Value", parVal);
        QVERIFY2(parVal.compare("-5"), "Failed to load sub component parameter value!");
        pStep->getParameterValue("y_A#Value", parVal);
        QVERIFY2(parVal.compare("5"),"Failed to load sub component parameter value!");
        pStep->getParameterValue("t_step#Value", parVal);
        QVERIFY2(parVal.compare("apa"), "Failed to load sub component parameter value (system parameter)!");
    }

    void Load_Parameter_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void Set_System_Parameter()
    {
        QFETCH(ComponentSystem*, system);

        system->setParameterValue("apa", "12");
        HString value;
        system->getParameterValue("apa", value);
        QVERIFY2(value.compare("12"), "Failed to set system parameter.");
    }

    void Set_System_Parameter_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

    void Set_Parameter()
    {
        QFETCH(ComponentSystem*, system);

        system->getSubComponent("TestStep")->setParameterValue("y_0#Value", "4");
        HString value;
        system->getSubComponent("TestStep")->getParameterValue("y_0#Value", value);
        QVERIFY2(value.compare("4"), "Failed to set parameter value.");

        system->getSubComponent("TestStep")->setParameterValue("y_0#Value", "apa");
        system->getSubComponent("TestStep")->getParameterValue("y_0#Value", value);
        QVERIFY2(value.compare("apa"), "Failed to map parameter to system parameter.");
    }

    void Set_Parameter_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::newRow("0") << mpSystemFromText;
        QTest::newRow("1") << mpSystemFromFile;
    }

//    void Initialize()
//    {
//        QFETCH(ComponentSystem*, system);
//        QString failmsg("Failed to initialize system!");
//        QVERIFY2(system->initialize(0,1), failmsg.toStdString().c_str());
//    }

//    void Initialize_data()
//    {
//        QTest::addColumn<ComponentSystem*>("system");
//        QTest::newRow("0") << mpSystemFromText;
//        QTest::newRow("1") << mpSystemFromFile;
//    }
};

QTEST_APPLESS_MAIN(SimulationTests)

#include "tst_simulationtest.moc"
