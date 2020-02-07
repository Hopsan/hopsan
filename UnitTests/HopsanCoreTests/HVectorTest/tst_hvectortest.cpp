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
#include <QtTest>

#include "HopsanTypes.h"

using namespace hopsan;

Q_DECLARE_METATYPE(HVector<double>);
typedef HVector<double> HVectord;

class HVectorTest : public QObject
{
    Q_OBJECT

private slots:
    void sizeTest();
    void sizeTest_data();

    void first() {
        HVector<int> vec;
        vec.append(1);
        QVERIFY(vec.first() == 1);
        vec.append(2);
        QVERIFY(vec.first() == 1);
    }

    void last() {
        HVector<int> vec;
        vec.append(1);
        QVERIFY(vec.last() == 1);
        QVERIFY(vec.first() == vec.last());
        vec.append(2);
        QVERIFY(vec.last() == 2);
        vec.append(3);
        QVERIFY(vec.last() == 3);
        vec.resize(2);
        QVERIFY(vec.last() == 2);
    }
};

void HVectorTest::sizeTest()
{
    QFETCH(HVectord, Vector);
    QFETCH(size_t, correctSize);
    QVERIFY2(Vector.size() == correctSize, QString("Failure! The HVector size is not correct %1!=%2").arg(Vector.size()).arg(correctSize).toStdString().c_str());
}

void HVectorTest::sizeTest_data()
{
    QTest::addColumn<HVectord>("Vector");
    QTest::addColumn<size_t>("correctSize");
    HVectord vec1;
    QTest::newRow("HVectord vec1") << vec1 << size_t(0);
    vec1.resize(5);
    QTest::newRow("vec1.resize(5)") << vec1 << size_t(5);
    vec1.resize(5000000);
    QTest::newRow("vec1.resize(5000000)") << vec1 << size_t(5000000);
    vec1.resize(2);
    QTest::newRow("vec1.resize(2)") << vec1 << size_t(2);
    vec1.resize(4, 4.0);
    QTest::newRow("vec1.resize(4, 4.0)") << vec1 << size_t(4);
    vec1.append(44.9);
    QTest::newRow("vec1.append(44.9)") << vec1 << size_t(5);
    vec1.clear();
    QTest::newRow("vec1.clear()") << vec1 << size_t(0);
}

QTEST_APPLESS_MAIN(HVectorTest)

#include "tst_hvectortest.moc"
