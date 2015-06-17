/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

public:
    HVectorTest();

private Q_SLOTS:
    void sizeTest();
    void sizeTest_data();
};

HVectorTest::HVectorTest()
{
}

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
