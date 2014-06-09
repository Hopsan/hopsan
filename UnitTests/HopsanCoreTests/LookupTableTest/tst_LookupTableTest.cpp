#include <QString>
#include <QVector>
#include <QtTest>

Q_DECLARE_METATYPE(QVector<double>);

class LookupTableTest : public QObject
{
    Q_OBJECT

public:
    LookupTableTest();

private Q_SLOTS:
    void lookup1D();
    void lookup1D_data();
};

LookupTableTest::LookupTableTest()
{
}

void LookupTableTest::lookup1D()
{
    QFETCH(QVector<double>, IndexData);
    QFETCH(QVector<double>, ValueData);
    QFETCH(double, in);
    QFETCH(double, out);

    //! @todo do stuff here

    QVERIFY2(true, "Failure");
}

void LookupTableTest::lookup1D_data()
{
    QDataStream input;
    QVector<double> index;
    QVector<double> value;

    QTest::addColumn< QVector<double> >("IndexData");
    QTest::addColumn< QVector<double> >("ValueData");
    QTest::addColumn< double >("in");
    QTest::addColumn< double >("out");

    input << 1 << 2 << 3 << 4 << 5;
    input >> index;
    input << 1 << 2 << 3 << 4 << 5;
    input >> value;

    QTest::newRow("test1") << index << value << 0.1 << 1;
    QTest::newRow("test1") << index << value << 2 << 2;
    QTest::newRow("test1") << index << value << 3 << 3;
    QTest::newRow("test1") << index << value << 23 << 5;


    input << 1 << 2 << 3 << 4 << 5;
    input >> index;
    input << 2 << 4 << 9 << 16 << 25;
    input >> value;

    QTest::newRow("test1") << index << value << 2 << 4;
    QTest::newRow("test1") << index << value << 3 << 9;
}

QTEST_APPLESS_MAIN(LookupTableTest)

#include "tst_LookupTableTest.moc"
