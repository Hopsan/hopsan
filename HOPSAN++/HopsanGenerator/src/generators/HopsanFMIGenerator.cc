#include "generators/HopsanFMIGenerator.h"
#include "GeneratorUtilities.h"
#include "ComponentSystem.h"
#include <QApplication>
#include <cassert>
#include <QProcess>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace hopsan;



HopsanFMIGenerator::HopsanFMIGenerator(QString coreIncludePath, QString binPath, bool showDialog)
    : HopsanGenerator(coreIncludePath, binPath, showDialog)
{

}



void HopsanFMIGenerator::generateFromFmu(QString path)
{
    printMessage("Initializing FMU import");

    QFileInfo fmuFileInfo = QFileInfo(path);
    fmuFileInfo.setFile(path);

    //! @todo Make global
    QString gExecPath = qApp->applicationDirPath().append('/');

    QDir zipDir;
    zipDir = QDir::cleanPath(gExecPath + "../ThirdParty/7z");

    QDir gccDir;
    gccDir = QDir::cleanPath(gExecPath + "../ThirdParty/mingw32/bin");

    QString fmuName = fmuFileInfo.fileName();
    fmuName.chop(4);

    //! @todo We cannot use gExecPath for this because Windows may not allow it

    //Create import directory if it does not exist
    if(!QDir(gExecPath + "../import").exists())
        QDir().mkdir(gExecPath + "../import");

    //Create FMU directory if it does not exist
    if(!QDir(gExecPath + "../import/FMU").exists())
        QDir().mkdir(gExecPath + "../import/FMU");

    //Remove output directory if it already exists
    if(QDir(gExecPath+"../import/FMU/"+fmuName).exists() && !removeDir(gExecPath+"../import/FMU/"+fmuName))
    {
        printErrorMessage("Unable to remove output directory: "+QDir().cleanPath(gExecPath+"../import/FMU/"+fmuName)+". Please remove it manually and try again.");
        return;
    }

    //Create output directory
    if(!QDir().mkdir(gExecPath + "../import/FMU/" + fmuName))
    {
        printErrorMessage("Unable to create output directory: "+QDir().cleanPath(gExecPath+"../import/FMU/"+fmuName)+". Please remove it manually and try again.");
        return;
    }


    QString fmuPath = gExecPath + "../import/FMU/" + fmuName;
    QDir fmuDir = QDir::cleanPath(fmuPath);

    printMessage("Unpacking files");


    //Unzip .fmu file
#ifdef WIN32
    QProcess zipProcess;
    zipProcess.setWorkingDirectory(zipDir.path());
    QStringList arguments;
    arguments << "x" << fmuFileInfo.filePath() << "-o" + fmuDir.path() << "-aoa";
    zipProcess.start(zipDir.path() + "/7z.exe", arguments);
    zipProcess.waitForFinished();
    QByteArray zipResult = zipProcess.readAll();
    QList<QByteArray> zipResultList = zipResult.split('\n');
    for(int i=0; i<zipResultList.size(); ++i)
    {
        QString msg = zipResultList.at(i);
        msg = msg.remove(msg.size()-1, 1);
        if(!msg.isEmpty())
        {
            printMessage(msg);
        }
    }
#else
    QString command = "unzip "+fmuFileInfo.filePath()+" -d "+fmuDir.path();
    qDebug() << "Command = " << command;
    FILE *fp;
    char line[130];
    command +=" 2>&1";
    fp = popen(  (const char *) command.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + command + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            printMessage((const QString &)line);
        }
    }
#endif

    //Move all binary files to FMU directory
#ifdef WIN32
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/win32");
#elif WIN64
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/win64");
#elif linux && __i386__
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/linux32");
#elif linux && __x86_64__
    QDir binaryDir = QDir::cleanPath(fmuDir.path() + "/binaries/linux64");
