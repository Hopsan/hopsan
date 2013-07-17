#include <QtTest>
#include <QtTest>
#include "HopsanEssentials.h"

#ifndef BUILTINDEFAULTCOMPONENTLIB
#ifdef WIN32
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/defaultComponentLibrary.dll"
#else
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/libdefaultComponentLibrary.so"
#endif
#endif

using namespace hopsan;

Q_DECLARE_METATYPE(bool);

class SimulationTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void Load()
    {
        QFETCH(bool, value);
        QString failmsg("Failure!");
        QVERIFY2(value, failmsg.toStdString().c_str());
    }
    void Load_data()
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

        ComponentSystem *pSystem = pHopsanCore.loadHMFModel(xmlStr);
        bool systemOk = pSystem;

        QTest::addColumn<bool>("value");
        QTest::newRow("0") << systemOk;
        QTest::newRow("1") << pSystem->hasParameter("apa");
        QTest::newRow("2") << pSystem->getParameter("apa")->getValue().compare("7");
        QTest::newRow("3") << pSystem->haveSubComponent("TestStep");
        QTest::newRow("4") << pSystem->haveSubComponent("TestScope");
        Component *pStep = pSystem->getSubComponent("TestStep");
        Component *pScope = pSystem->getSubComponent("TestScope");
        QTest::newRow("5") << pStep->getPort("out")->isConnectedTo(pScope->getPort("in"));
        HString parVal;
        pStep->getParameterValue("y_0#Value", parVal);
        QTest::newRow("6") << parVal.compare("-5");
        pStep->getParameterValue("y_A#Value", parVal);
        QTest::newRow("7") << parVal.compare("5");
        pStep->getParameterValue("t_step#Value", parVal);
        QTest::newRow("8") << parVal.compare("apa");

    }
};

QTEST_APPLESS_MAIN(SimulationTests)

#include "tst_simulationtest.moc"
