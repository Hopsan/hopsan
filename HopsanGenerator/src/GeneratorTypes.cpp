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

#include "GeneratorTypes.h"
#include "GeneratorUtilities.h"

#include "HopsanEssentials.h"
#include "Port.h"
#include "Parameters.h"
#include "Nodes.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QXmlStreamReader>

namespace {

QString extractComponentClassName(const QFileInfo& fileInfo)
{
    constexpr auto componentBaseClass = "Component";
    QRegExp extractClassRegexp(R"(return\s+new\s+(\S+)\(\))");

    QFile codeFile(fileInfo.absoluteFilePath());
    if (codeFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&codeFile);
        QString prevLine, line;
        while (!ts.atEnd()) {
            prevLine = line;
            line = ts.readLine().trimmed();

            // User may not write code according to example, but if a Creator() functions is found,
            // the Component type shoule preceed it on the same or previous line.
            if (line.contains("Creator()")) {
                if (line.contains(componentBaseClass) || prevLine.contains(componentBaseClass)) {
                    // Loop until a line that returns a new object is found (it may be on the same line or likely two further down
                    while (!ts.atEnd()) {
                        int i = extractClassRegexp.indexIn(line);
                        if (i >= 0) {
                            return extractClassRegexp.cap(1);
                        }
                        line = ts.readLine().trimmed();
                    }
                }
            }
        }
    }
    return {};
}

QStringList extractRegisterTypenames(const QFileInfo& fileInfo)
{
    QRegExp includeExtractorRx(R"(["<]\s*(\S+)\s*[">])");
    includeExtractorRx.setMinimal(true);

    QRegExp registerClassExtractor(R"(,\s*(\S+)::Creator)");
    registerClassExtractor.setMinimal(true);

    QStringList extractedTypeNames;

    QFile file(fileInfo.absoluteFilePath());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&file);
        while (!ts.atEnd()) {
            QString line = ts.readLine().trimmed();
            // Handle relative (below root dir) includes, headers or .cci / .cppi (include sources)
            if (line.startsWith("//")) {
                continue;
            }
            else if (line.startsWith("#include")) {
                int i = includeExtractorRx.indexIn(line);
                if (i >= 0) {
                    const QString includeFileName = includeExtractorRx.cap(1);
                    QString includeFilePath = fileInfo.absoluteDir().filePath(includeFileName);
                    if (QFile::exists(includeFilePath)) {
                        extractedTypeNames.append(extractRegisterTypenames(includeFilePath));
                    }
                }
            }
            else if (line.contains("registerCreatorFunction")) {
                int i = registerClassExtractor.indexIn(line);
                if (i >= 0) {
                    QString className = registerClassExtractor.cap(1);
                    extractedTypeNames.append(className);
               }

            }
        }
    }
    return extractedTypeNames;
}

}

bool ComponentLibrary::saveToXML(QString filepath) const
{
    QFile templateFile(":/templates/library_template.xml");
    templateFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&templateFile);
    QString contents = in.readAll();
    templateFile.close();

    contents.replace("<<<libid>>>", mId);
    contents.replace("<<<libname>>>", mName);
    contents.replace("<<<libdebugext>>>", mSharedLibraryDebugExtension);

    // Local help function to generate multiline xml code
    auto writeXmlFileList = [&contents](const QString& pattern, const QString& tagname, const QStringList& files) {
        QString filesXml;
        for (const auto& file : files) {
            filesXml.append(QString("<%1>%2</%1>\n").arg(tagname).arg(file));
        }
        replacePattern(pattern, filesXml, contents);
    };

    writeXmlFileList("<<<sources>>>", "source", mSourceFiles);
    writeXmlFileList("<<<extrasources>>>", "extrasource", mExtraSourceFiles);
    writeXmlFileList("<<<components>>>", "component", mComponentCodeFiles);
    writeXmlFileList("<<<componentxmls>>>", "componentxml", mComponentXMLFiles);
    writeXmlFileList("<<<auxiliaryfiles>>>", "auxiliary", mAuxFiles);

    // Write build flags
    QString compilerFlagsXml, linkerFlagsXml;
    for (const auto& buildFlagSet : mBuildFlags) {
        if (!buildFlagSet.mCompilerFlags.isEmpty()) {
            compilerFlagsXml.append(QString("<cflags os=\"%1\">%2</cflags>\n").arg(buildFlagSet.platformString())
                                                                              .arg(buildFlagSet.mCompilerFlags.join(" ")));
        }
        if (!buildFlagSet.mLinkerFlags.isEmpty()) {
            linkerFlagsXml.append(QString("<lflags os=\"%1\">%2</lflags>\n").arg(buildFlagSet.platformString())
                                                                            .arg(buildFlagSet.mLinkerFlags.join(" ")));
        }
    }
    // Remove final \n
    if (!compilerFlagsXml.isEmpty()) {
        compilerFlagsXml.chop(1);
    }
    if (!linkerFlagsXml.isEmpty()) {
        linkerFlagsXml.chop(1);
    }

    replacePattern("<<<cflags>>>", compilerFlagsXml, contents);
    replacePattern("<<<lflags>>>", linkerFlagsXml, contents);

    writeXmlFileList("<<<includepaths>>>", "includepath", mIncludePaths);
    writeXmlFileList("<<<linkpaths>>>", "linkpath", mLinkPaths);
    writeXmlFileList("<<<linklibraries>>>", "linklibrary", mLinkLibraries);

    QFile outFile(filepath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream outStream(&outFile);
        outStream << contents;
        outStream.flush();
        outFile.close();
        return true;
    }
    return false;
}

