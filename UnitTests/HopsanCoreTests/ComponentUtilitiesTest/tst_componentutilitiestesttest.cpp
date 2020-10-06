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

#include "ComponentUtilities.h"

using namespace hopsan;

Q_DECLARE_METATYPE(QVector<double>)

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

        QVERIFY2(fuzzyEqual(fabs(x)*pow(1.0/(pow(x0,2.0) + pow(fabs(x),2.0)),0.25)*sign(x), signedSquareL(x,x0)), "signedSquareL() returned wrong value!");
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

    void Dx_Signed_Square_L()
    {
        QFETCH(double, x);
        QFETCH(double, x0);

        QVERIFY2(fuzzyEqual((pow(1.0/(pow(x0,2.0) + pow(fabs(x),2.0)),1.25)*(2.0*pow(x0,2.0) + pow(fabs(x),2.0))*
                  dxAbs(x)*sign(x))/2.0, dxSignedSquareL(x,x0)), "dxSignedSquareL() returned wrong value!");
    }

    void Dx_Signed_Square_L_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("x0");

        QTest::newRow("0") << 0.0 << 1.0;
        for(int i=1; i<100; ++i)
        {
            QTest::newRow("i") << (double)rand() - (double)RAND_MAX*0.5 << (double)rand()- (double)RAND_MAX*0.5;
        }
    }


    void Square_Abs_L()
    {
        QFETCH(double, x);
        QFETCH(double, x0);

        QVERIFY2(fuzzyEqual(-sqrt(x0) + sqrt(x0 + fabs(x)), squareAbsL(x,x0)), "squareAbsL() returned wrong value!");
    }

    void Square_Abs_L_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("x0");

        QTest::newRow("0") << 0.0 << 1.0;
        for(int i=1; i<100; ++i)
        {
            QTest::newRow("i") << (double)rand() - (double)RAND_MAX*0.5 << (double)rand();
        }
    }

    void Dx_Square_Abs_L()
    {
        QFETCH(double, x);
        QFETCH(double, x0);

        QVERIFY2(fuzzyEqual(1.0 / (sqrt(x0 + fabs(x)) * 2.0) * sign(x), dxSquareAbsL(x,x0)), "dxSquareAbsL() returned wrong value!");
    }

    void Dx_Square_Abs_L_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("x0");

        QTest::newRow("0") << 0.0 << 1.0;
        for(int i=1; i<100; ++i)
        {
            QTest::newRow("i") << (double)rand() - (double)RAND_MAX*0.5 << (double)rand();
        }
    }

    void Atan2_L()
    {
        QFETCH(double,y);
        QFETCH(double,x);
        QFETCH(double,ret);

        QVERIFY2(fuzzyEqual(Atan2L(y,x), ret), "Atan2L() returned wrong value!");
    }

    void Atan2_L_data()
    {
        QTest::addColumn<double>("y");
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("ret");

        QTest::newRow("0") << 0.0 << 1.0 << 0.0;
        QTest::newRow("1") << 1.0 << 1.0 << 3.141592653589793/4.0;
        QTest::newRow("2") << 1.0 << -1.0 << 3*3.141592653589793/4.0;
        QTest::newRow("3") << -1.0 << 1.0 << -3.141592653589793/4.0;
        QTest::newRow("4") << -1.0 << -1.0 << -3*3.141592653589793/4.0;
        QTest::newRow("5") << 1.0 << 0.0 << 3.141592653589793/2.0;
        QTest::newRow("6") << -1.0 << 0.0 << -3.141592653589793/2.0;
        QTest::newRow("6") << 0.0 << 0.0 << 0.0;
    }

    void ArcSin_L()
    {
        QFETCH(double,x);
        QFETCH(double,ret);

        QVERIFY2(fuzzyEqual(ArcSinL(x), ret), "ArcSinL() returned wrong value!");
    }

    void ArcSin_L_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("ret");

        QTest::newRow("0") << -1.0 << -3.141592653589793/2.0;
        QTest::newRow("2") << -0.8660254 << -3.141592653589793/3.0;
        QTest::newRow("3") << -0.7071068 << -3.141592653589793/4.0;
        QTest::newRow("4") << -0.5 << -3.141592653589793/6.0;
        QTest::newRow("5") << 0.0 << 0.0;
        QTest::newRow("6") << 0.5 << 3.141592653589793/6.0;
        QTest::newRow("7") << 0.7071068 << 3.141592653589793/4.0;
        QTest::newRow("8") << 0.8660254 << 3.141592653589793/3.0;
        QTest::newRow("9") << 1.0 << 3.141592653589793/2.0;
    }

    void Dx_ArcSin_L()
    {
        QFETCH(double,x);
        QFETCH(double,ret);

        QVERIFY2(fuzzyEqual(dxArcSinL(x), ret), "dxArcSinL() returned wrong value!");
    }

    void Dx_ArcSin_L_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("ret");

        QTest::newRow("4") << -0.5 << 1.1547;
        QTest::newRow("5") << 0.0 << 1.0;
        QTest::newRow("6") << 0.5 << 1.1547;
    }

    void Diff_Angle()
    {
        QFETCH(double,a1);
        QFETCH(double,a2);
        QFETCH(double,diff);

        QVERIFY2(fuzzyEqual(diffAngle(a1,a2), diff), "diffAngle() returned wrong value!");
    }

    void Diff_Angle_data()
    {
        QTest::addColumn<double>("a1");
        QTest::addColumn<double>("a2");
        QTest::addColumn<double>("diff");

        QTest::newRow("0") << 0.0 << 1.0 << -1.0;
        QTest::newRow("1") << 1.0 << -2.0 << 3.0;
        QTest::newRow("2") << -3.0 << -4.0 << 1.0;
    }


    void Limit()
    {
        QFETCH(double,x);
        QFETCH(double,min);
        QFETCH(double,max);
        QFETCH(double,x_limited);

        QVERIFY2(fuzzyEqual(limit(x,min,max), x_limited), "limit() returned wrong value!");
    }

    void Limit_data()
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


    void Low_Limit()
    {
        QFETCH(double,x);
        QFETCH(double,min);
        QFETCH(double,x_limited);

        QVERIFY2(fuzzyEqual(lowLimit(x,min), x_limited), "lowLimit() returned wrong value!");
    }

    void Low_Limit_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("x_limited");

        QTest::newRow("0") << 1.0 << 1.7 << 1.7;
        QTest::newRow("1") << 3.0 << 1.7 << 3.0;
        QTest::newRow("2") << 5.0 << -1.7 << 5.0;
        QTest::newRow("3") << -3.0 << -1.7 << -1.7;
    }


    void Dx_Limit()
    {
        QFETCH(double,x);
        QFETCH(double,min);
        QFETCH(double,max);
        QFETCH(double,x_limited);

        QVERIFY2(fuzzyEqual(dxLimit(x,min,max), x_limited), "dxLimit() returned wrong value!");
    }

    void Dx_Limit_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("max");
        QTest::addColumn<double>("x_limited");

        QTest::newRow("0") << 5.0 << 1.7 << 4.0 << 0.000000001;
        QTest::newRow("1") << 3.0 << 1.7 << 4.0 << 1.0;
        QTest::newRow("2") << 1.0 << 1.7 << 4.0 << 0.000000001;
        QTest::newRow("3") << 5.0 << -1.7 << 4.0 << 0.000000001;
        QTest::newRow("4") << -1.0 << -1.7 << 4.0 << 1.0;
        QTest::newRow("5") << -10.0 << -1.7 << 4.0 << 0.000000001;
        QTest::newRow("6") << -5.0 << -10.0 << -6.0 << 0.000000001;
    }


    void Dx_Low_Limit()
    {
        QFETCH(double,x);
        QFETCH(double,min);
        QFETCH(double,x_limited);

        QVERIFY2(fuzzyEqual(dxLowLimit(x,min), x_limited), "dxLowLimit() returned wrong value!");
    }

    void Dx_Low_Limit_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("x_limited");

        QTest::newRow("0") << 1.0 << 1.7 << 0.000000001;
        QTest::newRow("1") << 3.0 << 1.7 << 1.0;
        QTest::newRow("2") << 5.0 << -1.7 << 1.0;
        QTest::newRow("3") << -3.0 << -1.7 << 0.000000001;
    }

    void Dx_Low_Limit2()
    {
        QFETCH(double,x);
        QFETCH(double,sx);
        QFETCH(double,min);
        QFETCH(double,x_limited);

        QVERIFY2(fuzzyEqual(dxLowLimit2(x,sx,min), x_limited), "dxLowLimit2() returned wrong value!");
    }

    void Dx_Low_Limit2_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("sx");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("x_limited");

        QTest::newRow("0") << 3.0 << 1.0 << 1.7 << 1.0;
        QTest::newRow("1") << 1.0 << 1.0 << 1.7 << 1.0;
        QTest::newRow("2") << 3.0 << -1.0 << 1.7 << 1.0;
        QTest::newRow("3") << 1.0 << -1.0 << 1.7 << 0.000000001;
        QTest::newRow("4") << 5.0 << 1.0 << -1.7 << 1.0;
        QTest::newRow("5") << 5.0 << -1.0 << -1.7 << 1.0;
        QTest::newRow("6") << -3.0 << 1.0 << -1.7 << 1.0;
        QTest::newRow("7") << -3.0 << -1.0 << -1.7 << 0.000000001;
    }

    void Dx_Limit2()
    {
        QFETCH(double,x);
        QFETCH(double,sx);
        QFETCH(double,min);
        QFETCH(double,max);
        QFETCH(double,x_limited);

        QVERIFY2(fuzzyEqual(dxLimit2(x,sx,min,max), x_limited), "dxLimit2() returned wrong value!");
    }

    void Dx_Limit2_data()
    {
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("sx");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("max");
        QTest::addColumn<double>("x_limited");

        QTest::newRow("0") << 3.0 << 1.0 << 1.7 << 5.0 << 1.0;
        QTest::newRow("1") << 1.0 << 1.0 << 1.7 << 5.0 << 1.0;
        QTest::newRow("2") << 7.0 << 1.0 << 1.7 << 5.0 << 0.000000001;
        QTest::newRow("3") << 3.0 << -1.0 << 1.7 << 5.0 << 1.0;
        QTest::newRow("4") << 1.0 << -1.0 << 1.7 << 5.0 << 0.000000001;
        QTest::newRow("5") << 7.0 << -1.0 << 1.7 << 5.0 << 1.0;

        QTest::newRow("6") << -3.0 << 1.0 << -5.0 << -1.7 << 1.0;
        QTest::newRow("7") << -1.0 << 1.0 << -5.0 << -1.7 << 0.000000001;
        QTest::newRow("8") << -7.0 << 1.0 << -5.0 << -1.7 << 1.0;
        QTest::newRow("9") << -3.0 << -1.0 << -5.0 << -1.7 << 1.0;
        QTest::newRow("10") << -1.0 << -1.0 << -5.0 << -1.7 << 1.0;
        QTest::newRow("11") << -7.0 << -1.0 << -5.0 << -1.7 << 0.000000001;
    }

    void Div_L()
    {
        QFETCH(double,y);
        QFETCH(double,x);
        QFETCH(double,ret);

        QVERIFY2(fuzzyEqual(hopsan::div(y,x), ret), "div() returned wrong value!");
    }

    void Div_L_data()
    {
        QTest::addColumn<double>("y");
        QTest::addColumn<double>("x");
        QTest::addColumn<double>("ret");

        QTest::newRow("0") << 10.0 << 2.0 << 5.0;
        QTest::newRow("1") << 10.0 << 3.0 << 3.0;
        QTest::newRow("2") << -10.0 << 3.0 << -3.0;
    }

    void Integrator_Test()
    {
        QFETCH(QVector<double>, data);
        QFETCH(double, dt);
        QFETCH(double, init);
        QFETCH(double, res);

        Integrator x;
        x.initialize(dt, init, 0);
        for(int i=0; i<data.size(); ++i)
        {
            x.update(data.at(i));
        }
        QVERIFY2(fuzzyEqual(x.value(), res), "Integrator integrated wrong.");
    }

    void Integrator_Test_data()
    {
        QTest::addColumn<QVector<double> >("data");
        QTest::addColumn<double>("dt");
        QTest::addColumn<double>("init");
        QTest::addColumn<double>("res");

        QVector<double> tempVec;
        for(int i=1; i<=10000; ++i)
        {
            tempVec << i;
        }
        QTest::newRow("0") << tempVec << 0.001 << 0.0 << (10000.0)/2.0*10000.0*0.001;

        tempVec.clear();
        for(int i=1; i>=-10000; --i)
        {
            tempVec << i;
        }
        QTest::newRow("1") << tempVec << 0.001 << 0.0 << -(10000.0)/2.0*10000.0*0.001;

        tempVec.clear();
        for(int i=1; i<=10000; ++i)
        {
            tempVec << 1.0;
        }

        QTest::newRow("2") << tempVec << 0.001 << 1.0 << (10000.0)*0.001;
    }



    void Integrator_Limited_Test()
    {
        QFETCH(QVector<double>, data);
        QFETCH(double, dt);
        QFETCH(double, init);
        QFETCH(double, min);
        QFETCH(double, max);
        QFETCH(double, res);

        IntegratorLimited x;
        x.initialize(dt, init, 0, min, max);
        for(int i=0; i<data.size(); ++i)
        {
            x.update(data.at(i));
        }
        QVERIFY2(fuzzyEqual(x.value(), res), "Integrator integrated wrong.");
    }

    void Integrator_Limited_Test_data()
    {
        QTest::addColumn<QVector<double> >("data");
        QTest::addColumn<double>("dt");
        QTest::addColumn<double>("init");
        QTest::addColumn<double>("min");
        QTest::addColumn<double>("max");
        QTest::addColumn<double>("res");

        QVector<double> tempVec;
        for(int i=1; i<=10000; ++i)
        {
            tempVec << i;
        }
        QTest::newRow("0") << tempVec << 0.001 << 0.0 << -1000000000.0 << 1000000000.0 << (10000.0)/2.0*10000.0*0.001;
        QTest::newRow("1") << tempVec << 0.001 << 0.0 << -1000000000.0 << 52.0 << 52.0;

        tempVec.clear();
        for(int i=1; i>=-10000; --i)
        {
            tempVec << i;
        }
        QTest::newRow("2") << tempVec << 0.001 << 0.0 << -1000000000.0 << 1000000000.0 << -(10000.0)/2.0*10000.0*0.001;
        QTest::newRow("3") << tempVec << 0.001 << 0.0 << -42.0 << 1000000000.0 << -42.0;

        tempVec.clear();
        for(int i=1; i<=10000; ++i)
        {
            tempVec << 1.0;
        }

        QTest::newRow("4") << tempVec << 0.001 << 1.0 << -1000000000.0 << 1000000000.0 << (10000.0)*0.001;
        QTest::newRow("5") << tempVec << 0.001 << 1.0 << -1000000000.0 << 2.0 << 2.0;
    }

    void ploParser()
    {
        QFETCH( QString, ploData);
        QFETCH( QString, indexDataName);
        QFETCH( QString, outputDataName);
        QFETCH( bool, expectParseOK);
        QFETCH( int, expectedInIndex);
        QFETCH( int, expectedOutIndex);
        QFETCH( double, expectedLastValue);

        PLOParser parser;
        bool parsedOK = parser.readText(qPrintable(ploData));
        QCOMPARE(parsedOK, expectParseOK);
        int iIn = parser.getColIdxForDataName(qPrintable(indexDataName));
        int iOut = parser.getColIdxForDataName(qPrintable(outputDataName));
        QCOMPARE(iIn, expectedInIndex);
        QCOMPARE(iOut, expectedOutIndex);
        if ((iIn >= 0) && (iOut >= 0)) {
            std::vector<double> values;
            bool copyOK = parser.copyColumn(iOut, values);
            QVERIFY(copyOK);
            QCOMPARE(values.back(), expectedLastValue);
        }
    }

    void ploParser_data()
    {
        QTest::addColumn< QString >("ploData");
        QTest::addColumn< QString >("indexDataName");
        QTest::addColumn< QString >("outputDataName");
        QTest::addColumn< bool >("expectParseOK");
        QTest::addColumn< int >("expectedInIndex");
        QTest::addColumn< int >("expectedOutIndex");
        QTest::addColumn< double >("expectedLastValue");

        QString ploData1 = R"(
          'VERSION'
          1
          'temp1.PLO'
          2    4
          'Time',    'x', 'y'
          1.0 1.0 1.0
          0  10 100
          1  20 200
          2  30 300
          3  40 400
          test1.PLO.DAT_-1
          Test Model.hmf)";

        QString ploData2 = R"(
          'VERSION'
          2
          'temp.PLO'
          2    4
          'Time',    'x', 'y'
          Time  Position Position
          0  10 100
          1  20 200
          2  30 300
          3  40 400
          test2.PLO.DAT_-1
          Test Model.hmf)";

        QTest::newRow("plo1 0") << ploData1 << "Time" << "x" << true << 0 << 1 << 40.0;
        QTest::newRow("plo1 1") << ploData1 << "Time" << "y" << true << 0 << 2 << 400.0;
        QTest::newRow("plo1 2") << ploData1 << "x" << "y" << true << 1 << 2 << 400.0;
        QTest::newRow("plo1 3") << ploData1 << "x" << "notExist" << true << 1 << -1 << -1.0;
        QTest::newRow("plo1 4") << ploData1 << "notExist" << "y" << true << -1 << 2 << -1.0;

        QTest::newRow("plo2 0") << ploData2 << "Time" << "x" << true << 0 << 1 << 40.0;
        QTest::newRow("plo2 1") << ploData2 << "Time" << "y" << true << 0 << 2 << 400.0;
        QTest::newRow("plo2 2") << ploData2 << "x" << "y" << true << 1 << 2 << 400.0;
        QTest::newRow("plo2 3") << ploData2 << "x" << "notExist" << true << 1 << -1 << 40.0;
        QTest::newRow("plo2 4") << ploData2 << "notExist" << "y" << true << -1 << 2 << 40.0;

    }
};


QTEST_APPLESS_MAIN(ComponentUtilitiesTestTest)

#include "tst_componentutilitiestesttest.moc"