#endif
    if(!binaryDir.exists())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: Unable to unpack files");
        return;
    }
    QFileInfoList binaryFiles = binaryDir.entryInfoList(QDir::Files);
    for(int i=0; i<binaryFiles.size(); ++i)
    {
        QFile tempFile;
        tempFile.setFileName(binaryFiles.at(i).filePath());
        tempFile.copy(fmuDir.path() + "/" + binaryFiles.at(i).fileName());
        printMessage("Copying " + tempFile.fileName() + " to " + fmuDir.path() + "/" + binaryFiles.at(i).fileName());
        tempFile.remove();
    }


    //Move all resource files to FMU directory
    QDir resDir = QDir::cleanPath(fmuDir.path() + "/resources");
    QFileInfoList resFiles = resDir.entryInfoList(QDir::Files);
    for(int i=0; i<resFiles.size(); ++i)
    {
        QFile tempFile;
        tempFile.setFileName(resFiles.at(i).filePath());
        tempFile.copy(fmuDir.path() + "/" + resFiles.at(i).fileName());
        printMessage("Copying " + tempFile.fileName() + " to " + fmuDir.path() + "/" + resFiles.at(i).fileName());
        tempFile.remove();
    }


    //Move hmf files to execution path (ugly hack for importing hopsan fmu to hopsan)
    QStringList filters;
    filters << "*.hmf";
    fmuDir.setNameFilters(filters);
    QStringList hmfList = fmuDir.entryList();
    for (int i = 0; i < hmfList.size(); ++i)
    {
        QFile hmfFile;
        hmfFile.setFileName(fmuDir.path() + "/" + hmfList.at(i));
        if(hmfFile.exists())
        {
            QFile().remove(gExecPath+hmfList.at(i));
            if(QFile().exists(gExecPath+hmfList.at(i)))
            {
                printErrorMessage("Unable to copy "+hmfFile.fileName()+" to /bin directory: File already exists.");
                return;
            }
            hmfFile.copy(gExecPath + hmfList.at(i));
            hmfFile.remove();
            hmfFile.setFileName(gExecPath + hmfList.at(i));
            printMessage("Copying " + hmfFile.fileName() + " to " + gExecPath + hmfList.at(i));
        }
    }
    fmuDir.setFilter(QDir::NoFilter);

    printMessage("Parsing modelDescription.xml");

    //Load XML data from modelDescription.xml
    //Copy xml-file to this directory
    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(fmuDir.path() + "/modelDescription.xml");
    if(!binaryDir.exists())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: modelDescription.xml not found.");
        return;
    }
    QDomDocument fmuDomDocument;
    QDomElement fmuRoot = loadXMLDomDocument(modelDescriptionFile, fmuDomDocument, "fmiModelDescription");
    modelDescriptionFile.close();

    if(fmuRoot == QDomElement())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: Could not parse modelDescription.xml.");
        return;
    }


    printMessage("Defining variables");


    //Define lists with input and output variables

    QStringList inVarValueRefs, inVarPortNames;
    QStringList outVarValueRefs, outVarPortNames;
    QStringList inoutVarValueRefs, inoutVarPortNames;
    QDomElement variablesElement = fmuRoot.firstChildElement("ModelVariables");
    QDomElement varElement = variablesElement.firstChildElement("ScalarVariable");
    int i=0;
    //! @todo We cannot use value reference as port names, because several ports can point to the same value
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") != "parameter" && varElement.attribute("variability") != "constant")
        {
            if(!varElement.hasAttribute("causality"))
            {
                //inoutVarValueRefs << varElement.attribute("valueReference");
                inoutVarValueRefs << QString::number(i);
                inoutVarPortNames << varElement.attribute("name");
            }
            else if(varElement.attribute("causality") == "input")
            {
                //inVarValueRefs << varElement.attribute("valueReference");
                inVarValueRefs << QString::number(i);
                inVarPortNames << varElement.attribute("name");
            }
            else if(varElement.attribute("causality") == "output")
            {
                //outVarValueRefs << varElement.attribute("valueReference");
                outVarValueRefs << QString::number(i);
                outVarPortNames << varElement.attribute("name");
            }
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
        ++i;
    }

    i=0;

    QStringList tlmPortTypes;
    QList<QStringList> tlmPortVarNames;
    QList<QStringList> tlmPortValueRefs;


    //Read from [modelName]_TLM.xml if it exists, to define TLM powerports
    QFile tlmSpecFile;
    tlmSpecFile.setFileName(fmuFileInfo.path() + "/" + fmuName + "_TLM.xml");
    QDomDocument tlmDomDocument;
    QDomElement tlmRoot;
    if(tlmSpecFile.exists())
    {
        tlmRoot = loadXMLDomDocument(tlmSpecFile, tlmDomDocument, "hopsanfmu");
        tlmSpecFile.close();
    }

    if(tlmRoot != QDomElement())
    {
        printMessage("Parsing "+fmuName+"_TLM.xml");

        QStringList input;

        QDomElement portElement = tlmRoot.firstChildElement("tlmport");
        while(!portElement.isNull())
        {
            input.clear();

            QString type = portElement.attribute("type");
            input.append(type);

            QDomElement outputElement = portElement.firstChildElement("output");
            while(!outputElement.isNull())
            {
                QString name = outputElement.text();
                outputElement = outputElement.nextSiblingElement("output");
                input.append(name);
            }

            QDomElement inputElement = portElement.firstChildElement("input");
            while(!inputElement.isNull())
            {
                QString name = inputElement.text();
                inputElement = inputElement.nextSiblingElement("input");
                input.append(name);
            }


            if(input.first() == "hydraulic" && input.size() == 5)
            {
                if(outVarPortNames.contains(input[1]) && outVarPortNames.contains(input[2]) && inVarPortNames.contains(input[3]) && inVarPortNames.contains(input[4]))
                {
                    printMessage("Adding hydraulic port");

                    tlmPortTypes.append(input[0]);
                    input.removeFirst();
                    tlmPortVarNames.append(input);

                    tlmPortValueRefs.append(QStringList());
                    tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(input[0])]);
                    tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(input[1])]);
                    tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(input[2])]);
                    tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(input[3])]);

                    outVarValueRefs.removeAt(outVarPortNames.indexOf(input[0]));
                    outVarPortNames.removeAll(input[0]);

                    outVarValueRefs.removeAt(outVarPortNames.indexOf(input[1]));
                    outVarPortNames.removeAll(input[1]);

                    inVarValueRefs.removeAt(inVarPortNames.indexOf(input[2]));
                    inVarPortNames.removeAll(input[2]);

                    inVarValueRefs.removeAt(inVarPortNames.indexOf(input[3]));
                    inVarPortNames.removeAll(input[3]);
                }
            }
            else if(input.first() == "mechanic" && input.size() == 7)
            {
                if(outVarPortNames.contains(input[1]) && outVarPortNames.contains(input[2]) && outVarPortNames.contains(input[3]) && outVarPortNames.contains(input[4]) && inVarPortNames.contains(input[5]) && inVarPortNames.contains(input[6]))
                {
                    printMessage("Adding mechanical port");

                    tlmPortTypes.append(input[0]);
                    input.removeFirst();
                    tlmPortVarNames.append(input);

                    tlmPortValueRefs.append(QStringList());
                    tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(input[0])]);
                    tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(input[1])]);
                    tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(input[2])]);
                    tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(input[3])]);
                    tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(input[4])]);
                    tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(input[5])]);

                    outVarValueRefs.removeAt(outVarPortNames.indexOf(input[0]));
                    outVarPortNames.removeAll(input[0]);

                    outVarValueRefs.removeAt(outVarPortNames.indexOf(input[1]));
                    outVarPortNames.removeAll(input[1]);

                    outVarValueRefs.removeAt(outVarPortNames.indexOf(input[2]));
                    outVarPortNames.removeAll(input[2]);

                    outVarValueRefs.removeAt(outVarPortNames.indexOf(input[3]));
                    outVarPortNames.removeAll(input[3]);

                    inVarValueRefs.removeAt(inVarPortNames.indexOf(input[4]));
                    inVarPortNames.removeAll(input[4]);

                    inVarValueRefs.removeAt(inVarPortNames.indexOf(input[5]));
                    inVarPortNames.removeAll(input[5]);
                }
            }

            portElement = portElement.nextSiblingElement("tlmport");
        }
    }


    ////////////////////////////////
    // DEFINE PORT SPECIFICATIONS //
    ////////////////////////////////


    QList<FMIPortSpecification> portSpecs;
    for(int i=0; i<inVarPortNames.size(); ++i)
    {
        QString varName = toVarName("mpIn_"+inVarPortNames[i]);
        QString portName = toVarName(inVarPortNames[i]+"In");
        QString mpndName = toVarName("mpND_in"+inVarPortNames[i]);
        portSpecs << FMIPortSpecification(varName, portName, mpndName, inVarValueRefs[i], "ReadPort", "NodeSignal", "NodeSignal::VALUE", "input");
    }
    for(int i=0; i<outVarPortNames.size(); ++i)
    {
        portSpecs << FMIPortSpecification(toVarName("mpOut"+outVarPortNames[i]), toVarName(outVarPortNames[i]+"Out"),
                                          toVarName("mpND_out"+outVarPortNames[i]), outVarValueRefs[i],
                                          "WritePort", "NodeSignal", "NodeSignal::VALUE", "output");
    }
    for(int i=0; i<inoutVarPortNames.size(); ++i)
    {
        portSpecs << FMIPortSpecification(toVarName("mpIn"+inoutVarPortNames[i]), toVarName(inoutVarPortNames[i]+"In"),
                                          toVarName("mpND_in"+inoutVarPortNames[i]), inoutVarValueRefs[i],
                                          "ReadPort", "NodeSignal", "NodeSignal::VALUE", "");
        portSpecs << FMIPortSpecification(toVarName("mpOut"+inoutVarPortNames[i]), toVarName(inoutVarPortNames[i]+"Out"),
                                          toVarName("mpND_out"+inoutVarPortNames[i]), inoutVarValueRefs[i],
                                          "WritePort", "NodeSignal", "NodeSignal::VALUE", "");
    }
    for(int i=0; i<tlmPortVarNames.size(); ++i)
    {
        QString numStr = QString::number(i);
        QString varName = "mpP"+numStr;
        QString portName = "P"+numStr;
        QString portType = "PowerPort";
        QString nodeType;
        QStringList mpndNames, dataTypes, causalities;
        if(tlmPortTypes[i] == "hydraulic")
        {
            nodeType = "NodeHydraulic";
            mpndNames << "p"+numStr;
            mpndNames << "q"+numStr;
            mpndNames << "c"+numStr;
            mpndNames << "Zc"+numStr;
            dataTypes << "NodeHydraulic::PRESSURE";
            dataTypes << "NodeHydraulic::FLOW";
            dataTypes << "NodeHydraulic::WAVEVARIABLE";
            dataTypes << "NodeHydraulic::CHARIMP";
            causalities << "output" << "output" << "input" << "input";
        }
        else if(tlmPortTypes[i] == "mechanic")
        {
            nodeType = "NodeMechanic";
            mpndNames << "f"+numStr;
            mpndNames << "x"+numStr;
            mpndNames << "v"+numStr;
            mpndNames << "me"+numStr;
            mpndNames << "c"+numStr;
            mpndNames << "Zc"+numStr;
            dataTypes << "NodeMechanic::FORCE";
            dataTypes << "NodeMechanic::POSITION";
            dataTypes << "NodeMechanic::VELOCITY";
            dataTypes << "NodeMechanic::EQMASS";
            dataTypes << "NodeMechanic::WAVEVARIABLE";
            dataTypes << "NodeMechanic::CHARIMP";
            causalities << "output" << "output" << "output" << "output" << "input" << "input";
        }
        for(int j=0; j<mpndNames.size(); ++j)
        {
            portSpecs << FMIPortSpecification(varName, portName, mpndNames[j], tlmPortValueRefs[i][j], portType, nodeType, dataTypes[j], causalities[j]);
        }
    }

    //Print port info to user
    for(int p=0; p<portSpecs.size(); ++p)
        printMessage(QString("\nPort:")+
                     "  \n  Variable        = \""+portSpecs[p].varName +
                     "\"\n  Name            = \""+portSpecs[p].portName +
                     "\"\n  Node pointer    = \""+portSpecs[p].mpndName+
                     "\"\n  Value reference = \""+portSpecs[p].valueRef+
                     "\"\n  Port type       = \""+portSpecs[p].valueRef+
                     "\"\n  Node type       = \""+portSpecs[p].nodeType+
                     "\"\n  Data type       = \""+portSpecs[p].nodeType+
                     "\"\n  Causality       = \""+portSpecs[p].causality+"\"\n");


    /////////////////////////////////////
    // DEFINE PARAMETER SPECIFICATIONS //
    /////////////////////////////////////


    QList<FMIParameterSpecification> parSpecs;
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    int j=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            QString varName = toVarName("par"+QString::number(i));
            QString initValue = varElement.firstChildElement("Real").attribute("start", "0");
            QString parName = toVarName(varElement.attribute("name"));
            QString description = varElement.attribute("description");
            //QString valueRef = varElement.attribute("valueReference");
            QString valueRef = QString::number(j);
            parSpecs << FMIParameterSpecification(varName, parName, description, initValue, valueRef);
            ++i;

        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
        ++j;
    }

    //Print parameter info to user
    for(int p=0; p<parSpecs.size(); ++p)
        printMessage(QString("\nParameter:")+
                     "  \n  Variable        = \""+parSpecs[p].varName +
                     "\"\n  Name            = \""+parSpecs[p].parName +
                     "\"\n  Description     = \""+parSpecs[p].description+
                     "\"\n  Initial value   = \""+parSpecs[p].initValue+
                     "\"\n  Value reference = \""+parSpecs[p].valueRef+"\"\n");


    ///////////////////////////////////
    // GENERATE FILES FROM TEMPLATES //
    ///////////////////////////////////


    printMessage("Creating fmuLib.cc");

    //Create fmuLib.cc
    QFile fmuLibFile;
    fmuLibFile.setFileName(fmuDir.path() + "/fmuLib.cc");
    if(!fmuLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open fmuLib.cc for writing.");
        removeDir(fmuDir.path());
        return;
    }

    printMessage("Writing fmuLib.cc");

    QFile fmuLibTemplateFile(":templates/fmuLibTemplate.cc");
    assert(fmuLibTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuLibCode;
    QTextStream t(&fmuLibTemplateFile);
    fmuLibCode = t.readAll();
    fmuLibTemplateFile.close();
    assert(!fmuLibCode.isEmpty());

    fmuLibCode.replace("<<<0>>>", fmuName);
    fmuLibCode.replace("<<<1>>>", mCoreIncludePath);

    QTextStream fmuLibStream(&fmuLibFile);
    fmuLibStream << fmuLibCode;
    fmuLibFile.close();


    printMessage("Creating " + fmuName + ".hpp");

    //Create <fmuname>.hpp
    QDir().mkdir(fmuDir.path() + "/component_code");
    QFile fmuComponentHppFile;
    fmuComponentHppFile.setFileName(fmuDir.path() + "/component_code/" + fmuName + ".hpp");
    if(!fmuComponentHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".hpp for writing.");
        removeDir(fmuDir.path());
        return;
    }

    printMessage("Writing " + fmuName + ".hpp");

    QFile fmuComponentTemplateFile(":templates/fmuComponentTemplate.hpp");
    assert(fmuComponentTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuComponentCode;
    QTextStream t2(&fmuComponentTemplateFile);
    fmuComponentCode = t2.readAll();
    fmuComponentTemplateFile.close();
    assert(!fmuComponentCode.isEmpty());

    //Declare ports
    QString fmuComponentReplace2;
    QString portLine = extractTaggedSection(fmuComponentCode, "2");
    QStringList addedPorts;
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(!addedPorts.contains(portSpecs[p].varName))
        {
            fmuComponentReplace2.append(replaceTag(portLine, "varname", portSpecs[p].varName));
            addedPorts << portSpecs[p].varName;
        }
    }

    //Declare node data pointers
    QString fmuComponentReplace3;
    QString mpndLine = extractTaggedSection(fmuComponentCode, "3");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        fmuComponentReplace3.append(replaceTag(mpndLine, "mpndname", portSpecs[p].mpndName));
    }

    //Declare parameters
    QString fmuComponentReplace4;
    QString parLine = extractTaggedSection(fmuComponentCode, "4");
    for(int p=0; p<parSpecs.size(); ++p)
    {
        fmuComponentReplace4.append(replaceTag(parLine, "varname", parSpecs[p].varName));
    }

    //Add ports
    QString fmuComponentReplace7;
    QString addPortLine = extractTaggedSection(fmuComponentCode, "7");
    addedPorts.clear();
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(!addedPorts.contains(portSpecs[p].varName))
        {
            fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "varname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                    QStringList() << portSpecs[p].varName << portSpecs[p].portName << portSpecs[p].portType << portSpecs[p].nodeType << "Port::NOTREQUIRED"));
            addedPorts << portSpecs[p].varName;
        }
    }

    //Initialize and register parameters
    QString fmuComponentReplace8;
    QString regParLine = extractTaggedSection(fmuComponentCode, "8");
    for(int p=0; p<parSpecs.size(); ++p)
    {
        fmuComponentReplace8.append(replaceTags(regParLine, QStringList() << "varname" << "initvalue" << "parname" << "description",
                                                QStringList() << parSpecs[p].varName << parSpecs[p].initValue << parSpecs[p].parName << parSpecs[p].description));
    }

    //Declare node data pointers
    QString fmuComponentReplace9;
    QString getNodePtrLine = extractTaggedSection(fmuComponentCode, "9");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "mpndname" << "varname" << "datatype", QStringList() << portSpecs[p].mpndName << portSpecs[p].varName << portSpecs[p].dataType));
    }


    //Set parameters
    QString fmuComponentReplace10;
    QString writeParLines = extractTaggedSection(fmuComponentCode, "10");
    for(int p=0; p<parSpecs.size(); ++p)
    {
        fmuComponentReplace10.append(replaceTags(writeParLines, QStringList() << "valueref" << "varname", QStringList() << parSpecs[p].valueRef << parSpecs[p].varName));
    }

    //Write input values
    QString fmuComponentReplace11;
    QString writeVarLines = extractTaggedSection(fmuComponentCode, "11");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(portSpecs[p].causality != "output")
            fmuComponentReplace11.append(replaceTags(writeVarLines, QStringList() << "varname" << "valueref" << "mpndname", QStringList() << portSpecs[p].varName << portSpecs[p].valueRef << portSpecs[p].mpndName));
    }

    //Write back output values
    QString fmuComponentReplace12;
    QString readVarLines = extractTaggedSection(fmuComponentCode, "12");
    for(int p=0; p<portSpecs.size(); ++p)
    {
        if(portSpecs[p].causality != "input")
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << portSpecs[p].valueRef << portSpecs[p].mpndName));
    }

    QString fmuComponentReplace13;
