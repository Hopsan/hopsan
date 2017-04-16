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

#include <QtTest>
#include "HopsanEssentials.h"
#include "HopsanCoreMacros.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/GeneratorHandler.h"
#include <assert.h>

#ifndef BUILTINDEFAULTCOMPONENTLIB
#define DEFAULTCOMPONENTLIB "../componentLibraries/defaultLibrary/" TO_STR(DLL_PREFIX) "defaultComponentLibrary" TO_STR(DEBUG_EXT) TO_STR(DLL_EXT)
#endif
#define LIBEXT TO_STR(DLL_EXT)

using namespace hopsan;

Q_DECLARE_METATYPE(bool)
Q_DECLARE_METATYPE(ComponentSystem*)
Q_DECLARE_METATYPE(Component*)
Q_DECLARE_METATYPE(Port*)
Q_DECLARE_METATYPE(HString)
Q_DECLARE_METATYPE(Node*)

class GeneratorTests : public QObject
{
    Q_OBJECT

public:
    GeneratorTests()
    {
#ifndef BUILTINDEFAULTCOMPONENTLIB
        mHopsanCore.loadExternalComponentLib(DEFAULTCOMPONENTLIB);
#endif
    }

private:
    HopsanEssentials mHopsanCore;
    ComponentSystem *mpSystemFromText;
    ComponentSystem *mpSystemFromFile;

private Q_SLOTS:
#define WIN32
#ifdef WIN32
    void Generator_FMU_Export()
    {
        QFETCH(ComponentSystem*, system);

        QString pwd = QDir::currentPath();

        //Generate FMU
        GeneratorHandler *pHandler = new GeneratorHandler();
        HString includePath = HString(pwd.toStdString().c_str())+"/../HopsanCore/include/";
        HString binPath = HString(pwd.toStdString().c_str())+"/../bin/";
        pHandler->callFmuExportGenerator(HString(pwd.toStdString().c_str())+"/fmu1_32/", system, includePath, binPath, "c:/mingw/bin", 1, false, false);
        pHandler->callFmuExportGenerator(HString(pwd.toStdString().c_str())+"/fmu1_64/", system, includePath, binPath, "c:/mingw64/bin", 1, true, false);
        pHandler->callFmuExportGenerator(HString(pwd.toStdString().c_str())+"/fmu2_32/", system, includePath, binPath, "c:/mingw/bin", 2, false, false);
        pHandler->callFmuExportGenerator(HString(pwd.toStdString().c_str())+"/fmu2_64/", system, includePath, binPath, "c:/mingw64/bin", 2, true, false);

//        QString code = "#include \"ComponentEssentials.h\"\n"
//                "namespace hopsan {\n"
//                "  class HydraulicLaminarOrifice : public ComponentQ\n"
//                "    {\n"
//                "      public:\n"
//                "        double ko;\n"
//                "      private:\n"
//                "        int gris;\n"
//                "        Integrator katt;\n"
//                "      void simulateOneTimestep()\n"
//                "      {\n"
//                "        gris=ko+5;\n"
//                "      }\n"
//                "      bool initialize()\n"
//                "      {\n"
//                "        int x=gris*3;\n"
//                "        gris= 3;\n"
//                "        ko=5;\n"
//                "      }\n"
//                "    };\n"
//                "}\n";
//        QStringList errorMsgs;
        //examineCode(code, errorMsgs);

        //Run FMUChecker for FMU 1.0 32-bit export
        QStringList args;
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << QDir::currentPath()+"/fmu1_32/unittestmodel_export.fmu";
        QProcess p;
        p.start(QDir::currentPath()+"/../Dependencies/tools/FMUChecker/fmuCheck.win32.exe", args);
        p.waitForReadyRead();
        QString output = p.readAllStandardError();
        QStringList errors = output.split("\n");

        QVERIFY2(errors.contains("\t0 warning(s) and error(s)\r"), "Failed to generate FMU 1.0 (32-bit), FMU not accepted by FMUChecker.");

        //Run FMUChecker for FMU 1.0 64-bit export
        args.clear();
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << QDir::currentPath()+"/fmu1_64/unittestmodel_export.fmu";
        p.start(QDir::currentPath()+"/../Dependencies/tools/FMUChecker/fmuCheck.win64.exe", args);
        p.waitForReadyRead();
        output = p.readAllStandardError();
        errors = output.split("\n");

        QVERIFY2(errors.contains("\t0 warning(s) and error(s)\r"), "Failed to generate FMU 1.0 (64-bit), FMU not accepted by FMUChecker.");

        //Run FMUChecker for FMU 2.0 32-bit export
        args.clear();
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << QDir::currentPath()+"/fmu2_32/unittestmodel_export.fmu";
        p.start(QDir::currentPath()+"/../Dependencies/tools/FMUChecker/fmuCheck.win32.exe", args);
        p.waitForReadyRead();
        output = p.readAllStandardError();
        errors = output.split("\n");

        QVERIFY2(errors.contains("\t0 warning(s) and error(s)\r"), "Failed to generate FMU 2.0 (32-bit), FMU not accepted by FMUChecker.");

        //Run FMUChecker for FMU 2.0 64-bit export
        args.clear();
        args << "-l" << "2";
        args << "-o" << "log.txt";
        args << QDir::currentPath()+"/fmu2_64/unittestmodel_export.fmu";
        p.start(QDir::currentPath()+"/../Dependencies/tools/FMUChecker/fmuCheck.win64.exe", args);
        p.waitForReadyRead();
        output = p.readAllStandardError();
        errors = output.split("\n");

        QVERIFY2(errors.contains("\t0 warning(s) and error(s)\r"), "Failed to generate FMU 2.0 (64-bit), FMU not accepted by FMUChecker.");
    }

