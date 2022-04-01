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
#include "generators/HopsanExeGenerator.h"
#include "GeneratorUtilities.h"
#include "GeneratorTypes.h"

#include <QFileInfo>
#include <QDir>

#include <memory>

namespace hopsan {
class ComponentSystem;
}

namespace {
QString toHppFilename(QString filename) {
    if (filename.endsWith(".mo")) {
        filename.chop(3);
        filename.append(".hpp");
    }
    return filename;
}
QString toMoFilename(QString filename) {
    if (filename.endsWith(".hpp")) {
        filename.chop(4);
        filename.append(".mo");
    }
    return filename;
}
}

//! @brief Calls the Modelica generator
//! @param moFilePath Path to the modelica file (also output directory)
//! @param compilerPath Path to compiler bin directory
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param[in] quiet Hide generator output
bool callModelicaGenerator(const char* moFilePath, const char* compilerPath, messagehandler_t messageHandler, void* pMessageObject, bool compile, const char* hopsanInstallPath)
{
    auto pGenerator = std::unique_ptr<HopsanModelicaGenerator>(new HopsanModelicaGenerator(hopsanInstallPath, compilerPath));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    bool genImportOK = pGenerator->generateFromModelica(moFilePath);
    if(genImportOK && compile)
    {
        QFileInfo mofile(moFilePath);
        QString dir = mofile.absolutePath()+"/";
        QString typeName = mofile.baseName();
        QStringList hppFiles {typeName+".hpp"};
        QString libName = QDir(dir).dirName();
        //! @todo Is my use of {libName+".xml"} correct here for modelica generator usage ?
        bool genLibraryOK = pGenerator->generateNewLibrary(dir, libName, hppFiles, {libName+".xml"});
        if (!genLibraryOK) {
            return false;
        }
        return compileComponentLibrary(dir+typeName+"_lib.xml", pGenerator.get());
    }
    return genImportOK;
}


//! @brief Generates .cpp and .xml files for a library from a list of .hpp files
//! @param outputPath Path to where the files shall be created
//! @param hppFiles Vector with filenames for .hpp files
//! @param[in] quiet Hide generator output
bool callLibraryGenerator(const char*  outputPath, const char* const hppFiles[], const int numFiles, messagehandler_t messageHandler, void* pMessageObject)
{
    auto pGenerator = std::unique_ptr<HopsanGeneratorBase>(new HopsanGeneratorBase("", {}, ""));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    QStringList tempList;
    for(int i=0; i<numFiles; ++i)
    {
        tempList.append(hppFiles[i]);
    }
    QString libName = QDir(outputPath).dirName();
    //! @todo what about caf files? It seems that this code is only called from HopsanGUI to generate a new empty library right now, it is also in a block of EXPERIMENTAL code in the component properties dialog
    //! @todo The for now this works since its only called to create a new empty library, but the callLibraryGenerator should take the caf files as input also
    return pGenerator->generateNewLibrary(outputPath, libName, tempList);
}


//! @brief Calls the C++ generator
//! @param hppPath C++ code
//! @param compilerPath Path to the compiler bin directory
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param[in] quiet Hide generator output
bool callCppGenerator(const char* hppPath, const char* compilerPath, bool compile, const char* hopsanInstallPath, messagehandler_t messageHandler, void* pMessageObject)
{
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
            return false;
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
        const auto cs = CompilerSelection(defaultCompiler(currentPlatform()), compilerPath);
        auto pGenerator = std::unique_ptr<HopsanGeneratorBase>(new HopsanGeneratorBase(hopsanInstallPath, cs));
        pGenerator->setMessageHandler(messageHandler, pMessageObject);
        QFileInfo hp(hppPath);
        QString dir = hp.absolutePath()+"/";
        QString typeName = hp.baseName();
        QStringList hppFiles {typeName+".hpp"};
        QString libName = QDir(dir).dirName();
        //! @todo what about caf files, this code seems to be called ONLY from EXPERIMENTAL code in LibraryHandler (create new component) and Component Properties dialog.
        //! @todo This code is not used unless EXPERIMENTAL is defined, maybe it can be removed
        bool genLibraryOK = pGenerator->generateNewLibrary(dir, libName, hppFiles);
        if (!genLibraryOK) {
            return false;
        }
        return compileComponentLibrary(dir+typeName+"_lib.xml", pGenerator.get());
    }

    return true;
}


