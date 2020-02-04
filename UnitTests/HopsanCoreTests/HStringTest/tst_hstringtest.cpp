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

#include <QtTest>
#include "HopsanTypes.h"

using namespace hopsan;

Q_DECLARE_METATYPE(HString);

class HStringTests : public QObject
{
    Q_OBJECT
    
private Q_SLOTS:
    void HString_Less()
    {
        QFETCH(HString, String1);
        QFETCH(HString, String2);
        HString failmsg("Failure! " + String1 + " !< " + String2);
        QVERIFY2(String1 < String2, failmsg.c_str() );
        QVERIFY2(String1 < String2.c_str(), failmsg.c_str() );
        //QVERIFY2(String1.c_str() < String2, failmsg.c_str() );
    }
    void HString_Less_data()
    {
        QTest::addColumn<HString>("String1");
        QTest::addColumn<HString>("String2");
        QTest::newRow("0") << HString("0") << HString("1");
        QTest::newRow("1") << HString("Apa") << HString("aPA");
        QTest::newRow("2") << HString("APA") << HString("ApA");
        QTest::newRow("3") << HString("0.6.x_r4567") << HString("0.6.x_r4588");
        QTest::newRow("4") << HString() << HString(" ");
    }

    void HString_EQ()
    {
        QFETCH(HString, String1);
        QFETCH(HString, String2);
        HString failmsg("Failure! " + String1 + " != " + String2);
        QVERIFY2(String1 == String2,            failmsg.c_str() );
        QVERIFY2(String1 == String2.c_str(),    failmsg.c_str() );
        QVERIFY2(String1.c_str() == String2,    failmsg.c_str() );
        QVERIFY2(String1.compare(String2),      failmsg.c_str() );
    }
    void HString_EQ_data()
    {
        QTest::addColumn<HString>("String1");
        QTest::addColumn<HString>("String2");
        QTest::newRow("0") << HString("0") << HString("0");
        QTest::newRow("Apa") << HString("Apa") << HString("Apa");
        QTest::newRow("0.6.x_r4567") << HString("0.6.x_r4567") << HString("0.6.x_r4567");
        QTest::newRow("EmptyString") << HString() << HString("");
    }

    void HString_Substr()
    {
        QFETCH(HString, String);
        QFETCH(size_t, pos);
        QFETCH(size_t, len);
        QFETCH(HString, Result);
        HString failmsg("Failure! " + String.substr(pos,len) + " != " + Result);
        QVERIFY2(String.substr(pos,len) == Result, failmsg.c_str());

    }
    void HString_Substr_data()
    {
        QTest::addColumn<HString>("String");
        QTest::addColumn<size_t>("pos");
        QTest::addColumn<size_t>("len");
        QTest::addColumn<HString>("Result");
        QTest::newRow("0,npos") << HString("TheBrownFoxJumps") << size_t(0) << HString::npos << HString("TheBrownFoxJumps");
        QTest::newRow("8,npos") << HString("TheBrownFoxJumps") << size_t(8) << HString::npos << HString("FoxJumps");
        QTest::newRow("0,8") << HString("TheBrownFoxJumps") << size_t(0) << size_t(8) << HString("TheBrown");
        QTest::newRow("0,16") << HString("TheBrownFoxJumps") << size_t(0) << size_t(16) << HString("TheBrownFoxJumps");
        QTest::newRow("3,5") << HString("TheBrownFoxJumps") << size_t(3) << size_t(5) << HString("Brown");
        QTest::newRow("1,1") << HString("TheBrownFoxJumps") << size_t(1) << size_t(1) << HString("h");
        QTest::newRow("3,1000") << HString("TheBrownFoxJumps") << size_t(3) << size_t(1000) << HString("BrownFoxJumps");
        QTest::newRow("16,1") << HString("TheBrownFoxJumps") << size_t(16) << size_t(1) << HString("");
        QTest::newRow("0,16") << HString("TheBrownFoxJumps") << size_t(0) << size_t(16) << HString("TheBrownFoxJumps");
        QTest::newRow("0,17") << HString("TheBrownFoxJumps") << size_t(0) << size_t(17) << HString("TheBrownFoxJumps");
        QTest::newRow("-1,16") << HString("TheBrownFoxJumps") << size_t(-1) << size_t(16) << HString("");
        QTest::newRow("-1,1") << HString("TheBrownFoxJumps") << size_t(-1) << size_t(1) << HString("");
        QTest::newRow("16,0") << HString("TheBrownFoxJumps") << size_t(16) << size_t(0) << HString("");
        QTest::newRow("8,0") << HString("TheBrownFoxJumps") << size_t(16) << size_t(0) << HString("");
        QTest::newRow("15,1") << HString("TheBrownFoxJumps") << size_t(15) << size_t(1) << HString("s");
    }

