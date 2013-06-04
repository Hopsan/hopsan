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

    }
};

QTEST_APPLESS_MAIN(HStringTests)

#include "tst_hstringtest.moc"
