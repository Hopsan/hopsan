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
#include <QVector>
#include <QFileInfoList>

#include "HopsanEssentials.h"
#include "HopsanCoreVersion.h"
#include "compiler_info.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "hopsangenerator.h"
#include "GeneratorTypes.h"
#include <assert.h>
#include <iostream>

#ifndef DEFAULT_LIBRARY_ROOT
#define DEFAULT_LIBRARY_ROOT "../componentLibraries/defaultLibrary"
#endif

#ifndef EXTERNAL_LIBRARIES_ROOT
#define EXTERNAL_LIBRARIES_ROOT "../componentLibraries"
#endif

#ifndef TEST_DATA_ROOT
#define TEST_DATA_ROOT "../UnitTests/GeneratorTest"
#endif

#ifndef HOPSAN_INSTALL_ROOT
#define HOPSAN_INSTALL_ROOT ".."
#endif

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE SHAREDLIB_PREFIX "defaultcomponentlibrary" HOPSAN_DEBUG_POSTFIX "." SHAREDLIB_SUFFIX
const std::string defaultLibraryFilePath = DEFAULT_LIBRARY_ROOT "/" DEFAULTLIBFILE;
#else
const std::string defaultLibraryFilePath = "";
#endif

constexpr bool gAllwaysShowMessages = false;
void generatorMessageCallback(const char* msg, const char type, void* pObject);

void removeDir(QString path)
{
    QDir dir;
    dir.setPath(path);
    for(QFileInfo info : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (info.isDir()) {
            removeDir(info.absoluteFilePath());
        }
        else {
            QFile::remove(info.absoluteFilePath());
        }
    }
    dir.rmdir(path);
}

QString getLibFile(const ComponentLibrary& cl) {
    QFileInfo libXmlFi(cl.mLoadFilePath);
    QString libfile = QString("%1/%2%3").arg(libXmlFi.absolutePath()).arg(SHAREDLIB_PREFIX).arg(cl.mSharedLibraryName);
#ifdef HOPSAN_BUILD_TYPE_DEBUG
    libfile.append(cl.mSharedLibraryDebugExtension);
#endif
    libfile.append("." SHAREDLIB_SUFFIX);
    return libfile;
}


using namespace hopsan;

Q_DECLARE_METATYPE(bool)
Q_DECLARE_METATYPE(ComponentSystem*)
Q_DECLARE_METATYPE(Component*)
Q_DECLARE_METATYPE(Port*)
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(Node*)
Q_DECLARE_METATYPE(HString)


class GeneratorTests : public QObject
{
    Q_OBJECT

public:

    const std::string& compilerPathForThisArch() const
    {
#ifdef HOPSANCOMPILED64BIT
        return gcc64Path;
#else
        return gcc32Path;
#endif
    }

    void addMessage(const QString& message)
    {
        messages.append(message);
    }

    void printMessages() const
    {
        std::cout << qPrintable(messages.join("\n")) << std::endl;
    }

    void clearMessages()
    {
        messages.clear();
    }

    void printCoreMessages()
    {
        while (mHopsanCore.getCoreMessageHandler()->getNumWaitingMessages() > 0) {
            HString message, type, tag;
            mHopsanCore.getCoreMessageHandler()->getMessage(message, type, tag);
            std::cout << type.c_str() << ": " << message.c_str() << std::endl;
        }
    }

 private:
    QString qcwd;
    std::string cwd;
    std::string mHopsanInstallRoot;
    QString mTestDataRoot;

    std::string gcc32Path;
    std::string gcc64Path;

    QStringList messages;