#ifdef WIN32
    fmuComponentReplace13 = "dll";
#elif linux
    fmuComponentReplace13 = "so";
#endif

    fmuComponentCode.replace("<<<modelname>>>", fmuName);
    fmuComponentCode.replace("<<<includepath>>>", mCoreIncludePath);
    replaceTaggedSection(fmuComponentCode, "2", fmuComponentReplace2);
    replaceTaggedSection(fmuComponentCode, "3", fmuComponentReplace3);
    replaceTaggedSection(fmuComponentCode, "4", fmuComponentReplace4);
    fmuComponentCode.replace("<<<fmudir>>>", fmuDir.path());
    replaceTaggedSection(fmuComponentCode, "7", fmuComponentReplace7);
    replaceTaggedSection(fmuComponentCode, "8", fmuComponentReplace8);
    replaceTaggedSection(fmuComponentCode, "9", fmuComponentReplace9);
    replaceTaggedSection(fmuComponentCode, "10", fmuComponentReplace10);
    replaceTaggedSection(fmuComponentCode, "11", fmuComponentReplace11);
    replaceTaggedSection(fmuComponentCode, "12", fmuComponentReplace12);
    fmuComponentCode.replace("<<<13>>>", fmuComponentReplace13);

    QTextStream fmuComponentHppStream(&fmuComponentHppFile);
    fmuComponentHppStream << fmuComponentCode;
    fmuComponentHppFile.close();


    QString iconName = "fmucomponent.svg";
    QImage *pIconImage = new QImage(fmuPath+"/model.png");
    if(!pIconImage->isNull())
    {
        iconName = "model.svg";
    }


    printMessage("Writing "+fmuName+".xml");

    //Create <fmuname>.xml
    //! @todo Use dom elements for generating xml (this is just stupid)
    QFile fmuXmlFile;
    fmuXmlFile.setFileName(fmuDir.path() + "/" + fmuName + ".xml");
    if(!fmuXmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".xml for writing.");
        removeDir(fmuDir.path());
        return;
    }

    int nInputPorts = inVarValueRefs.size()+inoutVarValueRefs.size();
    int nOutputPorts = outVarValueRefs.size()+inoutVarValueRefs.size();
    double scale = max(max(nInputPorts, nOutputPorts)/2.0, 1.0);
    QString scaleStr = QString::number(scale);

    QTextStream fmuXmlStream(&fmuXmlFile);
    fmuXmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    fmuXmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
    fmuXmlStream << "    <modelobject typename=\""+fmuName+"\" displayname=\""+fmuName+"\">\n";
    fmuXmlStream << "        <icons>\n";
    fmuXmlStream << "            <icon type=\"user\" path=\"fmucomponent.svg\" iconrotation=\"ON\" scale=\""+scaleStr+"\"/>\n";
    fmuXmlStream << "        </icons>\n";
    fmuXmlStream << "        <ports>\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;

    double tlmPosStep=1.0/(tlmPortTypes.size()+1.0);      //These 2 variables are used for TLM port positioning
    double tlmPos=0;

    double inputPosStep=1.0/(nInputPorts+1.0);      //These 4 variables are used for input/output port positioning
    double outputPosStep=1.0/(nOutputPorts+1.0);
    double inputPos=0;
    double outputPos=0;

    QString numStr, numStr2;
    for(int i=0; i<tlmPortTypes.size(); ++i)
    {
        tlmPos += tlmPosStep;
        numStr = QString::number(i);
        numStr2.setNum(tlmPos);
        fmuXmlStream << "            <port name=\"P"+numStr+"\" x=\""+numStr2+"\" y=\"0.0\"  a=\"270\"/>\n";
    }
    for(int i=0; i<inoutVarValueRefs.size(); ++i)
    {
        inputPos += inputPosStep;
        numStr2.setNum(inputPos);
        fmuXmlStream << "            <port name=\""+toVarName(inoutVarPortNames[i])+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
        outputPos += outputPosStep;
        numStr2.setNum(outputPos);
        fmuXmlStream << "            <port name=\""+toVarName(inoutVarPortNames[i])+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
    }
    for(int i=0; i<inVarValueRefs.size(); ++i)
    {
        inputPos += inputPosStep;
        numStr2.setNum(inputPos);
        fmuXmlStream << "            <port name=\""+toVarName(inVarPortNames[i])+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
    }
    for(int i=0; i<outVarValueRefs.size(); ++i)
    {
        outputPos += outputPosStep;
        numStr2.setNum(outputPos);
        fmuXmlStream << "            <port name=\""+toVarName(outVarPortNames[i])+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
    }
    fmuXmlStream << "        </ports>\n";
    fmuXmlStream << "    </modelobject>\n";
    fmuXmlStream << "</hopsanobjectappearance>\n";
    fmuXmlFile.close();

    //Move FMI source files to compile directory
    QFile simSupportSourceFile;
#ifdef WIN32
    QString fmiSrcPath = gExecPath + "../ThirdParty/fmi/";
#elif linux
    QString fmiSrcPath = gExecPath + "../ThirdParty/fmi/linux/";
#endif
    simSupportSourceFile.setFileName(fmiSrcPath+"sim_support.c");
    if(simSupportSourceFile.copy(fmuDir.path() + "/sim_support.c"))
    {
        printMessage("Copying sim_support.c");
        printMessage("Copying " + simSupportSourceFile.fileName() + " to " + fmuDir.path() + "/sim_support.c");
    }

    QFile stackSourceFile;
    stackSourceFile.setFileName(fmiSrcPath+"stack.cc");
    if(stackSourceFile.copy(fmuDir.path() + "/stack.cc"))
    {
        printMessage("Copying stack.cc");
        printMessage("Copying " + stackSourceFile.fileName() + " to " + fmuDir.path() + "/stack.cc");
    }

    QFile xmlParserSourceFile;
    xmlParserSourceFile.setFileName(fmiSrcPath+"xml_parser.h");
    if(xmlParserSourceFile.copy(fmuDir.path() + "/xml_parser.h"))
    {
        printMessage("Copying xml_parser.h");
        printMessage("Copying " + xmlParserSourceFile.fileName() + " to " + fmuDir.path() + "/xml_parser.h");
    }

    QFile simSupportHeaderFile;
    simSupportHeaderFile.setFileName(fmiSrcPath+"sim_support.h");
    if(simSupportHeaderFile.copy(fmuDir.path() + "/sim_support.h"))
    {
        printMessage("Copying sim_support.h");
        printMessage("Copying " + simSupportHeaderFile.fileName() + " to " + fmuDir.path() + "/sim_support.h");
    }

    QFile stackHeaderFile;
    stackHeaderFile.setFileName(fmiSrcPath+"stack.h");
    if(stackHeaderFile.copy(fmuDir.path() + "/stack.h"))
    {
        printMessage("Copying stack.h");
        printMessage("Copying " + stackHeaderFile.fileName() + " to " + fmuDir.path() + "/stack.h");
    }

    QFile xmlParserHeaderFile;
    xmlParserHeaderFile.setFileName(fmiSrcPath+"xml_parser.cc");
    if(xmlParserHeaderFile.copy(fmuDir.path() + "/xml_parser.cc"))
    {
        printMessage("Copying xml_parser.cc");
        printMessage("Copying " + xmlParserHeaderFile.fileName() + " to " + fmuDir.path() + "/xml_parser.cc");
    }

    QFile expatFile;
    expatFile.setFileName(fmiSrcPath+"expat.h");
    if(expatFile.copy(fmuDir.path() + "/expat.h"))
    {
        printMessage("Copying expat.h");
        printMessage("Copying " + expatFile.fileName() + " to " + fmuDir.path() + "/expat.h");
    }

    QFile expatExternalFile;
    expatExternalFile.setFileName(fmiSrcPath+"expat_external.h");
    if(expatExternalFile.copy(fmuDir.path() + "/expat_external.h"))
    {
        printMessage("Copying expat_external.h");
        printMessage("Copying " + expatExternalFile.fileName() + " to " + fmuDir.path() + "/expat_external.h");
    }

