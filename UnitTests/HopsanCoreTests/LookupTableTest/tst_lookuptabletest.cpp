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

#include <QString>
#include <QVector>
#include <QPointF>
#include <QtTest>
#include <QTextStream>


#include "ComponentUtilities/LookupTable.h"

#ifndef TEST_DATA_ROOT
const QString relpath = "../UnitTests/HopsanCoreTests/LookupTableTest/";
#else
const QString relpath = TEST_DATA_ROOT;
#endif

class Point3DF
{
public:
    Point3DF()
    {
        r=0; c=0; p=0;
    }

    Point3DF(double x, double y, double z)
    {
        r = x; c = y; p = z;
    }
    double r;
    double c;
    double p;
};

Q_DECLARE_METATYPE(QVector<double>);
Q_DECLARE_METATYPE(Point3DF);

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
    void lookup3D();
    void lookup3D_data();
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

    LookupTable1D lookup1d;

    // Set the index and value vectors
    lookup1d.getIndexDataRef() = indexData.toStdVector();
    lookup1d.getValueDataRef() = valueData.toStdVector();

    // Check if data is OK, triggering sort if needed
    bool dataIsOk = lookup1d.isDataOK();
    if (!dataIsOk && !lookup1d.allIndexStrictlyIncreasing())
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
    QFETCH(double, eps);
    QFETCH(QPointF, in);
    QFETCH(double, out);

    LookupTable2D lookup2d;

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
            QVERIFY2(fc(val, out, eps), QString("Interpolate returned the wrong result: %1!=%2").arg(val).arg(out).toLatin1());
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

