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
#include "HopsanEssentials.h"
#include "HopsanCoreMacros.h"
#include "compiler_info.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "hopsangenerator.h"
#include <assert.h>
#include <iostream>

#define DEFAULTLIBPATH "../componentLibraries/defaultLibrary"

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE TO_STR(DLL_PREFIX) "defaultcomponentlibrary" TO_STR(DEBUG_EXT) TO_STR(DLL_EXT)
const std::string defaultLibraryFilePath = DEFAULTLIBPATH "/" DEFAULTLIBFILE;
#else
const std::string defaultLibraryFilePath = "";
#endif

namespace {
    void removeDir(QString path)
    {
        QDir dir;
        dir.setPath(path);
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir())
            {
                removeDir(info.absoluteFilePath());
            }
            else
            {
                QFile::remove(info.absoluteFilePath());
            }
        }
        dir.rmdir(path);
    }

    constexpr bool showmessages = false;
    void generatorMessageCallback(const char* msg, const char type, void*)
    {
        if (showmessages) {
            std::cout << "Type: " << type << " Message: " << msg << std::endl;
        }
    }
}

using namespace hopsan;

Q_DECLARE_METATYPE(bool)
Q_DECLARE_METATYPE(ComponentSystem*)
Q_DECLARE_METATYPE(Component*)
Q_DECLARE_METATYPE(Port*)
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(Node*)


class GeneratorTests : public QObject
{
    Q_OBJECT

public:
    GeneratorTests()
    {
        qcwd = QDir::currentPath();
        cwd = qcwd.toStdString();
        hopsanRoot = cwd+"/../";

#ifdef _WIN32
        //! @todo do not hard-code these
        gcc32Path="c:/mingw/bin";
        gcc64Path="c:/mingw64/bin";
#else
        gcc32Path="";
        gcc64Path="";
#endif

        if (!defaultLibraryFilePath.empty())
        {
            mHopsanCore.loadExternalComponentLib(defaultLibraryFilePath.c_str());
        }
    }

private:
    QString qcwd;
    std::string cwd;
    std::string hopsanRoot;

    std::string gcc32Path;
    std::string gcc64Path;

    HopsanEssentials mHopsanCore;

private Q_SLOTS:
    void Generator_FMU_Export()
    {
        QFETCH(ComponentSystem*, system);
        QFETCH(double, modelstoptime);
#if defined(__APPLE__)
        QWARN("Generator FMU tests are disbaled on MacOS, until generator code works there");
#else
        const QString fmuCheckPath=QDir::cleanPath(qcwd+"/../Dependencies/tools/FMUChecker");

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
        std::string outpath = cwd+"/fmu1_32/";
        callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, hopsanRoot.c_str(),  gcc32Path.c_str(), 1, 32,
                               &generatorMessageCallback);

        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << qcwd+"/fmu1_32/unittestmodel_export.fmu";
        p.start(fmuChecker32, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 1.0 (32-bit), FMUChecker crashed");
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 1.0 (32-bit), FMU not accepted by FMUChecker.");

        // Run FMUChecker for FMU 2.0 32-bit export
        outpath = cwd+"/fmu2_32/";
        callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, hopsanRoot.c_str(),  gcc32Path.c_str(), 2, 32,
                               &generatorMessageCallback);

        args.clear();
        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << qcwd+"/fmu2_32/unittestmodel_export.fmu";
        p.start(fmuChecker32, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 2.0 (32-bit), FMUChecker crashed");
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 2.0 (32-bit), FMU not accepted by FMUChecker.");
#endif

#if defined (HOPSANCOMPILED64BIT)
        // Run FMUChecker for FMU 1.0 64-bit export
        std::string outpath = cwd+"/fmu1_64/";
        callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, hopsanRoot.c_str(),  gcc64Path.c_str(), 1, 64,
                               &generatorMessageCallback);

        args.clear();
        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << qcwd+"/fmu1_64/unittestmodel_export.fmu";
        p.start(fmuChecker64, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 1.0 (64-bit), FMUChecker crashed");
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 1.0 (64-bit), FMU not accepted by FMUChecker.");

        // Run FMUChecker for FMU 2.0 64-bit export
        outpath = cwd+"/fmu2_64/";
        callFmuExportGenerator(outpath.c_str(), system, externalLibraries.data(), numExternalLibraries, hopsanRoot.c_str(),  gcc64Path.c_str(), 2, 64,
                               &generatorMessageCallback);

        args.clear();
        args << "-s" << testStopTime;
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << qcwd+"/fmu2_64/unittestmodel_export.fmu";
        p.start(fmuChecker64, args);
        p.waitForFinished();

        QVERIFY2(p.exitStatus() == QProcess::NormalExit,
                 "Failed to generate valid FMU 2.0 (64-bit), FMUChecker crashed");
        QVERIFY2(p.exitCode() == 0,
                 "Failed to generate valid FMU 2.0 (64-bit), FMU not accepted by FMUChecker.");
#endif
#endif
    }