#ifdef WIN32
    QFile libExpatAFile;
    libExpatAFile.setFileName(fmiSrcPath+"libexpat.a");
    if(libExpatAFile.copy(fmuDir.path() + "/libexpat.a"))
    {
        printMessage("Copying libexpat.a");
        printMessage("Copying " + libExpatAFile.fileName() + " to " + fmuDir.path() + "/libexpat.a");
    }

    QFile libExpatDllFile;
    libExpatDllFile.setFileName(fmiSrcPath+"libexpat.dll");
    if(libExpatDllFile.copy(fmuDir.path() + "/libexpat.dll"))
    {
        printMessage("Copying libexpat.dll");
        printMessage("Copying " + libExpatDllFile.fileName() + " to " + fmuDir.path() + "/libexpat.dll");
    }
#elif linux
    QFile libExpatMTLibFile;
    libExpatMTLibFile.setFileName(fmiSrcPath+"libexpatMT.lib");
    if(libExpatMTLibFile.copy(fmuDir.path() + "/libexpatMT.lib"))
    {
        printMessage("Copying libexpatMT.lib");
        printMessage("Copying " + libExpatMTLibFile.fileName() + " to " + fmuDir.path() + "/libexpatMT.lib");
    }
#endif

    QFile libExpatwAFile;
    libExpatwAFile.setFileName(fmiSrcPath+"libexpatw.a");
    if(libExpatwAFile.copy(fmuDir.path() + "/libexpatw.a"))
    {
        printMessage("Copying libexpatw.a");
        printMessage("Copying " + libExpatwAFile.fileName() + " to " + fmuDir.path() + "/libexpatw.a");
    }

    QFile libExpatwDllFile;
    libExpatwDllFile.setFileName(fmiSrcPath+"libexpatw.dll");
    if(libExpatwDllFile.copy(fmuDir.path() + "/libexpatw.dll"))
    {
        printMessage("Copying libexpatw.dll");
        printMessage("Copying " + libExpatwDllFile.fileName() + " to " + fmuDir.path() + "/libexpatw.dll");
    }

    QFile fmiMeFile;
    fmiMeFile.setFileName(fmiSrcPath+"fmi_me.h");
    if(fmiMeFile.copy(fmuDir.path() + "/fmi_me.h"))
    {
        printMessage("Copying fmi_me.h");
        printMessage("Copying " + fmiMeFile.fileName() + " to " + fmuDir.path() + "/fmi_me.h");
    }

    QFile fmiModelFunctionsFile;
    fmiModelFunctionsFile.setFileName(fmiSrcPath+"fmiModelFunctions.h");
    if(fmiModelFunctionsFile.copy(fmuDir.path() + "/fmiModelFunctions.h"))
    {
        printMessage("Copying fmiModelFunctions.h");
        printMessage("Copying " + fmiModelFunctionsFile.fileName() + " to " + fmuDir.path() + "/fmiModelFunctions.h");
    }

    QFile fmiModelTypesFile;
    fmiModelTypesFile.setFileName(fmiSrcPath+"fmiModelTypes.h");
    if(fmiModelTypesFile.copy(fmuDir.path() + "/fmiModelTypes.h"))
    {
        printMessage("Copying fmiModelTypes.h");
        printMessage("Copying " + fmiModelTypesFile.fileName() + " to " + fmuDir.path() + "/fmiModelTypes.h");
    }

#ifdef WIN32
    if(!compileComponentLibrary(fmuDir.path(), "fmuLib", this, "-L./ -llibexpat"))
#else
    if(!compileComponentLibrary(fmuDir.path(), "fmuLib", this))
#endif
    {
        printMessage("Failed to import fmu.");
        return;
    }
    else
    {
        printMessage("Removing temporary files");

        //Cleanup temporary files
        fmuDir.remove("sim_support.h");
        fmuDir.remove("sim_support.c");
        fmuDir.remove("stack.h");
        fmuDir.remove("stack.cc");
        fmuDir.remove("xml_parser.h");
        fmuDir.remove("xml_parser.cc");
        fmuDir.remove("expat.h");
        fmuDir.remove("expat_external.h");
        fmuDir.remove("fmi_me.h");
        fmuDir.remove("fmiModelFunctions.h");
        fmuDir.remove("fmiModelTypes.h");
        fmuDir.remove("compile.bat");
        fmuComponentHppFile.remove();
        fmuLibFile.remove();
        fmuDir.rmdir("component_code");
        QDir binDir;
        binDir.setPath(fmuDir.path() + "/binaries");
        binDir.rmdir("win32");
        fmuDir.rmdir("binaries");
        printMessage("Finished!");
    }
}


void HopsanFMIGenerator::generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem)
{
    printMessage("Initializing FMU export...");


    QDir saveDir;
    saveDir.setPath(savePath);

    //! @todo Make global
    QString gExecPath = qApp->applicationDirPath().append('/');

    printMessage("Verifying that required files exist...");

    //Make sure HopsanCore source files are available
    QStringList srcFiles;
    srcFiles << "Component.cc" <<
                "ComponentSystem.cc" <<
                "HopsanEssentials.cc" <<
                "Node.cc" <<
                "Nodes.cc" <<
                "Parameters.cc" <<
                "Port.cc";
    if(!assertFilesExist(gExecPath+"../HopsanCore/src", srcFiles))
        return;
    srcFiles.clear();
    srcFiles << "AuxiliarySimulationFunctions.cc" <<
                "CSVParser.cc" <<
                "DoubleIntegratorWithDamping.cc" <<
                "DoubleIntegratorWithDampingAndCoulumbFriction.cc" <<
                "EquationSystemSolver.cpp" <<
                "FirstOrderTransferFunction.cc" <<
                "Integrator.cc" <<
                "IntegratorLimited.cc" <<
                "ludcmp.cc" <<
                "matrix.cc" <<
                "SecondOrderTransferFunction.cc" <<
                "TurbulentFlowFunction.cc" <<
                "ValveHysteresis.cc" <<
                "WhiteGaussianNoise.cc";
    if(!assertFilesExist(gExecPath+"../HopsanCore/src/ComponentUtilities/", srcFiles))
        return;
    srcFiles.clear();
    srcFiles << "CoSimulationUtilities.cpp" <<
                "GeneratorHandler.cpp" <<
                "HmfLoader.cc" <<
                "HopsanCoreMessageHandler.cc" <<
                "LoadExternal.cc" <<
                "MultiThreadingUtilities.cpp";
    if(!assertFilesExist(gExecPath+"../HopsanCore/src/CoreUtilities/", srcFiles))
        return;
    if(!assertFilesExist(gExecPath+"../componentLibraries/defaultLibrary/code/", QStringList() << "defaultComponentLibraryInternal.cc"))
        return;
    if(!assertFilesExist(gExecPath+"../HopsanCore/Dependencies/libcsv_parser++-1.0.0/", QStringList() << "csv_parser.cpp"))
        return;


    //Tells if user selected the gcc compiler or not (= visual studio)
    //bool gccCompiler = mpExportFmuGccRadioButton->isChecked();

    //Write the FMU ID
    int random = rand() % 1000;
    QString randomString = QString::number(random);
    QString ID = "{8c4e810f-3df3-4a00-8276-176fa3c9f"+randomString+"}";  //!< @todo How is this ID defined?

    //Collect information about input ports
    QStringList inputVariables;
    QStringList inputComponents;
    QStringList inputPorts;
    QList<int> inputDatatypes;

    std::vector<std::string> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        if(pSystem->getSubComponent(names[i])->getTypeName() == "SignalInputInterface")
        {
            inputVariables.append(QString(names[i].c_str()).remove(' '));
            inputComponents.append(QString(names[i].c_str()));
            inputPorts.append("out");
            inputDatatypes.append(0);
        }
    }

    //Collect information about output ports
    QStringList outputVariables;
    QStringList outputComponents;
    QStringList outputPorts;
    QList<int> outputDatatypes;

    names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        if(pSystem->getSubComponent(names[i])->getTypeName() == "SignalOutputInterface")
        {
            outputVariables.append(QString(names[i].c_str()).remove(' '));
            outputComponents.append(QString(names[i].c_str()));
            outputPorts.append("in");
            outputDatatypes.append(0);
        }
    }

    //Collect information about system parameters
    QStringList parameterNames;
    QStringList parameterValues;
    std::vector<std::string> parameterNamesStd;
    pSystem->getParameterNames(parameterNamesStd);
    for(int p=0; p<parameterNamesStd.size(); ++p)
    {
        parameterNames.append(QString(parameterNamesStd[p].c_str()));
    }
    for(int p=0; p<parameterNames.size(); ++p)
    {
        std::string value;
        pSystem->getParameterValue(parameterNamesStd[p], value);
        parameterValues.append(QString(value.c_str()));
    }


    //Create file objects for all files that shall be created
    QFile modelSourceFile;
    QString modelName = QString::fromStdString(pSystem->getName());
   // modelName.chop(4);
    QString realModelName = modelName;          //Actual model name (used for hmf file)
    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open " + modelName + ".c for writing.");
        return;
    }

    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QFile fmuHeaderFile;
    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QFile fmuSourceFile;
    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

