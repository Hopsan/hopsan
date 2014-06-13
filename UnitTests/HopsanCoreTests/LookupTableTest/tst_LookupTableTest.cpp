#include <QString>
#include <QVector>
#include <QPointF>
#include <QtTest>


#include "ComponentUtilities/LookupTable.h"

Q_DECLARE_METATYPE(QVector<double>);

inline double fc(const double a, const double b, const double eps=1e-9)
{
    return fabs(a-b) < eps;
}

void reverseVector(QVector<double> &rVec)
{
    QVector<double> tmp(rVec.size());
    for (int i=0; i<rVec.size(); ++i)
    {
        tmp[rVec.size()-i-1] = rVec[i];
    }
    rVec.swap(tmp);
}

//! @brief Shuffle the two vectors (each get the same row swap)
void randomizeVectors(QVector<double> &rIndex, QVector<double> &rValue)
{
    if (rIndex.size() == rValue.size())
    {
        QVector<double> tmpI(rIndex.size());
        QVector<double> tmpV(rValue.size());

        int ctr=0;
        while (!rIndex.isEmpty())
        {
            // Choose a random index
            int ri = rand() % rIndex.size();

            tmpI[ctr] = rIndex[ri];
            tmpV[ctr] = rValue[ri];

            rIndex.remove(ri);
            rValue.remove(ri);

            ++ctr;
        }

        rIndex.swap(tmpI);
        rValue.swap(tmpV);
    }
}

class LookupTableTest : public QObject
{
    Q_OBJECT

public:
    LookupTableTest();

private Q_SLOTS:
    void lookup1D();
    void lookup1D_data();
    void lookup2D();
    void lookup2D_data();
};

LookupTableTest::LookupTableTest()
{
}

void LookupTableTest::lookup1D()
{
    QFETCH(QVector<double>, indexData);
    QFETCH(QVector<double>, valueData);
    QFETCH(bool, shouldBeOK);
    QFETCH(double, in);
    QFETCH(double, out);

    LookupTableND<1> lookup1d;

    // Set the index and value vectors
    lookup1d.getIndexDataRef() = indexData.toStdVector();
    lookup1d.getValueDataRef() = valueData.toStdVector();

    // Check if data is OK, triggering sort if needed
    bool dataIsOk = lookup1d.isDataOK();
    if (!dataIsOk && lookup1d.isIndexIncreasingOrDecresing() != LookupTableND<1>::StrictlyIncreasing)
    {
        lookup1d.sortIncreasing();
    }
    dataIsOk = lookup1d.isDataOK();


    // Lookup values and check results
    if (shouldBeOK)
    {
        if (dataIsOk)
        {
            const double val = lookup1d.interpolate(in);
            QVERIFY2(fc(val, out), QString("Interpolate returned the wrong result: %1!=%2").arg(val).arg(out).toLatin1());
        }
        else
        {
            QVERIFY2(dataIsOk, "Failed: Data is NOT OK");
        }
    }
    else
    {
        QVERIFY2(!dataIsOk, "isValid() should have failed but did not!");
    }
}

void LookupTableTest::lookup2D()
{
    QFETCH(QVector<double>, rowIndexData);
    QFETCH(QVector<double>, colIndexData);
    QFETCH(QVector<double>, valueData);
    QFETCH(bool, shouldBeOK);
    QFETCH(QPointF, in);
    QFETCH(double, out);

    LookupTableND<2> lookup2d;

    // Set the index and value vectors
    lookup2d.getIndexDataRef(0) = rowIndexData.toStdVector();
    lookup2d.getIndexDataRef(1) = colIndexData.toStdVector();
    lookup2d.getValueDataRef() = valueData.toStdVector();

    // Check if data is OK, try sort if not
    bool dataIsOk = lookup2d.isDataOK();
    if (!dataIsOk)
    {
        lookup2d.sortIncreasing();
    }
    dataIsOk = lookup2d.isDataOK();

    // Lookup values and check results
    if (shouldBeOK)
    {
        if (dataIsOk)
        {
            const double val = lookup2d.interpolate(in.x(), in.y());
            QVERIFY2(fc(val, out), QString("Interpolate returned the wrong result: %1!=%2").arg(val).arg(out).toLatin1());
        }
        else
        {
            QVERIFY2(dataIsOk, "Failed: Data is NOT OK");
        }
    }
    else
    {
        QVERIFY2(!dataIsOk, "isValid() should have failed but did not!");
    }
}