void LookupTableTest::lookup3D()
{
    QFETCH(QVector<double>, rowIndexData);
    QFETCH(QVector<double>, colIndexData);
    QFETCH(QVector<double>, planeIndexData);
    QFETCH(QVector<double>, valueData);
    QFETCH(bool, shouldBeOK);
    QFETCH(double, eps);
    QFETCH(Point3DF, in);
    QFETCH(double, out);

    LookupTable3D lookup3d;

    // Set the index and value vectors
    lookup3d.getIndexDataRef(0) = rowIndexData.toStdVector();
    lookup3d.getIndexDataRef(1) = colIndexData.toStdVector();
    lookup3d.getIndexDataRef(2) = planeIndexData.toStdVector();
    lookup3d.getValueDataRef() = valueData.toStdVector();

    // Check if data is OK, try sort if not
    bool dataIsOk = lookup3d.isDataOK();
    if (!dataIsOk)
    {
        lookup3d.sortIncreasing();
    }
    dataIsOk = lookup3d.isDataOK();

    // Lookup values and check results
    if (shouldBeOK)
    {
        if (dataIsOk)
        {
            const double val = lookup3d.interpolate(in.r, in.c, in.p);
            QVERIFY2(fc(val, out, eps), QString("Interpolate returned the wrong result: %1!=%2, diff:%3").arg(val).arg(out).arg(val-out).toLatin1());
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
    QTest::addColumn< double >("eps");
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
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(0,0) << 0.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(1,1) << 0.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(2,2) << 6.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(3,5) << 14.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(5,5) << 24.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(6,8) << 24.0;

    // Lookup interpolated values
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(2.5,2.5) << 9.0;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(2.5,3.8) << 10.3;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(4.67789,1.8345) << 19.22395;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(4.9,5.0) << 23.5;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << valueVec << true << 1e-9 << QPointF(4.9,6.0) << 23.5;

    // ========== Read test data from file ==========
    QStringList files;
    files << "2DTestData0_UT.dat" << "2DTestData1_UT.dat";
    foreach (QString filename, files)
    {
        QFile file(relpath+filename);
        file.open(QIODevice::ReadOnly);
        QVERIFY2(file.isOpen(), QString("File could not be opened: %1").arg(relpath+filename).toStdString().c_str());
        QTextStream testData(&file);
        rowIndexVec.clear(); colIndexVec.clear(); valueVec.clear();
        QStringList rows = testData.readLine().split(' ', QString::SkipEmptyParts);
        QStringList cols = testData.readLine().split(' ', QString::SkipEmptyParts);
        QStringList datas = testData.readLine().split(' ', QString::SkipEmptyParts);
        for (int r=0; r<rows.size(); ++r)
        {
            rowIndexVec << rows[r].toDouble();
        }
        for (int c=0; c<cols.size(); ++c)
        {
            colIndexVec << cols[c].toDouble();
        }
        for (int d=0; d<datas.size(); ++d)
        {
            valueVec << datas[d].toDouble();
        }

        // read test points and expected value
        ctr=0;
        while (!testData.atEnd())
        {
            QStringList line = testData.readLine().split(' ', QString::SkipEmptyParts);
            QTest::newRow((filename+QString("_%1").arg(ctr)).toLatin1()) << rowIndexVec << colIndexVec << valueVec << true << 1e-6 << QPointF(line[0].toDouble(),line[1].toDouble()) << line[2].toDouble();
            ++ctr;
        }
        file.close();
    }
}

void LookupTableTest::lookup3D_data()
{
    QTest::addColumn< QVector<double> >("rowIndexData");
    QTest::addColumn< QVector<double> >("colIndexData");
    QTest::addColumn< QVector<double> >("planeIndexData");
    QTest::addColumn< QVector<double> >("valueData");
    QTest::addColumn< bool >("shouldBeOK");
    QTest::addColumn< double >("eps");
    QTest::addColumn< Point3DF >("in");
    QTest::addColumn< double >("out");

    QVector<double> rowIndexVec, colIndexVec, planeIndexVec, valueVec;
    rowIndexVec << 1 << 2 << 3;
    colIndexVec << 1 << 2 << 3 << 4;
    planeIndexVec << 1 << 2;
    double ctr=-8;
    for (int ri=0; ri<rowIndexVec.size(); ++ri)
    {
        for (int ci=0; ci<colIndexVec.size(); ++ci)
        {
            for (int pi=0; pi<planeIndexVec.size(); ++pi)
            {
                valueVec << ctr;
                ++ctr;
            }
        }
    }

    // Lookup exact values
    QTest::newRow("test1") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(1,1,1) << -8.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(1,1,2) << -7.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(1,4,2) << -1.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(2,1,1) << 0.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(3,2,1) << 10.0;
    QTest::newRow("test1") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(3,4,2) << 15.0;

    // Out of range values
    QTest::newRow("test2") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(0,0,0) << -8.0;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(0,0,2) << -7.0;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(1,4,3) << -1.0;
    QTest::newRow("test2") << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-9 << Point3DF(3,4,3) << 15.0;

    // ========== Read test data from file ==========
    QStringList files;
    files << "3DTestData0_UT.dat" << "3DTestData1_UT.dat";
    foreach (QString filename, files)
    {
        QFile file(relpath+filename);
        file.open(QIODevice::ReadOnly);
        QVERIFY2(file.isOpen(), QString("File could not be opened: %1").arg(relpath+filename).toStdString().c_str());
        QTextStream testData(&file);
        rowIndexVec.clear(); colIndexVec.clear(); planeIndexVec.clear(); valueVec.clear();
        QStringList rows = testData.readLine().split(' ', QString::SkipEmptyParts);
        QStringList cols = testData.readLine().split(' ', QString::SkipEmptyParts);
        QStringList planes = testData.readLine().split(' ', QString::SkipEmptyParts);
        QStringList datas = testData.readLine().split(' ', QString::SkipEmptyParts);
        for (int r=0; r<rows.size(); ++r)
        {
            rowIndexVec << rows[r].toDouble();
        }
        for (int c=0; c<cols.size(); ++c)
        {
            colIndexVec << cols[c].toDouble();
        }
        for (int p=0; p<planes.size(); ++p)
        {
            planeIndexVec << planes[p].toDouble();
        }
        for (int d=0; d<datas.size(); ++d)
        {
            valueVec << datas[d].toDouble();
        }

        // read test points and expected value
        ctr=0;
        while (!testData.atEnd())
        {
            QStringList line = testData.readLine().split(' ', QString::SkipEmptyParts);
            QTest::newRow((filename+QString("_%1").arg(ctr)).toLatin1()) << rowIndexVec << colIndexVec << planeIndexVec << valueVec << true << 1e-5 << Point3DF(line[0].toDouble(),line[1].toDouble(),line[2].toDouble()) << line[3].toDouble();
            ++ctr;
        }
        file.close();
    }


}

QTEST_APPLESS_MAIN(LookupTableTest)

#include "tst_lookuptabletest.moc"