#ifdef WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(savePath + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
#endif

    QFile modelHppFile;
    modelHppFile.setFileName(savePath + "/model.hpp");
    if(!modelHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open model.hpp for writing.");
        return;
    }

    printMessage("Writing modelDescription.xml...");

    QFile xmlTemplatefile(":templates/fmuModelDescriptionTemplate.xml");
    if(!xmlTemplatefile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QString xmlCode;
    QTextStream t(&xmlTemplatefile);
    xmlCode = t.readAll();
    xmlTemplatefile.close();
    if(xmlCode.isEmpty())
    {
        printErrorMessage("Failed to generate XML code for modelDescription.xml.");
        return;
    }

    QString xmlReplace3;
    QString scalarVarLines = extractTaggedSection(xmlCode, "3");
    int i, j;
    for(i=0; i<inputVariables.size(); ++i)
    {
        QString refString = QString::number(i);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << inputVariables.at(i) << refString << "input"));
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QString refString = QString::number(i+j);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << outputVariables.at(j) << refString << "output"));
    }

    QString xmlReplace4;
    QString paramLines = extractTaggedSection(xmlCode, "4");
    for(int k=0; k<parameterNames.size(); ++k)
    {
        QString refString = QString::number(i+j+k);
        xmlReplace4.append(replaceTags(paramLines, QStringList() << "varname" << "varref" << "variability" << "start", QStringList() << parameterNames[k] << refString << "parameter" << parameterValues[k]));
    }

    xmlCode.replace("<<<0>>>", modelName);
    xmlCode.replace("<<<1>>>", ID);
    xmlCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));
    replaceTaggedSection(xmlCode, "3", xmlReplace3);
    replaceTaggedSection(xmlCode, "4", xmlReplace4);

    QTextStream modelDescriptionStream(&modelDescriptionFile);
    modelDescriptionStream << xmlCode;
    modelDescriptionFile.close();


    printMessage("Writing " + modelName + ".c...");

    QFile sourceTemplateFile(":templates/fmuModelSourceTemplate.c");
    if(!sourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+modelName+".c for writing.");
        return;
    }
    QString modelSourceCode;
    QTextStream t2(&sourceTemplateFile);
    modelSourceCode = t2.readAll();
    sourceTemplateFile.close();
    if(modelSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for "+modelName+".c.");
        return;
    }

    QString sourceReplace4;
    QString varDefLine = extractTaggedSection(modelSourceCode, "4");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << inputVariables.at(i) << QString::number(i)));
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << outputVariables.at(j) << QString::number(i+j)));
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << parameterNames.at(k) << QString::number(i+j+k)));

    QString sourceReplace5;
    i=0;
    j=0;
    if(!inputVariables.isEmpty())
    {
        sourceReplace5.append(inputVariables.at(0)+"_");
        ++i;
    }
    else if(!outputVariables.isEmpty())
    {
        sourceReplace5.append(outputVariables.at(0)+"_");
        ++j;
    }
    for(; i<inputVariables.size(); ++i)
        sourceReplace5.append(", "+inputVariables.at(i)+"_");
    for(; j<outputVariables.size(); ++j)
        sourceReplace5.append(", "+outputVariables.at(j)+"_");

    QString sourceReplace6;
    QString startValueLine = extractTaggedSection(modelSourceCode, "6");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace6.append(replaceTag(startValueLine, "varname", inputVariables.at(i)));         //!< Fix start value handling
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace6.append(replaceTag(startValueLine, "varname", outputVariables.at(j)));        //!< Fix start value handling

    QString sourceReplace8;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace8.append("           case "+inputVariables.at(i)+"_: return getVariable(\""+inputComponents.at(i)+"\", \""+inputPorts.at(i)+"\", "+QString::number(inputDatatypes.at(i))+");\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace8.append("           case "+outputVariables.at(j)+"_: return getVariable(\""+outputComponents.at(j)+"\", \""+outputPorts.at(j)+"\", "+QString::number(outputDatatypes.at(j))+");\n");

    QString sourceReplace9;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace9.append("           case "+inputVariables.at(i)+"_: setVariable(\""+inputComponents.at(i)+"\", \""+inputPorts.at(i)+"\", "+QString::number(inputDatatypes.at(i))+", value); break;\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace9.append("           case "+outputVariables.at(j)+"_: setVariable(\""+outputComponents.at(j)+"\", \""+outputPorts.at(j)+"\", "+QString::number(outputDatatypes.at(j))+", value); break;\n");
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace9.append("           case "+parameterNames.at(k)+"_: setParameter(\""+parameterNames.at(k)+"\", value); break;\n");

    modelSourceCode.replace("<<<0>>>", modelName);
    modelSourceCode.replace("<<<1>>>", ID);
    modelSourceCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));
    modelSourceCode.replace("<<<3>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));  //!< @todo Does number of variables equal number of states?
    replaceTaggedSection(modelSourceCode, "4", sourceReplace4);
    modelSourceCode.replace("<<<5>>>", sourceReplace5);
    replaceTaggedSection(modelSourceCode, "6", sourceReplace6);
    modelSourceCode.replace("<<<7>>>", modelName);
    modelSourceCode.replace("<<<8>>>", sourceReplace8);
    modelSourceCode.replace("<<<9>>>", sourceReplace9);

    QTextStream modelSourceStream(&modelSourceFile);
    modelSourceStream << modelSourceCode;
    modelSourceFile.close();


    printMessage("Writing HopsanFMU.h...");

    QFile fmuHeaderTemplateFile(":templates/fmuHeaderTemplate.h");
    if(!fmuHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QString fmuHeaderCode;
    QTextStream t3(&fmuHeaderTemplateFile);
    fmuHeaderCode = t3.readAll();
    fmuHeaderTemplateFile.close();
    if(fmuHeaderCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.h.");
        return;
    }

    QTextStream fmuHeaderStream(&fmuHeaderFile);
    fmuHeaderStream << fmuHeaderCode;
    fmuHeaderFile.close();


    printMessage("Writing HopsanFMU.cpp...");
    //! @todo Time step should not be hard coded

    QFile fmuSourceTemplateFile(":templates/fmuSourceTemplate.c");
    if(!fmuSourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

    QString fmuSourceCode;
    QTextStream t4(&fmuSourceTemplateFile);
    fmuSourceCode = t4.readAll();
    fmuSourceTemplateFile.close();
    if(fmuSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.cpp.");
        return;
    }

    QTextStream fmuSourceStream(&fmuSourceFile);
    fmuSourceStream << fmuSourceCode;
    fmuSourceFile.close();

    printMessage("Copying HopsanCore files...");


    //Copy include files to export directory
    copyIncludeFilesToDir2(savePath);
    copySourceFilesToDir(savePath);
    copyDefaultComponentCodeToDir(savePath);


    printMessage("Copying FMI files...");

    QFile fmuModelFunctionsHFile(gExecPath + "/../ThirdParty/fmi/fmiModelFunctions.h");
    fmuModelFunctionsHFile.copy(savePath + "/fmiModelFunctions.h");
    QFile fmiModelTypesHFile(gExecPath + "/../ThirdParty/fmi/fmiModelTypes.h");
    fmiModelTypesHFile.copy(savePath + "/fmiModelTypes.h");
    QFile fmiTemplateCFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.c");
    fmiTemplateCFile.copy(savePath + "/fmuTemplate.c");
    QFile fmiTemplateHFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.h");
    fmiTemplateHFile.copy(savePath + "/fmuTemplate.h");

    if(!assertFilesExist(savePath, QStringList() << "fmiModelFunctions.h" << "fmiModelTypes.h" << "fmuTemplate.c" << "fmuTemplate.h"))
        return;

    printMessage("Generating model file...");

    QStringList modelLines;
    QFile modelFile(savePath + "/" + realModelName + ".hmf");
    if (!modelFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Could not open "+realModelName+".hmf for reading.");
        return;
    }
    while (!modelFile.atEnd())
    {
        QString line = modelFile.readLine();
        line.chop(1);
        line.replace("\"", "\\\"");
        modelLines.append(line);
    }
    modelFile.close();

    QTextStream modelHppStream(&modelHppFile);
    modelHppStream << "#include <vector>\n\n";
    modelHppStream << "std::vector<unsigned char> getModelString()\n{\n";
    modelHppStream << "    std::string model = ";
    Q_FOREACH(const QString line, modelLines)
    {
        modelHppStream << "\""+line+"\"\n";
    }
    modelHppStream << "    ;\n\n";
    modelHppStream << "    std::vector<unsigned char> cModel;\n";
    modelHppStream << "    for(size_t i=0; i<model.size(); ++i)\n    {\n";
    modelHppStream << "        cModel.push_back(model[i]);\n    }\n\n";
    modelHppStream << "    return cModel;\n}\n";
    modelHppFile.close();

#ifdef WIN32
    printMessage("Compiling "+modelName+".dll...");
    //Write the compilation script file
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "gcc.exe -c -w -shared -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c\n";
    clBatchStream << "g++ -w -shared -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB -o "+modelName+".dll "+modelName+".o HopsanFMU.cpp src/Component.cc src/ComponentSystem.cc src/HopsanEssentials.cc src/Node.cc src/Nodes.cc src/Parameters.cc src/Port.cc src/ComponentUtilities/AuxiliarySimulationFunctions.cc src/ComponentUtilities/CSVParser.cc src/ComponentUtilities/DoubleIntegratorWithDamping.cc src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc src/ComponentUtilities/EquationSystemSolver.cpp src/ComponentUtilities/FirstOrderTransferFunction.cc src/ComponentUtilities/Integrator.cc src/ComponentUtilities/IntegratorLimited.cc src/ComponentUtilities/ludcmp.cc src/ComponentUtilities/matrix.cc src/ComponentUtilities/SecondOrderTransferFunction.cc src/ComponentUtilities/TurbulentFlowFunction.cc src/ComponentUtilities/ValveHysteresis.cc src/ComponentUtilities/WhiteGaussianNoise.cc src/CoreUtilities/CoSimulationUtilities.cpp src/CoreUtilities/GeneratorHandler.cpp src/CoreUtilities/HmfLoader.cc src/CoreUtilities/HopsanCoreMessageHandler.cc src/CoreUtilities/LoadExternal.cc src/CoreUtilities/MultiThreadingUtilities.cpp componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp -Iinclude -IcomponentLibraries/defaultLibrary/code -IDependencies/rapidxml-1.13 -IDependencies/libcsv_parser++-1.0.0/include/csv_parser\n";
    clBatchFile.close();

    callProcess("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & compile.bat");

    if(!assertFilesExist(savePath, QStringList() << modelName+".dll"))
        return;

#elif linux
    printMessage("Compiling "+modelName+".so");
    //! @todo Fix compilation for linux

        QString gccCommand1 = "cd "+savePath+" && gcc -c -w -shared -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c\n";
        QString gccCommand2 = "cd "+savePath+" && g++ -w -shared -fPIC -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB -o "+modelName+".so "+modelName+".o HopsanFMU.cpp src/Component.cc src/ComponentSystem.cc src/HopsanEssentials.cc src/Node.cc src/Nodes.cc src/Parameters.cc src/Port.cc src/ComponentUtilities/AuxiliarySimulationFunctions.cc src/ComponentUtilities/CSVParser.cc src/ComponentUtilities/DoubleIntegratorWithDamping.cc src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc src/ComponentUtilities/EquationSystemSolver.cpp src/ComponentUtilities/FirstOrderTransferFunction.cc src/ComponentUtilities/Integrator.cc src/ComponentUtilities/IntegratorLimited.cc src/ComponentUtilities/ludcmp.cc src/ComponentUtilities/matrix.cc src/ComponentUtilities/SecondOrderTransferFunction.cc src/ComponentUtilities/TurbulentFlowFunction.cc src/ComponentUtilities/ValveHysteresis.cc src/ComponentUtilities/WhiteGaussianNoise.cc src/CoreUtilities/CoSimulationUtilities.cpp src/CoreUtilities/GeneratorHandler.cpp src/CoreUtilities/HmfLoader.cc src/CoreUtilities/HopsanCoreMessageHandler.cc src/CoreUtilities/LoadExternal.cc src/CoreUtilities/MultiThreadingUtilities.cpp componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp -Iinclude -IcomponentLibraries/defaultLibrary/code -IDependencies/rapidxml-1.13 -IDependencies/libcsv_parser++-1.0.0/include/csv_parser\n";

        qDebug() << "Command 1 = " << gccCommand1;
        qDebug() << "Command 2 = " << gccCommand2;

        callProcess("gcc", QStringList() << "-c" << "-w" << "-shared" << "-fPIC" << "-Wl,--rpath,'$ORIGIN/.'" << modelName+".c", savePath);

        callProcess("g++", QStringList() << "-w" << "-shared" << "-fPIC" << "-DDOCOREDLLEXPORT" << "-DBUILTINDEFAULTCOMPONENTLIB" << "-o"+modelName+".so" << modelName+".o" << "HopsanFMU.cpp" << "src/Component.cc" << "src/ComponentSystem.cc" << "src/HopsanEssentials.cc" << "src/Node.cc" << "src/Nodes.cc" << "src/Parameters.cc" << "src/Port.cc" << "src/ComponentUtilities/AuxiliarySimulationFunctions.cc" << "src/ComponentUtilities/CSVParser.cc" << "src/ComponentUtilities/DoubleIntegratorWithDamping.cc" << "src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc" << "src/ComponentUtilities/EquationSystemSolver.cpp" << "src/ComponentUtilities/FirstOrderTransferFunction.cc" << "src/ComponentUtilities/Integrator.cc" << "src/ComponentUtilities/IntegratorLimited.cc" << "src/ComponentUtilities/ludcmp.cc" << "src/ComponentUtilities/matrix.cc" << "src/ComponentUtilities/SecondOrderTransferFunction.cc" << "src/ComponentUtilities/TurbulentFlowFunction.cc" << "src/ComponentUtilities/ValveHysteresis.cc" << "src/ComponentUtilities/WhiteGaussianNoise.cc" << "src/CoreUtilities/CoSimulationUtilities.cpp" << "src/CoreUtilities/GeneratorHandler.cpp" << "src/CoreUtilities/HmfLoader.cc" << "src/CoreUtilities/HopsanCoreMessageHandler.cc" << "src/CoreUtilities/LoadExternal.cc" << "src/CoreUtilities/MultiThreadingUtilities.cpp" << "componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc" << "Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp" << "-Iinclude" << "-IcomponentLibraries/defaultLibrary/code" << "-IDependencies/rapidxml-1.13" << "-IDependencies/libcsv_parser++-1.0.0/include/csv_parser", savePath);
//        QString output;
//        if(!runUnixCommand(gccCommand1))
//            return;

//        output.clear();
//        if(!runUnixCommand(gccCommand1))
//            return;

//        sleep(30);
        if(!assertFilesExist(savePath, QStringList() << modelName+".so"))
            return;
#endif

    printMessage("Sorting files...");

#ifdef WIN32
    saveDir.mkpath("fmu/binaries/win32");
    saveDir.mkpath("fmu/resources");
    QFile modelDllFile(savePath + "/" + modelName + ".dll");
    modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
    QFile modelLibFile(savePath + "/" + modelName + ".lib");
    modelLibFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".lib");
#elif linux && __i386__
    saveDir.mkpath("fmu/binaries/linux32");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux32/" + modelName + ".so");
#elif linux && __x86_64__
    saveDir.mkpath("fmu/binaries/linux64");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux64/" + modelName + ".so");
#endif
   // QFile modelFile(savePath + "/" + realModelName + ".hmf");
    modelFile.copy(savePath + "/fmu/resources/" + realModelName + ".hmf");
    modelDescriptionFile.copy(savePath + "/fmu/modelDescription.xml");

    QString fmuFileName = savePath + "/fmu/" + modelName + ".fmu";

    printMessage("Compressing files...");

#ifdef WIN32
    QString program = gExecPath + "../ThirdParty/7z/7z";
    QStringList arguments = QStringList() << "a" << "-tzip" << fmuFileName << savePath+"/fmu/modelDescription.xml" << "-r" << savePath + "/fmu/binaries";
    callProcess(program, arguments, savePath+"/fmu");
    QFile::copy(fmuFileName, savePath+"/"+modelName+".fmu");
    QFile::remove(fmuFileName);

#elif linux
    QString command = "cd "+savePath+"/fmu && zip -r ../"+modelName+".fmu *";
    qDebug() << "Command = " << command;
    command +=" 2>&1";
    char line[130];
    FILE *fp = popen(  (const char *) command.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + command + "'! err=%d.");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            printMessage((const QString &)line);
        }
    }