void ComponentLibrary::clear()
{
    mLoadFilePath.clear();
    mId.clear();
    mName.clear();
    mSourceFiles.clear();
    mSharedLibraryName.clear();
    mSharedLibraryDebugExtension.clear();
    mComponentCodeFiles.clear();
    mComponentXMLFiles.clear();
    mAuxFiles.clear();
    mBuildFlags.clear();
    mIncludePaths.clear();
    mLinkPaths.clear();
    mLinkLibraries.clear();
}

bool ComponentLibrary::loadFromXML(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        //mpMessageHandler->addErrorMessage("Cannot open file for reading: "+path);
        return false;
    }
    clear();
    mLoadFilePath = filepath;


    QXmlStreamReader reader(file.readAll());

    bool foundCorrectRootElement=false;
    QString libraryVersion="0.1";
    // Read root element
    while (reader.readNextStartElement()) {
        if(reader.name() == "hopsancomponentlibrary")
        {
            foundCorrectRootElement=true;
            auto attributes = reader.attributes();
            // Read deprecated name attribute if present
            if (attributes.hasAttribute("name")) {
                mName = attributes.value("name").toString();
            }

            libraryVersion = attributes.value("version").toString();
            break;
        }
    }
    if (!foundCorrectRootElement) {
        //mpMessageHandler->addErrorMessage(tr("Not a Hopsan component library! Root tag: %1 != hopsancomponentlibrary").arg(libRoot.tagName()));
        return false;
    }

    // Read contents
    while (reader.readNextStartElement())
    {
        const QStringRef elementName = reader.name();
        if (elementName == "id") {
           mId = reader.readElementText();
        }
        else if (elementName == "name") {
            mName = reader.readElementText();
        }
        else if (elementName == "lib") {
            mSharedLibraryDebugExtension = reader.attributes().value("debug_ext").toString();
            mSharedLibraryName = reader.readElementText();
        }
        else if (elementName == "source") {
            mSourceFiles.append(reader.readElementText());
        }
        else if (elementName == "extrasource") {
            mExtraSourceFiles.append(reader.readElementText());
        }
        else if (elementName == "component") {
            mComponentCodeFiles.append(reader.readElementText());
        }
        else if (elementName == "componentxml" ||
                 elementName == "hopsanobjectappearance" ||
                 elementName == "caf") {
            mComponentXMLFiles.append(reader.readElementText());
        }
        else if (elementName == "auxiliary") {
            mAuxFiles.append(reader.readElementText());
        }
        else if (elementName == "buildflags") {
            while (reader.readNext()) {
                if (reader.isEndElement() && reader.name() == "buildflags") {
                    break;
                } else if (reader.isStartElement()) {
                    const auto platform = BuildFlags::platformFromString(reader.attributes().value("os").toString());
                    if (reader.name() == "cflags") {
                        QString cflags = reader.readElementText();
                        mBuildFlags.append(BuildFlags(platform, cflags.split(" "), {}));
                    } else if (reader.name() == "lflags") {
                        QString lflags = reader.readElementText();
                        mBuildFlags.append(BuildFlags(platform, {}, lflags.split(" ")));
                    } else {
                        // Discard to proceed
                        reader.readElementText(QXmlStreamReader::SkipChildElements);
                    }
                }
            }
        }
        else if (elementName == "includepath") {
            mIncludePaths.append(reader.readElementText());
        }
        else if (elementName == "linkpath") {
            mLinkPaths.append(reader.readElementText());
        }
        else if (elementName == "linklibrary") {
            mLinkLibraries.append(reader.readElementText());
        }
        else {
            // Discard to proceed
            reader.readElementText(QXmlStreamReader::SkipChildElements);
        }
    }
    file.close();
    return true;
}