    HopsanEssentials mHopsanCore;

private slots:
    void initTestCase()
    {
        qcwd = QDir::currentPath();
        cwd = qcwd.toStdString();
        if (QDir::isRelativePath(HOPSAN_INSTALL_ROOT)) {
            mHopsanInstallRoot = cwd + "/" HOPSAN_INSTALL_ROOT;
        }
        else {
            mHopsanInstallRoot = HOPSAN_INSTALL_ROOT;
        }

        if (QDir::isRelativePath(TEST_DATA_ROOT)) {
            mTestDataRoot = qcwd + "/" TEST_DATA_ROOT;
        }
        else {
            mTestDataRoot = TEST_DATA_ROOT;
        }



#ifdef _WIN32
        //! @todo do not hard-code these
        gcc32Path="c:/mingw/bin";
        gcc64Path="c:/mingw64/bin";
#else
        gcc32Path="";
        gcc64Path="";
#endif

        bool did_load = mHopsanCore.loadExternalComponentLib(defaultLibraryFilePath.c_str());
        if (!did_load) {
            printCoreMessages();
        }
        QVERIFY2(did_load, "Could not load default component library");
    }

    void init()
    {
        clearMessages();
    }

    void Generator_LibraryImport()
    {
        QFETCH(QString, libraryPath);

        const std::string gccPath = compilerPathForThisArch();
        constexpr auto cflags = "";
        constexpr auto lflags = "";

        bool compileOK = callComponentLibraryCompiler(qPrintable(libraryPath), cflags, lflags, mHopsanInstallRoot.c_str(), gccPath.c_str(), &generatorMessageCallback,
                                                      this);
        if (!compileOK) {
            printMessages();
        }
        QVERIFY2(compileOK, qPrintable(QString("Could not compile component library: %1").arg(libraryPath)));

        ComponentLibrary cl;
        cl.loadFromXML(libraryPath);
        QString libfile = getLibFile(cl);
        //! @todo The core should be able to load xml
        bool loadOK = mHopsanCore.loadExternalComponentLib(qPrintable(libfile));
        if (!loadOK) {
            printCoreMessages();
        }
        QVERIFY2(loadOK, qPrintable(QString("Could not load component library: %1").arg(libraryPath)));
    }

    void Generator_LibraryImport_data()
    {
        QTest::addColumn<QString>("libraryPath");
        const QString externalLibsPath = EXTERNAL_LIBRARIES_ROOT;

        QFileInfoList libs;
        libs.append(externalLibsPath+"/exampleComponentLib/exampleComponentLib.xml");
        libs.append(externalLibsPath+"/extensionLibrary/extensionLibrary.xml");

        for (auto& lib : libs) {
            QVERIFY2(lib.exists(), qPrintable(QString("File %1 does not exist").arg(lib.filePath())));
            QTest::newRow(qPrintable(lib.fileName())) << lib.absoluteFilePath();
        }
    }

    void Generator_FMU_Export()
    {
        QFETCH(ComponentSystem*, system);
        QFETCH(double, modelstoptime);
#if defined(__APPLE__)
        QWARN("Generator FMU tests are disbaled on MacOS, until generator code works there");
#else
        const QString fmuCheckPath=QDir::cleanPath(QString::fromStdString(mHopsanInstallRoot+"/dependencies/tools/FMUChecker"));

#ifdef _WIN32
        QString fmuChecker32 = QString("%1/%2").arg(fmuCheckPath).arg("fmuCheck.win32.exe");
        QString fmuChecker64 = QString("%1/%2").arg(fmuCheckPath).arg("fmuCheck.win64.exe");
#else
        QString fmuChecker32 = QString("%1/%2").arg(fmuCheckPath).arg("fmuCheck.linux32");
        QString fmuChecker64 = QString("%1/%2").arg(fmuCheckPath).arg("fmuCheck.linux64");
#endif

        QStringList args;
        QProcess p;

        QString testStopTime = QString::number(modelstoptime*2);

        std::vector<char*> externalLibraries;
        constexpr int numExternalLibraries = 0;

#if !defined(HOPSANCOMPILED64BIT)
        // Run FMUChecker for FMU 1.0 32-bit export
        std::string outpath = cwd+"/fmu1 32/";
        bool exportOK = callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, mHopsanInstallRoot.c_str(),  gcc32Path.c_str(),
                                               1, 32, &generatorMessageCallback, this);
        if (!exportOK) {
            printMessages();
        }

        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-e" << "log.txt";
        args << "-o" << "results.txt";
        args << qcwd+"/fmu1 32/unittestmodel_export.fmu";
        p.start(fmuChecker32, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 1.0 (32-bit), FMUChecker crashed");
        if(p.exitCode() != 0) {
            QFile f("log.txt");

            if(f.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&f);
                std::cout << in.readAll().toStdString().c_str() << "\n";
            }
        }
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 1.0 (32-bit), FMU not accepted by FMUChecker.");