//! @brief Calls the functional mockup interface (FMU) export generator
//! @param[in] outputPath Path to export to
//! @param[in] pSystem Pointer to system that shall be exported
//! @param[in] externalLibraries C array with paths to external library xml files
//! @param[in] numLibraries The number of elements in the C array
//! @param[in] hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param[in] compilerPath Path to the compiler binaries
//! @param[in] version The FMU version to export 1 or 2
//! @param[in] architecture 32 or 64
//! @param[in] quiet Hide generator output
bool callFmuExportGenerator(const char* outputPath, void* pHopsanSystem, const char* const externalLibraries[], const int numLibraries, const char* hopsanInstallPath, const char* compilerPath, int version, int architecture, messagehandler_t messageHandler, void* pMessageObject)
{
    auto pGenerator = std::unique_ptr<HopsanFMIGenerator>(new HopsanFMIGenerator(hopsanInstallPath, compilerPath));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    const bool isArchitecture64 = (architecture==64);
    QStringList externalLibs;
    for(int i=0; i<numLibraries; ++i)
    {
        externalLibs.append(externalLibraries[i]);
    }
    return pGenerator->generateToFmu(outputPath, static_cast<hopsan::ComponentSystem*>(pHopsanSystem), externalLibs,  version, isArchitecture64);
}


//! @brief Calls the Simulink S-function generator
//! @param outputPath Path to export to
//! @param modelFile Path to the hopsan model file
//! @param pSystem Pointer to system that shall be exported
//! @param[in] externalLibraries C array with paths to external library xml files
//! @param[in] numLibraries The number of elements in the C array
//! @param disablePortLabels Tells whether or not port labels shall be disabled (for compatibility with older MATLAB versions)
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param quiet Hide generator output
bool callSimulinkExportGenerator(const char* outputPath, const char* modelFile, void* pHopsanSystem, const char* const externalLibraries[], const int numLibraries, bool disablePortLabels, const char* hopsanInstallPath, messagehandler_t messageHandler, void* pMessageObject)
{
    auto pGenerator = std::unique_ptr<HopsanSimulinkGenerator>(new HopsanSimulinkGenerator(hopsanInstallPath));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    QStringList externalLibs;
    for(int i=0; i<numLibraries; ++i)
    {
        externalLibs.append(externalLibraries[i]);
    }
    return pGenerator->generateToSimulink(outputPath, modelFile, static_cast<hopsan::ComponentSystem*>(pHopsanSystem), externalLibs, disablePortLabels);
}


//! @brief Calls the LabVIEW SIT generator
//! @param outputPath Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param quiet Hide generator output
bool callLabViewSITGenerator(const char* outputPath, void* pHopsanSystem, const char* hopsanInstallPath, messagehandler_t messageHandler, void* pMessageObject)
{
    auto pGenerator = std::unique_ptr<HopsanLabViewGenerator>(new HopsanLabViewGenerator(hopsanInstallPath));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    return pGenerator->generateToLabViewSIT(outputPath, static_cast<hopsan::ComponentSystem*>(pHopsanSystem));
}


//! @brief Calls the component library compile utility
//! @param libraryPath Path to library
//! @param extraCFlags Additional compile flags
//! @param extraLFlags Additional linker flags
//! @param hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param compilerPath Path to the compiler bin directory
//! @param quiet Hide generator output
bool callComponentLibraryCompiler(const char* libraryPath, const char* extraCFlags, const char* extraLFlags, const char* hopsanInstallPath, const char* compilerPath, messagehandler_t messageHandler, void* pMessageObject)
{
    const auto cs = CompilerSelection(defaultCompiler(currentPlatform()), compilerPath);
    auto pGenerator = std::unique_ptr<HopsanGeneratorBase>(new HopsanGeneratorBase(hopsanInstallPath, cs));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    return compileComponentLibrary(libraryPath, pGenerator.get(), extraCFlags, extraLFlags);
}

bool callCheckComponentLibrary(const char* libraryXMLPath, messagehandler_t messageHandler, void* pMessageObject)
{
    ComponentLibrary lib;
    lib.loadFromXML(libraryXMLPath);

    if (messageHandler) {
        auto message = QString("Checking component registration consistency for library: %1").arg(lib.mName);
        messageHandler(qPrintable(message), 'I', pMessageObject);
    }

    QStringList differences = lib.checkSourceXMLConsistency();
    if (messageHandler) {
        for (const auto& item : differences) {
            messageHandler(qPrintable(item), 'I', pMessageObject);
        }
        if (differences.empty()) {
            messageHandler("No differences found", 'I', pMessageObject);
        } else {
            messageHandler("There are differences that needs to be resolved. Edit the files manually, or choose to \"Add existing component\" if files are missing.", 'W', pMessageObject);
        }
    }
    return differences.empty();
}