bool ComponentLibrary::generateRegistrationCode(const QString& libraryRootPath, QString& rIncludeCode, QString& rRegisterCode, QString &rGeneratorError) const
{
    rIncludeCode.append(QString("// From library: %1\n").arg(mName));
    rRegisterCode.append(QString("  // From library: %1\n").arg(mName));
    for (const auto& relCodeFilePath : mComponentCodeFiles ) {
        rIncludeCode.append(QString(R"(#include "%1/%2")").arg(mName).arg(relCodeFilePath)).append("\n");
        QString codeFilePath = QDir(libraryRootPath).filePath(relCodeFilePath);
        QString componentTypeName = extractComponentClassName(codeFilePath);
        if (!componentTypeName.isEmpty()) {
            QString reg_line = QString(R"(  pComponentFactory->registerCreatorFunction("%1",%1::Creator);)").arg(componentTypeName);
            rRegisterCode.append(reg_line).append("\n");
        } else {
            rGeneratorError = QString("Could not parse component typename from code file: %1").arg(codeFilePath);
            return false;
        }
    }
    rGeneratorError.clear();
    return true;
}

QStringList ComponentLibrary::checkSourceXMLConsistency() const
{
    QStringList differences;
    const QDir rootDir = QFileInfo(mLoadFilePath).absoluteDir();
    // It is assumed that the first source file is the main cpp file where all components are registered
    if (!mSourceFiles.isEmpty()) {
        const QString sourceFilePath = rootDir.filePath(mSourceFiles.first());
        QStringList sourceTypeNames = extractRegisterTypenames(sourceFilePath);
        QStringList componentCodeTypeNames;
        for (const auto& relCodeFilePath : mComponentCodeFiles ) {
            QString codeFilePath = rootDir.filePath(relCodeFilePath);
            QString componentTypeName = extractComponentClassName(codeFilePath);
            if (!componentTypeName.isEmpty()) {
                componentCodeTypeNames.append(componentTypeName);
            }
        }

        // Compare all registered components,  remove those that match
        for (int i=0; i<componentCodeTypeNames.size(); ++i) {
            int j = sourceTypeNames.indexOf(componentCodeTypeNames[i]);
            if (j > -1) {
                // The type name exists in both data sets, remove it from them;
                componentCodeTypeNames.removeAt(i);
                sourceTypeNames.removeAt(j);
                --i;
            }
        }

        // Remaining type names are the difference
        for (const auto& ctn : componentCodeTypeNames) {
            differences.append(QString("Typename (class) '%1' does not seem to exist in the library source file: %2").arg(ctn).arg(sourceFilePath));
        }
        for (const auto& ctn : sourceTypeNames) {
            differences.append(QString("Typename (class) '%1' does not seem to exist in the library XML file: %2").arg(ctn).arg(mLoadFilePath));
        }
    }
    return differences;
}

PortSpecification::PortSpecification(QString porttype, QString nodetype, QString name, bool notrequired, QString defaultvalue, QString description, QString unit)
{
    this->porttype = porttype;
    this->nodetype = nodetype;
    this->name = name;
    this->notrequired = notrequired;
    this->defaultvalue = defaultvalue;
    this->description = description;
    this->unit = unit;
}


ParameterSpecification::ParameterSpecification(QString name, QString displayName, QString description, QString unit, QString init)
{
    this->name = name;
    this->displayName = displayName;
    this->description = description;
    this->unit = unit;
    this->init = init;
}


VariableSpecification::VariableSpecification(QString name, QString init)
{
    this->name = name;
    this->init = init;
}


ComponentSpecification::ComponentSpecification(QString typeName, QString displayName, QString cqsType)
{
    this->typeName = typeName;
    this->displayName = displayName;
    if(cqsType == "S")
        cqsType = "Signal";
    this->cqsType = cqsType;

    this->auxiliaryFunctions = QStringList();
}

