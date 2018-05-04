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
//! @file   ComponentGeneratorLib.cpp
//!
//! @brief Contains the exported functions for component generator library
//!
//$Id$

#include "hopsangenerator.h"

#include "generators/HopsanModelicaGenerator.h"
#include "generators/HopsanSimulinkGenerator.h"
#include "generators/HopsanLabViewGenerator.h"
#include "generators/HopsanFMIGenerator.h"
#include "GeneratorUtilities.h"

#include <QFileInfo>
#include <QDir>

namespace hopsan {
class ComponentSystem;
}

//! @brief Calls the Modelica generator
//! @param moFilePath Path to the modelica file (also output directory)
//! @param compilerPath Path to compiler bin directory
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param[in] quiet Hide generator output
void callModelicaGenerator(const char* moFilePath, const char* compilerPath, messagehandler_t messageHandler, void* pMessageObject, int solver, bool compile, const char* hopsanInstallPath)
{
    HopsanModelicaGenerator *pGenerator = new HopsanModelicaGenerator(hopsanInstallPath, compilerPath);
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    pGenerator->generateFromModelica(moFilePath, HopsanGeneratorBase::SolverT(solver));
    if(compile)
    {
        QFileInfo mofile(moFilePath);
        QString dir = mofile.absolutePath()+"/";
        QString typeName = mofile.baseName();
        QStringList hppFiles {typeName+".hpp"};
        pGenerator->generateNewLibrary(dir, hppFiles);
        compileComponentLibrary(dir+typeName+"_lib.xml", pGenerator);
    }
    delete(pGenerator);
}


//! @brief Generates .cpp and .xml files for a library from a list of .hpp files
//! @param outputPath Path to where the files shall be created
//! @param hppFiles Vector with filenames for .hpp files
//! @param[in] quiet Hide generator output
void callLibraryGenerator(const char*  outputPath, const char* const hppFiles[], const int numFiles, messagehandler_t messageHandler, void* pMessageObject)
{
    HopsanGeneratorBase *pGenerator = new HopsanGeneratorBase("", "", "");
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    QStringList tempList;
    for(size_t i=0; i<numFiles; ++i)
    {
        tempList.append(hppFiles[i]);
    }
    pGenerator->generateNewLibrary(outputPath, tempList);
    delete(pGenerator);
}


//! @brief Calls the C++ generator
//! @param hppPath C++ code
//! @param compilerPath Path to the compiler bin directory
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param[in] quiet Hide generator output
void callCppGenerator(const char* hppPath, const char* compilerPath, bool compile, const char* hopsanInstallPath, messagehandler_t messageHandler, void* pMessageObject)
{
    qDebug() << "Called C++ generator (in dll)!";

    QFile hppFile(hppPath);
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
        xmlStream << "  <modelobject typename=\"" << typeName << "\" displayname=\"" << displayName << "\" sourcecode=\"" << QFileInfo(xmlFile).dir().relativeFilePath(hppPath) << "\">\n";
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
        HopsanGeneratorBase *pGenerator = new HopsanGeneratorBase(hopsanInstallPath, compilerPath);
        pGenerator->setMessageHandler(messageHandler, pMessageObject);
        QFileInfo hp(hppPath);
        QString dir = hp.absolutePath()+"/";
        QString typeName = hp.baseName();
        QStringList hppFiles {typeName+".hpp"};
        pGenerator->generateNewLibrary(dir, hppFiles);
        compileComponentLibrary(dir+typeName+"_lib.xml", pGenerator);
        delete(pGenerator);
    }
}