#endif

    if(!assertFilesExist(savePath, QStringList() << modelName+".fmu"))
        return;

    //! @todo Shall we activate cleanup function or not?

    //printMessage("Cleaning up...");

    //Clean up temporary files
//    saveDir.setPath(savePath);
//    saveDir.remove("compile.bat");
//    saveDir.remove("fmiModelFunctions.h");
//    saveDir.remove("fmiModelTypes.h");
//    saveDir.remove("fmuTemplate.c");
//    saveDir.remove("fmuTemplate.h");
//    saveDir.remove(modelName + ".c");
//    saveDir.remove(modelName + ".exp");
//    //saveDir.remove(modelName + ".lib");
//    saveDir.remove(modelName + ".obj");
//    saveDir.remove("HopsanFMU.exp");
//    saveDir.remove("HopsanFMU.h");
//    removeDir(savePath + "/include");
//    removeDir(savePath + "/fmu");

    printMessage("Finished.");
}






void HopsanFMIGenerator::generateToFmuOld(QString savePath, hopsan::ComponentSystem *pSystem)
{
    printMessage("Initializing FMU export");

    QDir saveDir;
    saveDir.setPath(savePath);

    //! @todo Make global
    QString gExecPath = qApp->applicationDirPath().append('/');


    //Tells if user selected the gcc compiler or not (= visual studio)
    //bool gccCompiler = mpExportFmuGccRadioButton->isChecked();

    //Write the FMU ID
    int random = rand() % 1000;
    QString randomString = QString::number(random);
    QString ID = "{8c4e810f-3df3-4a00-8276-176fa3c9f"+randomString+"}";  //!< @todo How is this ID defined?

    //Collect information about input ports
    QStringList inputVariables;
    QStringList inputComponents;
    QStringList inputPorts;
    QList<int> inputDatatypes;

    std::vector<std::string> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        if(pSystem->getSubComponent(names[i])->getTypeName() == "SignalInputInterface")
        {
            inputVariables.append(QString(names[i].c_str()).remove(' '));
            inputComponents.append(QString(names[i].c_str()));
            inputPorts.append("out");
            inputDatatypes.append(0);
        }
    }

    //Collect information about output ports
    QStringList outputVariables;
    QStringList outputComponents;
    QStringList outputPorts;
    QList<int> outputDatatypes;

    names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        if(pSystem->getSubComponent(names[i])->getTypeName() == "SignalOutputInterface")
        {
            outputVariables.append(QString(names[i].c_str()).remove(' '));
            outputComponents.append(QString(names[i].c_str()));
            outputPorts.append("in");
            outputDatatypes.append(0);
        }
    }

    //Collect information about system parameters
    QStringList parameterNames;
    QStringList parameterValues;
    std::vector<std::string> parameterNamesStd;
    pSystem->getParameterNames(parameterNamesStd);
    for(int p=0; p<parameterNamesStd.size(); ++p)
    {
        parameterNames.append(QString(parameterNamesStd[p].c_str()));
    }
    for(int p=0; p<parameterNames.size(); ++p)
    {
        std::string value;
        pSystem->getParameterValue(parameterNamesStd[p], value);
        parameterValues.append(QString(value.c_str()));
    }


    //Create file objects for all files that shall be created
    QFile modelSourceFile;
    QString modelName = QString::fromStdString(pSystem->getName());
   // modelName.chop(4);
    QString realModelName = modelName;          //Actual model name (used for hmf file)
    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open " + modelName + ".c for writing.");
        return;
    }

    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QFile fmuHeaderFile;
    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QFile fmuSourceFile;
    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

