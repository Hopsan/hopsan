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

#include "CoreUtilities/HmfLoader.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"
#include "CoreUtilities/MultiThreadingUtilities.h"

using namespace hopsan;

Q_DECLARE_METATYPE(HString)

class UtilitiesTestTest : public QObject
{
    Q_OBJECT

public:
    UtilitiesTestTest()
    {
        mpMsgHandler = new HopsanCoreMessageHandler();
    }

private:
    HopsanCoreMessageHandler *mpMsgHandler;
    std::map<HString, int> mUniqueNamesDataMap;

private Q_SLOTS:
    void Version_Check()
    {
        QFETCH(QString, version1);
        QFETCH(QString, version2);
        QFETCH(bool, ans);
        bool test = isVersionAGreaterThanB(version1.toStdString().c_str(), version2.toStdString().c_str());
        QVERIFY2(test == ans, "Version check returned wrong answer.");
    }

    void Version_Check_data()
    {
        QTest::addColumn<QString>("version1");
        QTest::addColumn<QString>("version2");
        QTest::addColumn<bool>("ans");
        QTest::newRow("0") << "0.6.7" << "0.6.6" << true;
        QTest::newRow("1") << "0.6.7" << "0.6.8" << false;
        QTest::newRow("2") << "0.7.7" << "0.6.7" << true;
        QTest::newRow("3") << "0.7.7" << "0.8.7" << false;
        QTest::newRow("4") << "0.6.x_r7236" << "0.6.6" << true;
        QTest::newRow("5") << "0.6.6" << "0.6.x_r7236" << false;
        QTest::newRow("6") << "0.6.7" << "0.6.7" << false;
        QTest::newRow("7") << "0.6.x_r7236" << "0.6.x_r7236" << false;
        QTest::newRow("8") << "0.6.7a" << "0.6.7" << true;
        QTest::newRow("9") << "0.6.7b" << "0.6.7a" << true;
        QTest::newRow("10") << "0.6.7a" << "0.6.6" << true;
        QTest::newRow("11") << "0.6.7a" << "0.6.8" << false;
        QTest::newRow("12") << "0.7.0" << "0.6.10" << true;
        QTest::newRow("13") << "0.7.10" << "0.6.10" << true;
        QTest::newRow("14") << "0.6.11" << "0.6.9" << true;

    }


    void Message_Handler()
    {
        QFETCH(HString, msg);
        QFETCH(HString, tag);
        QFETCH(HString, type);
        QFETCH(int, num);
        QFETCH(int, num_err);
        QFETCH(int, num_warning);
        QFETCH(int, num_info);
        QFETCH(int, num_debug);

        QVERIFY2(mpMsgHandler->getNumWaitingMessages() == size_t(num), "Wrong number of waiting messages!");
        QVERIFY2(mpMsgHandler->getNumErrorMessages() == size_t(num_err), "Wrong number of waiting error messages!");
        QVERIFY2(mpMsgHandler->getNumWarningMessages() == size_t(num_warning), "Wrong number of waiting warning messages!");
        QVERIFY2(mpMsgHandler->getNumInfoMessages() == size_t(num_info), "Wrong number of waiting info messages!");
        QVERIFY2(mpMsgHandler->getNumDebugMessages() == size_t(num_debug), "Wrong number of waiting debug messages!");

        HString msg_out, type_out, tag_out;
        mpMsgHandler->getMessage(msg_out, type_out, tag_out);
        QVERIFY2(msg_out == msg, "Message handler returned wrong text!");
        QVERIFY2(tag_out == tag, "Message handler returned wrong tag!");
        QVERIFY2(type_out == type, "Message handler returned wrong type!");
    }