//! @brief Calls the executable model export generator
//! @param[in] outputPath Path to export to
//! @param[in] pSystem Pointer to system that shall be exported
//! @param[in] externalLibraries C array with paths to external library xml files
//! @param[in] numLibraries The number of elements in the C array
//! @param[in] hopsanInstallPath Path to the Hopsan installation where HopsanCore/include exists
//! @param[in] compilerPath Path to the compiler binaries
//! @param[in] architecture 32 or 64
//! @param[in] quiet Hide generator output
bool callExeExportGenerator(const char* outputPath, void* pHopsanSystem, const char* const externalLibraries[], const int numLibraries, const char* hopsanInstallPath, const char* compilerPath, int architecture, messagehandler_t messageHandler, void* pMessageObject)
{
    auto pGenerator = std::unique_ptr<HopsanExeGenerator>(new HopsanExeGenerator(hopsanInstallPath, compilerPath));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);
    const bool isArchitecture64 = (architecture==64);
    QStringList externalLibs;
    for(int i=0; i<numLibraries; ++i)
    {
        externalLibs.append(externalLibraries[i]);
    }
    return pGenerator->generateToExe(outputPath, static_cast<hopsan::ComponentSystem*>(pHopsanSystem), externalLibs, isArchitecture64);
}


//! @brief Adds a component to an existing library
//! @param[in] libraryXmlPath Absolute path to library XML file
//! @param[in] librarySourcePath Path to library CPP file relative to path for library XML file
//! @param[in] typeName Typename for new component
//! @param[in] displayName Display name for new component
bool callAddComponentToLibrary(const char* libraryXmlPath, const char* targetPath, const char* typeName, const char* displayName, const char* cqsType, const char* transform,
                               const char* const constantNames[], const int numConstantNames,
                               const char* const constantDescriptions[], const int numConstantDescriptions,
                               const char* const constantUnits[], const int numConstantUnits,
                               const char* const constantInits[], const int numConstantInits,
                               const char* const inputNames[], const int numInputNames,
                               const char* const inputDescriptions[], const int numInputDescriptions,
                               const char* const inputUnits[], const int numInputUnits,
                               const char* const inputInits[], const int numInputInits,
                               const char* const outputNames[], const int numOutputNames,
                               const char* const outputDescriptions[], const int numOutputDescriptions,
                               const char* const outputUnits[], const int numOutputUnits,
                               const char* const outputInits[], const int numOutputInits,
                               const char* const portNames[], const int numPortNames,
                               const char* const portDescriptions[], const int numPortDescriptions,
                               const char* const portTypes[], const int numPortTypes,
                               const int portsRequired[], const int numPortsRequired,
                               bool modelica, messagehandler_t messageHandler, void* pMessageObject)
{
    QFileInfo xmlPath(libraryXmlPath);
    QString cafPath = QDir(targetPath).absoluteFilePath(QString(typeName)+".xml");
    QString sourcePath;
    if(modelica) {
        sourcePath = QDir(targetPath).absoluteFilePath(QString(typeName)+".mo");
    }
    else {
        sourcePath = QDir(targetPath).absoluteFilePath(QString(typeName)+".hpp");
    }

    QString dummy;
    auto pGenerator = std::unique_ptr<HopsanGeneratorBase>(new HopsanGeneratorBase(dummy, dummy));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);

    //Generate CAF file for new component
    ComponentAppearanceSpecification cafSpec(typeName);
    cafSpec.mDisplayName = displayName;
    cafSpec.mSourceCode = QFileInfo(sourcePath).fileName();
    cafSpec.mRecompilable = true;
    for(int p=0; p<numPortNames; ++p) {
        double x = (p+1.0)/((double)numPortNames+1.0);
        cafSpec.addPort(portNames[p], x,0,270);
    }
    for(int i=0; i<numInputNames; ++i) {
        double y = (i+1.0)/((double)numInputNames+1.0);
        cafSpec.addPort(inputNames[i], 0,y,180);
    }
    for(int i=0; i<numOutputNames; ++i) {
        double y = (i+1.0)/((double)numOutputNames+1.0);
        cafSpec.addPort(outputNames[i], 1,y,0);
    }
    if(!pGenerator->generateCafFile(cafPath, cafSpec)) {
        pGenerator->printErrorMessage("Failed to generate component appearance file.");
        return false;
    }

    //Generate source file for new component
    ComponentSpecification compSpec;
    compSpec.typeName = typeName;
    compSpec.displayName = displayName;
    compSpec.cqsType = cqsType;
    compSpec.transform = transform;
    for(int i=0; i<numConstantNames; ++i) {
        compSpec.parDisplayNames.append(constantNames[i]);
        if(modelica) {
            compSpec.parNames.append(QString(constantNames[i]));
        }
        else {
            compSpec.parNames.append("m"+QString(constantNames[i]));
        }
    }
    for(int i=0; i<numConstantDescriptions; ++i) {
        compSpec.parDescriptions.append(constantDescriptions[i]);
    }
    for(int i=0; i<numConstantUnits; ++i) {
        compSpec.parUnits.append(constantUnits[i]);
    }
    for(int i=0; i<numConstantInits; ++i) {
        compSpec.parInits.append(constantInits[i]);
    }
    for(int i=0; i<numInputNames; ++i) {
        compSpec.portNames.append(inputNames[i]);
        compSpec.portTypes.append("ReadPort");
        compSpec.portNodeTypes.append("NodeSignal");
        compSpec.portNotReq.append(true);
    }
    for(int i=0; i<numInputDescriptions; ++i) {
        compSpec.portDescriptions.append(inputDescriptions[i]);
    }
    for(int i=0; i<numInputUnits; ++i) {
        compSpec.portUnits.append(inputUnits[i]);
    }
    for(int i=0; i<numInputInits; ++i) {
        compSpec.portDefaults.append(inputInits[i]);
    }
    for(int i=0; i<numOutputNames; ++i) {
        compSpec.portNames.append(outputNames[i]);
        compSpec.portTypes.append("WritePort");
        compSpec.portNodeTypes.append("NodeSignal");
        compSpec.portNotReq.append(true);
        compSpec.portDefaults.append(outputInits[i]);
    }
    for(int i=0; i<numOutputDescriptions; ++i) {
        compSpec.portDescriptions.append(outputDescriptions[i]);
    }
    for(int i=0; i<numOutputUnits; ++i) {
        compSpec.portUnits.append(outputUnits[i]);
    }
    for(int i=0; i<numPortNames; ++i) {
        compSpec.portNames.append(portNames[i]);
        compSpec.portTypes.append("PowerPort");
        compSpec.portDefaults.append("");
        compSpec.portUnits.append("");
    }

    for(int i=0; i<numPortTypes; ++i) {
        compSpec.portNodeTypes.append(portTypes[i]);
    }
    for(int i=0; i<numPortDescriptions; ++i) {
        compSpec.portDescriptions.append(portDescriptions[i]); //Not yet implemented
    }
    for(int i=0; i<numPortsRequired; ++i) {
        compSpec.portNotReq.append(!portsRequired[i]);
    }
    HopsanGeneratorBase::TargetLanguageT target = HopsanGeneratorBase::Cpp;
    if(modelica) {
        target = HopsanGeneratorBase::Modelica;
    }
    if(!pGenerator->generateComponentSourceFile(sourcePath, compSpec, target)) {
        pGenerator->printErrorMessage("Failed to generate component source file.");
        return false;
    }

    //Load component library from XML
    ComponentLibrary lib;
    if(!lib.loadFromXML(xmlPath.absoluteFilePath())) {
        pGenerator->printErrorMessage("Cannot open "+xmlPath.absoluteFilePath()+" for reading.");
        return false;
    }

    //Add new component to library
    lib.mComponentXMLFiles.append(QDir(xmlPath.absolutePath()).relativeFilePath(QFileInfo(cafPath).absoluteFilePath()));
    const QString relativeSourcePath = QDir(xmlPath.absolutePath()).relativeFilePath(QFileInfo((sourcePath)).absoluteFilePath());
    lib.mComponentCodeFiles.append(toHppFilename(relativeSourcePath));
    if (modelica) {
        lib.mAuxFiles.append(relativeSourcePath);
    }

    //Write back component library to XML
    if(!lib.saveToXML(xmlPath.absoluteFilePath())) {
        pGenerator->printErrorMessage("Cannot open "+xmlPath.absoluteFilePath()+" for writing.");
        return false;
    }

    //Generate main source file
    if(!pGenerator->generateLibrarySourceFile(lib)) {
        pGenerator->printErrorMessage("Failed to generate library source file.");
        return false;
    }

    return true;
}