        // Run FMUChecker for FMU 2.0 32-bit export
        clearMessages();
        outpath = cwd+"/fmu2 32/";
        exportOK = callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, mHopsanInstallRoot.c_str(),  gcc32Path.c_str(),
                                          2, 32, &generatorMessageCallback, this);
        if (!exportOK) {
            printMessages();
        }

        QVERIFY2(QFile::exists(fmuChecker32), qPrintable(QString("FMUChecker is not installed in the expected location: %1").arg(fmuChecker32)));

        args.clear();
        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-e" << "log.txt";
        args << "-o" << "results.txt";
        args << qcwd+"/fmu2 32/unittestmodel_export.fmu";
        p.start(fmuChecker32, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 2.0 (32-bit), FMUChecker crashed");
        if(p.exitCode() != 0) {
            QFile f("log.txt");

            if(f.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&f);
                std::cout << in.readAll().toStdString().c_str() << "\n";
            }
        }
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 2.0 (32-bit), FMU not accepted by FMUChecker.");
#endif

#if defined (HOPSANCOMPILED64BIT)
        // Run FMUChecker for FMU 1.0 64-bit export
        std::string outpath = cwd+"/fmu1 64/";
        bool exportOK = callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, mHopsanInstallRoot.c_str(),  gcc64Path.c_str(),
                                               1, 64, &generatorMessageCallback, this);
        if (!exportOK) {
            printMessages();
        }

        QVERIFY2(QFile::exists(fmuChecker64), qPrintable(QString("FMUChecker is not installed in the expected location: %1").arg(fmuChecker64)));

        args.clear();
        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-e" << "log.txt";
        args << "-o" << "results.txt";
        args << qcwd+"/fmu1 64/unittestmodel_export.fmu";
        p.start(fmuChecker64, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 1.0 (64-bit), FMUChecker crashed");
        if(p.exitCode() != 0) {
            QFile f("log.txt");

            if(f.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&f);
                std::cout << in.readAll().toStdString().c_str() << "\n";
            }
        }
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 1.0 (64-bit), FMU not accepted by FMUChecker.");

        // Run FMUChecker for FMU 2.0 64-bit export
        clearMessages();
        outpath = cwd+"/fmu2 64/";
        exportOK = callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, mHopsanInstallRoot.c_str(),  gcc64Path.c_str(),
                                          2, 64, &generatorMessageCallback, this);
        if (!exportOK) {
            printMessages();
        }

        args.clear();
        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-e" << "log.txt";
        args << "-o" << "results.txt";
        args << qcwd+"/fmu2 64/unittestmodel_export.fmu";
        p.start(fmuChecker64, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 2.0 (64-bit), FMUChecker crashed");
        if(p.exitCode() != 0) {
            QFile f("log.txt");

            if(f.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&f);
                std::cout << in.readAll().toStdString().c_str() << "\n";
            }
        }
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 2.0 (64-bit), FMU not accepted by FMUChecker.");
#endif
#endif
    }

    void Generator_FMU_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::addColumn<double>("modelstoptime");
        QString modelpath = mTestDataRoot + "/unittestmodel_export.hmf";
        QFile file(modelpath);

#if !defined (HOPSANCOMPILED64BIT)
        removeDir(QDir::currentPath()+"/fmu1 32/");
        removeDir(QDir::currentPath()+"/fmu2 32/");
        QDir().mkpath(QDir::currentPath()+"/fmu1 32/");
        QDir().mkpath(QDir::currentPath()+"/fmu2 32/");
        file.copy(QDir::currentPath()+"/fmu1 32/unittestmodel_export.hmf");
        file.copy(QDir::currentPath()+"/fmu2 32/unittestmodel_export.hmf");
#endif

