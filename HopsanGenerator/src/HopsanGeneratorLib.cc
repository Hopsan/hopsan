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

//!
//! @file   ComponentGeneratorLib.cc
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains the exported functions for component generator library
//!
//$Id$

#include "generators/HopsanGenerator.h"
#include "generators/HopsanModelicaGenerator.h"
#include "generators/HopsanSimulinkGenerator.h"
#include "generators/HopsanLabViewGenerator.h"
#include "generators/HopsanFMIGenerator.h"
#include "HopsanTypes.h"
#include "hopsangenerator_win32dll.h"

//#include <QMessageBox>

using namespace std;


//! @brief Calls the Modelica generator
//! @param modelicaCode Modelica code
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callModelicaGenerator(hopsan::HString path, hopsan::HString gccPath, bool showDialog=false, int solver=0, bool compile=false, hopsan::HString coreIncludePath="", hopsan::HString binPath="")
{
    HopsanModelicaGenerator *pGenerator = new HopsanModelicaGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), QString(gccPath.c_str()), showDialog);
    pGenerator->generateFromModelica(QString(path.c_str()), HopsanGenerator::SolverT(solver));
    if(compile)
    {
        QString dir = QFileInfo(QString(path.c_str())).absolutePath()+"/";
        QString typeName = QFileInfo(QString(path.c_str())).baseName();
        pGenerator->generateNewLibrary(dir, QStringList() << typeName+".hpp");
        compileComponentLibrary(dir+typeName+"_lib.xml", pGenerator);
    }
    delete(pGenerator);
}


//! @brief Generates .cc and .xml files for a library from a list of .hpp files
//! @param path Path to where the files shall be created
//! @param hppFiles Vector with filenames for .hpp files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callLibraryGenerator(hopsan::HString path, vector<hopsan::HString> hppFiles, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator("", "", "", showDialog);
    QStringList tempList;
    for(size_t i=0; i<hppFiles.size(); ++i)
    {
        tempList.append(QString(hppFiles[i].c_str()));
    }
    pGenerator->generateNewLibrary(QString(path.c_str()), tempList);
    delete(pGenerator);
}


//! @brief Calls the C++ generator
//! @param cppCode C++ code
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callCppGenerator(hopsan::HString hppPath, hopsan::HString gccPath, bool compile=false, hopsan::HString coreIncludePath="", hopsan::HString binPath="")
{
    qDebug() << "Called C++ generator (in dll)!";

    QFile hppFile(QString(hppPath.c_str()));
    hppFile.open(QFile::ReadOnly);
    QString code = hppFile.readAll();
    hppFile.close();

    //Needed: typeName, displayName, cqsType
    QString typeName = code.section("class ", 1, 1).section(" ",0,0);
    QString displayName = typeName;

    QStringList portNames;
    QStringList lines = code.split("\n");
    for(int l=0; l<lines.size(); ++l)
    {
        if(lines.at(l).contains("addPowerPort") || lines.at(l).contains("addReadPort") || lines.at(l).contains("addWritePort") ||
           lines.at(l).contains("addPowerMultiPort") || lines.at(l).contains("addReadMultiPort") ||
           lines.at(l).contains("addInputVariable") || lines.at(l).contains("addOutputVariable"))
        {
            portNames.append(lines.at(l).section("\"",1,1));
        }
    }

    QFile xmlFile;
    xmlFile.setFileName(QFileInfo(hppFile).path()+"/"+typeName+".xml");
    if(!xmlFile.exists())
    {
        if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return;
        }
        QTextStream xmlStream(&xmlFile);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
        xmlStream << "  <modelobject typename=\"" << typeName << "\" displayname=\"" << displayName << "\" sourcecode=\"" << QFileInfo(xmlFile).dir().relativeFilePath(QString(hppPath.c_str())) << "\">\n";
        xmlStream << "    <icons/>\n";
        xmlStream << "    <ports>\n";
        double xDelay = 1.0/(portNames.size()+1.0);
        double xPos = xDelay;
        double yPos = 0;
        for(int i=0; i<portNames.size(); ++i)
        {
            xmlStream << "      <port name=\"" << portNames[i] << "\" x=\"" << xPos << "\" y=\"" << yPos << "\" a=\"" << 270 << "\"/>\n";
            xPos += xDelay;
        }
        xmlStream << "    </ports>\n";
        xmlStream << "  </modelobject>\n";
        xmlStream << "</hopsanobjectappearance>\n";
        xmlFile.close();
    }

    if(compile)
    {
        HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), QString(gccPath.c_str()), true);
        QString dir = QFileInfo(QString(hppPath.c_str())).absolutePath()+"/";
        QString typeName = QFileInfo(QString(hppPath.c_str())).baseName();
        pGenerator->generateNewLibrary(dir, QStringList() << typeName+".hpp");
        compileComponentLibrary(dir+typeName+"_lib.xml", pGenerator);
        delete(pGenerator);
    }
}