    void Generator_FMU_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        QTest::addColumn<double>("modelstoptime");
        QString modelpath=qcwd+"/../Models/unittestmodel_export.hmf";
        QFile file(modelpath);

#if !defined (HOPSANCOMPILED64BIT)
        removeDir(QDir::currentPath()+"/fmu1_32/");
        removeDir(QDir::currentPath()+"/fmu2_32/");
        QDir().mkpath(QDir::currentPath()+"/fmu1_32/");
        QDir().mkpath(QDir::currentPath()+"/fmu2_32/");
        file.copy(QDir::currentPath()+"/fmu1_32/unittestmodel_export.hmf");
        file.copy(QDir::currentPath()+"/fmu2_32/unittestmodel_export.hmf");
#endif

#if defined (HOPSANCOMPILED64BIT)
        removeDir(QDir::currentPath()+"/fmu1_64/");
        removeDir(QDir::currentPath()+"/fmu2_64/");
        QDir().mkpath(QDir::currentPath()+"/fmu1_64/");
        QDir().mkpath(QDir::currentPath()+"/fmu2_64/");
        file.copy(QDir::currentPath()+"/fmu1_64/unittestmodel_export.hmf");
        file.copy(QDir::currentPath()+"/fmu2_64/unittestmodel_export.hmf");
#endif

        double start, stop;
        QTest::newRow("0") << mHopsanCore.loadHMFModelFile(modelpath.toStdString().c_str(),start,stop) << stop;
    }

    void Generator_Simulink_Export()
    {
        QFETCH(ComponentSystem*, system);

        std::vector<char*> externalLibraries;
        constexpr int numExternalLibraries = 0;

        //Generate S-function
        std::string outpath = cwd+"/simulink/";
        callSimulinkExportGenerator(outpath.c_str(), "unittestmodel_export.hmf", system, externalLibraries.data(), numExternalLibraries, false,
                                    hopsanRoot.c_str(), &generatorMessageCallback);

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
        QString path = QDir::currentPath()+"/../Models/unittestmodel_export.hmf";
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
        callLabViewSITGenerator(outfile.c_str(), system, hopsanRoot.c_str(), &generatorMessageCallback);

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
        QString path = QDir::currentPath()+"/../Models/unittestmodel_export.hmf";
        removeDir(QDir::currentPath()+"/labview/");
        QDir().mkpath(QDir::currentPath()+"/labview/");
        QTest::newRow("0") << mHopsanCore.loadHMFModelFile(path.toStdString().c_str(),start,stop);
    }

    void Generator_Modelica()
    {
        QFETCH(std::string, code);
        QFETCH(std::string, name);

        //Generate FMU
        QFile moFile(qcwd+"/modelica/motest.mo");
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
        callModelicaGenerator(moFilePath.c_str(), gccPath.c_str(), &generatorMessageCallback, nullptr, 0, true, hopsanRoot.c_str());

//        QVERIFY2(QDir().exists((cwd+"/modelica/"+name+std::string(TO_STR(DLL_EXT))).c_str()),
//                 "Failure! Modelica generator failed to generate .dll/.so.");
        QWARN("Modelica generator test is disabled");
    }

    void Generator_Modelica_data()
    {
        QTest::addColumn<std::string>("code");
        QTest::addColumn<std::string>("name");

        const char* moCode = "model MyLaminarOrifice \"Hydraulic Laminar Orifice\"\n"
              "   annotation(hopsanCqsType = \"Q\");\n"
              "   parameter Real Kc(unit=\"-\")=1e-11 \"Pressure-Flow Coefficient\";\n"
              "   NodeHydraulic P1, P2;\n"
              "equation\n"
              "   P2.q = Kc*(P1.p-P2.p);\n"
              "   P1.q = -P2.q;\n"
              "   P1.p = P1.c + P1.Zc*P1.q;\n"
              "   P2.p = P2.c + P2.Zc*P2.q;\n"
              "end LaminarOrifice;\n";

        removeDir(QDir::currentPath()+"/modelica");
        QDir().mkpath(QDir::currentPath()+"/modelica");

        QTest::newRow("0") << std::string(moCode) << std::string("MyLaminarOrifice");
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

QTEST_APPLESS_MAIN(GeneratorTests)

#include "tst_generatortest.moc"