#ifdef WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(savePath + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
#endif

    printMessage("Writing modelDescription.xml");

    QFile xmlTemplatefile(":templates/fmuModelDescriptionTemplate.xml");
    if(!xmlTemplatefile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QString xmlCode;
    QTextStream t(&xmlTemplatefile);
    xmlCode = t.readAll();
    xmlTemplatefile.close();
    if(xmlCode.isEmpty())
    {
        printErrorMessage("Failed to generate XML code for modelDescription.xml");
        return;
    }

    QString xmlReplace3;
    QString scalarVarLines = extractTaggedSection(xmlCode, "3");
    int i, j;
    for(i=0; i<inputVariables.size(); ++i)
    {
        QString refString = QString::number(i);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << inputVariables.at(i) << refString << "input"));
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QString refString = QString::number(i+j);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << outputVariables.at(j) << refString << "output"));
    }

    QString xmlReplace4;
    QString paramLines = extractTaggedSection(xmlCode, "4");
    for(int k=0; k<parameterNames.size(); ++k)
    {
        QString refString = QString::number(i+j+k);
        xmlReplace4.append(replaceTags(paramLines, QStringList() << "varname" << "varref" << "variability" << "start", QStringList() << parameterNames[k] << refString << "parameter" << parameterValues[k]));
    }

    xmlCode.replace("<<<0>>>", modelName);
    xmlCode.replace("<<<1>>>", ID);
    xmlCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));
    replaceTaggedSection(xmlCode, "3", xmlReplace3);
    replaceTaggedSection(xmlCode, "4", xmlReplace4);

    QTextStream modelDescriptionStream(&modelDescriptionFile);
    modelDescriptionStream << xmlCode;
    modelDescriptionFile.close();


    printMessage("Writing " + modelName + ".c");

    QFile sourceTemplateFile(":templates/fmuModelSourceTemplate.c");
    if(!sourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+modelName+".c for writing.");
        return;
    }
    QString modelSourceCode;
    QTextStream t2(&sourceTemplateFile);
    modelSourceCode = t2.readAll();
    sourceTemplateFile.close();
    if(modelSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for "+modelName+".c");
        return;
    }

    QString sourceReplace4;
    QString varDefLine = extractTaggedSection(modelSourceCode, "4");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << inputVariables.at(i) << QString::number(i)));
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << outputVariables.at(j) << QString::number(i+j)));
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << parameterNames.at(k) << QString::number(i+j+k)));

    QString sourceReplace5;
    i=0;
    j=0;
    if(!inputVariables.isEmpty())
    {
        sourceReplace5.append(inputVariables.at(0)+"_");
        ++i;
    }
    else if(!outputVariables.isEmpty())
    {
        sourceReplace5.append(outputVariables.at(0)+"_");
        ++j;
    }
    for(; i<inputVariables.size(); ++i)
        sourceReplace5.append(", "+inputVariables.at(i)+"_");
    for(; j<outputVariables.size(); ++j)
        sourceReplace5.append(", "+outputVariables.at(j)+"_");

    QString sourceReplace6;
    QString startValueLine = extractTaggedSection(modelSourceCode, "6");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace6.append(replaceTag(startValueLine, "varname", inputVariables.at(i)));         //!< Fix start value handling
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace6.append(replaceTag(startValueLine, "varname", outputVariables.at(j)));        //!< Fix start value handling

    QString sourceReplace8;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace8.append("           case "+inputVariables.at(i)+"_: return getVariable(\""+inputComponents.at(i)+"\", \""+inputPorts.at(i)+"\", "+QString::number(inputDatatypes.at(i))+");\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace8.append("           case "+outputVariables.at(j)+"_: return getVariable(\""+outputComponents.at(j)+"\", \""+outputPorts.at(j)+"\", "+QString::number(outputDatatypes.at(j))+");\n");

    QString sourceReplace9;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace9.append("           case "+inputVariables.at(i)+"_: setVariable(\""+inputComponents.at(i)+"\", \""+inputPorts.at(i)+"\", "+QString::number(inputDatatypes.at(i))+", value); break;\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace9.append("           case "+outputVariables.at(j)+"_: setVariable(\""+outputComponents.at(j)+"\", \""+outputPorts.at(j)+"\", "+QString::number(outputDatatypes.at(j))+", value); break;\n");
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace9.append("           case "+parameterNames.at(k)+"_: setParameter(\""+parameterNames.at(k)+"\", value); break;\n");

    modelSourceCode.replace("<<<0>>>", modelName);
    modelSourceCode.replace("<<<1>>>", ID);
    modelSourceCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));
    modelSourceCode.replace("<<<3>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));  //!< @todo Does number of variables equal number of states?
    replaceTaggedSection(modelSourceCode, "4", sourceReplace4);
    modelSourceCode.replace("<<<5>>>", sourceReplace5);
    replaceTaggedSection(modelSourceCode, "6", sourceReplace6);
    modelSourceCode.replace("<<<7>>>", modelName);
    modelSourceCode.replace("<<<8>>>", sourceReplace8);
    modelSourceCode.replace("<<<9>>>", sourceReplace9);

    QTextStream modelSourceStream(&modelSourceFile);
    modelSourceStream << modelSourceCode;
    modelSourceFile.close();


    printMessage("Writing HopsanFMU.h");

    QFile fmuHeaderTemplateFile(":templates/fmuHeaderTemplate.h");
    if(!fmuHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QString fmuHeaderCode;
    QTextStream t3(&fmuHeaderTemplateFile);
    fmuHeaderCode = t3.readAll();
    fmuHeaderTemplateFile.close();
    if(fmuHeaderCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.h");
        return;
    }

    QTextStream fmuHeaderStream(&fmuHeaderFile);
    fmuHeaderStream << fmuHeaderCode;
    fmuHeaderFile.close();


    printMessage("Writing HopsanFMU.cpp");
    //! @todo Time step should not be hard coded

    QFile fmuSourceTemplateFile(":templates/fmuSourceTemplate.c");
    if(!fmuSourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

    QString fmuSourceCode;
    QTextStream t4(&fmuSourceTemplateFile);
    fmuSourceCode = t4.readAll();
    fmuSourceTemplateFile.close();
    if(fmuSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.cpp");
        return;
    }

    QTextStream fmuSourceStream(&fmuSourceFile);
    fmuSourceStream << fmuSourceCode;
    fmuSourceFile.close();



#ifdef WIN32
    printMessage("Writing to compile.bat");

    //Write the compilation script file
    QTextStream clBatchStream(&clBatchFile);
//    if(gccCompiler)
//    {
        //! @todo Ship Mingw with Hopsan, or check if it exists in system and inform user if it does not.
    clBatchStream << "g++ -DWRAPPERCOMPILATION -c -Wl,--rpath,'$ORIGIN/.' HopsanFMU.cpp -I./include\n";
    clBatchStream << "g++ -shared -Wl,--rpath,'$ORIGIN/.' -o HopsanFMU.dll HopsanFMU.o -L./ -lHopsanCore";
//    }
//    else
//    {
//        //! @todo Check that Visual Studio is installed, and warn user if not
//        clBatchStream << "echo Compiling Visual Studio libraries...\n";
//        clBatchStream << "if defined VS90COMNTOOLS (call \"%VS90COMNTOOLS%\\vsvars32.bat\") else ^\n";
//        clBatchStream << "if defined VS80COMNTOOLS (call \"%VS80COMNTOOLS%\\vsvars32.bat\")\n";
//        clBatchStream << "cl -LD -nologo -DWIN32 -DWRAPPERCOMPILATION HopsanFMU.cpp /I \\. /I \\include\\HopsanCore.h HopsanCore.lib\n";
//    }
    clBatchFile.close();
#endif

    printMessage("Copying binary files");


    //Copy binaries to export directory
#ifdef WIN32
    QFile dllFile;
    QFile componentLibFile;

    dllFile.setFileName(gExecPath + "HopsanCore.dll");
    dllFile.copy(savePath + "/HopsanCore.dll");

    componentLibFile.setFileName(gExecPath + "../componentLibraries/defaultLibrary/components/defaultComponentLibrary.dll");
    if(componentLibFile.exists())
        componentLibFile.copy(savePath+"/defaultComponentLibrary.dll");
#elif linux
    QFile soFile;
    QFile componentLibFile;

    soFile.setFileName(gExecPath + "libHopsanCore.so");
    soFile.copy(savePath + "/libHopsanCore.so");

    componentLibFile.setFileName(gExecPath + "../componentLibraries/defaultLibrary/components/libdefaultComponentLibrary.so");
    if(componentLibFile.exists())
        componentLibFile.copy(savePath+"/libdefaultComponentLibrary.so");
#endif


    printMessage("Copying include files");


    //Copy include files to export directory
    copyIncludeFilesToDir2(savePath);


    printMessage("Writing "+realModelName+".hmf");


    //! @todo Use core save function

#ifdef WIN32
    printMessage("Compiling HopsanFMU.dll");
#elif linux
    printMessage("Compiling HopsanFMU.so");
#endif


#ifdef WIN32
    //Execute HopsanFMU compile script
    QProcess p;
    p.setWorkingDirectory(savePath);
    p.start("cmd.exe", QStringList() << "/c" << "compile.bat");
    bool succeeded = p.waitForFinished();
    qDebug() << "Success: " << succeeded << ", savePath = " << savePath;

    if(!assertFilesExist(savePath, QStringList() << "HopsanFMU.dll"))
        return;

#elif linux
    QString gccCommand1 = "cd "+savePath+" && g++ -DWRAPPERCOMPILATION -fPIC -Wl,--rpath,'$ORIGIN/.' -c HopsanFMU.cpp -I./include\n";
    QString gccCommand2 = "cd "+savePath+" && g++ -shared -Wl,--rpath,'$ORIGIN/.' -o libHopsanFMU.so HopsanFMU.o -L./ -lHopsanCore";

    qDebug() << "Command 1 = " << gccCommand1;
    qDebug() << "Command 2 = " << gccCommand2;

    char line[130];
    gccCommand1 +=" 2>&1";
    FILE *fp = popen(  (const char *) gccCommand1.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + gccCommand1 + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
           printMessage((const QString &)line);
        }
    }

    gccCommand2 +=" 2>&1";
    fp = popen(  (const char *) gccCommand2.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + gccCommand2 + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            printMessage((const QString &)line);
        }
    }
#endif


    printMessage("Copying compilation files");


    //Copy FMI compilation files to export directory
#ifdef WIN32
    QFile buildFmuFile;
//    if(gccCompiler)
//    {
        buildFmuFile.setFileName(gExecPath + "/../ThirdParty/fmi/build_fmu_gcc.bat");
//    }
//    else
//    {
//        buildFmuFile.setFileName(gExecPath + "/../ThirdParty/fmi/build_fmu_vc.bat");
//    }
    buildFmuFile.copy(savePath + "/build_fmu.bat");
#endif
    QFile fmuModelFunctionsHFile(gExecPath + "/../ThirdParty/fmi/fmiModelFunctions.h");
    fmuModelFunctionsHFile.copy(savePath + "/fmiModelFunctions.h");
    QFile fmiModelTypesHFile(gExecPath + "/../ThirdParty/fmi/fmiModelTypes.h");
    fmiModelTypesHFile.copy(savePath + "/fmiModelTypes.h");
    QFile fmiTemplateCFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.c");
    fmiTemplateCFile.copy(savePath + "/fmuTemplate.c");
    QFile fmiTemplateHFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.h");
    fmiTemplateHFile.copy(savePath + "/fmuTemplate.h");

#ifdef WIN32
    printMessage("Compiling "+modelName+".dll");
#elif linux
    printMessage("Compiling "+modelName+".so");
#endif


    QString c = modelName+".c";
    QString inc = "";
    QString l = "-L./ -lHopsanFMU";
    QString flags = "-shared -fPIC -Wl,--rpath,'$ORIGIN/.'";

    printMessage("\nCalling compiler utility:");
    printMessage("Path: "+savePath);
    printMessage("Objective: "+modelName);
    printMessage("Source files: "+c);
    printMessage("Includes: "+inc);
    printMessage("Links: "+l+"\n");
    printMessage("Flags: "+flags);

    QString output;
    compile(savePath, modelName, c, inc, l, flags, output);
    printMessage(output);

#ifdef WIN32
    if(!assertFilesExist(savePath, QStringList() << modelName+".dll"))
        return;
#elif LINUX
    if(!assertFilesExist(savePath, QStringList() << modelName+".so"))
        return;
#endif

//#ifdef WIN32
//    //Execute FMU compile script
//    p.start("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & build_fmu.bat me " + modelName);
//    p.waitForFinished();
//#elif linux
//    gccCommand1 = "cd "+savePath+" && gcc -c -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c";
//    gccCommand2 = "cd "+savePath+" && gcc -shared -Wl,--rpath,'$ORIGIN/.' -o "+modelName+".so "+modelName+".o -L./ -lHopsanFMU";

//    qDebug() << "Command 1 = " << gccCommand1;
//    qDebug() << "Command 2 = " << gccCommand2;

//    gccCommand1 +=" 2>&1";
//    fp = popen(  (const char *) gccCommand1.toStdString().c_str(), "r");
//    if ( !fp )
//    {
//        printErrorMessage("Could not execute '" + gccCommand1 + "'! err=%d");
//        return;
//    }
//    else
//    {
//        while ( fgets( line, sizeof line, fp))
//        {
//            printMessage((const QString &)line);
//        }
//    }

//    gccCommand2 +=" 2>&1";
//    fp = popen(  (const char *) gccCommand2.toStdString().c_str(), "r");
//    if ( !fp )
//    {
//        printErrorMessage("Could not execute '" + gccCommand2 + "'! err=%d");
//        return;
//    }
//    else
//    {
//        while ( fgets( line, sizeof line, fp))
//        {
//            printMessage((const QString &)line);
//        }
//    }
//#endif

    printMessage("Sorting files");


#ifdef WIN32
    saveDir.mkpath("fmu/binaries/win32");
    saveDir.mkpath("fmu/resources");
    QFile modelDllFile(savePath + "/" + modelName + ".dll");
    if(!modelDllFile.exists())
    {
        printErrorMessage("Failed to compile " + modelName + ".dll");
        return;
    }
    modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
    QFile modelLibFile(savePath + "/" + modelName + ".lib");
    modelLibFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".lib");
    dllFile.copy(savePath + "/fmu/binaries/win32/HopsanCore.dll");
    componentLibFile.copy(savePath +"/fmu/binaries/win32/defaultComponentLibrary.dll");
    QFile hopsanFMUdllFile(savePath + "/HopsanFMU.dll");
    if(!hopsanFMUdllFile.exists())
    {
        printErrorMessage("Failed to compile HopsanFMU.dll");
        return;
    }
    hopsanFMUdllFile.copy(savePath + "/fmu/binaries/win32/HopsanFMU.dll");
    QFile hopsanFMUlibFile(savePath + "/HopsanFMU.lib");
    hopsanFMUlibFile.copy(savePath + "/fmu/binaries/win32/HopsanFMU.lib");