    void Message_Handler_data()
    {
        QTest::addColumn<HString>("msg");
        QTest::addColumn<HString>("tag");
        QTest::addColumn<HString>("type");
        QTest::addColumn<int>("num");
        QTest::addColumn<int>("num_err");
        QTest::addColumn<int>("num_warning");
        QTest::addColumn<int>("num_info");
        QTest::addColumn<int>("num_debug");

        mpMsgHandler->addErrorMessage("Testing error message!");
        mpMsgHandler->addErrorMessage("Testing error message with tag!", "errortag");
        mpMsgHandler->addWarningMessage("Testing warning message!");
        mpMsgHandler->addWarningMessage("Testing warning message with tag!", "warningtag");
        mpMsgHandler->addInfoMessage("Testing info message!");
        mpMsgHandler->addInfoMessage("Testing info message with tag!", "infotag");
        mpMsgHandler->addDebugMessage("Testing debug message!");
        mpMsgHandler->addDebugMessage("Testing debug message with tag!", "debugtag");

        QTest::newRow("0") << HString("Error: Testing error message!") << HString("") << HString("error") << 8 << 2 << 2 << 2 << 2;
        QTest::newRow("1") << HString("Error: Testing error message with tag!") << HString("errortag") << HString("error") << 7 << 1 << 2 << 2 << 2;
        QTest::newRow("2") << HString("Warning: Testing warning message!") << HString("") << HString("warning") << 6 << 0 << 2 << 2 << 2;
        QTest::newRow("3") << HString("Warning: Testing warning message with tag!") << HString("warningtag") << HString("warning") << 5 << 0 << 1 << 2 << 2;
        QTest::newRow("4") << HString("Info: Testing info message!") << HString("") << HString("info") << 4 << 0 << 0 << 2 << 2;
        QTest::newRow("5") << HString("Info: Testing info message with tag!") << HString("infotag") << HString("info") << 3 << 0 << 0 << 1 << 2;
        QTest::newRow("6") << HString("Debug: Testing debug message!") << HString("") << HString("debug") << 2 << 0 << 0 << 0 << 2;
        QTest::newRow("7") << HString("Debug: Testing debug message with tag!") << HString("debugtag") << HString("debug") << 1 << 0 << 0 << 0 << 1;
    }

    void Sanitize_Name()
    {
        QFETCH(HString, str1);
        QFETCH(HString, str2);

        santizeName(str1);
        QVERIFY2(str1 == str2, "sanitizeName() returned wrong string!");
    }

    void Sanitize_Name_data()
    {
        QTest::addColumn<HString>("str1");
        QTest::addColumn<HString>("str2");

        QTest::newRow("0") << HString("Weird & #great test-string?") << HString("Weird____great_test_string_");
        QTest::newRow("1") << HString("Not_so_weird_String") << HString("Not_so_weird_String");
    }

    void Is_Name_valid()
    {
        QFETCH(HString, str);
        QFETCH(HString, exceptions);
        QFETCH(bool, valid);

        QVERIFY2(isNameValid(str, exceptions) == valid, "isNameValid() returned wrong value!");
    }

    void Is_Name_valid_data()
    {
        QTest::addColumn<HString>("str");
        QTest::addColumn<HString>("exceptions");
        QTest::addColumn<bool>("valid");

        QTest::newRow("0") << HString("Invalid & #weird test-string?") << HString("") << false;
        QTest::newRow("1") << HString("Invalid & #weird test-string?") << HString("& #-?") << true;
        QTest::newRow("2") << HString("Valid_test_string") << HString("") << true;
    }

    void Find_Unique_Name()
    {
        QFETCH(HString, str1);
        QFETCH(HString, str2);

        HString newName = findUniqueName(mUniqueNamesDataMap, str1);
        mUniqueNamesDataMap.insert(std::pair<HString,int>(newName,0));

        QVERIFY2(newName == str2, "findUniqueName generated wrong name!");
    }

    void Find_Unique_Name_data()
    {
        QTest::addColumn<HString>("str1");
        QTest::addColumn<HString>("str2");

        mUniqueNamesDataMap.insert(std::pair<HString,int>("apa",0));
        mUniqueNamesDataMap.insert(std::pair<HString,int>("katt",0));
        mUniqueNamesDataMap.insert(std::pair<HString,int>("ko",0));

        QTest::newRow("0") << HString("kanin") << HString("kanin");
        QTest::newRow("1") << HString("kanin") << HString("kanin_1");
        QTest::newRow("2") << HString("ko") << HString("ko_1");
        QTest::newRow("3") << HString("ko_1") << HString("ko_2");
        QTest::newRow("4") << HString("ko") << HString("ko_3");
    }

    void Determine_Actual_Number_Of_Threads()
    {
        QFETCH(int,desired);

        QVERIFY2(determineActualNumberOfThreads(size_t(desired)) == (size_t)std::min(desired, QThread::idealThreadCount()), "determineActualNumberOfThreads() returned wrong value!");
    }

    void Determine_Actual_Number_Of_Threads_data()
    {
        QTest::addColumn<int>("desired");

        QTest::newRow("0") << 1;
        QTest::newRow("1") << 2;
        QTest::newRow("2") << 3;
        QTest::newRow("3") << 4;
        QTest::newRow("4") << 5;
        QTest::newRow("5") << 6;
        QTest::newRow("6") << 7;
        QTest::newRow("7") << 8;
        QTest::newRow("8") << 9;
    }
};
QTEST_APPLESS_MAIN(UtilitiesTestTest)

#include "tst_utilitiestesttest.moc"