    void HString_Erase()
    {
        QFETCH(HString, String);
        QFETCH(size_t, pos);
        QFETCH(size_t, len);
        QFETCH(HString, Result);
        HString failmsg("Failure! " + HString(String).erase(pos,len) + " != " + Result);
        QVERIFY2(String.erase(pos,len) == Result, failmsg.c_str());

    }
    void HString_Erase_data()
    {
        QTest::addColumn<HString>("String");
        QTest::addColumn<size_t>("pos");
        QTest::addColumn<size_t>("len");
        QTest::addColumn<HString>("Result");
        QTest::newRow("0,npos") << HString("TheBrownFoxJumps") << size_t(0) << HString::npos << HString("");
        QTest::newRow("8,npos") << HString("TheBrownFoxJumps") << size_t(8) << HString::npos << HString("TheBrown");
        QTest::newRow("0,8") << HString("TheBrownFoxJumps") << size_t(0) << size_t(8) << HString("FoxJumps");
        QTest::newRow("0,16") << HString("TheBrownFoxJumps") << size_t(0) << size_t(16) << HString("");
        QTest::newRow("0,17") << HString("TheBrownFoxJumps") << size_t(0) << size_t(17) << HString("");
        QTest::newRow("3,5") << HString("TheBrownFoxJumps") << size_t(3) << size_t(5) << HString("TheFoxJumps");
        QTest::newRow("1,1") << HString("TheBrownFoxJumps") << size_t(1) << size_t(1) << HString("TeBrownFoxJumps");
        QTest::newRow("3,1000") << HString("TheBrownFoxJumps") << size_t(3) << size_t(1000) << HString("The");
        QTest::newRow("16,1") << HString("TheBrownFoxJumps") << size_t(16) << size_t(1) << HString("TheBrownFoxJumps");
        QTest::newRow("15,1") << HString("TheBrownFoxJumps") << size_t(15) << size_t(1) << HString("TheBrownFoxJump");
        QTest::newRow("-1,16") << HString("TheBrownFoxJumps") << size_t(-1) << size_t(16) << HString("TheBrownFoxJumps");
        QTest::newRow("-1,1") << HString("TheBrownFoxJumps") << size_t(-1) << size_t(1) << HString("TheBrownFoxJumps");
        QTest::newRow("8,0") << HString("TheBrownFoxJumps") << size_t(8) << size_t(0) << HString("TheBrownFoxJumps");
        QTest::newRow("0,0") << HString("TheBrownFoxJumps") << size_t(0) << size_t(0) << HString("TheBrownFoxJumps");
        QTest::newRow("16,0") << HString("TheBrownFoxJumps") << size_t(16) << size_t(0) << HString("TheBrownFoxJumps");
    }