void LookupTableTest::lookup1D_data()
{
    QTest::addColumn< QVector<double> >("indexData");
    QTest::addColumn< QVector<double> >("valueData");
    QTest::addColumn< bool >("shouldBeOK");
    QTest::addColumn< double >("in");
    QTest::addColumn< double >("out");

    QVector<double> indexVec, valueVec;
    indexVec << 1 << 2 << 3 << 4 << 5;
    valueVec << 1 << 2 << 3 << 4 << 5;

    QTest::newRow("test1") << indexVec << valueVec << true << 0.1 << 1.0;
    QTest::newRow("test1") << indexVec << valueVec << true << 2.0 << 2.0;
    QTest::newRow("test1") << indexVec << valueVec << true << 3.0 << 3.0;
    QTest::newRow("test1") << indexVec << valueVec << true << 23.0 << 5.0;

    indexVec.clear(); valueVec.clear();
    indexVec << 1 << 2 << 3 << 4 << 5;
    valueVec << 2 << 4 << 9 << 16 << 25;

    QTest::newRow("test2") << indexVec << valueVec << true << 2.5 << 6.5;
    QTest::newRow("test2") << indexVec << valueVec << true << 4.9 << 24.1;
    QTest::newRow("test2") << indexVec << valueVec << true << 5.1 << 25.0;

    indexVec.clear(); valueVec.clear();
    indexVec << -14 << 1 << 8 << 14 << 36 << 54;
    valueVec <<  -2 << 0 << 4 << 19 << 135 << 72;
    QTest::newRow("test3") << indexVec << valueVec << true <<  -1.0 << -0.266666666666667;
    QTest::newRow("test3") << indexVec << valueVec << true <<  6.0 << 2.85714285714286;
    QTest::newRow("test3") << indexVec << valueVec << true << 40.0 << 121.0;
    QTest::newRow("test3") << indexVec << valueVec << true << 53.99999999999999999999999 << 72.0;

    // Now lets try data vectors that are sorted but in reverse
    reverseVector(indexVec); reverseVector(valueVec);
    QTest::newRow("test4") << indexVec << valueVec << true <<  -1.0 << -0.266666666666667;
    QTest::newRow("test4") << indexVec << valueVec << true <<  6.0 << 2.85714285714286;
    QTest::newRow("test4") << indexVec << valueVec << true << 40.0 << 121.0;
    QTest::newRow("test4") << indexVec << valueVec << true << 53.99999999999999999999999 << 72.0;

    // Now lets try vectors that are randomly sorted but with unique indexes
    randomizeVectors(indexVec, valueVec);
    QTest::newRow("test5") << indexVec << valueVec << true <<  -1.0 << -0.266666666666667;
    QTest::newRow("test5") << indexVec << valueVec << true <<  6.0 << 2.85714285714286;
    QTest::newRow("test5") << indexVec << valueVec << true << 40.0 << 121.0;
    QTest::newRow("test5") << indexVec << valueVec << true << 53.99999999999999999999999 << 72.0;

    // Now lets try some that should fail isOK check
    indexVec.clear(); valueVec.clear();
    indexVec << 1 << 2 << 3;
    valueVec << -2 << 0 << 4 << 19;
    QTest::newRow("test6") << indexVec << valueVec << false <<  -1.0 << -2.0;

    indexVec.clear(); valueVec.clear();
    indexVec << 1;
    valueVec << -2;
    QTest::newRow("test7") << indexVec << valueVec << false <<  -1.0 << -2.0;

    indexVec.clear(); valueVec.clear();
    indexVec << 1 << 1 << 4 << -1;
    valueVec << -2 << 4 << 5 << 6;
    QTest::newRow("test8") << indexVec << valueVec << false <<  -1.0 << -2.0;

}

void LookupTableTest::lookup2D_data()
{
    QTest::addColumn< QVector<double> >("rowIndexData");
    QTest::addColumn< QVector<double> >("colIndexData");
    QTest::addColumn< QVector<double> >("valueData");
    QTest::addColumn< bool >("shouldBeOK");
    QTest::addColumn< QPointF >("in");
    QTest::addColumn< double >("out");

    QVector<double> rowIndexVec, colIndexVec, valueVec;
    rowIndexVec << 1 << 2 << 3 << 4 << 5;
    colIndexVec << 1 << 2 << 3 << 4 << 5;
    double ctr=0;
    for (int ri=0; ri<rowIndexVec.size(); ++ri)
    {
        for (int ci=0; ci<colIndexVec.size(); ++ci)
        {
            valueVec << ctr;
            ++ctr;
        }
    }

    // Lookup exact values
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << QPointF(0,0) << 0.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << QPointF(1,1) << 0.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << QPointF(2,2) << 6.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << QPointF(3,5) << 14.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << QPointF(5,5) << 24.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << QPointF(6,8) << 24.0;

    // Lookup interpolated values
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << QPointF(2.5,2.5) << 9.0;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << QPointF(2.5,3.8) << 10.3;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << QPointF(4.67789,1.8345) << 19.22395;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << QPointF(4.9,5.0) << 23.5;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << QPointF(4.9,6.0) << 23.5;



}

QTEST_APPLESS_MAIN(LookupTableTest)

#include "tst_LookupTableTest.moc"
