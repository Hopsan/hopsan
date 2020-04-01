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
        mpHcom->executeCommand("adpa syspar 42");
        mpHcom->executeCommand("chpa step.y_0.y 43");
        mpHcom->executeCommand("chpa step.t_step.y syspar");
        mpHcom->executeCommand("chpa step.y_A.y \"self.y_0.Value\"");
        mpHcom->executeCommand("adco Subsystem subsystem -a 150 150 0"); //! @todo Cant enter system with commands so cant do more with it now
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
        QCOMPARE(mpHcom->mAnsScalar, 42);

        mpHcom->executeCommand("self.syspar");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 42);

        mpHcom->executeCommand("self.syspar+1");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43);

        mpHcom->executeCommand("self.syspar = self.syspar+1");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43);
        mpHcom->executeCommand("syspar");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43);

        mpHcom->executeCommand("chpa self.syspar 42");
        mpHcom->executeCommand("step.t_step.y");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 42);

        mpHcom->executeCommand("step.y_0.y");
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43);
        mpHcom->executeCommand("step.y_0"); //<- Evaluate using short name without explicit .y or .Value
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43);
        mpHcom->executeCommand("step.y_A.y"); // t_A should point to self.y_0.Value
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 43);
        mpHcom->executeCommand("chpa step.y_A.y \"self.y_0\""); // Set without explicit self.y_0.Value
        mpHcom->executeCommand("chpa step.y_0.y 44"); // Set with explicit .Value
        //mpHcom->executeCommand("chpa step.y_0 44"); // Set without explicit .Value <- This will maybe be added in the future
        mpHcom->executeCommand("step.y_A.y"); // t_A should point to self.y_0
        QCOMPARE(mpHcom->mAnsType, HcomHandler::Scalar);
        QCOMPARE(mpHcom->mAnsScalar, 44);
    }


};