    void HString_Append()
    {
        QFETCH(HString, base);
        QFETCH(HString, app1);
        QFETCH(HString, app2);
        QFETCH(HString, result);

        HString failmsg("Failure0! " + base+app1+app2 + " != " + result);
        QVERIFY2(base+app1+app2 == result, failmsg.c_str());

        HString str = base;
        str.append(app1).append(app2);
        failmsg = "Failure1! " + str + " != " + result;
        QVERIFY2(str == result, failmsg.c_str());

        HString str2;
        (str2 += base) += app1;
        str2 += app2;
        failmsg = "Failure2! " + str2 + " != " + result;
        QVERIFY2(str2 == result, failmsg.c_str());

        HString str3;
        str3.append(base.c_str());
        str3 += app1.c_str();
        str3.append(app2.c_str());
        failmsg = "Failure3! " + str3 + " != " + result;
        QVERIFY2(str3 == result, failmsg.c_str());

        HString str4;
        for (size_t i=0; i<base.size(); ++i)
        {
            str4.append(base[i]);
        }
        for (size_t i=0; i<app1.size(); ++i)
        {
            str4 += (app1[i]);
        }
        str4.append(app2);
        failmsg = "Failure4! " + str4 + " != " + result;
        QVERIFY2(str4 == result, failmsg.c_str());

    }
    void HString_Append_data()
    {
        QTest::addColumn<HString>("base");
        QTest::addColumn<HString>("app1");
        QTest::addColumn<HString>("app2");
        QTest::addColumn<HString>("result");

        QTest::newRow("Brown+Fox+Jumps") << HString("Brown") << HString("Fox") << HString("Jumps") << HString("BrownFoxJumps");
        QTest::newRow("empty+Fox+empty") << HString("") << HString("Fox") << HString("") << HString("Fox");
        QTest::newRow("Brown+Fox+empty") << HString("Brown") << HString("Fox") << HString("") << HString("BrownFox");
        QTest::newRow("empty+Fox+Jumps") << HString("") << HString("Fox") << HString("Jumps") << HString("FoxJumps");
        QTest::newRow("Fox+empty+Jumps") << HString("Fox") << HString("") << HString("Jumps") << HString("FoxJumps");

    }

    void HString_Replace()
    {
        QFETCH(HString, base);
        QFETCH(HString, old);
        QFETCH(HString, news);
        QFETCH(HString, result);

        HString op,failmsg;

        op = base;
        op.replace(old,news);
        failmsg = "Failed0! "+op+ " != " +result;
        QVERIFY2(op == result, failmsg.c_str());

        op = base;
        op.replace(old.c_str(),news.c_str());
        failmsg = "Failed1! "+op+ " != " +result;
        QVERIFY2(op == result, failmsg.c_str());


    }
    void HString_Replace_data()
    {
        QTest::addColumn<HString>("base");
        QTest::addColumn<HString>("old");
        QTest::addColumn<HString>("news");
        QTest::addColumn<HString>("result");

        QTest::newRow(" to _") << HString("The Brown Fox Jumps") << HString(" ") << HString("_") << HString("The_Brown_Fox_Jumps");
        QTest::newRow("Brown Fox to Yellow Beaver") << HString("The Brown Fox Jumps") << HString("Brown Fox") << HString("Yellow Beaver") << HString("The Yellow Beaver Jumps");
        QTest::newRow("Replace empty0") << HString("The Brown Fox Jumps") << HString("") << HString("_") << HString("The Brown Fox Jumps");
        QTest::newRow("Replace empty1") << HString("") << HString("_") << HString("_") << HString("");
        QTest::newRow("Replace empty2") << HString("") << HString("") << HString("_") << HString("");
        QTest::newRow("Replace # with ##") << HString("Some text #1 some more #2 done") << HString("#") << HString("##") << HString("Some text ##1 some more ##2 done");
        QTest::newRow("Replace A with AB multiple times") << HString("Some A text AA more AAA") << HString("A") << HString("AB") << HString("Some AB text ABAB more ABABAB");
        QTest::newRow("Replace A with BB multiple times") << HString("Some AAA text AAAA") << HString("A") << HString("BB") << HString("Some BBBBBB text BBBBBBBB");

    }