    void Generator_FMU_Export_data()
    {
        QTest::addColumn<ComponentSystem*>("system");
        double start, stop;
        removeDir(QDir::currentPath()+"/fmu1_32/");
        removeDir(QDir::currentPath()+"/fmu1_64/");
        removeDir(QDir::currentPath()+"/fmu2_32/");
        removeDir(QDir::currentPath()+"/fmu2_64/");
        QDir().mkpath(QDir::currentPath()+"/fmu1_32/");
        QDir().mkpath(QDir::currentPath()+"/fmu1_64/");
        QDir().mkpath(QDir::currentPath()+"/fmu2_32/");
        QDir().mkpath(QDir::currentPath()+"/fmu2_64/");
        QString path = QDir::currentPath()+"/../Models/unittestmodel_export.hmf";
        QFile file(path);
        file.copy(QDir::currentPath()+"/fmu1_32/unittestmodel_export.hmf");
        file.copy(QDir::currentPath()+"/fmu1_64/unittestmodel_export.hmf");
        file.copy(QDir::currentPath()+"/fmu2_32/unittestmodel_export.hmf");
        file.copy(QDir::currentPath()+"/fmu2_64/unittestmodel_export.hmf");
        QTest::newRow("0") << mHopsanCore.loadHMFModelFile(path.toStdString().c_str(),start,stop);
    }

    void Generator_Simulink_Export()
    {
        QFETCH(ComponentSystem*, system);

        QString pwd = QDir::currentPath();

        //Generate S-function
        GeneratorHandler *pHandler = new GeneratorHandler();
        pHandler->callSimulinkExportGenerator(HString(pwd.toStdString().c_str())+"/simulink/", "unittestmodel_export.hmf", system, false, HString(pwd.toStdString().c_str())+"/../HopsanCore/include/", HString(pwd.toStdString().c_str())+"/../bin/", false);

        QVERIFY2(QFile::exists(pwd+"/simulink/externalLibs.txt"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanCore.dll"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanCore.exp"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanCore.lib"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/"+system->getName().c_str()+".cpp"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/HopsanSimulinkCompile.m"), "Failed to generate S-function, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/simulink/"+system->getName().c_str()+"PortLabels.m"), "Failed to generate S-function, all files not found.");
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
#endif

    void Generator_Labview_Export()
    {
        QFETCH(ComponentSystem*, system);

        QString pwd = QDir::currentPath();
        HString pwdPath = HString(pwd.toStdString().c_str());

        //Generate S-function
        GeneratorHandler *pHandler = new GeneratorHandler();
        pHandler->callLabViewSITGenerator(pwdPath+"/labview/unittestmodel_export.cpp", system, HString(pwd.toStdString().c_str())+"/../HopsanCore/include/", HString(pwd.toStdString().c_str())+"/", false);

        QVERIFY2(QFile::exists(pwd+"/labview/codegen.c"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/hopsanrt-wrapper.h"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/HOW_TO_COMPILE.txt"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/model.h"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/SIT_API.h"), "Failed to generate LabVIEW files, all files not found.");
        QVERIFY2(QFile::exists(pwd+"/labview/unittestmodel_export.cpp"), "Failed to generate LabVIEW files, all files not found.");

        QDir includeDir(pwd+"/labview/HopsanCore/include");
        QVERIFY2(includeDir.exists() && !includeDir.entryList().isEmpty(), "Failed to generate LabVIEW files: Include files not found.");
        QDir srcDir(pwd+"/labview/HopsanCore/src");
        QVERIFY2(srcDir.exists() && !srcDir.entryList().isEmpty(), "Failed to generate LabVIEW files: Source files not found.");
        QDir dependenciesDir(pwd+"/labview/HopsanCore/Dependencies");
        QVERIFY2(dependenciesDir.exists() && !dependenciesDir.entryList(QDir::AllEntries).isEmpty(), "Failed to generate LabVIEW files: Dependency files not found.");
        QDir libDir(pwd+"/labview/componentLibraries/defaultLibrary");
        QVERIFY2(libDir.exists() && !libDir.entryList().isEmpty(), "Failed to generate LabVIEW files: Default library files not found.");
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
        QFETCH(HString, code);
        QFETCH(HString, name);

        QString pwd = QDir::currentPath();

        //Generate FMU
        QFile moFile(pwd+"/modelica/motest.mo");
        moFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
        moFile.write(QString(code.c_str()).toUtf8());
        moFile.close();

        GeneratorHandler *pHandler = new GeneratorHandler();
        //pHandler->callModelicaGenerator(HString(QFileInfo(moFile).absoluteFilePath().toStdString().c_str()), false, 0, true, HString(pwd.toStdString().c_str())+"/../HopsanCore/include/", HString(pwd.toStdString().c_str())+"/");

        QVERIFY2(QDir().exists(QString(HString(HString(pwd.toStdString().c_str())+HString("/modelica/")+name+HString(LIBEXT)).c_str())), "Failure! Modelica generator failed to generate dll.");
    }

    void Generator_Modelica_data()
    {
        QTest::addColumn<HString>("code");
        QTest::addColumn<HString>("name");

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

        QTest::newRow("0") << HString(moCode) << HString("MyLaminarOrifice");
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
};

QTEST_APPLESS_MAIN(GeneratorTests)

#include "tst_generatortest.moc"