#if defined (HOPSANCOMPILED64BIT)
        removeDir(QDir::currentPath()+"/fmu1 64/");
        removeDir(QDir::currentPath()+"/fmu2 64/");
        QDir().mkpath(QDir::currentPath()+"/fmu1 64/");
        QDir().mkpath(QDir::currentPath()+"/fmu2 64/");
        file.copy(QDir::currentPath()+"/fmu1 64/unittestmodel_export.hmf");
        file.copy(QDir::currentPath()+"/fmu2 64/unittestmodel_export.hmf");
#endif

        double start, stop;
        QTest::newRow("0") << mHopsanCore.loadHMFModelFile(modelpath.toStdString().c_str(),start,stop) << stop;
    }

    void Generator_FMU_Import()
    {
        ComponentSystem * pSystem = mHopsanCore.createComponentSystem();
        Component *pFmuComponent = mHopsanCore.createComponent("FMIWrapper");
        pSystem->addComponent(pFmuComponent);

        QFETCH(HString, fmuPath);
        pFmuComponent->setParameterValue("path", fmuPath, true);
        QVERIFY2(pFmuComponent->hasParameter("loggingOn"), "Failed to import FMU");
        Component *pSourceComponent = mHopsanCore.createComponent("SignalSineWave");
        pSystem->addComponent(pSourceComponent);
        Port *pPort1 = pSourceComponent->getPort("out");
        Port *pPort2 = pFmuComponent->getPort("in1");
        QVERIFY2(pPort2 != nullptr, "Input port is missing from imported FMU");
        Port *pPort3 = pFmuComponent->getPort("out1");
        QVERIFY2(pPort3 != nullptr, "Output port is missing from imported FMU");
        pSystem->connect(pPort1, pPort2);
        pSystem->initialize(0, 100);
        pSystem->simulate(0.01);
        std::vector<double> *pInData = pPort1->getDataVectorPtr();
        std::vector<double> *pOutData = pPort3->getDataVectorPtr();
        bool ok = true;
        for(size_t i=0; i<pInData->size(); ++i) {
            if(pInData[i] != pOutData[i]) {
                ok = false;
            }
        }
        QVERIFY2(ok, "Simulation result from imported FMU is not correct");
    }

    void Generator_FMU_Import_data() {
        QTest::addColumn<HString>("fmuPath");

#if defined (HOPSANCOMPILED64BIT)
        QTest::newRow("0") << HString(QDir::currentPath().toStdString().c_str())+"/fmu1 64/unittestmodel_export.fmu";
        QTest::newRow("1") << HString(QDir::currentPath().toStdString().c_str())+"/fmu2 64/unittestmodel_export.fmu";
#else
        QTest::newRow("0") << HString(QDir::currentPath().toStdString().c_str())+"/fmu1 32/unittestmodel_export.fmu";
        QTest::newRow("1") << HString(QDir::currentPath().toStdString().c_str())+"/fmu2 32/unittestmodel_export.fmu";
#endif
    }

    void Generator_Simulink_Export()
    {
        QFETCH(ComponentSystem*, system);

        std::vector<char*> externalLibraries;
        constexpr int numExternalLibraries = 0;

        //Generate S-function
        std::string outpath = cwd+"/simulink/";
        bool exportOK = callSimulinkExportGenerator(outpath.c_str(), "unittestmodel_export.hmf", system, externalLibraries.data(), numExternalLibraries, false,
                                    mHopsanInstallRoot.c_str(), &generatorMessageCallback, this);
        if (!exportOK) {
            printMessages();
        }

        QDir coreDir(qcwd+"/simulink/HopsanCore");
        QVERIFY2(coreDir.exists() && !coreDir.entryList().isEmpty(),
                 "Failed to generate S-function, all files not found.");
        QDir compDir(qcwd+"/simulink/componentLibraries/defaultLibrary");
        QVERIFY2(compDir.exists() && !compDir.entryList().isEmpty(),
                 "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/simulink/"+system->getName().c_str()+".cpp"),
                 "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/simulink/HopsanSimulinkCompile.m"),
                 "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/simulink/"+system->getName().c_str()+"MaskSetup.m"),
                 "Failed to generate S-function, all files not found.");
    }

    void Generator_Simulink_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        double start, stop;
        QString path = mTestDataRoot + "/unittestmodel_export.hmf";
        removeDir(QDir::currentPath()+"/simulink/");
        QDir().mkpath(QDir::currentPath()+"/simulink/");
        QFile file(path);
        file.copy(QDir::currentPath()+"/simulink/unittestmodel_export.hmf");
        QTest::newRow("0") << mHopsanCore.loadHMFModelFile(path.toStdString().c_str(),start,stop);
    }

    void Generator_Labview_Export()
    {
        QFETCH(ComponentSystem*, system);

        //Generate S-function
        std::string outfile = cwd+"/labview/unittestmodel_export.cpp";
        bool exportOK = callLabViewSITGenerator(outfile.c_str(), system, mHopsanInstallRoot.c_str(), &generatorMessageCallback, this);
        if (!exportOK) {
            printMessages();
        }

        QVERIFY2(QFile::exists(qcwd+"/labview/codegen.c"),
                 "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/labview/hopsanrt-wrapper.h"),
                 "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/labview/HOW_TO_COMPILE.txt"),
                 "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/labview/model.h"),
                 "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/labview/SIT_API.h"),
                 "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(qcwd+"/labview/unittestmodel_export.cpp"),
                 "Failed to generate LabVIEW files, all files not found.");

        QDir includeDir(qcwd+"/labview/HopsanCore/include");
        QVERIFY2(includeDir.exists() && !includeDir.entryList().isEmpty(),
                 "Failed to generate LabVIEW files: Include files not found.");
        QDir srcDir(qcwd+"/labview/HopsanCore/src");
        QVERIFY2(srcDir.exists() && !srcDir.entryList().isEmpty(),
                 "Failed to generate LabVIEW files: Source files not found.");
        QDir dependenciesDir(qcwd+"/labview/HopsanCore/dependencies");
        QVERIFY2(dependenciesDir.exists() && !dependenciesDir.entryList(QDir::AllEntries).isEmpty(),
                 "Failed to generate LabVIEW files: dependency files not found.");
        QDir libDir(qcwd+"/labview/componentLibraries/defaultLibrary");
        QVERIFY2(libDir.exists() && !libDir.entryList().isEmpty(),
                 "Failed to generate LabVIEW files: Default library files not found.");
    }

    void Generator_Labview_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        double start, stop;
        QString path = mTestDataRoot + "/unittestmodel_export.hmf";
        removeDir(QDir::currentPath()+"/labview/");
        QDir().mkpath(QDir::currentPath()+"/labview/");
        QTest::newRow("0") << mHopsanCore.loadHMFModelFile(path.toStdString().c_str(),start,stop);
    }

    void Generator_Modelica()
    {
#if defined(__APPLE__)
        QWARN("Generator Modelica tests are disbaled on MacOS, until generator code works there");
#else
        QFETCH(std::string, code);
        QFETCH(std::string, name);

        //Generate FMU
        QFile moFile(qcwd+"/modelica/modelica.mo");
        moFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
        moFile.write(QString(code.c_str()).toUtf8());
        moFile.close();

        std::string moFilePath = QFileInfo(moFile).absoluteFilePath().toStdString();
        std::string gccPath;
#ifdef HOPSANCOMPILED64BIT
        gccPath = gcc64Path;
#else
        gccPath = gcc32Path;
#endif
        bool success = callModelicaGenerator(moFilePath.c_str(), gccPath.c_str(), &generatorMessageCallback, this, true, mHopsanInstallRoot.c_str());

        if(!success) {
            printMessages();
        }

        QVERIFY2(success, "Failure! Failed to translate Modelica to C++.");

        std::string xmlFile = cwd+"/modelica/modelica_lib.xml";
        std::string lflags, cflags;
        success = callComponentLibraryCompiler(xmlFile.c_str(),lflags.c_str(),cflags.c_str(),mHopsanInstallRoot.c_str(),gccPath.c_str(),&generatorMessageCallback, this);

        if(!success) {
            printMessages();
        }

        QVERIFY2(success, "Failure! Failed to compile Modelica library.");

        std::string libfile=cwd+"/modelica/";
        #ifndef _WIN32
            libfile.append("lib");
        #endif
            libfile.append("modelica");
        #ifdef HOPSAN_BUILD_TYPE_DEBUG
            libfile.append("_d");
        #endif
        libfile.append("." SHAREDLIB_SUFFIX);

        QVERIFY2(QDir().exists(libfile.c_str()),
                 std::string("Failure! Modelica generator failed to generate .dll/.so: "+libfile).c_str());
#endif
    }

    void Generator_Modelica_data()
    {
        QTest::addColumn<std::string>("code");
        QTest::addColumn<std::string>("name");

        const char* moCode = "model TestLaminarOrifice \"Hydraulic Laminar Orifice\"\n"
              "   annotation(hopsanCqsType = \"Q\", linearTransform=\"bdf2\");\n"
              "   parameter Real Kc(unit=\"-\")=1e-11 \"Pressure-Flow Coefficient\";\n"
              "   NodeHydraulic P1, P2;\n"
              "equation\n"
              "   P2.q = Kc*(P1.p-P2.p);\n"
              "   P1.q = -P2.q;\n"
              "   P1.p = P1.c + P1.Zc*P1.q;\n"
              "   P2.p = P2.c + P2.Zc*P2.q;\n"
              "end TestLaminarOrifice;\n";

        removeDir(QDir::currentPath()+"/modelica");
        QDir().mkpath(QDir::currentPath()+"/modelica");

        QTest::newRow("0") << std::string(moCode) << std::string("MyLaminarOrifice");
    }

    void Generator_Exe_Export()
    {
        QFETCH(ComponentSystem*, system);
#if defined(__APPLE__)
        QWARN("Generator FMU tests are disbaled on MacOS, until generator code works there");
#else

        QString suffix;
        QString outDir = "/exe 32";
        std::string compilerPath = gcc32Path;
        int ai32_64 = 32;
#if defined (HOPSANCOMPILED64BIT)
        outDir =  "/exe 64";
        compilerPath = gcc64Path;
        ai32_64 = 64;
#endif

#if defined(_WIN32)
        suffix = ".exe";
#endif

        std::string outpath = cwd+outDir.toStdString();
        std::vector<char*> externalLibraries;
        constexpr int numExternalLibraries = 0;

        bool exportOK = callExeExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, mHopsanInstallRoot.c_str(),  gcc32Path.c_str(), ai32_64, &generatorMessageCallback, this);
        if (!exportOK) {
            printMessages();
        }

        QStringList args;
        QProcess p;

        args << "-s";
        p.setWorkingDirectory(outpath.c_str());
        p.start(QString::fromStdString(outpath)+"/unittestmodel_export"+suffix, args);
        p.waitForFinished();

        if (p.exitCode() != 0) {
            std::cout << "stdout: " << std::endl << QString(p.readAllStandardOutput()).toStdString() << std::endl;
            std::cout << "stderr: " << std::endl << QString(p.readAllStandardError()).toStdString() << std::endl;
        }

        QVERIFY2(p.exitStatus() == QProcess::NormalExit, "The generated EXE crashed");
        QVERIFY2(p.exitCode() == 0, "The generated EXE failed simulation.");