    void HString_isNummeric()
    {
        QFETCH(HString, str);
        QFETCH(bool, isnum);
        QVERIFY2(str.isNummeric() == isnum, (HString("isNummeric produced the wrong result for: ")+str).c_str());
    }
    void HString_isNummeric_data()
    {
        QTest::addColumn<HString>("str");
        QTest::addColumn<bool>("isnum");

        QTest::newRow("0")      << HString("0") << true;
        QTest::newRow("+0")     << HString("+0") << true;
        QTest::newRow("-0")     << HString("-0") << true;
        QTest::newRow("1")      << HString("1") << true;
        QTest::newRow("+1")     << HString("+1") << true;
        QTest::newRow("-1")     << HString("-1") << true;
        QTest::newRow("-3467")  << HString("-3467") << true;
        QTest::newRow("0.056")  << HString("0.056") << true;
        QTest::newRow("+0.056") << HString("+0.056") << true;
        QTest::newRow("-0.056") << HString("-0.056") << true;
        QTest::newRow("0.056e-12")  << HString("0.056e-12") << true;
        QTest::newRow("-0.056e-12") << HString("-0.056e-12") << true;
        QTest::newRow("+0.056e-12") << HString("+0.056e-12") << true;
        QTest::newRow("0.056E-12")  << HString("0.056E-12") << true;
        QTest::newRow("+0.056E-12") << HString("+0.056E-12") << true;
        QTest::newRow("-0.056E-12") << HString("-0.056E-12") << true;
        QTest::newRow("2346346.457457")     << HString("2346346.457457") << true;

        QTest::newRow("0.056d-12")  << HString("0.056d-12") << false;
        QTest::newRow("apa")        << HString("apa") << false;
        QTest::newRow("1234f")      << HString("1234f") << false;
        QTest::newRow("AB10F")      << HString("AB10F") << false;
        QTest::newRow("0.056e")     << HString("0.056e") << false;
        QTest::newRow("234634 6.457457")     << HString("234634 6.457457") << false;
        QTest::newRow(" 2346346.457457")     << HString(" 2346346.457457") << false;
        QTest::newRow("2346346.457457 ")     << HString("2346346.457457 ") << false;
    }

    void HString_isBool()
    {
        bool convertOK;
        QFETCH(HString, str);
        QFETCH(bool, expectedIsBool);
        QFETCH(bool, expectConvertOK);
        QFETCH(bool, expectedValue);

        QVERIFY(str.isBool() == expectedIsBool);
        QVERIFY(str.toBool(&convertOK) == expectedValue);
        QVERIFY(convertOK == expectConvertOK);
    }
    void HString_isBool_data()
    {
        QTest::addColumn<HString>("str");
        QTest::addColumn<bool>("expectedIsBool");
        QTest::addColumn<bool>("expectConvertOK");
        QTest::addColumn<bool>("expectedValue");

        QTest::newRow("0")      << HString("0") << true << true << false;
        QTest::newRow("1")      << HString("1") << true << true << true;
        QTest::newRow("true")   << HString("true") << true << true << true;
        QTest::newRow("false")  << HString("false") << true << true << false;

        QTest::newRow("+0")     << HString("+0") << false << false << false;
        QTest::newRow("-0")     << HString("-0") << false << false << false;
        QTest::newRow("+1")     << HString("+1") << false << false << false;
        QTest::newRow("-1")     << HString("-1") << false << false << false;
        QTest::newRow(" true")  << HString(" true") << false << false << false;
        QTest::newRow("True")   << HString("True") << false << false << false;
        QTest::newRow("false ") << HString("false ") << false << false << false;
        QTest::newRow("False")  << HString("False") << false << false << false;
    }