//! @brief Adds a component to an existing library
//! @param[in] libraryXmlPath Absolute path to library XML file
//! @param[in] librarySourcePath Path to library CPP file relative to path for library XML file
//! @param[in] typeName Typename for new component
//! @param[in] displayName Display name for new component
bool callAddExistingComponentToLibrary(const char* libraryXmlPath, const char* cafPath, messagehandler_t messageHandler, void* pMessageObject)
{
    QFileInfo libFileInfo(libraryXmlPath);
    QFileInfo cafFileInfo(cafPath);

    QString dummy;
    auto pGenerator = std::unique_ptr<HopsanGeneratorBase>(new HopsanGeneratorBase(dummy, dummy));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);

    //Load component library from XML
    ComponentLibrary lib;
    if(!lib.loadFromXML(libraryXmlPath)) {
        pGenerator->printErrorMessage("Cannot open "+cafFileInfo.absoluteFilePath()+" for reading.");
        return false;
    }

    //Read CAF XML and extract source file (.hpp)
    QFile cafFile(cafPath);
    QDomDocument cafDomDocument;
    QDomElement cafRootElement;
    cafRootElement = loadXMLDomDocument(cafFile,cafDomDocument,"hopsanobjectappearance");
    if(QDomElement() == cafRootElement) {
        pGenerator->printErrorMessage("Unable to parse XML file: "+QString(cafPath));
        return false;
    }
    QDomElement modelObjectElement = cafRootElement.firstChildElement("modelobject");
    if(modelObjectElement.isNull()) {
        pGenerator->printErrorMessage("Unable to parse XML file: "+QString(cafPath)+" (cannot find \"modelobject\" element)");
        return false;
    }
    QString sourceFile = modelObjectElement.attribute("sourcecode");
    if(sourceFile.isEmpty()) {
        pGenerator->printErrorMessage("Source code not specified in component XML file.");
        return false;
    }
    QString sourcePath = cafFileInfo.absoluteDir().absoluteFilePath(sourceFile);
    QFileInfo sourceFileInfo(sourcePath);

    //Add new component to library
    lib.mComponentXMLFiles.append(libFileInfo.absoluteDir().relativeFilePath(cafFileInfo.absoluteFilePath()));
    const QString relativeSourcePath = libFileInfo.absoluteDir().relativeFilePath(sourceFileInfo.absoluteFilePath());
    lib.mComponentCodeFiles.append(toHppFilename(relativeSourcePath));
    if (relativeSourcePath.endsWith(".mo")) {
        lib.mAuxFiles.append(relativeSourcePath);
    }

    //Write back component library to XML
    if(!lib.saveToXML(libFileInfo.absoluteFilePath())) {
        pGenerator->printErrorMessage("Cannot open "+libFileInfo.absoluteFilePath()+" for writing.");
        return false;
    }

    //Generate main source file
    if(!pGenerator->generateLibrarySourceFile(lib)) {
        pGenerator->printErrorMessage("Failed to generate library source file.");
        return false;
    }

    return true;
}