//! @brief Calls the functional mockup interface (FMU) import generator
//! @param fmuFilePath Path to the .fmu file
//! @param targetPath Destination of generated fmu import wrapper
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param compilerPath Path to the compiler binaries
//! @param[in] quiet Hide generator output
void callFmuImportGenerator(const char* fmuFilePath, const char* targetPath, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler, void* pMessageObject)
{
    HopsanFMIGenerator *pGenerator = new HopsanFMIGenerator(hopsanInstallPath, compilerPath);
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    QString typeName, hppFile;
    if(!pGenerator->generateFromFmu(fmuFilePath, targetPath, typeName, hppFile))
    {
        pGenerator->printErrorMessage("Import of FMU failed.");
        return;
    }

    QString fmuFileName = QFileInfo(fmuFilePath).baseName();
    QFileInfo fmuImportRoot(QString("%1/%2").arg(targetPath).arg(fmuFileName));
    QFileInfo hppFileInfo(QDir(fmuImportRoot.absoluteFilePath()).relativeFilePath(hppFile));

    const QString fmiLibraryDir=pGenerator->getHopsanRootPath()+"/Dependencies/FMILibrary";

    QStringList cflags, lflags;
    cflags << QString("-I\"%1\"").arg(fmiLibraryDir+"/include");
    lflags << QString("-L\"%1\"").arg(fmiLibraryDir+"/lib");
#ifdef _WIN32
    lflags << " -llibfmilib_shared";
#else
    lflags << " -lfmilib_shared";  //Remove extra "lib" prefix in Linux
#endif

    // Generate the component library files
    QStringList hppFiles {hppFileInfo.filePath()};
    pGenerator->generateNewLibrary(fmuImportRoot.canonicalFilePath(), hppFiles , cflags, lflags);

    // Compile the generated component library
    compileComponentLibrary(fmuImportRoot.canonicalFilePath()+"/"+fmuFileName+"_lib.xml", pGenerator);

    delete(pGenerator);
}


//! @brief Calls the functional mockup interface (FMU) export generator
//! @param[in] outputPath Path to export to
//! @param[in] pSystem Pointer to system that shall be exported
//! @param[in] hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param[in] compilerPath Path to the compiler binaries
//! @param[in] version The FMU version to export 1 or 2
//! @param[in] architecture 32 or 64
//! @param[in] quiet Hide generator output
void callFmuExportGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, const char* compilerPath, int version, int architecture, messagehandler_t messageHandler, void* pMessageObject)
{
    HopsanFMIGenerator *pGenerator = new HopsanFMIGenerator(hopsanInstallPath, compilerPath);
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    pGenerator->generateToFmu(outputPath, static_cast<hopsan::ComponentSystem*>(pHopsanSystem), version, architecture);
    delete(pGenerator);
}


//! @brief Calls the Simulink S-function generator
//! @param outputPath Path to export to
//! @param modelFile Path to the hopsan model file
//! @param pSystem Pointer to system that shall be exported
//! @param disablePortLabels Tells whether or not port labels shall be disabled (for compatibility with older MATLAB versions)
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param quiet Hide generator output
void callSimulinkExportGenerator(const char* outputPath, const char* modelFile, void* pHopsanSystem, bool disablePortLabels, const char* hopsanInstallPath, messagehandler_t messageHandler, void* pMessageObject)
{
    HopsanSimulinkGenerator *pGenerator = new HopsanSimulinkGenerator(hopsanInstallPath);
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    pGenerator->generateToSimulink(outputPath, modelFile, static_cast<hopsan::ComponentSystem*>(pHopsanSystem), disablePortLabels);
    delete(pGenerator);
}


//! @brief Calls the LabVIEW SIT generator
//! @param outputPath Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param quiet Hide generator output
void callLabViewSITGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, messagehandler_t messageHandler, void* pMessageObject)
{
    HopsanLabViewGenerator *pGenerator = new HopsanLabViewGenerator(hopsanInstallPath);
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    pGenerator->generateToLabViewSIT(outputPath, static_cast<hopsan::ComponentSystem*>(pHopsanSystem));
    delete(pGenerator);
}


//! @brief Calls the component library compile utility
//! @param outputPath Path to library
//! @param extraCFlags Additional compile flags
//! @param extraLFlags Additional linker flags
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param compilerPath Path to the compiler bin directory
//! @param quiet Hide generator output
void callComponentLibraryCompiler(const char* outputPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler, void* pMessageObject)
{
    HopsanGeneratorBase *pGenerator = new HopsanGeneratorBase(hopsanInstallPath, compilerPath);
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    compileComponentLibrary(outputPath, pGenerator, extraCFlags, extraLFlags);
    delete(pGenerator);
}