    void HString_toDouble()
    {
        bool isOK;
        QFETCH(HString, str);
        QFETCH(double, val);
        QFETCH(bool, isok);
        const double rv = str.toDouble(&isOK);
        QVERIFY2( rv == val, QString("toDouble produced the wrong result (%1) for: '%2'").arg(rv).arg(str.c_str()).toLatin1());
        QVERIFY2( isOK == isok, QString("toDouble produced the wrong isOK result (%1) for: '%2'").arg(isOK).arg(str.c_str()).toLatin1());
    }
    void HString_toDouble_data()
    {
        QTest::addColumn<HString>("str");
        QTest::addColumn<double>("val");
        QTest::addColumn<bool>("isok");

        QTest::newRow("0")      << HString("0")  << 0.0 << true;
        QTest::newRow("+0")     << HString("+0") << 0.0 << true;
        QTest::newRow("-0")     << HString("-0") << 0.0 << true;
        QTest::newRow("1")      << HString("1")  << 1.0 << true;
        QTest::newRow("+1")     << HString("+1") << 1.0 << true;
        QTest::newRow("-1")     << HString("-1") << -1.0 << true;
        QTest::newRow("-3467")  << HString("-3467") << -3467.0 << true;
        QTest::newRow("0.056")  << HString("0.056") << 0.056 << true;
        QTest::newRow("+0.056") << HString("+0.056") << 0.056 << true;
        QTest::newRow("-0.056") << HString("-0.056") << -0.056 << true;
        QTest::newRow("0.056e-12")  << HString("0.056e-12") << 0.056e-12 << true;
        QTest::newRow("-0.056e-12") << HString("-0.056e-12") << -0.056e-12 << true;
        QTest::newRow("+0.056e-12") << HString("+0.056e-12") << 0.056e-12 << true;
        QTest::newRow("0.056E-12")  << HString("0.056E-12") << 0.056e-12 << true;
        QTest::newRow("+0.056E-12") << HString("+0.056E-12") << 0.056e-12 << true;
        QTest::newRow("-0.056E-12") << HString("-0.056E-12") << -0.056e-12 << true;
        QTest::newRow("2346346.457457")     << HString("2346346.457457") << 2346346.457457 << true;
        QTest::newRow(" 2346346.457457")     << HString(" 2346346.457457") << 2346346.457457 << true;

        QTest::newRow("0.056d-12")  << HString("0.056d-12") << 0.056 << false;
        QTest::newRow("apa")        << HString("apa") << 0.0 << false;
        QTest::newRow("1234f")      << HString("1234f") << 1234.0 << false;
        QTest::newRow("AB10F")      << HString("AB10F") << 0.0 << false;
        QTest::newRow("0.056e")     << HString("0.056e") << 0.056 << false;
        QTest::newRow("234634 6.457457")     << HString("234634 6.457457") << 234634.0 << false;
        QTest::newRow("2346346.457457 ")     << HString("2346346.457457 ") << 2346346.457457 << false;
    }

    void HString_toLongInt()
    {
        bool isOK;
        QFETCH(HString, str);
        QFETCH(int, val);
        QFETCH(bool, isok);
        const long int rv = str.toLongInt(&isOK);
        QVERIFY2( rv == val, QString("toLongInt produced the wrong result (%1) for: '%2'").arg(rv).arg(str.c_str()).toLatin1());
        QVERIFY2( isOK == isok, QString("toLongInt produced the wrong isOK result (%1) for: '%2'").arg(isOK).arg(str.c_str()).toLatin1());
    }
    void HString_toLongInt_data()
    {
        QTest::addColumn<HString>("str");
        QTest::addColumn<int>("val");
        QTest::addColumn<bool>("isok");

        QTest::newRow("0")      << HString("0")  << 0 << true;
        QTest::newRow("+0")     << HString("+0") << 0 << true;
        QTest::newRow("-0")     << HString("-0") << 0 << true;
        QTest::newRow("1")      << HString("1")  << 1 << true;
        QTest::newRow("+1")     << HString("+1") << 1 << true;
        QTest::newRow("-1")     << HString("-1") << -1 << true;
        QTest::newRow("-3467")  << HString("-3467") << -3467 << true;
        QTest::newRow(" 2346346")   << HString(" 2346346") << 2346346 << true;

        QTest::newRow("2e10")   << HString("2e10") << 2 << false;
        QTest::newRow("-2E10")  << HString("-2E10") << -2 << false;
        QTest::newRow("1.16")   << HString("1.16") << 1 << false;
        QTest::newRow("0.056")  << HString("0.056") << 0 << false;
        QTest::newRow("+0.056") << HString("+0.056") << 0 << false;
        QTest::newRow("-0.056") << HString("-0.056") << 0 << false;
        QTest::newRow("apa")    << HString("apa") << 0 << false;
        QTest::newRow("AB10F")  << HString("AB10F") << 0 << false;
        QTest::newRow("234634 64")  << HString("234634 64") << 234634 << false;
        QTest::newRow("2346346 ")   << HString("2346346 ") << 2346346 << false;
    }