//! @brief Removes a component from a component library
//! @param[in] libraryXmlPath Absolute path to library XML file
//! @param[in] cafPath Path to component description file
//! @param[in] sourceCodePath Path to component source file
//! @param[in] deleteFiles Flag for deleting actual files
bool callRemoveComponentFromLibrary(const char* libraryXmlPath, const char* cafPath, const char* sourceCodePath, bool deleteFiles, messagehandler_t messageHandler, void* pMessageObject)
{
    QFileInfo xmlPath(libraryXmlPath);
    QDir xmlRootDir = QFileInfo(libraryXmlPath).absoluteDir();

    QString dummy;
    auto pGenerator = std::unique_ptr<HopsanGeneratorBase>(new HopsanGeneratorBase(dummy, dummy));
    pGenerator->setMessageHandler(messageHandler, pMessageObject);

    //Load component library from XML
    ComponentLibrary lib;
    if(!lib.loadFromXML(xmlPath.absoluteFilePath())) {
        pGenerator->printErrorMessage("Cannot open "+xmlPath.absoluteFilePath()+" for reading.");
        return false;
    }

    //Remove component from library
    const QString relativeCafFile = xmlRootDir.relativeFilePath(cafPath);
    lib.mComponentXMLFiles.removeAll(relativeCafFile);
    const QString relativeSourceFile = xmlRootDir.relativeFilePath(sourceCodePath);
    lib.mComponentCodeFiles.removeAll(toHppFilename(relativeSourceFile));
    if (QFile::exists(toMoFilename(sourceCodePath))) {
        lib.mAuxFiles.removeAll(toMoFilename(relativeSourceFile));
    }

    //Write back component library to XML
    if(!lib.saveToXML(xmlPath.absoluteFilePath())) {
        pGenerator->printErrorMessage("Cannot open "+xmlPath.absoluteFilePath()+" for writing.");
        return false;
    }

    //Generate main source file
    if(!pGenerator->generateLibrarySourceFile(lib)) {
        pGenerator->printErrorMessage("Failed to generate library source file.");
        return false;
    }

    if(deleteFiles) {
        QFile::remove(cafPath);
        QFile::remove(sourceCodePath);
    }

    return true;
}