GeneratorNodeInfo::GeneratorNodeInfo(QString nodeType)
{
    //! @todo this will only be able to create the default included nodes (which may be a problem in the future)
    hopsan::HopsanEssentials hopsanCore;
    hopsan::Node *pNode = hopsanCore.createNode(nodeType.toStdString().c_str());
    isValidNode = false;
    if (pNode)
    {
        isValidNode = true;
        niceName = pNode->getNiceName().c_str();
        QStringList qVariableLabels, cVariableLabels;
        QList<int> qVarIdx, cVarIdx;
        for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
        {
            const hopsan::NodeDataDescription *pVarDesc = pNode->getDataDescription(i);
            hopsan::NodeDataVariableTypeEnumT varType = pVarDesc->varType;
            // Check if  "Q-type variable"
            if(varType == hopsan::DefaultType || varType == hopsan::FlowType || varType == hopsan::IntensityType)
            {
                qVariables << pVarDesc->shortname.c_str();
                qVariableIds << pVarDesc->id;
                qVariableLabels << pVarDesc->name.c_str();
                qVarIdx << pVarDesc->id;
            }
            // Check if "C-type variable"
            else if(varType == hopsan::TLMType)
            {
                cVariables << pVarDesc->shortname.c_str();
                cVariableIds << pVarDesc->id;
                cVariableLabels << pVarDesc->name.c_str();
                cVarIdx << pVarDesc->id;
            }
        }
        variableLabels << qVariableLabels << cVariableLabels;
        varIdx << qVarIdx << cVarIdx;
        hopsanCore.removeNode(pNode);
    }
}

void GeneratorNodeInfo::getNodeTypes(QStringList &nodeTypes)
{
    //! @todo this will only be able to list the default included nodes (which may be a problem in the future)
    hopsan::HopsanEssentials hopsanCore;
    std::vector<hopsan::HString> types = hopsanCore.getRegisteredNodeTypes();
    for(const hopsan::HString &type : types) {
        nodeTypes << type.c_str();
    }
}


