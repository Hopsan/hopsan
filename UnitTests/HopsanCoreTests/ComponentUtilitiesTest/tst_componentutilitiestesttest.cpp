#include <QString>
#include <QtTest>

#include "ComponentUtilities.h"

using namespace hopsan;

class ComponentUtilitiesTestTest : public QObject
{
    Q_OBJECT

public:
    ComponentUtilitiesTestTest()
    {

    }

private Q_SLOTS:
    void Limit_Value()
    {
        QFETCH(double, x);
        QFETCH(double, min);
        QFETCH(double, max);
        QFETCH(double, x_limited);

        limitValue(x, min, max);
        QVERIFY2(x == x_limited, "limitValue() returned wrong value!");
    }

    void Limit_Value_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("max");
        QTest::addColumn<double>("x_limited");

        QTest::newRow("0") << 5.0 << 1.7 << 4.0 << 4.0;
        QTest::newRow("1") << 3.0 << 1.7 << 4.0 << 3.0;
        QTest::newRow("2") << 1.0 << 1.7 << 4.0 << 1.7;
        QTest::newRow("3") << 5.0 << -1.7 << 4.0 << 4.0;
        QTest::newRow("4") << -1.0 << -1.7 << 4.0 << -1.0;
        QTest::newRow("5") << -10.0 << -1.7 << 4.0 << -1.7;
        QTest::newRow("6") << -5.0 << -10.0 << -6.0 << -6.0;
    }

    void Fuzzy_Equal()
    {
        QFETCH(double, x);
        QFETCH(double, y);
        QFETCH(bool, equal);

        QVERIFY2(fuzzyEqual(x,y) == equal, "fuzzyEqual() returned wrong value!");
    }

    void Fuzzy_Equal_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("y");
        QTest::addColumn<bool>("equal");

        QTest::newRow("0") << 12.345 << 12.345 << true;
        QTest::newRow("1") << 12.345 << 12.346 << false;
        QTest::newRow("2") << -12.345 << -12.345 << true;
        QTest::newRow("3") << -12.345 << -12.346 << false;
    }

    void Signed_Square_L()
    {
        QFETCH(double, x);
        QFETCH(double, x0);

        QVERIFY2(fabs(x)*pow(1.0/(pow(x0,4.0) + pow(fabs(x),2.0)),0.25)*sign(x) == signedSquareL(x,x0), "signedSquareL() returned wrong value!");
    }

    void Signed_Square_L_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("x0");

        QTest::newRow("0") << 0.0 << 1.0;
        for(int i=1; i<100; ++i)
        {
            QTest::newRow("i") << (double)rand() - (double)RAND_MAX*0.5 << (double)rand()- (double)RAND_MAX*0.5;
        }
    }
};


QTEST_APPLESS_MAIN(ComponentUtilitiesTestTest)

#include "tst_componentutilitiestesttest.moc"