#endif
    }

    void Generator_Exe_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QString originalModelPath=mTestDataRoot+"/unittestmodel_export.hmf";
        QFile originalModelFile(originalModelPath);

        QString outPath = qcwd+"/exe 32";
#if defined (HOPSANCOMPILED64BIT)
        outPath = qcwd+"/exe 64";
#endif

        removeDir(outPath);
        QDir().mkpath(outPath);
        originalModelFile.copy(outPath+"/unittestmodel_export.hmf");

        double start, stop;
        QTest::newRow("0") << mHopsanCore.loadHMFModelFile(originalModelPath.toStdString().c_str(), start, stop);
    }

    void examineCode(QString code, QStringList &errors)
    {
        QStringList lines = code.split("\n");
        bool includesOk=false;
        bool nameSpaceOk=false;
        bool inClass=false;
        bool inPublic=false;
        bool inPrivate=false;
        bool publicMembers = false;
        bool inFunction=false;
        QString name, cqsType, funcName;
        QStringList memberNames, memberTypes;
        QList<bool> assigned, used, assignedBeforeUse;
        int bal=0;
        QStringList initLines, simLines;
        for(int i=0; i<lines.size(); ++i)
        {
            QString l = lines[i].simplified();

            if(!inClass && l == "#include \"ComponentEssentials.h\"") { includesOk = true; }
            if(!inClass && l.startsWith("namespace hopsan {")) { nameSpaceOk = true; }
            if(l.startsWith("class ") && l.contains(" : public Component"))
            {
                inClass = true;
                name = l.section(" ",1,1);
                cqsType = l.section(" : public Component",1,1);
            }
            if(inClass && "public:") inPublic=true;
            if(inClass && l == "private:") inPrivate=true;
            if(!inFunction && inClass && (inPublic || inPrivate) && l.count(" ") == 1 && l.endsWith(";"))
            {
                memberTypes.append(l.section(" ",0,0));
                memberNames.append(l.section(" ",1,1).remove(";"));
                assigned.append(false);
                used.append(false);
                assignedBeforeUse.append(true);
                if(inPublic) publicMembers = true;
            }
            if((inPublic || inPrivate) && l.count("(") == 1 && l.count(")") == 1)
            {
                inFunction = true;
                funcName = l.section(" ",1,1).section("(",0,0);
            }
            if(l.contains("{"))
            {
                bal = bal+l.count("{");
            }
            if(l.contains("}"))
            {
                bal = bal-l.count("}");
                if(bal==2)
                {
                    inFunction=false;
                    funcName="";
                }
                if(bal==1)
                {
                    inClass = false;
                    inPrivate = false;
                    inPublic = false;
                }
            }
            if(inFunction && funcName == "initialize") initLines.append(l);
            if(inFunction && funcName == "simulateOneTimestep") simLines.append(l);
        }
        initLines.removeAt(0);
        initLines.removeAt(0);
        simLines.removeAt(0);
        simLines.removeAt(0);

        QStringList initAndSimulate = initLines;
        initAndSimulate.append(simLines);

        for(int i=0; i<initAndSimulate.size(); ++i)
        {
            QString l = initAndSimulate[i];

            if(l.count("=") == 1)
            {
                QString name = l.section("=",0,0).trimmed();
                if(memberNames.contains(name))
                    assigned[memberNames.indexOf(name)] = true;
            }
            l.remove(0,l.indexOf("="));
            for(int j=0; j<l.size(); ++j)
            {
                if(!l[j].isLetterOrNumber())
                {
                    l[j] = ' ';
                }
            }
            QStringList splitLine = l.split(" ");
            for(int j=0; j<memberNames.size(); ++j)
            {
                if(splitLine.contains(memberNames[j]))
                {
                    used[j] = true;
                    if(!assigned[j])
                    {
                        assignedBeforeUse[j] = false;
                    }
                }
            }
        }

        if(!includesOk)
            errors.append("WARNING: ComponentEssentials.h is not included!");
        if(!nameSpaceOk)
            errors.append("WARNING: \"hopsan\" namespace is not used!");
        if(publicMembers)
            errors.append("WARNING: Public member variables are not recommended!");
        for(int i=0; i<used.size(); ++i)
        {
            if(!used[i])
                errors.append("WARNING: Unused member variable \""+memberNames[i]+"\"!");
            if(!assignedBeforeUse[i])
                errors.append("WARNING: Member variable \""+memberNames[i]+"\" is used uninitialized!");
        }
    }
};

void generatorMessageCallback(const char* msg, const char type, void* pObject)
{
    auto message = QString("%1: %2").arg(type).arg(msg);
    if (pObject != nullptr) {
        auto pTestObject = static_cast<GeneratorTests*>(pObject);
        pTestObject->addMessage(message);
    }
    if (gAllwaysShowMessages) {
        std::cout << qPrintable(message) << std::endl;
    }
}


QTEST_APPLESS_MAIN(GeneratorTests)

#include "tst_generatortest.moc"