//! @brief Calls the functional mockup interface (FMU) import generator
//! @param fmuFilePath Path to the .fmu file
//! @param targetPath Destination of generated fmu import wrapper
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callFmuImportGenerator(hopsan::HString fmuFilePath, hopsan::HString targetPath, hopsan::HString coreIncludePath, hopsan::HString binPath, hopsan::HString gccPath, bool showDialog=false)
{
    HopsanFMIGenerator *pGenerator = new HopsanFMIGenerator(coreIncludePath.c_str(), binPath.c_str(), gccPath.c_str(), showDialog);
    QString typeName, hppFile;
    if(!pGenerator->generateFromFmu(fmuFilePath.c_str(), targetPath.c_str(), typeName, hppFile))
    {
        pGenerator->printErrorMessage("Import of FMU failed.");
        return;
    }

    QString fmuFileName = QFileInfo(fmuFilePath.c_str()).baseName();
    QFileInfo fmuImportRoot(QString("%1/%2").arg(targetPath.c_str()).arg(fmuFileName));
    QFileInfo hppFileInfo(QDir(fmuImportRoot.absoluteFilePath()).relativeFilePath(hppFile));

    const QString fmiLibDir="/Dependencies/FMILibrary";

    QStringList cflags, lflags;
    cflags << QString("-I\"%1\"").arg(pGenerator->getHopsanRootPath()+fmiLibDir+"/include/");
    lflags << QString("-L\"%1\"").arg(pGenerator->getHopsanRootPath()+fmiLibDir+"/lib");

#ifdef _WIN32
    lflags << " -llibfmilib_shared";
#else
    lflags << " -lfmilib_shared";  //Remove extra "lib" prefix in Linux
#endif

    // Generate the component library files
    pGenerator->generateNewLibrary(fmuImportRoot.canonicalFilePath(), QStringList() << hppFileInfo.filePath(), cflags, lflags);

    // Compile the generated component library
    compileComponentLibrary(fmuImportRoot.canonicalFilePath()+"/"+fmuFileName+"_lib.xml", pGenerator);

    delete(pGenerator);
}


//! @brief Calls the functional mockup interface (FMU) export generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callFmuExportGenerator(hopsan::HString path, hopsan::ComponentSystem *pSystem, hopsan::HString coreIncludePath, hopsan::HString binPath, hopsan::HString gccPath, int version=2, bool x64=false, bool showDialog=false)
{
    HopsanFMIGenerator *pGenerator = new HopsanFMIGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), QString(gccPath.c_str()), showDialog);
    pGenerator->generateToFmu(QString(path.c_str()), pSystem, version, x64);
    delete(pGenerator);
}


//! @brief Calls the Simulink S-function generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param compiler Compiler to use, 0 = MSVC2008 32-bit, 1 = MSVC2008 64-bit, 2 = MSVC2010 32-bit, 3 = MSVC2010 64-bit
//! @param disablePortLabels Tells whether or not port labels shall be disabled (for compatibility with older MATLAB versions)
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callSimulinkExportGenerator(const hopsan::HString path, const hopsan::HString modelFile, hopsan::ComponentSystem *pSystem, bool disablePortLabels, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanSimulinkGenerator *pGenerator = new HopsanSimulinkGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToSimulink(QString(path.c_str()), QString(modelFile.c_str()), pSystem, disablePortLabels);
    delete(pGenerator);
}


//! @brief Calls the Simulink S-function generator for co-simulations
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param compiler Compiler to use, 0 = MSVC2008 32-bit, 1 = MSVC2008 64-bit, 2 = MSVC2010 32-bit, 3 = MSVC2010 64-bit
//! @param disablePortLabels Tells whether or not port labels shall be disabled (for compatibility with older MATLAB versions)
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callSimulinkCoSimExportGenerator(hopsan::HString path, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanSimulinkGenerator *pGenerator = new HopsanSimulinkGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToSimulinkCoSim(QString(path.c_str()), pSystem, disablePortLabels, compiler);
    delete(pGenerator);
}


//! @brief Calls the LabVIEW SIT generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window

extern "C" HOPSANGENERATOR_DLLAPI void callLabViewSITGenerator(hopsan::HString path, hopsan::ComponentSystem *pSystem, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanLabViewGenerator *pGenerator = new HopsanLabViewGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToLabViewSIT(QString(path.c_str()), pSystem);
    delete(pGenerator);
}


//! @brief Calls the component library compile utility
//! @param path Path to library
//! @param name Name of output dll
//! @param extraCFlags Additional compile flags
//! @param extraLFlags Additional linker flags
//! @param coreIncludePath Path to HopsanCore include files
//! @param binpath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" HOPSANGENERATOR_DLLAPI void callComponentLibraryCompiler(hopsan::HString path, hopsan::HString extraCFlags, hopsan::HString extraLFlags, hopsan::HString coreIncludePath, hopsan::HString binPath, hopsan::HString gccPath, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), QString(gccPath.c_str()), showDialog);
    compileComponentLibrary(path.c_str(), pGenerator, extraCFlags.c_str(), extraLFlags.c_str());
    delete(pGenerator);
}