InterfacePortSpec::InterfacePortSpec(InterfaceTypesEnumT type, QString component, QString port, QStringList path)
{
    this->type = type;
    this->path = path;
    this->component = component;
    this->port = port;

    QStringList inputDataNames;
    QStringList outputDataNames;
    QList<int> inputDataIds, outputDataIds;

    switch(type)
    {
    case InterfacePortSpec::Input:
        inputDataNames << "";
        inputDataIds << 0;
        break;
    case InterfacePortSpec::Output:
        outputDataNames << "";
        outputDataIds << 0;
        break;
    case InterfacePortSpec::MechanicQ:
    {
        GeneratorNodeInfo gni("NodeMechanic");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::MechanicC:
    {
        GeneratorNodeInfo gni("NodeMechanic");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::MechanicRotationalQ:
    {
        GeneratorNodeInfo gni("NodeMechanicRotational");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::MechanicRotationalC:
    {
        GeneratorNodeInfo gni("NodeMechanicRotational");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::HydraulicQ:
    {
        GeneratorNodeInfo gni("NodeHydraulic");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::HydraulicC:
    {
        GeneratorNodeInfo gni("NodeHydraulic");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::PneumaticQ:
    {
        GeneratorNodeInfo gni("NodePneumatic");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::PneumaticC:
    {
        GeneratorNodeInfo gni("NodePneumatic");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::ElectricQ:
    {
        GeneratorNodeInfo gni("NodeElectric");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::ElectricC:
    {
        GeneratorNodeInfo gni("NodeElectric");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::PetriNetQ:
    {
        inputDataNames  << "s";
        inputDataIds << 0;
        outputDataNames << "q";
        outputDataIds << 1;
        break;
    }
    case InterfacePortSpec::PetriNetC:
    {
        inputDataNames  << "q";
        inputDataIds << 1;
        outputDataNames << "s";
        outputDataIds << 0;
        break;
    }
    default:
        break;
    }

    for(const QString &dataName : inputDataNames) {
        vars.append(InterfaceVarSpec(dataName, inputDataIds.takeFirst(), InterfaceVarSpec::Input));
    }
    for(const QString &dataName : outputDataNames) {
        vars.append(InterfaceVarSpec(dataName, outputDataIds.takeFirst(), InterfaceVarSpec::Output));
    }
}


InterfaceVarSpec::InterfaceVarSpec(QString dataName, int dataId, InterfaceVarSpec::CausalityEnumT causality)
{
    this->dataName = dataName;
    this->dataId = dataId;
    this->causality = causality;
}

void getInterfaces(QList<InterfacePortSpec> &interfaces, hopsan::ComponentSystem *pSystem, QStringList &path)
{
    std::vector<hopsan::HString> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        hopsan::Component *pComponent = pSystem->getSubComponent(names[i]);
        hopsan::HString typeName = pComponent->getTypeName();

        if(typeName == "SignalInputInterface")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::Input, names[i].c_str(), "out", path));
        }
        else if(typeName == "SignalOutputInterface")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::Output, names[i].c_str(), "in", path));
        }
        else if(typeName == "MechanicInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicRotationalInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicRotationalQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicRotationalInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicRotationalC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "HydraulicInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::HydraulicQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "HydraulicInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::HydraulicC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PneumaticInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PneumaticQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PneumaticInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PneumaticC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "ElectricInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::ElectricQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "ElectricInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::ElectricC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PetriNetInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PetriNetQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PetriNetInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PetriNetC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "Subsystem")
        {
            QStringList path2 = path;
            getInterfaces(interfaces, dynamic_cast<hopsan::ComponentSystem *>(pComponent), path2 << pComponent->getName().c_str());
        }
    }
}

void getModelVariables(hopsan::ComponentSystem *pSystem, QList<ModelVariableSpecification> &vars, QStringList &systemHierarchy) {
    std::vector<hopsan::HString> names = pSystem->getSubComponentNames();


    for(size_t i=0; i<names.size(); ++i)
    {
        QString portName, cqio;

        hopsan::Component *pComponent = pSystem->getSubComponent(names[i]);
        hopsan::HString typeName = pComponent->getTypeName();
        if(typeName == "SignalInputInterface")
        {
            portName = "out";
            cqio = "i";
        }
        else if(typeName == "SignalOutputInterface")
        {
            portName = "in";
            cqio = "o";
        }
        else if(typeName == "MechanicInterfaceQ")
        {
            portName = "P1";
            cqio = "q";
        }
        else if(typeName == "MechanicInterfaceC")
        {
            portName = "P1";
            cqio = "c";
        }
        else if(typeName == "MechanicRotationalInterfaceQ")
        {
            portName = "P1";
            cqio = "q";
        }
        else if(typeName == "MechanicRotationalInterfaceC")
        {
            portName = "P1";
            cqio = "c";
        }
        else if(typeName == "HydraulicInterfaceQ")
        {
            portName = "P1";
            cqio = "q";
        }
        else if(typeName == "HydraulicInterfaceC")
        {
            portName = "P1";
            cqio = "c";
        }
        else if(typeName == "PneumaticInterfaceQ")
        {
            portName = "P1";
            cqio = "q";
        }
        else if(typeName == "PneumaticInterfaceC")
        {
            portName = "P1";
            cqio = "c";
        }
        else if(typeName == "ElectricInterfaceQ")
        {
            portName = "P1";
            cqio = "q";
        }
        else if(typeName == "ElectricInterfaceC")
        {
            portName = "P1";
            cqio = "c";
        }
        else if(typeName == "PetriNetInterfaceQ")
        {
            portName = "P1";
            cqio = "q";
        }
        else if(typeName == "PetriNetInterfaceC")
        {
            portName = "P1";
            cqio = "c";
        }
        else if(typeName == "Subsystem")
        {
            getModelVariables(dynamic_cast<hopsan::ComponentSystem *>(pComponent), vars, systemHierarchy << pComponent->getName().c_str());
            continue;
        }
        else {
            continue;   //Not an interface component
        }

        hopsan::Port *pPort = pSystem->getSubComponent(names[i])->getPort(portName.toStdString().c_str());
        for(const auto &node : *pPort->getNodeDataDescriptions()) {
            ModelVariableCausality causality;
            if(node.varType == hopsan::DefaultType) {
                if(cqio == "i") {
                    causality = ModelVariableCausality::Input;
                }
                else {
                    causality = ModelVariableCausality::Output;
                }
            }
            else if(node.varType == hopsan::FlowType || node.varType == hopsan::IntensityType || node.varType == hopsan::DefaultType) {
                if(cqio == "q") {
                    causality = ModelVariableCausality::Input;
                }
                else {
                    causality = ModelVariableCausality::Output;
                }
            }
            else {
                if(cqio == "q") {
                    causality = ModelVariableCausality::Output;
                }
                else {
                    causality = ModelVariableCausality::Input;
                }
            }
            vars.append(ModelVariableSpecification(systemHierarchy, names[i].c_str(), portName, node.shortname.c_str(), node.id, pPort->getStartValue(node.id), causality, QString(node.unit.c_str())));
        }
    }
}

BuildFlags::BuildFlags(const QStringList &cflags, const QStringList &lflags) : mCompilerFlags(cflags), mLinkerFlags(lflags) {}

BuildFlags::BuildFlags(const BuildFlags::Platform platform, const QStringList &cflags, const QStringList &lflags)
    : mCompilerFlags(cflags), mLinkerFlags(lflags), mPlatform(platform) {}

BuildFlags::BuildFlags(const BuildFlags::Compiler compiler, const QStringList &cflags, const QStringList &lflags)
    : mCompilerFlags(cflags), mLinkerFlags(lflags), mCompiler(compiler) {}

QString BuildFlags::platformString(BuildFlags::Platform platform) {
    switch (platform) {
    case Platform::win : return hopsan::os_strings::win ;
    case Platform::win32 : return hopsan::os_strings::win32 ;
    case Platform::win64 : return hopsan::os_strings::win64 ;
    case Platform::Linux : return hopsan::os_strings::Linux ;
    case Platform::apple : return hopsan::os_strings::apple ;
    default : return {} ;
    }
}

BuildFlags::Platform BuildFlags::platformFromString(const QString &platformString)
{
    if (platformString == hopsan::os_strings::win64) {
        return Platform::win64;
    } else if (platformString == hopsan::os_strings::win32) {
        return Platform::win32;
    } else if (platformString == hopsan::os_strings::win) {
        return Platform::win;
    } else if (platformString == hopsan::os_strings::Linux) {
        return Platform::Linux;
    } else if (platformString == hopsan::os_strings::apple) {
        return Platform::apple;
    } else {
        return Platform::notset;
    }
}

QString BuildFlags::compilerString(BuildFlags::Compiler compiler, Language language)
{
    switch (compiler) {
    case Compiler::GCC  :
        switch (language) {
        case Language::C   : return hopsan::compiler_strings::gcc ;
        case Language::Cpp : return hopsan::compiler_strings::gpp ;
        }
    case Compiler::Clang: return hopsan::compiler_strings::clang ;
    case Compiler::MSVC : return hopsan::compiler_strings::msvc  ;
    default: return {} ;
    }
}

QString BuildFlags::platformString() const
{
    return platformString(mPlatform);
}

//QString BuildFlags::compilerString() const
//{
//    return compilerString(mCompiler);
//}

void getParameters(QList<ParameterSpecification> &parameters, hopsan::ComponentSystem *pSystem)
{
    for(hopsan::ParameterEvaluator *par : (*pSystem->getParametersVectorPtr())) {
        if(par->isInternal()) {
            continue;
        }
        ParameterSpecification spec;
        if(par->getType() == "double") {
            spec.type = "Real";
        }
        else if(par->getType() == "integer") {
            spec.type = "Integer";
        }
        else if(par->getType() == "bool") {
            spec.type = "Boolean";
        }
        else if(par->getType() == "string") {
            spec.type = "String";
        }
        else if(par->getType() == "textblock") {
            spec.type = "String";
        }
        else if(par->getType() == "filepath") {
            spec.type = "String";
        }
        else if(par->getType() == "conditional") {
            spec.type = "Integer";
        }
        else {
            continue;
        }
        spec.name = par->getName().c_str();
        spec.displayName = par->getName().c_str();
        spec.init = par->getValue().c_str();
        spec.description = par->getDescription().c_str();
        spec.unit = par->getUnit().c_str();
        parameters.append(spec);
    }
}

ModelVariableSpecification::ModelVariableSpecification(QStringList systemHierarchy, QString componentName, QString portName, QString dataName, int dataId, double startValue, ModelVariableCausality causality, QString unit)
{
    this->systemHierarchy = systemHierarchy;
    this->componentName = componentName;
    this->portName = portName;
    this->dataName = dataName;
    this->dataId = dataId;
    this->startValue = startValue;
    this->causality = causality;
    this->unit = unit;
}

QString ModelVariableSpecification::getName() const
{
    if(systemHierarchy.isEmpty()) {
        if(dataName == "y") {
            return componentName;   //Only use component name for signal interfaces (does not affect DLL, only XML)
        }
        return componentName+"."+portName+"."+dataName;
    }
    else {
        return systemHierarchy.join(".")+"."+componentName+"."+portName+"."+dataName;
    }
}

QString ModelVariableSpecification::getCausalityStr() const
{
    if(causality == Input) {
        return "input";
    }
    else {
        return "output";
    }
}