#elif linux && __i386__
    saveDir.mkpath("fmu/binaries/linux32");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux32/" + modelName + ".so");
    QFile hopsanFMUsoFile(savePath + "/libHopsanFMU.so");
    hopsanFMUsoFile.copy(savePath + "/fmu/binaries/linux32/libHopsanFMU.so");
#elif linux && __x86_64__
    saveDir.mkpath("fmu/binaries/linux64");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux64/" + modelName + ".so");
    QFile hopsanFMUsoFile(savePath + "/libHopsanFMU.so");
    hopsanFMUsoFile.copy(savePath + "/fmu/binaries/linux64/libHopsanFMU.so");
#endif
    QFile modelFile(savePath + "/" + realModelName + ".hmf");
    modelFile.copy(savePath + "/fmu/resources/" + realModelName + ".hmf");
    modelDescriptionFile.copy(savePath + "/fmu/modelDescription.xml");

    QString fmuFileName = savePath + "/" + modelName + ".fmu";

    printMessage("Compressing files");


#ifdef WIN32
    p.start("cmd.exe", QStringList() << "/c" << gExecPath + "../ThirdParty/7z/7z.exe a -tzip " + fmuFileName + " " + savePath + "/fmu/modelDescription.xml " + savePath + "/fmu/binaries/ " + savePath + "/fmu/resources");
    p.waitForFinished();
    qDebug() << "Called: " << gExecPath + "../ThirdParty/7z/7z.exe a -tzip " + fmuFileName + " " + savePath + "/fmu/modelDescription.xml " + savePath + "/fmu/binaries/ " + savePath + "/fmu/resources";
#elif linux
    QString command = "cd "+savePath+"/fmu && zip -r ../"+modelName+".fmu *";
    qDebug() << "Command = " << command;
    command +=" 2>&1";
    fp = popen(  (const char *) command.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + command + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            printMessage((const QString &)line);
        }
    }
#endif

    if(!QFile().exists(fmuFileName))
    {
        printErrorMessage("Unable to compress "+modelName+".fmu");
        return;
    }

    //! @todo Shall we activate cleanup function or not?

    //printMessage("Cleaning up");

    //Clean up temporary files
//    saveDir.setPath(savePath);
//    saveDir.remove("compile.bat");
//    saveDir.remove("HopsanFMU.cpp");
//    saveDir.remove("HopsanFMU.obj");
//    //saveDir.remove("HopsanFMU.lib");
//    //saveDir.remove("HopsanCore.lib");
//    saveDir.remove("HopsanCore.exp");
//    saveDir.remove("build_fmu.bat");
//    saveDir.remove("fmiModelFunctions.h");
//    saveDir.remove("fmiModelTypes.h");
//    saveDir.remove("fmuTemplate.c");
//    saveDir.remove("fmuTemplate.h");
//    saveDir.remove(modelName + ".c");
//    saveDir.remove(modelName + ".exp");
//    //saveDir.remove(modelName + ".lib");
//    saveDir.remove(modelName + ".obj");
//    saveDir.remove("HopsanFMU.exp");
//    saveDir.remove("HopsanFMU.h");
//    removeDir(savePath + "/include");
//    removeDir(savePath + "/fmu");

    printMessage("Finished!");
}
