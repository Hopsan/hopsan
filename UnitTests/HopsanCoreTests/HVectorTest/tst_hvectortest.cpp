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

#include "HVector.hpp"
#include "HString.h"

using namespace hopsan;

class HVectorTest : public QObject
{
    Q_OBJECT

private slots:
    void sizeTest()
    {
        HVector<double> vec;
        QCOMPARE(vec.size(), static_cast<size_t>(0));
        QCOMPARE(vec.capacity(), static_cast<size_t>(0));
        QVERIFY(vec.empty());

        vec.reserve(16);
        QCOMPARE(vec.size(), static_cast<size_t>(0));
        QCOMPARE(vec.capacity(), static_cast<size_t>(16));
        QVERIFY(vec.empty());

        vec.resize(8);
        QCOMPARE(vec.size(), static_cast<size_t>(8));
        QCOMPARE(vec.capacity(), static_cast<size_t>(16));
        QVERIFY(!vec.empty());

        // Resize will allocate double the capacity when reallocating
        vec.resize(24);
        QCOMPARE(vec.size(), static_cast<size_t>(24));
        QCOMPARE(vec.capacity(), static_cast<size_t>(48));

        // Resize will shrink, but not reduce capacity
        vec.resize(16);
        QCOMPARE(vec.size(), static_cast<size_t>(16));
        QCOMPARE(vec.capacity(),static_cast<size_t>( 48));

        vec.resize(48);
        QCOMPARE(vec.size(),static_cast<size_t>(48));
        QCOMPARE(vec.capacity(), static_cast<size_t>(48));

        // Reserve wont shrink capacity
        vec.reserve(16);
        QCOMPARE(vec.size(), static_cast<size_t>(48));
        QCOMPARE(vec.capacity(), static_cast<size_t>(48));

        vec.append(1);
        QCOMPARE(vec.size(), static_cast<size_t>(49));
        QCOMPARE(vec.capacity(), static_cast<size_t>(49u*2));

        vec.append(1);
        QCOMPARE(vec.size(), static_cast<size_t>(50));
        QCOMPARE(vec.capacity(), static_cast<size_t>(49u*2));

        vec.clear();
        QCOMPARE(vec.size(), static_cast<size_t>(0));
        QCOMPARE(vec.capacity(), static_cast<size_t>(0));
        QVERIFY(vec.empty());
    }

    void append()
    {
        HVector<double> vec;
        vec.resize(3, 0);
        vec.append(1.0);
        QVERIFY(!vec.empty());
        QCOMPARE(vec[3], 1.);
    }

    void storeClass()
    {
        HVector<HString> vec;
        vec.resize(3, "abc");
        QCOMPARE(vec[1].c_str(), "abc");
    }

    void copy()
    {
        HVector<HString> vec1;
        vec1.resize(3, "abc");

        auto vec2 = vec1;
        for (size_t i = 0; i < vec1.size(); ++i) {
            QCOMPARE(vec1[i].c_str(), vec2[i].c_str());
        }

        vec1.clear();
        // Ensure vec2 was not touched by clearing vec1
        QCOMPARE(vec2[2].c_str(), "abc");
    }

    void assigFrom()
    {
        const size_t size = 50;
        HVector<double> vec;

        {
            double c_array[size];
            for (size_t i=0; i<size; ++i) {
                c_array[i] = i;
            }
            vec.assign_from(c_array, size);
        }

        QCOMPARE(vec.capacity(), size);
        QCOMPARE(vec.size(), size);
        for (size_t i=0; i<size; ++i) {
            QCOMPARE(vec[i], static_cast<double>(i));
        }
    }

    void resizeInit()
    {
        HVector<double> vec;
        vec.resize(3, 2.);
        QCOMPARE(vec[0], 2.);
        QCOMPARE(vec[1], 2.);
        QCOMPARE(vec[2], 2.);
    }

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

QTEST_APPLESS_MAIN(HVectorTest)

#include "tst_hvectortest.moc"
