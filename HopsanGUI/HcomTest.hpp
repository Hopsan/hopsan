#include "HcomHandler.h"
#include "Widgets/HcomWidget.h"
#include "ModelHandler.h"

#include "global.h"

#include <QtTest>

class HComTest: public QObject
{
    Q_OBJECT
private:
    HcomHandler* mpHcom;

    void createTestModel() {
        mpHcom->executeCommand("crmo");
        mpHcom->executeCommand("adco SignalStep step -a 100 100 0");
        mpHcom->executeCommand("adco SignalStep step2 -a 200 200 0");
        mpHcom->executeCommand("adpa syspar 42");
        mpHcom->executeCommand("chpa step.y_0.y 43");
        mpHcom->executeCommand("chpa step2.y_0.y 43");
        mpHcom->executeCommand("chpa step2.y_A.y 0");
        mpHcom->executeCommand("chpa step.t_step.y syspar");
        mpHcom->executeCommand("chpa step.y_A.y \"self.y_0.Value\"");
        //mpHcom->executeCommand("adco Subsystem subsystem -a 150 150 0"); //! @todo Cant enter system with commands so cant do more with it now
        mpHcom->executeCommand("sim");
    }

private slots:
    void initTestCase()
    {
        mpHcom = new HcomHandler(new TerminalConsole(nullptr));
        connect(gpModelHandler, SIGNAL(modelChanged(ModelWidget*)), mpHcom, SLOT(setModelPtr(ModelWidget*)));
    }

    void testScalarMath() {  
        QFETCH(QString, expression);
        QFETCH(bool, expectEvalOK);
        QFETCH(double, expectedValue);

        bool actualEvalOK;
        double evaluatedValue = mpHcom->evaluateScalarExpression(expression, actualEvalOK);
        QVERIFY(expectEvalOK == actualEvalOK);
        QCOMPARE(evaluatedValue, expectedValue);
    }

    void testScalarMath_data() {
        QTest::addColumn<QString>("expression");
        QTest::addColumn<bool>("expectEvalOK");
        QTest::addColumn<double>("expectedValue");

        QTest::newRow("1") << "1+1" << true << 2.0;
        QTest::newRow("2") << "1+(1+1)" << true << 3.0;
        QTest::newRow("3") << "1+(1-1)" << true << 1.0;
        QTest::newRow("4") << "1-(1-2)" << true << 2.0;
        QTest::newRow("5") << "-1+1" << true << 0.0;
        QTest::newRow("6") << "1*2*3" << true << 6.0;
        QTest::newRow("7") << "2*2/3" << true << 4.0/3.0;
        QTest::newRow("8") << "2+2/3" << true << 2+(2.0/3.0);
    }

    void testParameterCommands() {
        createTestModel();

        mpHcom->executeCommand("syspar");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 42.0);

        mpHcom->executeCommand("self.syspar");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 42.0);

        mpHcom->executeCommand("self.syspar+1");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43.0);

        mpHcom->executeCommand("self.syspar = self.syspar+1");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43.0);
        mpHcom->executeCommand("syspar");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43.0);

        mpHcom->executeCommand("chpa self.syspar 42");
        mpHcom->executeCommand("step.t_step.y");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 42.0);

        mpHcom->executeCommand("step.y_0.y");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43.0);
        mpHcom->executeCommand("step.y_0"); //<- Evaluate using short name without explicit .y or .Value
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43.0);
        mpHcom->executeCommand("step.y_A.y"); // t_A should point to self.y_0.Value
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43.0);
        mpHcom->executeCommand("chpa step.y_A.y \"self.y_0\""); // Set without explicit self.y_0.Value
        mpHcom->executeCommand("chpa step.y_0.y 44"); // Set with explicit .Value
        //mpHcom->executeCommand("chpa step.y_0 44"); // Set without explicit .Value <- This will maybe be added in the future
        mpHcom->executeCommand("step.y_A.y"); // t_A should point to self.y_0
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 44.0);
    }

    void testControlFlowIf() {
        QString script = R"(
                if (a > 4)
                  if (b > 2)
                    c = 1
                  elseif (b < 0)
                    c = 2
                  endif
                else
                  c = 3
                endif
                )";

        mpHcom->executeCommand("a = 5");
        mpHcom->executeCommand("b = 3");
        bool abort=false;
        QStringList lines = script.split("\n");
        mpHcom->runScriptCommands(lines, &abort);
        mpHcom->executeCommand("c");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 1.0);

        mpHcom->executeCommand("b = -1");
        abort=false;
        lines = script.split("\n");
        mpHcom->runScriptCommands(lines, &abort);
        mpHcom->executeCommand("c");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 2.0);

        mpHcom->executeCommand("a = 3");
        abort=false;
        lines = script.split("\n");
        mpHcom->runScriptCommands(lines, &abort);
        mpHcom->executeCommand("c");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 3.0);
    }

    void testControlFlowForeach() {
        //! @todo  Make foreach work with paramters and script variables, and a list of values
        QString script = R"(
                sum = 0
                foreach v step*.out.y
                  sum = sum + aver($v)
                endforeach
                )";
        bool abort=false;
        QStringList lines = script.split("\n");
        mpHcom->runScriptCommands(lines, &abort);
        mpHcom->executeCommand("sum");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 86.0);
    }

    void testControlFlowWhile() {
        QString script = R"(
                val = 2
                sum = 0
                while( sum < step2.y_0.y) # Short version step2.y_0 does not work here, need to fix
                  sum = sum + val
                repeat
                )";
        bool abort=false;
        QStringList lines = script.split("\n");
        mpHcom->runScriptCommands(lines, &abort);
        mpHcom->executeCommand("sum");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 44.0);
    }


};