    void HString_split()
    {
        QFETCH(HString, str_with_delim);
        QFETCH(int, expected_num_parts);
        QFETCH(HString, str_without_delim);

        HVector<HString> parts = str_with_delim.split('.');
        QVERIFY2(parts.size() == static_cast<size_t>(expected_num_parts), "Split did not produce the expected number of parts");

        HString str;
        for (size_t i=0; i<parts.size(); ++i)
        {
            str.append(parts[i]);
        }

        QVERIFY2(str == str_without_delim, ("split did not produce the expected result: "+str).c_str());
    }

    void HString_split_data()
    {
        QTest::addColumn<HString>("str_with_delim");
        QTest::addColumn<int>("expected_num_parts");
        QTest::addColumn<HString>("str_without_delim");

        QTest::newRow("0") << HString("2.8.0.12345.1234") << 5 << HString("280123451234");
        QTest::newRow("1") << HString("2.8..") << 4 << HString("28");
        QTest::newRow("2") << HString("...") << 4 << HString("");
        QTest::newRow("3") << HString(".4.7.0") << 4 << HString("470");
        QTest::newRow("4") << HString(".4.7.") << 4 << HString("47");
        QTest::newRow("5") << HString("4..0") << 3 << HString("40");
        QTest::newRow("6") << HString("") << 1 << HString("");
        QTest::newRow("7") << HString("kaka") << 1 << HString("kaka");
    }

    void HString_compare()
    {
        QFETCH(HString, str1);
        QFETCH(HString, str2);
        QFETCH(bool, issameornot);

        QVERIFY(str1.compare(str2) == issameornot);
        QVERIFY(str1.compare(str2.c_str()) == issameornot);

    }

    void HString_compare_data()
    {
        QTest::addColumn<HString>("str1");
        QTest::addColumn<HString>("str2");
        QTest::addColumn<bool>("issameornot");

        QTest::newRow("1") << HString("alpha") << HString("alpha") << true;
        QTest::newRow("2") << HString("alpha") << HString("beta") << false;
        QTest::newRow("3") << HString("") << HString("") << true;
        QTest::newRow("4") << HString("") << HString(" ") << false;
        QTest::newRow("5") << HString("alpha") << HString(" ") << false;
    }

    void HString_StartsWith()
    {
        QFETCH(HString, String);
        QFETCH(HString, testString);
        QFETCH(bool, Result);
        HString failmsg("Failure! startsWith produced wrong result for "+String+", "+testString);
        QVERIFY2(String.startsWith(testString) == Result, failmsg.c_str());

    }
    void HString_StartsWith_data()
    {
        QTest::addColumn<HString>("String");
        QTest::addColumn<HString>("testString");
        QTest::addColumn<bool>("Result");
        QTest::newRow("hopsan,hop") << HString("hopsan") << HString("hop") << true;
        QTest::newRow("hopsan,san") << HString("hopsan") << HString("san") << false;
        QTest::newRow("empty,empty") << HString("") << HString("") << true;
        QTest::newRow("hopsan,empty") << HString("hopsan") << HString("") << true;
        QTest::newRow("empty,hopsan") << HString("") << HString("hopsan") << false;
    }
};

QTEST_APPLESS_MAIN(HStringTests)

#include "tst_hstringtest.moc"
