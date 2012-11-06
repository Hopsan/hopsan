/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ComponentGeneratorUtilities.cc
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utiluties
//!
//$Id$

#include <QStringList>
#include <QProcess>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QApplication>
#include <QPushButton>
#include <QProgressDialog>
#include <QDomElement>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

#include <cassert>

#ifdef WIN32
#include <windows.h>
#endif

#include "HopsanComponentGenerator.h"
#include "SymHop.h"

#include "ComponentSystem.h"
#include "Port.h"
#include "version.h"


using namespace std;
using namespace SymHop;
using namespace hopsan;


PortSpecification::PortSpecification(QString porttype, QString nodetype, QString name, bool notrequired, QString defaultvalue)
{
    this->porttype = porttype;
    this->nodetype = nodetype;
    this->name = name;
    this->notrequired = notrequired;
    this->defaultvalue = defaultvalue;
}


ParameterSpecification::ParameterSpecification(QString name, QString displayName, QString description, QString unit, QString init)
{
    this->name = name;
    this->displayName = displayName;
    this->description = description;
    this->unit = unit;
    this->init = init;
}


UtilitySpecification::UtilitySpecification(QString utility, QString name)
{
    this->utility = utility;
    this->name = name;
}


StaticVariableSpecification::StaticVariableSpecification(QString datatype, QString name)
{
    this->datatype = datatype;
    this->name = name;
}



ComponentSpecification::ComponentSpecification(QString typeName, QString displayName, QString cqsType)
{
    this->typeName = typeName;
    this->displayName = displayName;
    if(cqsType == "S")
        cqsType = "Signal";
    this->cqsType = cqsType;
}



HopsanGenerator::HopsanGenerator(QString coreIncludePath, QString binPath, bool showDialog)
{
#ifdef WIN32
    mOutputPath = "C:/HopsanComponentGeneratorTempFiles/output/";
    mTempPath = "C:/HopsanComponentGeneratorTempFiles/temp/";
#else
    mOutputPath = QDir::currentPath()+"/output/";
    mTempPath = QDir::currentPath()+"/temp/";
#endif
    mCoreIncludePath = coreIncludePath;
    mBinPath = binPath;

    mShowDialog = showDialog;

    if(mShowDialog)
    {
        mpTextEdit = new QTextEdit();
        mpTextEdit->setReadOnly(true);
        QFont monoFont = QFont("Monospace", 10, 50);
        monoFont.setStyleHint(QFont::TypeWriter);
        mpTextEdit->setFont(monoFont);

        mpDoneButton = new QPushButton("Close");
        mpDoneButton->setFixedWidth(200);

        mpLayout = new QVBoxLayout();
        mpLayout->addWidget(mpTextEdit);
        mpLayout->addWidget(mpDoneButton);
        mpLayout->setAlignment(mpDoneButton, Qt::AlignCenter);

        mpDialog = new QWidget(0);
        mpDialog->setWindowModality(Qt::ApplicationModal);
        mpDialog->setLayout(mpLayout);
        mpDialog->setMinimumSize(640, 480);
        mpDialog->setWindowTitle("HopsanGenerator");

        mpDoneButton->connect(mpDoneButton, SIGNAL(clicked()), mpDialog, SLOT(hide()));

        mpDialog->show();
        QApplication::processEvents();

        printMessage("##########################\n# Loaded HopsanGenerator #\n##########################\n");
    }
}


void HopsanGenerator::printMessage(QString msg)
{
    if(mShowDialog)
    {
        mpTextEdit->setTextColor(QColor("Black"));
        mpTextEdit->append(msg);
        QApplication::processEvents();
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }
}


void HopsanGenerator::printErrorMessage(QString msg)
{
    if(mShowDialog)
    {
        mpTextEdit->setTextColor(QColor("Red"));
        mpTextEdit->append(msg);
        QApplication::processEvents();
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }
}


void HopsanGenerator::generateFromModelica(QString code)
{
    QString typeName, displayName, cqsType;
    QStringList initAlgorithms, equations, finalAlgorithms;
    QList<PortSpecification> portList;
    QList<ParameterSpecification> parametersList;
    ComponentSpecification comp;

    qDebug() << "Parsing!";
    printMessage("Parsing Modelica code...");

    //Parse Modelica code and generate equation system
    parseModelicaModel(code, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);

    qDebug() << "Transforming!";
    printMessage("Transforming...");

    //Transform equation system, generate Jacobian
    generateComponentObject(comp, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);

    qDebug() << "Compiling!";
    printMessage("Generating component...");

    //Compile component
    compileFromComponentObject(typeName+".hpp", comp, false);

    qDebug() << "Finished!";
    printMessage("HopsanGenerator finished!");
}


void HopsanGenerator::generateFromFmu(QString path)
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

    if(!QDir(gExecPath + "../import").exists())
        QDir().mkdir(gExecPath + "../import");

    if(!QDir(gExecPath + "../import/FMU").exists())
        QDir().mkdir(gExecPath + "../import/FMU");

    //DEBUG DEBUG DEBUG
    removeDir(gExecPath+"../import/FMU/"+fmuName);
    //END DEBUG

    if(!QDir(gExecPath + "../import/FMU/" + fmuName).exists())
    {
        QDir().mkdir(gExecPath + "../import/FMU/" + fmuName);
    }
    else
    {
        printErrorMessage("Directory already exists. Another FMU with same cannot be imported.");
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

    QStringList inVars, inVarNames;
    QStringList outVars, outVarNames;
    QStringList inoutVars, inoutVarNames;
    QDomElement variablesElement = fmuRoot.firstChildElement("ModelVariables");
    QDomElement varElement = variablesElement.firstChildElement("ScalarVariable");
    int i=0;
    while (!varElement.isNull() && varElement.attribute("variability") != "parameter")
    {
        if(!varElement.hasAttribute("causality"))
        {
            inoutVars << varElement.attribute("valueReference");
            inoutVarNames << varElement.attribute("name");
        }
        else if(varElement.attribute("causality") == "input")
        {
            inVars << varElement.attribute("valueReference");
            inVarNames << varElement.attribute("name");
        }
        else if(varElement.attribute("causality") == "output")
        {
            outVars << varElement.attribute("valueReference");
            outVarNames << varElement.attribute("name");
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }

    QStringList tlmPortTypes;
    QList<QStringList> tlmPortVars;
    QList<QStringList> tlmPortRefs;


    //Read from [modelName]_TLM.xml if it exists, to define TLM powerports
    QFile tlmSpecFile;
    tlmSpecFile.setFileName(fmuFileInfo.path() + "/" + fmuName + "_TLM.xml");
    QDomDocument tlmDomDocument;
    QDomElement tlmRoot = loadXMLDomDocument(tlmSpecFile, tlmDomDocument, "hopsanfmu");
    tlmSpecFile.close();

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
                if(outVarNames.contains(input[1]) && outVarNames.contains(input[2]) && inVarNames.contains(input[3]) && inVarNames.contains(input[4]))
                {
                    printMessage("Adding hydraulic port");

                    tlmPortTypes.append(input[0]);
                    input.removeFirst();
                    tlmPortVars.append(input);

                    tlmPortRefs.append(QStringList());
                    tlmPortRefs.last().append(outVars[outVarNames.indexOf(input[0])]);
                    tlmPortRefs.last().append(outVars[outVarNames.indexOf(input[1])]);
                    tlmPortRefs.last().append(inVars[inVarNames.indexOf(input[2])]);
                    tlmPortRefs.last().append(inVars[inVarNames.indexOf(input[3])]);

                    outVars.removeAt(outVarNames.indexOf(input[0]));
                    outVarNames.removeAll(input[0]);

                    outVars.removeAt(outVarNames.indexOf(input[1]));
                    outVarNames.removeAll(input[1]);

                    inVars.removeAt(inVarNames.indexOf(input[2]));
                    inVarNames.removeAll(input[2]);

                    inVars.removeAt(inVarNames.indexOf(input[3]));
                    inVarNames.removeAll(input[3]);
                }
            }
            else if(input.first() == "mechanic" && input.size() == 7)
            {
                if(outVarNames.contains(input[1]) && outVarNames.contains(input[2]) && outVarNames.contains(input[3]) && outVarNames.contains(input[4]) && inVarNames.contains(input[5]) && inVarNames.contains(input[6]))
                {
                    printMessage("Adding mechanical port");

                    tlmPortTypes.append(input[0]);
                    input.removeFirst();
                    tlmPortVars.append(input);

                    tlmPortRefs.append(QStringList());
                    tlmPortRefs.last().append(outVars[outVarNames.indexOf(input[0])]);
                    tlmPortRefs.last().append(outVars[outVarNames.indexOf(input[1])]);
                    tlmPortRefs.last().append(outVars[outVarNames.indexOf(input[2])]);
                    tlmPortRefs.last().append(outVars[outVarNames.indexOf(input[3])]);
                    tlmPortRefs.last().append(inVars[inVarNames.indexOf(input[4])]);
                    tlmPortRefs.last().append(inVars[inVarNames.indexOf(input[5])]);

                    outVars.removeAt(outVarNames.indexOf(input[0]));
                    outVarNames.removeAll(input[0]);

                    outVars.removeAt(outVarNames.indexOf(input[1]));
                    outVarNames.removeAll(input[1]);

                    outVars.removeAt(outVarNames.indexOf(input[2]));
                    outVarNames.removeAll(input[2]);

                    outVars.removeAt(outVarNames.indexOf(input[3]));
                    outVarNames.removeAll(input[3]);

                    inVars.removeAt(inVarNames.indexOf(input[4]));
                    inVarNames.removeAll(input[4]);

                    inVars.removeAt(inVarNames.indexOf(input[5]));
                    inVarNames.removeAll(input[5]);
                }
            }

            portElement = portElement.nextSiblingElement("tlmport");
        }
    }

    //! @todo Make parameters use predefined lists instead

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

    QString fmuComponentReplace2;
    QString portLine = extractTaggedSection(fmuComponentCode, "2");
    for(int i=0; i<tlmPortVars.size(); ++i)
    {
        QString numStr = QString::number(i);
        fmuComponentReplace2.append(replaceTag(portLine, "Portname", "P"+numStr));
    }
    int j=0;
    for(int i=0; i<inoutVars.size(); ++i)
    {
        QString numStr = inoutVars[i];
        fmuComponentReplace2.append(replaceTag(portLine, "Portname", "In"+numStr));
        fmuComponentReplace2.append(replaceTag(portLine, "Portname", "Out"+numStr));
        ++j;
    }
    for(int i=0; i<inVars.size(); ++i)
    {
        QString numStr = inVars[i];
        fmuComponentReplace2.append(replaceTag(portLine, "Portname", "In"+numStr));
        ++j;
    }
    for(int i=0; i<outVars.size(); ++i)
    {
        QString numStr = outVars[i];
        fmuComponentReplace2.append(replaceTag(portLine, "Portname", "Out"+numStr));
        ++j;
    }

    QString fmuComponentReplace3;
    QString mpndLine = extractTaggedSection(fmuComponentCode, "3");
    for(int i=0; i<tlmPortVars.size(); ++i)
    {
        QString numStr = QString::number(i);
        if(tlmPortTypes[i] == "hydraulic")
        {
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "p"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "q"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "c"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "Zc"+numStr));
        }
        else if(tlmPortTypes[i] == "mechanic")
        {
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "f"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "x"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "v"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "me"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "c"+numStr));
            fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "Zc"+numStr));
         }
    }
    j=0;
    for(int i=0; i<inoutVars.size(); ++i)
    {
        QString numStr = inoutVars[i];
        fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "in"+numStr));
        fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "out"+numStr));
        ++j;
    }
    for(int i=0; i<inVars.size(); ++i)
    {
        QString numStr = inVars[i];
        fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "in"+numStr));
        ++j;
    }
    for(int i=0; i<outVars.size(); ++i)
    {
        QString numStr = outVars[i];
        fmuComponentReplace3.append(replaceTag(mpndLine, "portname", "out"+numStr));
        ++j;
    }

    QString fmuComponentReplace4;
    QString parLine = extractTaggedSection(fmuComponentCode, "4");
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            fmuComponentReplace4.append(replaceTag(parLine, "parnum", QString::number(i)));
            ++i;
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }

    QString fmuComponentReplace7;
    QString addPortLine = extractTaggedSection(fmuComponentCode, "7");
    for(int i=0; i<tlmPortVars.size(); ++i)
    {
        QString numStr = QString::number(i);
        if(tlmPortTypes[i] == "hydraulic")
        {
            fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "Portname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                                 QStringList() << "P"+numStr << "P"+numStr << "PowerPort" << "NodeHydraulic" << ""));
        }
        else if(tlmPortTypes[i] == "mechanic")
        {
            fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "Portname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                                 QStringList() << "P"+numStr << "P"+numStr << "PowerPort" << "NodeMechanic" << ""));
        }
    }
    j=0;
    for(int i=0; i<inoutVars.size(); ++i)
    {
        QString numStr = inoutVars[i];
        fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "Portname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                             QStringList() << "In"+numStr << inoutVarNames[i]+"In" << "ReadPort" << "NodeSignal" << ", Port::NOTREQUIRED"));
        fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "Portname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                             QStringList() << "Out"+numStr << inoutVarNames[i]+"Out" << "WritePort" << "NodeSignal" << ", Port::NOTREQUIRED"));

        ++j;
    }
    for(int i=0; i<inVars.size(); ++i)
    {
        QString numStr = inVars[i];
        fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "Portname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                             QStringList() << "In"+numStr << inoutVarNames[i]+"In" << "ReadPort" << "NodeSignal" << ", Port::NOTREQUIRED"));
        ++j;
    }
    for(int i=0; i<outVars.size(); ++i)
    {
        QString numStr = outVars[i];
        fmuComponentReplace7.append(replaceTags(addPortLine, QStringList() << "Portname" << "portname" << "porttype" << "nodetype" << "notrequired",
                                                             QStringList() << "Out"+numStr << inoutVarNames[i]+"Out" << "WritePort" << "NodeSignal" << ", Port::NOTREQUIRED"));
        ++j;
    }

    QString fmuComponentReplace8;
    QString regParLine = extractTaggedSection(fmuComponentCode, "8");
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            QStringList tags = QStringList() << "parvalue" << "parnum" << "parname" << "pardesc";
            QStringList replacements = QStringList() << varElement.firstChildElement("Real").attribute("start") << QString::number(i) << varElement.attribute("name") << varElement.attribute("description");
            fmuComponentReplace8.append(replaceTags(regParLine, tags, replacements));
            ++i;
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }

    QString fmuComponentReplace9;
    QString getNodePtrLine = extractTaggedSection(fmuComponentCode, "9");
    for(int i=0; i<tlmPortTypes.size(); ++i)
    {
        QString numStr = QString::number(i);
        if(tlmPortTypes[i] == "hydraulic")
        {
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "p"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "q"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "c"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "Zc"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
        }
        else if(tlmPortTypes[i] == "mechanic")
        {
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "f"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "x"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "v"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "me"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "c"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
            fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "Zc"+numStr << "P"+numStr << "NodeHydraulic::PRESSURE"));
       }
    }
    j=0;
    for(int i=0; i<inoutVars.size(); ++i)
    {
        QString numStr = inoutVars[i];
        fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "in"+numStr << "In"+numStr << "NodeSignal::VALUE"));
        fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "out"+numStr << "Out"+numStr << "NodeSignal::VALUE"));
        ++j;
    }
    for(int i=0; i<inVars.size(); ++i)
    {
        QString numStr = inVars[i];
        fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "in"+numStr << "In"+numStr << "NodeSignal::VALUE"));
        ++j;
    }
    for(int i=0; i<outVars.size(); ++i)
    {
        QString numStr = outVars[i];
        fmuComponentReplace9.append(replaceTags(getNodePtrLine,QStringList() << "varname" << "portname" << "datatype", QStringList() << "out"+numStr << "Out"+numStr << "NodeSignal::VALUE"));
        ++j;
    }

    QString fmuComponentReplace10;
    QString writeParLines = extractTaggedSection(fmuComponentCode, "10");
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            fmuComponentReplace10.append(replaceTags(writeParLines, QStringList() << "valueref" << "parnum", QStringList() << varElement.attribute("valueReference") << QString::number(i)));
            ++i;
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }

    QString fmuComponentReplace11;
    QString writeVarLines = extractTaggedSection(fmuComponentCode, "11");
    for(int i=0; i<tlmPortTypes.size(); ++i)
    {
        QString numStr = QString::number(i);
        if(tlmPortTypes[i] == "hydraulic")
        {
           fmuComponentReplace11.append(replaceTags(writeVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][2] << "c"+numStr));
            fmuComponentReplace11.append(replaceTags(writeVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][3] << "Zc"+numStr));
        }
        if(tlmPortTypes[i] == "mechanic")
        {
            fmuComponentReplace11.append(replaceTags(writeVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][3] << "c"+numStr));
            fmuComponentReplace11.append(replaceTags(writeVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][4] << "Zc"+numStr));
        }
    }


    QString fmuComponentReplace6;
    QString writeVarSignalLines = extractTaggedSection(fmuComponentCode, "6");
    for(int i=0; i<inoutVars.size(); ++i)
    {
        QString numStr = inoutVars[i];
        fmuComponentReplace6.append(replaceTag(writeVarSignalLines, "valueref", numStr));
    }
    for(int i=0; i<inVars.size(); ++i)
    {
        QString numStr = inVars[i];
        fmuComponentReplace6.append(replaceTag(writeVarSignalLines, "valueref", numStr));
    }

    QString fmuComponentReplace12;
    QString readVarLines = extractTaggedSection(fmuComponentCode, "12");
    for(int i=0; i<tlmPortTypes.size(); ++i)
    {
        QString numStr = QString::number(i);
        if(tlmPortTypes[i] == "hydraulic")
        {
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][0] << "p"+numStr));
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][1] << "q"+numStr));
        }
        else if(tlmPortTypes[i] == "mechanic")
        {
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][0] << "f"+numStr));
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][1] << "x"+numStr));
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][2] << "v"+numStr));
            fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << tlmPortRefs[i][3] << "me"+numStr));
        }
    }
    for(int i=0; i<inoutVars.size(); ++i)
    {
        QString numStr = inoutVars[i];
        fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << numStr << "out"+numStr));
    }
    for(int i=0; i<outVars.size(); ++i)
    {
        QString numStr = outVars[i];
        fmuComponentReplace12.append(replaceTags(readVarLines, QStringList() << "valueref" << "varname", QStringList() << numStr << "out"+numStr));
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
    replaceTaggedSection(fmuComponentCode, "6", fmuComponentReplace6);
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

    QTextStream fmuXmlStream(&fmuXmlFile);
    fmuXmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    fmuXmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
    fmuXmlStream << "    <modelobject typename=\""+fmuName+"\" displayname=\""+fmuName+"\">\n";
    fmuXmlStream << "        <icons>\n";
    fmuXmlStream << "            <icon type=\"user\" path=\"fmucomponent.svg\" iconrotation=\"ON\" scale=\"1.0\"/>\n";
    fmuXmlStream << "        </icons>\n";
    fmuXmlStream << "        <ports>\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;

    double tlmPosStep=1.0/(tlmPortTypes.size()+1.0);      //These 2 variables are used for TLM port positioning
    double tlmPos=0;

    double inputPosStep=1.0/(inVars.size()+inoutVars.size()+1.0);      //These 4 variables are used for input/output port positioning
    double outputPosStep=1.0/(outVars.size()+inoutVars.size()+1.0);
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
    for(int i=0; i<inoutVars.size(); ++i)
    {
        inputPos += inputPosStep;
        numStr2.setNum(inputPos);
        fmuXmlStream << "            <port name=\""+inoutVarNames[i]+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
        outputPos += outputPosStep;
        numStr2.setNum(outputPos);
        fmuXmlStream << "            <port name=\""+inoutVarNames[i]+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
    }
    for(int i=0; i<inVars.size(); ++i)
    {
        inputPos += inputPosStep;
        numStr2.setNum(inputPos);
        fmuXmlStream << "            <port name=\""+inVarNames[i]+"\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
    }
    for(int i=0; i<outVars.size(); ++i)
    {
        outputPos += outputPosStep;
        numStr2.setNum(outputPos);
        fmuXmlStream << "            <port name=\""+outVarNames[i]+"\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
    }

//    while (!varElement.isNull())
//    {
//        QString numStr, numStr2;
//        numStr.setNum(i);
//        if(!varElement.hasAttribute("causality"))
//        {
//            inputPos += inputPosStep;
//            numStr2.setNum(inputPos);
//            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
//            outputPos += outputPosStep;
//            numStr2.setNum(outputPos);
//            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
//        }
//        else if(varElement.attribute("causality") == "input")
//        {
//            inputPos += inputPosStep;
//            numStr2.setNum(inputPos);
//            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
//        }
//        else if(varElement.attribute("causality") == "output")
//        {
//            outputPos += outputPosStep;
//            numStr2.setNum(outputPos);
//            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
//        }
//        ++i;
//        varElement = varElement.nextSiblingElement("ScalarVariable");
//    }
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

    printMessage("Writing compilation script");

    //Create compilation script file
#ifdef WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(fmuDir.path() + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open compile.bat for writing.");
        removeDir(fmuDir.path());
        return;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "g++.exe -shared fmuLib.cc stack.cc xml_parser.cc -o fmuLib.dll -L../../../bin/ -lHopsanCore -L./ -llibexpat\n";
    clBatchFile.close();
#endif

#ifdef WIN32
    printMessage("Compiling " + fmuName + ".dll");
#elif linux
    printMessage("Compiling " + fmuName + ".so");
#endif

    //Call compilation script file
#ifdef WIN32
    QProcess gccProcess;
    gccProcess.start("cmd.exe", QStringList() << "/c" << "cd " + fmuDir.path() + " & compile.bat");
    gccProcess.waitForFinished();
    QByteArray gccResult = gccProcess.readAll();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        QString msg = gccResultList.at(i);
        msg = msg.remove(msg.size()-1, 1);
        if(!msg.isEmpty())
        {
            printMessage(msg);
        }
    }
#elif linux
    QString gccCommand = "cd "+fmuDir.path()+" && g++ -fPIC -w -Wl,--rpath -Wl,"+fmuDir.path()+" -shared fmuLib.cc stack.cc xml_parser.cc -fpermissive -o fmuLib.so -I./ -L../../../bin/ -lHopsanCore";
    qDebug() << "Command = " << gccCommand;
    gccCommand +=" 2>&1";
    fp = popen(  (const char *) gccCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + gccCommand + "'! err=%d");
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

#ifdef WIN32
    if(!fmuDir.exists(fmuName + ".dll"))
    {
        printErrorMessage("Import of FMU failed: Compilation error.");
        removeDir(fmuDir.path());
        return;
    }
#elif linux
    if(!fmuDir.exists(fmuName + ".so"))
    {
        qDebug() << fmuDir.absolutePath();
        qDebug() << fmuName + ".so";
        printErrorMessage("Import of FMU failed: Compilation error.");
        //removeDir(fmuDir.path());
        return;
    }
#endif


    printMessage("Removing temporary files");

    //Cleanup temporary files
//    fmuDir.remove("sim_support.h");
//    fmuDir.remove("sim_support.c");
//    fmuDir.remove("stack.h");
//    fmuDir.remove("stack.cc");
//    fmuDir.remove("xml_parser.h");
//    fmuDir.remove("xml_parser.cc");
//    fmuDir.remove("expat.h");
//    fmuDir.remove("expat_external.h");
//    fmuDir.remove("fmi_me.h");
//    fmuDir.remove("fmiModelFunctions.h");
//    fmuDir.remove("fmiModelTypes.h");
//    fmuDir.remove("compile.bat");
//    fmuComponentHppFile.remove();
//    fmuLibFile.remove();
//    fmuDir.rmdir("component_code");
//    QDir binDir;
//    binDir.setPath(fmuDir.path() + "/binaries");
//    binDir.rmdir("win32");
//    fmuDir.rmdir("binaries");


    printMessage("Finished!");
}


void HopsanGenerator::generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem)
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
    assert(xmlTemplatefile.open(QIODevice::ReadOnly | QIODevice::Text));

    QString xmlCode;
    QTextStream t(&xmlTemplatefile);
    xmlCode = t.readAll();
    xmlTemplatefile.close();
    assert(!xmlCode.isEmpty());

    QString xmlReplace3;
    int i, j;
    for(i=0; i<inputVariables.size(); ++i)
    {
        QString refString = QString::number(i);
        xmlReplace3.append("  <ScalarVariable name=\""+inputVariables.at(i)+"\" valueReference=\""+refString+"\" description=\"input variable\" causality=\"input\">\n");
        xmlReplace3.append("     <Real start=\"0\" fixed=\"false\"/>\n");
        xmlReplace3.append("  </ScalarVariable>\n");
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QString refString = QString::number(i+j);
        xmlReplace3.append("  <ScalarVariable name=\""+outputVariables.at(j)+"\" valueReference=\""+refString+"\" description=\"output variable\" causality=\"output\">\n");
        xmlReplace3.append("     <Real start=\"0\" fixed=\"false\"/>\n");
        xmlReplace3.append("  </ScalarVariable>\n");
    }

    xmlCode.replace("<<<0>>>", modelName);
    xmlCode.replace("<<<1>>>", ID);
    xmlCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size()));
    xmlCode.replace("<<<3>>>", xmlReplace3);

    QTextStream modelDescriptionStream(&modelDescriptionFile);
    modelDescriptionStream << xmlCode;
    modelDescriptionFile.close();


    printMessage("Writing " + modelName + ".c");

    QFile sourceTemplateFile(":templates/fmuModelSourceTemplate.c");
    assert(sourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString modelSourceCode;
    QTextStream t2(&sourceTemplateFile);
    modelSourceCode = t2.readAll();
    sourceTemplateFile.close();
    assert(!modelSourceCode.isEmpty());

    QString sourceReplace4;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace4.append("    #define " + inputVariables.at(i) + "_ " + QString::number(i) + "\n\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace4.append("    #define " + outputVariables.at(j) + "_ " + QString::number(j+i) + "\n\n");

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
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace6.append("        r("+inputVariables.at(i)+"_) = 0;\n");        //!< Fix start value handling
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace6.append("        r("+outputVariables.at(j)+"_) = 0;\n");        //!< Fix start value handling

    QString sourceReplace8;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace8.append("           case "+inputVariables.at(i)+"_: return getVariable(\""+inputComponents.at(i)+"\", \""+inputPorts.at(i)+"\", "+QString::number(inputDatatypes.at(i))+");\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace8.append("           case "+outputVariables.at(j)+"_: return getVariable(\""+outputComponents.at(j)+"\", \""+outputPorts.at(j)+"\", "+QString::number(outputDatatypes.at(j))+");\n");

    QString sourceReplace9;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace9.append("           case "+inputVariables.at(i)+"_: setVariable(\""+inputComponents.at(i)+"\", \""+inputPorts.at(i)+"\", "+QString::number(inputDatatypes.at(i))+", value);\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace9.append("           case "+outputVariables.at(j)+"_: setVariable(\""+outputComponents.at(j)+"\", \""+outputPorts.at(j)+"\", "+QString::number(outputDatatypes.at(j))+", value);\n");

    modelSourceCode.replace("<<<0>>>", modelName);
    modelSourceCode.replace("<<<1>>>", ID);
    modelSourceCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size()));
    modelSourceCode.replace("<<<3>>>", QString::number(inputVariables.size() + outputVariables.size()));  //!< @todo Does number of variables equal number of states?
    modelSourceCode.replace("<<<4>>>", sourceReplace4);
    modelSourceCode.replace("<<<5>>>", sourceReplace5);
    modelSourceCode.replace("<<<6>>>", sourceReplace6);
    modelSourceCode.replace("<<<7>>>", modelName);
    modelSourceCode.replace("<<<8>>>", sourceReplace8);
    modelSourceCode.replace("<<<9>>>", sourceReplace9);

    QTextStream modelSourceStream(&modelSourceFile);
    modelSourceStream << modelSourceCode;
    modelSourceFile.close();


    printMessage("Writing HopsanFMU.h");

    QFile fmuHeaderTemplateFile(":templates/fmuHeaderTemplate.h");
    assert(fmuHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QString fmuHeaderCode;
    QTextStream t3(&fmuHeaderTemplateFile);
    fmuHeaderCode = t3.readAll();
    fmuHeaderTemplateFile.close();
    assert(!fmuHeaderCode.isEmpty());

    QTextStream fmuHeaderStream(&fmuHeaderFile);
    fmuHeaderStream << fmuHeaderCode;
    fmuHeaderFile.close();


    printMessage("Writing HopsanFMU.cpp");
    //! @todo Time step should not be hard coded

    QFile fmuSourceTemplateFile(":templates/fmuSourceTemplate.c");
    assert(fmuSourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QString fmuSourceCode;
    QTextStream t4(&fmuSourceTemplateFile);
    fmuSourceCode = t4.readAll();
    fmuSourceTemplateFile.close();
    assert(!fmuSourceCode.isEmpty());

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
    QFile libFile;
    QFile expFile;
//    if(gccCompiler)
//    {
        dllFile.setFileName(gExecPath + "HopsanCore.dll");
        dllFile.copy(savePath + "/HopsanCore.dll");
//    }
//    else
//    {
//        //! @todo this seem a bit hardcoded
//        dllFile.setFileName(QString(MSVC2008_X86_PATH) + "HopsanCore.dll");
//        dllFile.copy(savePath + "/HopsanCore.dll");
//        libFile.setFileName(QString(MSVC2008_X86_PATH) + "HopsanCore.lib");
//        libFile.copy(savePath + "/HopsanCore.lib");
//        expFile.setFileName(QString(MSVC2008_X86_PATH) + "HopsanCore.exp");
//        expFile.copy(savePath + "/HopsanCore.exp");
//    }
#elif linux
    QFile soFile;
    soFile.setFileName(gExecPath + "libHopsanCore.so");
    soFile.copy(savePath + "/libHopsanCore.so");
#endif


    printMessage("Copying include files");


    //Copy include files to export directory
    copyIncludeFilesToDir(savePath);


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
    p.start("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & compile.bat");
    p.waitForFinished();
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

#ifdef WIN32
    //Execute FMU compile script
    p.start("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & build_fmu.bat me " + modelName);
    p.waitForFinished();
#elif linux
    gccCommand1 = "cd "+savePath+" && gcc -c -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c";
    gccCommand2 = "cd "+savePath+" && gcc -shared -Wl,--rpath,'$ORIGIN/.' -o "+modelName+".so "+modelName+".o -L./ -lHopsanFMU";

    qDebug() << "Command 1 = " << gccCommand1;
    qDebug() << "Command 2 = " << gccCommand2;

    gccCommand1 +=" 2>&1";
    fp = popen(  (const char *) gccCommand1.toStdString().c_str(), "r");
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

    printMessage("Sorting files");


#ifdef WIN32
    saveDir.mkpath("fmu/binaries/win32");
    saveDir.mkpath("fmu/resources");
    QFile modelDllFile(savePath + "/" + modelName + ".dll");
    modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
    QFile modelLibFile(savePath + "/" + modelName + ".lib");
    modelLibFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".lib");
    dllFile.copy(savePath + "/fmu/binaries/win32/HopsanCore.dll");
//    if(!gccCompiler)
//    {
//        libFile.copy(savePath + "/fmu/binaries/win32/HopsanCore.lib");
//    }
    QFile hopsanFMUdllFile(savePath + "/HopsanFMU.dll");
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

    printMessage("Cleaning up");


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
}



void HopsanGenerator::generateToSimulink(QString savePath, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler)
{
    printMessage("Initializing FMU export");

    QDir saveDir;
    saveDir.setPath(savePath);

    //! @todo Make global
    QString gExecPath = qApp->applicationDirPath().append('/');


    std::vector<std::string> parameterNames;
    pSystem->getParameterNames(parameterNames);
    QStringList tunableParameters;
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        tunableParameters.append(QString(parameterNames[i].c_str()));
    }
    QStringList inputComponents;
    QStringList inputPorts;
    QStringList outputComponents;
    QStringList outputPorts;
    QStringList mechanicQComponents;
    QStringList mechanicQPorts;
    QStringList mechanicCComponents;
    QStringList mechanicCPorts;
    QStringList mechanicRotationalQComponents;
    QStringList mechanicRotationalQPorts;
    QStringList mechanicRotationalCComponents;
    QStringList mechanicRotationalCPorts;

    std::vector<std::string> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        Component *pComponent = pSystem->getSubComponent(names[i]);
        if(pComponent->getTypeName() == "SignalInputInterface")
        {
            inputComponents.append(names[i].c_str());
            inputPorts.append("out");
        }
        else if(pComponent->getTypeName() == "SignalOutputInterface")
        {
            outputComponents.append(names[i].c_str());
            outputPorts.append("in");
        }
        else if(pComponent->getTypeName() == "MechanicInterfaceQ")
        {
            mechanicQComponents.append(names[i].c_str());
            mechanicQPorts.append("P1");
        }
        else if(pComponent->getTypeName() == "MechanicInterfaceC")
        {
            mechanicCComponents.append(names[i].c_str());
            mechanicCPorts.append("P1");
        }
        else if(pComponent->getTypeName() == "MechanicRotationalInterfaceQ")
        {
            mechanicRotationalQComponents.append(names[i].c_str());
            mechanicRotationalQPorts.append("P1");
        }
        else if(pComponent->getTypeName() == "MechanicRotationalInterfaceC")
        {
            mechanicRotationalCComponents.append(names[i].c_str());
            mechanicRotationalCPorts.append("P1");
        }
        //! @todo what about pneumatic and electric nodes
        //! @todo this should not be hardcoded
    }

    int nInputs = inputComponents.size();
    QString nInputsString;
    nInputsString.setNum(nInputs);

    int nOutputs = outputComponents.size();
    QString nOutputsString;
    nOutputsString.setNum(nOutputs);

    int nMechanicQ = mechanicQComponents.size();
    QString nMechanicQString;
    nMechanicQString.setNum(nMechanicQ);

    int nMechanicC = mechanicCComponents.size();
    QString nMechanicCString;
    nMechanicCString.setNum(nMechanicC);

    int nMechanicRotationalQ = mechanicRotationalQComponents.size();
    QString nMechanicRotationalQString;
    nMechanicRotationalQString.setNum(nMechanicRotationalQ);

    int nMechanicRotationalC = mechanicRotationalCComponents.size();
    QString nMechanicRotationalCString;
    nMechanicRotationalCString.setNum(nMechanicRotationalC);

    int nTotalInputs = nInputs+nMechanicQ*2+nMechanicC*2+nMechanicRotationalQ*2+nMechanicRotationalC*2;
    QString nTotalInputsString;
    nTotalInputsString.setNum(nTotalInputs);

    int nTotalOutputs = nOutputs+nMechanicQ*2+nMechanicC*2+nMechanicRotationalQ*2+nMechanicRotationalC*2+1;
    QString nTotalOutputsString;
    nTotalOutputsString.setNum(nTotalOutputs);


    printMessage("Generating files");


    QFile wrapperFile;
    wrapperFile.setFileName(savePath + "/HopsanSimulink.cpp");
    if(!wrapperFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanSimulink.cpp for writing.");
        return;
    }

    QFile portLabelsFile;
    portLabelsFile.setFileName(savePath + "/HopsanSimulinkPortLabels.m");
    if(!portLabelsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanSimulinkPortLabels.m for writing.");
        return;
    }


    QFile compileFile;
    compileFile.setFileName(savePath + "/HopsanSimulinkCompile.m");
    if(!compileFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanSimulinkCompile.m for writing.");
        return;
    }


    printMessage("Writing HopsanSimulinkPortLabels.m");


    QTextStream portLabelsStream(&portLabelsFile);
    portLabelsStream << "set_param(gcb,'Mask','on')\n";
    portLabelsStream << "set_param(gcb,'MaskDisplay','";
    int i,j;
    int tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicQComponents.at(i) << ".x''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicQComponents.at(i) << ".v''); ";
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicCComponents.at(i) << ".cx''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicCComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicRotationalQComponents.at(i) << ".a''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicRotationalQComponents.at(i) << ".w''); ";
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicRotationalCComponents.at(i) << ".cx''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicRotationalCComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nInputs; ++i)
    {
        j=tot+i;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << inputComponents.at(i) << "''); ";
    }

    tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicQComponents.at(i) << ".cx''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicQComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicCComponents.at(i) << ".x''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicCComponents.at(i) << ".v''); ";
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicRotationalQComponents.at(i) << ".Zx''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicRotationalQComponents.at(i) << ".cx''); ";
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicRotationalCComponents.at(i) << ".a''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicRotationalCComponents.at(i) << ".w''); ";
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nOutputs; ++i)
    {
        j=tot+i;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << outputComponents.at(i) << "''); ";
    }
    j=nTotalOutputs-1;
    portLabelsStream << "port_label(''output''," << j+1 << ",''DEBUG'')'); \n";
    portLabelsStream << "set_param(gcb,'BackgroundColor','[0.721569, 0.858824, 0.905882]')\n";
    portLabelsStream << "set_param(gcb,'Name','" << pSystem->getName().c_str() << "')\n";
    portLabelsStream << "set_param(gcb,'MaskPrompts',{";
    for(int p=0; p<tunableParameters.size(); ++p)
    {
        portLabelsStream << "'"+tunableParameters[p]+"'";
        if(p<tunableParameters.size()-1)
            portLabelsStream << ",";
    }
    portLabelsStream << "})\n";
    portLabelsStream << "set_param(gcb,'MaskVariables','";
    for(int p=0; p<tunableParameters.size(); ++p)
    {
        portLabelsStream << tunableParameters[p]+"=&"+QString::number(p+1)+";";
    }
    portLabelsStream << "')\n";
    portLabelsFile.close();



    printMessage("Writing HopsanSimulink.cpp");


    //How to access dialog parameters:
    //double par1 = (*mxGetPr(ssGetSFcnParam(S, 0)));

    QFile wrapperTemplateFile(":templates/simulinkWrapperTemplate.cpp");
    assert(wrapperTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString wrapperCode;
    QTextStream t(&wrapperTemplateFile);
    wrapperCode = t.readAll();
    wrapperTemplateFile.close();
    assert(!wrapperCode.isEmpty());

    QString wrapperReplace1;
    tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nInputs; ++i)
    {
        j=tot+i;
        wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
    }

    QString wrapperReplace3;
    tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nOutputs; ++i)
    {
        j=tot+i;
        wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
    }
    j=nTotalOutputs-1;

    QString wrapperReplace5;
    if(!disablePortLabels)
    {
        wrapperReplace5 = "    mexCallMATLAB(0, 0, 0, 0, \"HopsanSimulinkPortLabels\");                              //Run the port label script";
    }

    QString wrapperReplace6;
    for(int p=0; p<tunableParameters.size(); ++p)
    {
        wrapperReplace6.append("    in = mexGetVariable(\"caller\",\"" + tunableParameters[p] + "\");\n");
        wrapperReplace6.append("    if(in == NULL )\n");
        wrapperReplace6.append("    {\n");
        wrapperReplace6.append("        mexErrMsgTxt(\"Unable to read parameter \\\""+tunableParameters[p]+"\\\"!\");\n");
        wrapperReplace6.append("    	return;\n");
        wrapperReplace6.append("    }\n");
        wrapperReplace6.append("\n");
        wrapperReplace6.append("    c_str = (const char*)mxGetData(in);\n");
        wrapperReplace6.append("\n");
        wrapperReplace6.append("    str = \"\";\n");
        wrapperReplace6.append("    for(int i=0; i<mxGetNumberOfElements(in); ++i)\n");
        wrapperReplace6.append("    {\n");
        wrapperReplace6.append("    	str.append(c_str);\n");
        wrapperReplace6.append("    	c_str += 2*sizeof(char);\n");
        wrapperReplace6.append("    }\n");
        wrapperReplace6.append("\n");
        wrapperReplace6.append("    pComponentSystem->setParameterValue(\""+tunableParameters[p]+"\", str);\n");
    }

    QString wrapperReplace7;
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperReplace7 = "    real_T *y" + QString::number(i) + " = ssGetOutputPortRealSignal(S," + QString::number(i) + ");\n";
    }

    QString wrapperReplace8;
    for(int i=0; i<nTotalInputs; ++i)
    {
        wrapperReplace7 = "    double input" + QString::number(i) + " = (*uPtrs1[" + QString::number(i) + "]);\n";
    }

    QString wrapperReplace9;
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperReplace9 = "    double output" + QString::number(i) + ";\n";
    }

    QString wrapperReplace11;
    tot = 0;
    for(int i=0; i<nMechanicQ; ++i)
    {
        j = tot+i*2;
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicQComponents.at(i) + "\")->getPort(\"" + mechanicQPorts.at(i) + "\")->writeNode(2, input" + QString::number(j) + ");\n");
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicQComponents.at(i) + "\")->getPort(\"" + mechanicQPorts.at(i) + "\")->writeNode(0, input" + QString::number(j+1) + ");\n");
    }
    tot+=nMechanicQ*2;
    for(int i=0; i<nMechanicC; ++i)
    {
        j = tot+i*2;
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicCComponents.at(i) + "\")->getPort(\"" + mechanicCPorts.at(i) + "\")->writeNode(3, input" + QString::number(j) + ");\n");
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicCComponents.at(i) + "\")->getPort(\"" + mechanicCPorts.at(i) + "\")->writeNode(4, input" + QString::number(j+1) + ");\n");
    }
    tot+=nMechanicC*2;
    for(int i=0; i<nMechanicRotationalQ; ++i)
    {
        j = tot+i*2;
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicRotationalQComponents.at(i) + "\")->getPort(\"" + mechanicRotationalQPorts.at(i) + "\")->writeNode(2, input" + QString::number(j) + ");\n");
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicRotationalQComponents.at(i) + "\")->getPort(\"" + mechanicRotationalQPorts.at(i) + "\")->writeNode(0, input" + QString::number(j+1) + ");\n");
    }
    tot+=nMechanicRotationalQ*2;
    for(int i=0; i<nMechanicRotationalC; ++i)
    {
        j = tot+i*2;
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicRotationalCComponents.at(i) + "\")->getPort(\"" + mechanicRotationalCPorts.at(i) + "\")->writeNode(3, input" + QString::number(j) + ");\n");
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + mechanicRotationalCComponents.at(i) + "\")->getPort(\"" + mechanicRotationalCPorts.at(i) + "\")->writeNode(4, input" + QString::number(j+1) + ");\n");
    }
    tot+=nMechanicRotationalC*2;
    for(int i=0; i<nInputs; ++i)
    {
        j = tot+i;
        wrapperReplace11.append("        pComponentSystem->getSubComponent(\"" + inputComponents.at(i) + "\")->getPort(\"" + inputPorts.at(i) + "\")->writeNode(0, input" + QString::number(i) + ");\n");
    }

    QString wrapperReplace12;

    tot = 0;
    for(int i=0; i<nMechanicQ; ++i)
    {
        j = tot+i*2;
        wrapperReplace12.append("        output" + QString::number(j) + " = pComponentSystem->getSubComponent(\"" + mechanicQComponents.at(i) + "\")->getPort(\"" + mechanicQPorts.at(i) + "\")->readNode(3);\n");
        wrapperReplace12.append("        output" + QString::number(j+1) + " = pComponentSystem->getSubComponent(\"" + mechanicQComponents.at(i) + "\")->getPort(\"" + mechanicQPorts.at(i) + "\")->readNode(4);\n");
    }
    tot+=nMechanicQ*2;
    for(int i=0; i<nMechanicC; ++i)
    {
        j = tot+i*2;
        wrapperReplace12.append("        output" + QString::number(j) + " = pComponentSystem->getSubComponent(\"" + mechanicCComponents.at(i) + "\")->getPort(\"" + mechanicCPorts.at(i) + "\")->readNode(2);\n");
        wrapperReplace12.append("        output" + QString::number(j+1) + " = pComponentSystem->getSubComponent(\"" + mechanicCComponents.at(i) + "\")->getPort(\"" + mechanicCPorts.at(i) + "\")->readNode(0);\n");
    }
    tot+=nMechanicC*2;
    for(int i=0; i<nMechanicRotationalQ; ++i)
    {
        j = tot+i*2;
        wrapperReplace12.append("        output" + QString::number(j) + " = pComponentSystem->getSubComponent(\"" + mechanicRotationalQComponents.at(i) + "\")->getPort(\"" + mechanicRotationalQPorts.at(i) + "\")->readNode(3);\n");
        wrapperReplace12.append("        output" + QString::number(j+1) + " = pComponentSystem->getSubComponent(\"" + mechanicRotationalQComponents.at(i) + "\")->getPort(\"" + mechanicRotationalQPorts.at(i) + "\")->readNode(4);\n");
    }
    tot+=nMechanicRotationalQ*2;
    for(int i=0; i<nMechanicRotationalC; ++i)
    {
        j = tot+i*2;
        wrapperReplace12.append("        output" + QString::number(j) + " = pComponentSystem->getSubComponent(\"" + mechanicRotationalCComponents.at(i) + "\")->getPort(\"" + mechanicRotationalCPorts.at(i) + "\")->readNode(2);\n");
        wrapperReplace12.append("        output" + QString::number(j+1) + " = pComponentSystem->getSubComponent(\"" + mechanicRotationalCComponents.at(i) + "\")->getPort(\"" + mechanicRotationalCPorts.at(i) + "\")->readNode(0);\n");
    }
    tot+=nMechanicRotationalC*2;
    for(int i=0; i<nOutputs; ++i)
    {
        j = tot+i;
        wrapperReplace12.append("        output" + QString::number(j) + " = pComponentSystem->getSubComponent(\"" + outputComponents.at(i) + "\")->getPort(\"" + outputPorts.at(i) + "\")->readNode(0);\n");
    }

    QString wrapperReplace13;
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperReplace13 = "    *y" + QString::number(i) + " = output" + QString::number(i) + ";\n";
    }

    wrapperCode.replace("<<<0>>>", nTotalInputsString);
    wrapperCode.replace("<<<1>>>", wrapperReplace1);
    wrapperCode.replace("<<<2>>>", nTotalOutputsString);
    wrapperCode.replace("<<<3>>>", wrapperReplace3);
    wrapperCode.replace("<<<14>>>", QString::number(nTotalOutputs-1));
    wrapperCode.replace("<<<4>>>", savePath);
    wrapperCode.replace("<<<5>>>", wrapperReplace5);
    wrapperCode.replace("<<<6>>>", wrapperReplace6);
    wrapperCode.replace("<<<7>>>", wrapperReplace7);
    wrapperCode.replace("<<<8>>>", wrapperReplace8);
    wrapperCode.replace("<<<9>>>", wrapperReplace9);
    wrapperCode.replace("<<<10>>>", QString::number(nTotalOutputs-1));
    wrapperCode.replace("<<<11>>>", wrapperReplace11);
    wrapperCode.replace("<<<12>>>", wrapperReplace12);
    wrapperCode.replace("<<<13>>>", wrapperReplace13);

    QTextStream wrapperStream(&wrapperFile);
    wrapperStream << wrapperCode;
    wrapperFile.close();


    printMessage("Writing HopsanSimulinkCompile.m");


    QTextStream compileStream(&compileFile);
#ifdef WIN32
    //compileStream << "%mex -DWIN32 -DSTATICCORE HopsanSimulink.cpp /include/Component.cc /include/ComponentSystem.cc /include/HopsanEssentials.cc /include/Node.cc /include/Port.cc /include/Components/Components.cc /include/CoreUtilities/HmfLoader.cc /include/CoreUtilities/HopsanCoreMessageHandler.cc /include/CoreUtilities/LoadExternal.cc /include/Nodes/Nodes.cc /include/ComponentUtilities/AuxiliarySimulationFunctions.cpp /include/ComponentUtilities/Delay.cc /include/ComponentUtilities/DoubleIntegratorWithDamping.cpp /include/ComponentUtilities/FirstOrderFilter.cc /include/ComponentUtilities/Integrator.cc /include/ComponentUtilities/IntegratorLimited.cc /include/ComponentUtilities/ludcmp.cc /include/ComponentUtilities/matrix.cc /include/ComponentUtilities/SecondOrderFilter.cc /include/ComponentUtilities/SecondOrderTransferFunction.cc /include/ComponentUtilities/TurbulentFlowFunction.cc /include/ComponentUtilities/ValveHysteresis.cc\n";
    compileStream << "mex -DWIN32 -DSTATICCORE -L./ -Iinclude -lHopsanCore HopsanSimulink.cpp\n";

    printMessage("Copying Visual Studio binaries");


    //Select path to MSVC library depending on user selection
    QString msvcPath;
    if(compiler == 0)   //MSVC2008 32-bit
    {
        msvcPath = gExecPath+"MSVC2008_x86/";
    }
    else if(compiler == 1)  //MSVC2008 64-bit
    {
        msvcPath = gExecPath+"MSVC2008_x64/";
    }
    else if(compiler == 2)  //MSVC2010 32-bit
    {
        msvcPath = gExecPath+"MSVC2010_x86/";
    }
    else if(compiler == 3)  //MSVC2010 64-bit
    {
        msvcPath = gExecPath+"MSVC2010_x64/";
    }


    //Copy MSVC binaries to export folder
    QFile dllFile(msvcPath + "HopsanCore.dll");
    dllFile.copy(savePath + "/HopsanCore.dll");
    QFile libFile(msvcPath + "HopsanCore.lib");
    libFile.copy(savePath + "/HopsanCore.lib");
    QFile expFile(msvcPath + "HopsanCore.exp");
    expFile.copy(savePath + "/HopsanCore.exp");

#else
    compileStream << "% You need to copy the .so files here or change the -L lib search path" << endl;
    compileStream << "mex -L./ -Iinclude -lHopsanCore HopsanSimulink.cpp" << endl;

    //! @todo copy all of the symolic links and the .so

#endif
    compileFile.close();

    printMessage("Copying include files");

    copyIncludeFilesToDir(savePath);

    //! @todo should not overwrite this wile if it already exists
    QFile externalLibsFile;
    externalLibsFile.setFileName(savePath + "/externalLibs.txt");
    if(!externalLibsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open externalLibs.txt for writing.");
        return;
    }
    QTextStream externalLibsFileStream(&externalLibsFile);
    externalLibsFileStream << "#Enter the relative path to each external component lib that needs to be loaded" << endl;
    externalLibsFileStream << "#Enter one per line, the filename is enough if you put the lib file (.dll or.so) in this directory.";
    externalLibsFile.close();
}


void HopsanGenerator::generateToLabViewSIT(QString savePath, hopsan::ComponentSystem *pSystem)
{
    printMessage("Initializing LabVIEW/SIT export");


    QFileInfo fileInfo;
    fileInfo.setFile(savePath);


    printMessage("Creating "+fileInfo.fileName());


    QFile file;
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open file for writing: " + savePath);
        return;
    }

    printMessage("Generating lists for input and output ports");

    //Create lists for input and output interface components
    QStringList inputs;
    QStringList outputs;
    QStringList mechCinterfaces;
    QStringList mechQinterfaces;
    QStringList hydCinterfaces;
    QStringList hydQinterfaces;

    std::vector<std::string> compNames = pSystem->getSubComponentNames();
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        if(pComp->getTypeName() == "SignalInputInterface")
        {
            inputs.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "SignalOutputInterface")
        {
            outputs.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "MechanicInterfaceC")
        {
            mechCinterfaces.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "MechanicInterfaceQ")
        {
            mechQinterfaces.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "HydraulicInterfaceC")
        {
            hydCinterfaces.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "HydraulicInterfaceQ")
        {
            hydQinterfaces.append(compNames[n].c_str());
        }
    }

    printMessage("Writing " + fileInfo.fileName());

    QFile wrapperTemplateFile(":templates/labviewWrapperTemplate.cpp");
    assert(wrapperTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString wrapperCode;
    QTextStream t(&wrapperTemplateFile);
    wrapperCode = t.readAll();
    wrapperTemplateFile.close();
    assert(!wrapperCode.isEmpty());

        //Write initial comment

    QString replaceInports;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toVarName(inputs.at(i));
        replaceInports.append("    double "+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceInports.append("    double "+tempString+"C;\n");
        replaceInports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceInports.append("    double "+tempString+"F;\n");
        replaceInports.append("    double "+tempString+"X;\n");
        replaceInports.append("    double "+tempString+"V;\n");
        replaceInports.append("    double "+tempString+"M;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceInports.append("    double "+tempString+"C;\n");
        replaceInports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceInports.append("    double "+tempString+"P;\n");
        replaceInports.append("    double "+tempString+"Q;\n");
    }

    QString replaceOutports;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceOutports.append("    double "+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"F;\n");
        replaceOutports.append("    double "+tempString+"X;\n");
        replaceOutports.append("    double "+tempString+"V;\n");
        replaceOutports.append("    double "+tempString+"M;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"C;\n");
        replaceOutports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"P;\n");
        replaceOutports.append("    double "+tempString+"Q;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"C;\n");
        replaceOutports.append("    double "+tempString+"Zc;\n");
    }

    QString replaceInportSize = QString::number(inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size());
    QString replaceOutportSize = QString::number(outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size());

    QString replaceInportAttribs;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toVarName(inputs.at(i));
        replaceInportAttribs.append("    { \""+tempString+"\", 1, 1},\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"F\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"X\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"V\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"M\", 1, 1},\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"P\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Q\", 1, 1},\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }

    QString replaceOutportAttribs;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"\", 1, 1},\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"F\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"X\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"V\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"M\", 1, 1},\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"P\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Q\", 1, 1},\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }

    QString replaceComponents;
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        replaceComponents.append("    addComponent(\"" + QString(compNames[n].c_str()) + QString("\", \"") + QString(pComp->getTypeName().c_str())+QString("\");\n"));
    }

    QString replaceConnections;
    QList<QPair<QPair<QString, QString>, QPair<QString, QString> > > connections;
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        for(size_t p=0; p<pComp->getPortNames().size(); ++p)
        {
            Port *pPort = pComp->getPortPtrVector()[p];
            if(pPort->isMultiPort())
            {
                continue;
            }
            for(size_t c=0; c<pPort->getConnectedPorts().size(); ++c)
            {
                Port *pPort2 = pPort->getConnectedPorts()[c];
                if(pPort2->getParentPort())
                {
                    pPort2 = pPort2->getParentPort();
                }
                QString comp1 = compNames[n].c_str();
                QString port1 = pPort->getPortName().c_str();
                QString comp2 = pPort2->getComponentName().c_str();
                QString port2 = pPort2->getPortName().c_str();

                QPair<QString, QString> pair1 = QPair<QString, QString>(comp1, port1);
                QPair<QString, QString> pair2 = QPair<QString, QString>(comp2, port2);

                if(!connections.contains(QPair<QPair<QString, QString>, QPair<QString, QString> >(pair1, pair2)) &&
                   !connections.contains(QPair<QPair<QString, QString>, QPair<QString, QString> >(pair2, pair1)))
                {
                    connections.append(QPair<QPair<QString, QString>, QPair<QString, QString> >(pair1, pair2));
                }
            }
        }
    }
    for(int i = 0; i != connections.size(); ++i)
    {
        replaceConnections.append("    connect(\"" +  connections[i].first.first + "\", \""+connections[i].first.second +
                      "\", \"" + connections[i].second.first + "\", \""+connections[i].second.second+"\");\n");
    }

    QString replaceParameters;
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        std::vector<std::string> parNames;
        pComp->getParameterNames(parNames);
        for(size_t p=0; p<parNames.size(); ++p)
        {
            std::string parValue;
            pComp->getParameterValue(parNames[p], parValue);
            replaceParameters.append("    setParameter(\"" + QString(compNames[n].c_str()) + "\", \"" + QString(parNames[p].c_str()) +  "\", " + QString(parValue.c_str()) + ");\n");
        }
    }

    QString replaceIndata;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toVarName(inputs.at(i));
        replaceIndata.append("        rtInport."+tempString+" = inData["+QString::number(i)+"];\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"C = inData["+QString::number(2*i+inputs.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Zc = inData["+QString::number(2*i+1+inputs.size())+"];\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"F = inData["+QString::number(4*i+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"X = inData["+QString::number(4*i+1+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"V = inData["+QString::number(4*i+2+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"M = inData["+QString::number(4*i+3+inputs.size()+2*mechCinterfaces.size())+"];\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"C = inData["+QString::number(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Zc = inData["+QString::number(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"P = inData["+QString::number(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Q = inData["+QString::number(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n");
    }

    QString replaceWriteNodeData;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toVarName(inputs.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+inputs.at(i)+"\", \"out\", 0, rtInport."+tempString+");\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n");
        replaceIndata.append("    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"F);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 2, rtInport."+tempString+"X);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"V);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 5, rtInport."+tempString+"M);\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"P);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"Q);\n");
    }

    QString replaceReadNodeData;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+" = readNodeData(\""+outputs.at(i)+"\", \"in\", 0);\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"F = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 1);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"X = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 2);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"V = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 0);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"M = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 5);\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"C = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 3);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Zc = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 4);\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"P = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 1);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Q = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 0);\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"C = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 3);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Zc = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 4);\n");
    }

    QString replaceOutData;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceOutData.append("        outData["+QString::number(i)+"] = rtOutport."+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(4*i+outputs.size())+"] = rtOutport."+tempString+"F;\n");
        replaceOutData.append("        outData["+QString::number(4*i+1+outputs.size())+"] = rtOutport."+tempString+"X;\n");
        replaceOutData.append("        outData["+QString::number(4*i+2+outputs.size())+"] = rtOutport."+tempString+"V;\n");
        replaceOutData.append("        outData["+QString::number(4*i+3+outputs.size())+"] = rtOutport."+tempString+"M;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"C;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"Zc;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"P;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"Q;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size())+"] = rtOutport."+tempString+"C;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size())+"] = rtOutport."+tempString+"Zc;\n");
    }

    wrapperCode.replace("<<<inports>>>", replaceInports);
    wrapperCode.replace("<<<outports>>>", replaceOutports);
    wrapperCode.replace("<<<inportsize>>>", replaceInportSize);
    wrapperCode.replace("<<<outportsize>>>", replaceOutportSize);
    wrapperCode.replace("<<<inportattribs>>>", replaceInportAttribs);
    wrapperCode.replace("<<<outportattribs>>>", replaceOutportAttribs);
    wrapperCode.replace("<<<components>>>", replaceComponents);
    wrapperCode.replace("<<<connections>>>", replaceConnections);
    wrapperCode.replace("<<<parameters>>>", replaceParameters);
    wrapperCode.replace("<<<indata>>>", replaceIndata);
    wrapperCode.replace("<<<writenodedata>>>", replaceWriteNodeData);
    wrapperCode.replace("<<<readnodedata>>>", replaceReadNodeData);
    wrapperCode.replace("<<<outdata>>>", replaceOutData);


    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    fileStream << wrapperCode;
    file.close();

    //! @todo Check if success, otherwise tell user with error message
    printMessage("Finished!");
}



//! @brief Parses a modelica model code to Hopsan classes
//! @param code Input Modelica code
//! @param typeName Type name of new component
//! @param displayName Display name of new component
//! @param initAlgorithms Initial algorithms for new component
//! @param equations Equations for new component
//! @param portList List of port specifications for new component
//! @param parametersList List of parameter specifications for new component
void HopsanGenerator::parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations,
                        QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList)
{
    QStringList lines = code.split("\n");
    QStringList portNames;
    bool initialAlgorithmPart = false;  //Are we in the intial "algorithms" part?
    bool equationPart = false;          //Are we in the "equations" part?
    bool finalAlgorithmPart = false;    //Are we in the final "algorithms" part?
    for(int l=0; l<lines.size(); ++l)
    {
        if(!initialAlgorithmPart && !equationPart && !finalAlgorithmPart)
        {
            //qDebug() << l << " - not in algorithms or equations";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "model")              //"model" keyword
            {
                typeName = words.at(1);
                if(words.size() > 2)
                {
                    displayName = words.at(2);
                    displayName.remove(0, 1);
                    int j=2;
                    while(!words.at(j).endsWith("\""))
                    {
                        ++j;
                        displayName.append(" " + words.at(j));
                    }
                    displayName.chop(1);
                }
            }
            else if(words.at(0).startsWith("annotation("))        //"annotation" keyword
            {
                QString tempLine = lines[l];
                tempLine.remove(" ");
                int idx = tempLine.indexOf("hopsanCqsType=");
                cqsType = tempLine.at(idx+15);
            }
            else if(words.at(0) == "parameter")         //"parameter" keyword
            {
                QString name = words.at(2).section("(",0,0);
                QString unit = lines.at(l).section("unit=",1,1).section("\"",1,1);
                QString init;
                //Default value can be written with white spaces in different way, test them all
                if(!words.at(2).section(")", 1).isEmpty())
                    init = words.at(2).section(")", 1).section("=", 1);             //...blabla)=x
                else if(words.at(2).endsWith("="))
                    init = words.at(3);                                             //...blabla)= x
                else if(words.at(3).startsWith("=") && words.at(3).size() > 1)
                    init = words.at(3).section("=", 1);                             //...blabla) =x
                else if(words.at(3) == "=")
                    init = words.at(4);                                             // ...blabla) = x

                QString parDisplayName = lines.at(l).section("\"", -2, -2);

                ParameterSpecification par(name, name, parDisplayName, unit, init);
                parametersList.append(par);
            }
            else if(words.at(0) == "NodeSignalOut")                //Signal connector (output)
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("WritePort", "NodeSignal", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeSignalIn")                //Signal connector (input)
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("ReadPort", "NodeSignal", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeMechanic")              //Mechanic connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeMechanic", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeMechanicRotational")    //Mechanic rotational connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeMechanicRotational", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeHydraulic")             //Hydraulic connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeHydraulic", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodePneumatic")             //Pneumatic connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodePneumatic", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeElectric")              //Electric connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeElectric", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "algorithm" && !equationPart)    //Initial algorithm part begins!
            {
                initialAlgorithmPart = true;
            }
            else if(words.at(0) == "equation")                      //Equation part begins!
            {
                initialAlgorithmPart = false;
                equationPart = true;
            }
            else if(words.at(0) == "algorithm" && equationPart)     //Final algorithm part begins!
            {
                equationPart = false;
                finalAlgorithmPart = true;
            }
            else if(words.at(0) == "end" && (initialAlgorithmPart || equationPart || finalAlgorithmPart))       //We are finished
            {
                break;
            }
        }
        else if(initialAlgorithmPart)
        {
            //qDebug() << l << " - in algorithms";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            if(words.at(0) == "equation")       //Equation part begings, end of algorithm section
            {
                initialAlgorithmPart = false;
                equationPart = true;
                continue;
            }
            initAlgorithms << lines.at(l).trimmed();
            initAlgorithms.last().replace(":=", "=");
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(initAlgorithms.last().contains(temp))
                {
                    if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                    {
                        int idx = initAlgorithms.last().indexOf(temp)+temp.size()-1;
                        if(portList.at(i).porttype == "WritePort")
                        {
                            initAlgorithms.last().remove(idx, 4);
                        }
                        else if(portList.at(i).porttype == "ReadPort")
                        {
                            initAlgorithms.last().remove(idx, 3);
                        }
                    }
                    else
                    {
                        int idx = initAlgorithms.last().indexOf(temp);
                        int idx2=idx+temp.size()+1;
                        while(idx2 < initAlgorithms.last().size()+1 && initAlgorithms.last().at(idx2).isLetterOrNumber())
                            ++idx2;
                        initAlgorithms.last().insert(idx2, QString::number(i+1));
                        initAlgorithms.last().remove(idx, temp.size());
                    }
                }
            }
            initAlgorithms.last().chop(1);
        }
        else if(equationPart)
        {
           // qDebug() << l << " - in equations";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            if(words.at(0) == "algorithm")       //Final algorithm section begins
            {
                equationPart = false;
                finalAlgorithmPart = true;
                continue;
            }
            equations << lines.at(l).trimmed();
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(equations.last().contains(temp))
                {
                    if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                    {
                        int idx = equations.last().indexOf(temp)+temp.size()-1;
                        if(portList.at(i).porttype == "WritePort")
                        {
                            equations.last().remove(idx, 4);
                        }
                        else if(portList.at(i).porttype == "ReadPort")
                        {
                            equations.last().remove(idx, 3);
                        }
                    }
                    else
                    {
                        int idx = equations.last().indexOf(temp);
                        int idx2=idx+temp.size()+1;
                        while(idx2 < equations.last().size()+1 && equations.last().at(idx2).isLetterOrNumber())
                            ++idx2;
                        equations.last().insert(idx2, QString::number(i+1));
                        equations.last().remove(idx, temp.size());
                    }
                }
            }
            equations.last().chop(1);
        }
        else if(finalAlgorithmPart)
        {
           // qDebug() << l << " - in algorithms";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            finalAlgorithms << lines.at(l).trimmed();
            finalAlgorithms.last().replace(":=", "=");
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(finalAlgorithms.last().contains(temp))
                {
                    if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                    {
                        int idx = finalAlgorithms.last().indexOf(temp)+temp.size()-1;
                        if(portList.at(i).porttype == "WritePort")
                        {
                            finalAlgorithms.last().remove(idx, 4);
                        }
                        else if(portList.at(i).porttype == "ReadPort")
                        {
                            finalAlgorithms.last().remove(idx, 3);
                        }
                    }
                    else
                    {
                        int idx = finalAlgorithms.last().indexOf(temp);
                        int idx2=idx+temp.size()+1;
                        while(idx2 < finalAlgorithms.last().size()+1 && finalAlgorithms.last().at(idx2).isLetterOrNumber())
                            ++idx2;
                        finalAlgorithms.last().insert(idx2, QString::number(i+1));
                        finalAlgorithms.last().remove(idx, temp.size());
                    }
                }
            }
            finalAlgorithms.last().chop(1);
        }
    }

    initAlgorithms.removeAll("\n");
    initAlgorithms.removeAll("");
    equations.removeAll("\n");
    equations.removeAll("");
    finalAlgorithms.removeAll("\n");
    finalAlgorithms.removeAll("");
}



//! @brief Generates XML and compiles the new component
void HopsanGenerator::generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &plainInitAlgorithms, QStringList &plainEquations, QStringList &plainFinalAlgorithms, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters)
{

    //Create list of initial algorithms
    QList<Expression> initAlgorithms;
    for(int i=0; i<plainInitAlgorithms.size(); ++i)
    {
        initAlgorithms.append(Expression(plainInitAlgorithms.at(i)));
    }

    //Create list of equqtions
    QList<Expression> equations;
    for(int i=0; i<plainEquations.size(); ++i)
    {
        equations.append(Expression(plainEquations.at(i)));
    }

    //Create list of final algorithms
    QList<Expression> finalAlgorithms;
    for(int i=0; i<plainFinalAlgorithms.size(); ++i)
    {
        finalAlgorithms.append(Expression(plainFinalAlgorithms.at(i)));
    }

    //Identify variable limitations, and remove them from the equations list
    QList<Expression> limitedVariables;
    QList<Expression> limitedDerivatives;
    QList<Expression> limitMinValues;
    QList<Expression> limitMaxValues;
    QList<int> limitedVariableEquations;
    QList<int> limitedDerivativeEquations;
    for(int i=0; i<equations.size(); ++i)
    {
        if(equations[i].getFunctionName() == "VariableLimits")
        {
            if(i<1)
            {
                //! @todo Use sorting instead?
                printErrorMessage("VariableLimits not preceeded by equations defining variable.");
                return;
            }

            limitedVariables << equations[i].getArgument(0);
            limitedDerivatives << Expression();
            limitMinValues << equations[i].getArgument(1);
            limitMaxValues << equations[i].getArgument(2);
            limitedVariableEquations << i-1;
            limitedDerivativeEquations << -1;

            equations.removeAt(i);
            --i;
        }
        else if(equations[i].getFunctionName()== "Variable2Limits")
        {
            if(i<2)
            {
                printErrorMessage("Variable2Limits not preeded by equations defining variable and derivative.");
                return;
            }

            limitedVariables << equations[i].getArgument(0);
            limitedDerivatives << equations[i].getArgument(1);
            limitMinValues << equations[i].getArgument(2);
            limitMaxValues << equations[i].getArgument(3);
            limitedVariableEquations << i-2;
            limitedDerivativeEquations << i-1;

            equations.removeAt(i);
            --i;
        }
    }

    //Verify each equation
    for(int i=0; i<equations.size(); ++i)
    {
        if(!equations[i].verifyExpression())
        {
            printErrorMessage("Component generation failed: Verification of variables failed.");
            return;
        }
    }

    QList<QList<Expression> > leftSymbols2, rightSymbols2;
    for(int i=0; i<equations.size(); ++i)
    {
        leftSymbols2.append(equations[i].getChild(0).getSymbols());
        rightSymbols2.append(equations[i].getChild(1).getSymbols());
    }

    //Sum up all used variables to a single list
    QList<Expression> allSymbols;
    for(int i=0; i<equations.size(); ++i)
    {
        allSymbols.append(leftSymbols2.at(i));
        allSymbols.append(rightSymbols2.at(i));
    }

    QStringList allSymbolsList;
    for(int a=0; a<allSymbols.size(); ++a)
    {
        allSymbolsList.append(allSymbols[a].toString());
    }
    qDebug() << "All symbols: " << allSymbolsList;


    QList<Expression> initSymbols2;
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        if(!initAlgorithms[i].isAssignment())
        {
            printErrorMessage("Component generation failed: Initial algorithms section contains non-algorithms.");
            return;
        }
        initSymbols2.append(initAlgorithms[i].getChild(0));
    }

    QList<Expression> finalSymbols2;
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        //! @todo We must check that all algorithms are actually algorithms before doing this!
        if(!finalAlgorithms[i].isAssignment())
        {
            printErrorMessage("Component generation failed: Final algorithms section contains non-algorithms.");
            return;
        }
        finalSymbols2.append(finalAlgorithms[i].getChild(0));
    }

    for(int i=0; i<parameters.size(); ++i)
    {
        allSymbols.append(Expression(parameters[i].name));
    }
    allSymbols.append(initSymbols2);
    allSymbols.append(finalSymbols2);
    removeDuplicates(allSymbols);

    //Generate a list of state variables (= "output" variables & local variables)
    QList<Expression> nonStateVars;

    for(int i=0; i<ports.size(); ++i)
    {
        QString num = QString::number(i+1);
        if(ports[i].porttype == "ReadPort")
        {
            nonStateVars.append(Expression(ports[i].name));
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "C")
        {
            QStringList qVars;
            qVars << getQVariables(ports[i].nodetype);
            for(int v=0; v<qVars.size(); ++v)
            {
                nonStateVars.append(Expression(qVars[v]+num));
            }
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "Q")
        {
            QStringList cVars;
            cVars << getCVariables(ports[i].nodetype);
            for(int v=0; v<cVars.size(); ++v)
            {
                nonStateVars.append(Expression(cVars[v]+num));
            }
        }
    }

    for(int i=0; i<parameters.size(); ++i)
    {
        nonStateVars.append(Expression(parameters[i].name));
    }
    for(int i=0; i<initSymbols2.size(); ++i)
    {
        nonStateVars.append(initSymbols2[i]);
    }
    for(int i=0; i<finalSymbols2.size(); ++i)
    {
        nonStateVars.append(finalSymbols2[i]);
    }

    QList<Expression> stateVars = allSymbols;
    for(int i=0; i<nonStateVars.size(); ++i)
    {
        stateVars.removeAll(nonStateVars[i]);
    }

    //Verify equation system
    if(!verifyEquationSystem(equations, stateVars))
    {
//        printErrorMessage("Verification of equation system failed.");
        return;
    }


    //Generate list of local variables (variables that are neither input nor output)
    QList<Expression> nonLocals;

    for(int i=0; i<ports.size(); ++i)
    {
        QString num = QString::number(i+1);
        if(ports[i].porttype == "ReadPort" || ports[i].porttype == "WritePort")
        {
            nonLocals.append(Expression(ports[i].name));     //Remove all readport/writeport varibles
        }
        else if(ports[i].porttype == "PowerPort")
        {
            QStringList qVars;
            QStringList cVars;
            qVars << getQVariables(ports[i].nodetype);
            cVars << getCVariables(ports[i].nodetype);
            for(int v=0; v<qVars.size(); ++v)
            {
                nonLocals.append(Expression(qVars[v]+num));      //Remove all Q-type variables
            }
            for(int v=0; v<cVars.size(); ++v)
            {
                nonLocals.append(Expression(cVars[v]+num));      //Remove all C-type variables
            }
        }
    }
    for(int i=0; i<parameters.size(); ++i)
    {
        nonLocals.append(Expression(parameters[i].name));   //Remove all parameters
    }

    QList<Expression> localVars = allSymbols;
    for(int i=0; i<nonLocals.size(); ++i)
    {
        localVars.removeAll(nonLocals[i]);
    }
    for(int i=0; i<localVars.size(); ++i)
    {
        allSymbols.removeAll(localVars[i]);
    }

    for(int i=0; i<equations.size(); ++i)
    {
        equations[i].toLeftSided();
        equations[i].replaceBy(equations[i].getChild(0));
    }

    for(int i=0; i<equations.size(); ++i)
    {
        equations[i] = equations[i].bilinearTransform();
    }


    //Linearize equations
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].linearize();
        equations[e].expandPowers();
        equations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
        qDebug() << "\nLINEARIZED: " << equations[e].toString();
    }


    //Transform delay operators to delay functions and store delay terms separately
    QList<Expression> delayTerms;
    QStringList delaySteps;
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].expand();
        equations[e].toDelayForm(delayTerms, delaySteps);
        equations[e]._simplify(Expression::FullSimplification);
    }


    for(int i=0; i<limitedVariableEquations.size(); ++i)
    {
        equations[limitedVariableEquations[i]].factor(limitedVariables[i]);

        Expression rem = equations[limitedVariableEquations[i]];
        rem.replace(limitedVariables[i], Expression(0));
        rem._simplify(Expression::FullSimplification, Expression::Recursive);

        qDebug() << "REM: " << rem.toString();

        Expression div = equations[limitedVariableEquations[i]];
        div.subtractBy(rem);
        div.replace(limitedVariables[i], Expression(1));
        div.expandParentheses();
       // div._simplify(Expression::FullSimplification, Expression::Recursive);

        qDebug() << "DIV: " << div.toString();

        rem = Expression(rem, "/", div);
        rem = rem.negative();
        rem._simplify(Expression::FullSimplification, Expression::Recursive);

        qDebug() << "REM AGAIN: " << rem.toString();

        qDebug() << "Limit string: -limit(("+rem.toString()+"),"+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")";
        equations[limitedVariableEquations[i]] = Expression(limitedVariables[i], "+", Expression("-limit(("+rem.toString()+"),"+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")"));

        qDebug() << "Limited: " << equations[limitedVariableEquations[i]].toString();

        if(!limitedDerivatives[i].toString().isEmpty())      //Variable2Limits (has a derivative)
        {
            equations[limitedDerivativeEquations[i]].factor(limitedDerivatives[i]);

            Expression rem = equations[limitedDerivativeEquations[i]];
            rem.replace(limitedDerivatives[i], Expression(0));
            rem._simplify(Expression::FullSimplification, Expression::Recursive);

            Expression div = equations[limitedDerivativeEquations[i]];
            div.subtractBy(rem);
            div.replace(limitedDerivatives[i], Expression(1));
            div.expandParentheses();
            div.factorMostCommonFactor();

            rem = Expression(rem, "/", div);
            rem = rem.negative();

            equations[limitedDerivativeEquations[i]] = Expression(limitedDerivatives[i], "+", Expression("-dxLimit("+limitedVariables[i].toString()+","+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")", "*", rem));
        }
    }

    //Differentiate each equation for each state variable to generate the Jacobian matrix
    QList<QList<Expression> > jacobian;
    for(int e=0; e<equations.size(); ++e)
    {
        //Remove all delay operators, since they shall not be in the Jacobian anyway
        Expression tempExpr = equations[e];

       // tempExpr.replace(Expression("Z", Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));
       // tempExpr.replace(Expression("-Z",Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));

        tempExpr._simplify(Expression::SimplifyWithoutMakingPowers, Expression::Recursive);

        //Now differentiate all jacobian elements
        jacobian.append(QList<Expression>());
        for(int j=0; j<stateVars.size(); ++j)
        {
            bool ok = true;
            tempExpr.replace(Expression(stateVars[j].negative()), Expression(stateVars[j], "*", Expression("-1.0", Expression::NoSimplifications), Expression::NoSimplifications));
            jacobian[e].append(tempExpr.derivative(stateVars[j], ok));
            if(!ok)
            {
                printErrorMessage("Failed to differentiate expression: " + equations[e].toString() + " for variable " + stateVars[j].toString());
                return;
            }
            jacobian[e].last().expandPowers();
            jacobian[e].last()._simplify(Expression::SimplifyWithoutMakingPowers);
        }
    }



    //Sort equation system so that each equation contains its corresponding state variable
    if(!sortEquationSystem(equations, jacobian, stateVars, limitedVariableEquations, limitedDerivativeEquations))
    {
        printErrorMessage("Could not sort equations. System is probably under-determined.");
        qDebug() << "Could not sort equations. System is probably under-determined.";
        return;
    }

    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].expandPowers();
        equations[e]._simplify(Expression::SimplifyWithoutMakingPowers);
    }

    //Generate appearance object
    //ModelObjectAppearance appearance = generateAppearance(ports, cqsType);

    QList<Expression> yExpressions;
    QList<Expression> xExpressions;
    QList<Expression> vExpressions;

//    QList<QList<Expression> > L;
//    for(int i=0; i<jacobian.size(); ++i)
//    {
//        L.append(QList<Expression>());
//        for(int j=0; j<jacobian.size(); ++j)
//        {
//            if(i == j)
//            {
//                L[i].append(Expression(1));
//            }
//            else
//            {
//                L[i].append(Expression(0));
//            }
//        }
//    }

//    QList<QList<Expression> > U = jacobian;

//    qDebug() << "\nJacobian matrix:";
//    for(int i=0; i<L.size(); ++i)
//    {
//        QString line;
//        for(int j=0; j<L.size(); ++j)
//        {
//            line.append(jacobian[i][j].toString());
//            line.append(",     ");
//        }
//        line.chop(6);
//        qDebug() << line;
//    }


//    for(int i=0; i<jacobian.size()-1; ++i)
//    {
//        for(int j=i+1; j<jacobian.size(); ++j)
//        {
//            Expression factor = Expression(1);
//            factor.divideBy(U[i][i]);
//            factor.multiplyBy(U[j][i]);
//            L[j][i] = factor;
//            L[j][i].expandParentheses();
//            L[j][i]._simplify(Expression::FullSimplification, Expression::Recursive);
//            for(int k=i; k<jacobian.size(); ++k)
//            {
//                if(j==1 && k==1)
//                {
//                    qDebug() << "factor: " << factor.toString();

//                    int apa=3;
//                }

//                Expression temp = U[i][k];
//                temp.multiplyBy(factor);
//                U[j][k].subtractBy(temp);
//                U[j][k].expandParentheses();
//                U[j][k]._simplify(Expression::FullSimplification, Expression::Recursive);
//                           }
//            U[j][i] = Expression(0);
//        }
//    }


//    qDebug() << "U matrix:";
//    for(int l=0; l<U.size(); ++l)
//    {
//        QString line;
//        for(int m=0; m<U.size(); ++m)
//        {
//            line.append(U[l][m].toString());
//            line.append(",     ");
//        }
//        line.chop(6);
//        qDebug() << line;
//    }

//    qDebug() << "\nL matrix:";
//    for(int i=0; i<L.size(); ++i)
//    {
//        QString line;
//        for(int j=0; j<L.size(); ++j)
//        {
//            line.append(L[i][j].toString());
//            line.append(",     ");
//        }
//        line.chop(6);
//        qDebug() << line;
//    }
//    qDebug() << "\n";

//    for(int r=0; r<L.size(); ++r)
//    {
//        Expression temp = Expression(0);
//        for(int c=0; c<L.size(); ++c)
//        {
//            Expression y = Expression("y_LU"+QString::number(c));
//            Expression element = L[r][c];
//            element.multiplyBy(y);
//            temp.addBy(element);
//        }
//        Expression y = Expression("y_LU"+QString::number(r));

//        temp.replace(y, Expression("0"));
//        Expression right = equations[r];
//        right.subtractBy(temp);
//        right.expand();
//        right._simplify(Expression::FullSimplification, Expression::Recursive);

//        yExpressions.append(Expression(y, "=", right));

//        qDebug() << "Y expression: " << yExpressions.last().toString();
//    }
//    qDebug() << "\n";

//    for(int r=U.size()-1; r>=0; --r)
//    {
//        Expression temp = Expression(0);
//        for(int c=0; c<U.size(); ++c)
//        {
//            Expression x = Expression("x_LU"+QString::number(c));
//            Expression element = L[r][c];
//            element.multiplyBy(x);
//            temp.addBy(element);
//        }
//        Expression x = Expression("x_LU"+QString::number(r));
//        temp.replace(x, Expression("0"));
//        Expression right = Expression("y_LU"+QString::number(r));
//        right.subtractBy(temp);
//        right.expand();
//        right._simplify(Expression::FullSimplification, Expression::Recursive);

//        xExpressions.append(Expression(x, "=", right));

//        qDebug() << "X expression: " << xExpressions.last().toString();
//    }
//    qDebug() << "\n";

//    for(int v=0; v<stateVars.size(); ++v)
//    {
//        Expression x = Expression("-x_LU"+QString::number(v));
//        Expression right = Expression(stateVars[v], "+", x);
//        vExpressions.append(Expression(stateVars[v], "=", right));
//        qDebug() << "V Expression: " << vExpressions.last().toString();
//    }
//    qDebug() << "\n";

//    for(int i=0; i<yExpressions.size(); ++i)
//    {
//        localVars.append(Expression("y_LU"+QString::number(i)));
//        localVars.append(Expression("x_LU"+QString::number(i)));
//    }


//    jacobian.clear();


    comp.typeName = typeName;
    comp.displayName = displayName;
    comp.cqsType = cqsType;
    if(comp.cqsType == "S") { comp.cqsType = "Signal"; }

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.utilities << "Delay";
        comp.utilityNames << "mDelay"+QString::number(i);
    }

    for(int i=0; i<ports.size(); ++i)
    {
        comp.portNames << ports[i].name;
        comp.portNodeTypes << ports[i].nodetype;
        comp.portTypes << ports[i].porttype;
        comp.portNotReq << ports[i].notrequired;
        comp.portDefaults << ports[i].defaultvalue;
    }

    for(int i=0; i<parameters.size(); ++i)
    {
        comp.parNames << parameters[i].name;
        comp.parDisplayNames << parameters[i].displayName;
        comp.parDescriptions << parameters[i].description;
        comp.parUnits << parameters[i].unit;
        comp.parInits << parameters[i].init;
    }

    if(!jacobian.isEmpty())
    {
        comp.varNames << "order["+QString::number(stateVars.size())+"]" << "jacobianMatrix" << "systemEquations" << "stateVariables" << "mpSolver";
        comp.varTypes << "double" << "Matrix" << "Vec" << "Vec" << "EquationSystemSolver*";

        comp.initEquations << "jacobianMatrix.create("+QString::number(equations.size())+","+QString::number(stateVars.size())+");";
        comp.initEquations << "systemEquations.create("+QString::number(equations.size())+");";
        comp.initEquations << "stateVariables.create("+QString::number(equations.size())+");";
        comp.initEquations << "";
    }

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.initEquations << "mDelay"+QString::number(i)+".initialize("+QString::number(delaySteps.at(i).toInt())+", "+delayTerms[i].toString()+");";
    }

    if(!jacobian.isEmpty())
    {
        comp.initEquations << "";
        //comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString::number(sysEquations.size())+");";
        comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString::number(equations.size())+", &jacobianMatrix, &systemEquations, &stateVariables);";
    }

    comp.simEquations << "//Initial algorithm section";
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        comp.simEquations << initAlgorithms[i].toString()+";";
    }
    comp.simEquations << "";

    if(!yExpressions.isEmpty())
    {
        //Forwards substitution
        for(int i=0; i<yExpressions.size(); ++i)
        {
            comp.simEquations << yExpressions[i].toString()+";";
        }

        //Backwards substitution
        for(int i=xExpressions.size()-1; i>=0; --i)
        {
            comp.simEquations << xExpressions[i].toString()+";";
        }

        //Newton-Rhapson
        for(int i=0; i<vExpressions.size(); ++i)
        {
            comp.simEquations << vExpressions[i].toString()+";";
        }
    }



        //! @todo Add support for using more than one iteration

    if(!jacobian.isEmpty())
    {
        for(int i=0; i<stateVars.size(); ++i)
        {
            comp.simEquations << "stateVariables["+QString::number(i)+"] = "+stateVars[i].toString()+";";
        }

        comp.simEquations << "";
        comp.simEquations << "    //System Equations";
        for(int i=0; i<equations.size(); ++i)
        {
            comp.simEquations << "    systemEquations["+QString::number(i)+"] = "+equations[i].toString()+";";
   //         comp.simEquations << "    "+stateVars[i]+" = " + resEquations[i]+";";
        }
        comp.simEquations << "";
        comp.simEquations << "    //Jacobian Matrix";
        for(int i=0; i<equations.size(); ++i)
        {
            for(int j=0; j<stateVars.size(); ++j)
            {
                comp.simEquations << "    jacobianMatrix["+QString::number(i)+"]["+QString::number(j)+"] = "+jacobian[i][j].toString()+";";
            }
        }

        comp.simEquations << "";
        comp.simEquations << "    //Solving equation using LU-faktorisation";
        comp.simEquations << "    mpSolver->solve();";
        comp.simEquations << "";
        for(int i=0; i<stateVars.size(); ++i)
        {
            comp.simEquations << "    "+stateVars[i].toString()+"=stateVariables["+QString::number(i)+"];";
        }
    }

    //Update delays
    comp.simEquations << "";
    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.simEquations << "mDelay"+QString::number(i)+".update("+delayTerms[i].toString()+");";
    }

    comp.simEquations << "";
    comp.simEquations << "//Final algorithm section";
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        comp.simEquations << finalAlgorithms[i].toString()+";";
    }

    if(!jacobian.isEmpty())
    {
        comp.finalEquations << "delete(mpSolver);";
    }

    for(int i=0; i<localVars.size(); ++i)
    {
        comp.varNames << localVars[i].toString();
        comp.varTypes << "double";
    }
}



QString HopsanGenerator::generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues)
{
    if(comp.cqsType == "S") { comp.cqsType = "Signal"; }


    QString code;
    QTextStream codeStream(&code);

    codeStream << "#ifndef " << comp.typeName.toUpper() << "_HPP_INCLUDED\n";
    codeStream << "#define " << comp.typeName.toUpper() << "_HPP_INCLUDED\n\n";
    codeStream << "#include <math.h>\n";
    codeStream << "#include \"ComponentEssentials.h\"\n";
    codeStream << "#include \"ComponentUtilities.h\"\n";
    codeStream << "#include <sstream>\n\n";
    codeStream << "using namespace std;\n\n";
    codeStream << "namespace hopsan {\n\n";
    codeStream << "    class " << comp.typeName << " : public Component" << comp.cqsType << "\n";
    codeStream << "    {\n";
    codeStream << "    private:\n";                         // Private section
    codeStream << "        double ";
    int portId=1;

    for(int i=0; i<comp.portNames.size(); ++i)              //Declare variables
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else
        {
            varNames << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size()-1; ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            codeStream << varName << ", ";
        }
        QString varName;
        if(comp.portNodeTypes[i] == "NodeSignal")
            varName = varNames.last();
        else
            varName = varNames.last() + QString::number(portId);
        codeStream << varName;
        ++portId;
        if(i < comp.portNames.size()-1)
        {
            codeStream << ", ";
        }
    }

    codeStream << ";\n";
    for(int i=0; i<comp.parNames.size(); ++i)                   //Declare parameters
    {
        codeStream << "        double " << comp.parNames[i] << ";\n";
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        codeStream << "        " << comp.varTypes[i] << " " << comp.varNames[i] << ";\n";
    }
    for(int i=0; i<comp.utilities.size(); ++i)
    {
        codeStream << "        " << comp.utilities[i] << " " << comp.utilityNames[i] << ";\n";
    }
    codeStream << "        double ";
    portId=1;
    QStringList allVarNames;                                    //Declare node data pointers
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QString id = QString::number(portId);
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            allVarNames << comp.portNames[i];
        }
        else
        {
            QStringList vars;
            vars << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);

            for(int v=0; v<vars.size(); ++v)
            {
                allVarNames << vars[v]+id;
            }
        }
        ++portId;
    }

    if(!allVarNames.isEmpty())
    {
        codeStream << "*mpND_" << allVarNames[0];
        for(int i=1; i<allVarNames.size(); ++i)
        {
            codeStream << ", *mpND_" << allVarNames[i];
        }
    }

    codeStream << ";\n";
    codeStream << "        Port ";                              //Declare ports
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        codeStream << "*mp" << comp.portNames[i];
        if(i<comp.portNames.size()-1)
        {
            codeStream << ", ";
        }
    }

    codeStream << ";\n\n";
    codeStream << "    public:\n";                              //Public section
    codeStream << "        static Component *Creator()\n";
    codeStream << "        {\n";
    codeStream << "            return new " << comp.typeName << "();\n";
    codeStream << "        }\n\n";
    codeStream << "        " << comp.typeName << "() : Component" << comp.cqsType << "()\n";
    codeStream << "        {\n";
    for(int i=0; i<comp.parNames.size(); ++i)
    {
        codeStream << "            " << comp.parNames[i] << " = " << comp.parInits[i] << ";\n";
    }
    codeStream << "\n";

    for(int i=0; i<comp.parNames.size(); ++i)
    {
        codeStream << "            registerParameter(\"" << comp.parDisplayNames[i] << "\", \""
                   << comp.parDescriptions[i] << "\", \"" << comp.parUnits[i] << "\", " << comp.parNames[i] << ");\n";
    }
    codeStream << "\n";
    for(int i=0; i<comp.portNames.size(); ++i)
    {

        codeStream << "            mp" << comp.portNames[i] << " = add" << comp.portTypes[i]
                   << "(\"" << comp.portNames[i] << "\", \"" << comp.portNodeTypes[i] << "\"";
        if(comp.portNotReq[i])
        {
            codeStream << ", Port::NOTREQUIRED);\n";
        }
        else
        {
            codeStream << ");\n";
        }
    }

    codeStream << "        }\n\n";
    codeStream << "        void initialize()\n";
    codeStream << "        {\n";
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        QStringList varLabels;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
            varLabels << "VALUE";
        }
        else
        {
            varNames << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);
            varLabels << getVariableLabels(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v]+QString::number(portId);
            codeStream << "            mpND_" << varName << " = getSafeNodeDataPtr(mp" << comp.portNames[i] << ", " << comp.portNodeTypes[i] << "::" << varLabels[v];
            if(comp.portNotReq[i])
            {
                codeStream << ", " << comp.portDefaults[i];
            }
            codeStream << ");\n";
        }
        ++portId;
    }

    codeStream << "\n";
    if(!comp.initEquations.isEmpty())
    {
        portId=1;
        for(int i=0; i<comp.portNames.size(); ++i)
        {
            QStringList varNames;
            if(comp.portNodeTypes[i] == "NodeSignal")
            {
                varNames << comp.portNames[i];
            }
            else
            {
                varNames << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);
            }

            for(int v=0; v<varNames.size(); ++v)
            {
                QString varName;
                if(comp.portNodeTypes[i] == "NodeSignal")
                    varName = varNames[v];
                else
                    varName = varNames[v] + QString::number(portId);
                codeStream << "            " << varName << " = (*mpND_" << varName << ");\n";
            }
            ++portId;
        }
        codeStream << "\n";
        for(int i=0; i<comp.initEquations.size(); ++i)
        {
            codeStream << "            " << comp.initEquations[i] << "\n";
        }
        if(overwriteStartValues)
        {
            codeStream << "\n";
            portId=1;
            for(int i=0; i<comp.portNames.size(); ++i)
            {
                QStringList varNames;
                if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "WritePort")
                {
                    varNames << comp.portNames[i];
                }
                if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "Q" || comp.cqsType == "S"))
                {
                    varNames << getQVariables(comp.portNodeTypes[i]);
                }
                if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
                {
                    varNames << getCVariables(comp.portNodeTypes[i]);
                }
                for(int v=0; v<varNames.size(); ++v)
                {
                    QString varName;
                    if(comp.portNodeTypes[i] == "NodeSignal")
                        varName = varNames[v];
                    else
                        varName = varNames[v] + QString::number(portId);
                    codeStream << "            (*mpND_" << varName << ") = " << varName << ";\n";
                }
            }
            ++portId;
        }
    }
    codeStream << "        }\n\n";

    //Simulate one time step
    codeStream << "        void simulateOneTimestep()\n";
    codeStream << "        {\n";
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "ReadPort")
        {
            varNames << comp.portNames[i];
        }
//        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
//        {
//            varNames << getQVariables(comp.portNodeTypes[i]);
//        }
//        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "Q" || comp.cqsType == "S"))
//        {
//            varNames << getCVariables(comp.portNodeTypes[i]);
//        }
        else
        {
            varNames << getQVariables(comp.portNodeTypes[i]);       //Always create both C- and Q-type variables, regaradless of component type (they may be needed)
            varNames << getCVariables(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            codeStream << "            " << varName << " = (*mpND_" << varName << ");\n";
        }
        ++portId;
    }

    codeStream << "\n";
    for(int i=0; i<comp.simEquations.size(); ++i)
    {
        codeStream << "            " << comp.simEquations[i] << "\n";
    }
    codeStream << "\n";
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "WritePort")
        {
            varNames << comp.portNames[i];
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "Q" || comp.cqsType == "S"))
        {
            varNames << getQVariables(comp.portNodeTypes[i]);
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
        {
            varNames << getCVariables(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            codeStream << "            (*mpND_" << varName << ") = " << varName << ";\n";
        }
        ++portId;
    }

    codeStream << "        }\n\n";
    codeStream << "        void finalize()\n";
    codeStream << "        {\n";
    for(int i=0; i<comp.finalEquations.size(); ++i)
    {
        codeStream << "            " << comp.finalEquations[i] << "\n";
    }
    codeStream << "        }\n\n";
    codeStream << "    };\n";
    codeStream << "}\n\n";

    codeStream << "#endif // " << comp.typeName.toUpper() << "_HPP_INCLUDED\n";

    return code;
}



//! @brief Generates and compiles component source code from a ComponentSpecification object
//! @param outputFile Name of output file
//! @param comp Component specification object
//! @param overwriteStartValues Tells whether or not this components overrides the built-in start values or not
void HopsanGenerator::compileFromComponentObject(QString outputFile, ComponentSpecification comp, /*ModelObjectAppearance appearance,*/ bool overwriteStartValues)
{
    QString code;

    if(comp.plainCode.isEmpty())
    {
        code = generateSourceCodefromComponentObject(comp, overwriteStartValues);
    }
    else
    {
        code = comp.plainCode;
    }

    qDebug() << "Code: " << code;

    if(!QDir(mTempPath).exists())
    {
        QDir().mkpath(mTempPath);
    }

    if(!QDir(mOutputPath).exists())
    {
        QDir().mkpath(mOutputPath);
    }

    printMessage("Writing "+outputFile+"...");

    //Initialize the file stream
    QFileInfo fileInfo;
    QFile file;
    fileInfo.setFile(QString(mTempPath)+outputFile);
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open file for writing: " + outputFile);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    fileStream << code;

    file.close();

    printMessage("Writing tempLib.cc...");

    QFile ccLibFile;
    ccLibFile.setFileName(QString(mTempPath)+"tempLib.cc");
    if(!ccLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open tempLib.cc for writing.");
        return;
    }
    QTextStream ccLibStream(&ccLibFile);
    ccLibStream << "#include \"" << outputFile << "\"\n";
    ccLibStream << "#include \"ComponentEssentials.h\"\n\n";
    ccLibStream << "using namespace hopsan;\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* /*nfact_ptr*/)\n";
    ccLibStream << "{\n";
    ccLibStream << "    cfact_ptr->registerCreatorFunction(\"" << comp.typeName<< "\", " << comp.typeName << "::Creator);\n";
    ccLibStream << "}\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n";
    ccLibStream << "{\n";
    ccLibStream << "    pHopsanExternalLibInfo->libName = (char*)\"HopsanGeneratedComponent_"+comp.typeName+"\";\n";
    ccLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    QFile clBatchFile;
    clBatchFile.setFileName(QString(mTempPath)+"compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
    QTextStream clBatchStream(&clBatchFile);
    QString choppedIncludePath = mCoreIncludePath;
    choppedIncludePath.chop(1);
    clBatchStream << "g++.exe -shared tempLib.cc -o " << comp.typeName << ".dll -I\"" << choppedIncludePath<< "\"  -I\"" << mCoreIncludePath + "\" -L\"" + mBinPath << "\" -lHopsanCore\n";
    clBatchFile.close();

    printMessage("Writing " + comp.typeName + ".xml...");

    //! @todo Fix appearance file generation without ModelObjectAppearance
    QFile xmlFile;
    xmlFile.setFileName(QString(mOutputPath)+comp.typeName+".xml");
    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open " + comp.typeName + ".xml  for writing.");
        return;
    }
    QTextStream xmlStream(&xmlFile);
    xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
    xmlStream << "  <modelobject typename=\"" << comp.typeName << "\" displayname=\"" << comp.displayName << "\">\n";
    xmlStream << "    <icons/>\n";
    xmlStream << "    <ports>\n";
    double xDelay = 1.0/(comp.portNames.size()+1.0);
    double xPos = xDelay;
    double yPos = 0;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        xmlStream << "      <port name=\"" << comp.portNames[i] << "\" x=\"" << xPos << "\" y=\"" << yPos << "\" a=\"" << 270 << "\"/>\n";
        xPos += xDelay;
    }
    xmlStream << "    </ports>\n";
    xmlStream << "  </modelobject>\n";
    xmlStream << "</hopsanobjectappearance>\n";
    xmlFile.close();

    QString libFileName = comp.typeName;
#ifdef WIN32
    libFileName.append(".dll");
#else
    libFileName.append(".so");
#endif

    printMessage("Compiling "+libFileName+"...");

    //Execute HopsanFMU compile script
#ifdef WIN32
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + mTempPath + " & compile.bat");
    p.waitForFinished();
#else
    //! @todo Add link path to bin dir! (../?)
    QString command = "cd "+QString(mTempPath)+" && g++ -shared -fPIC tempLib.cc -o " + comp.typeName + ".so -I" + mCoreIncludePath + " -L" + mBinPath + " -lHopsanCore\n";
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

    printMessage("Moving files to output directory...");

    QFile soFile(mTempPath+libFileName);
    QFile::remove(mOutputPath + libFileName);
    soFile.copy(mOutputPath + libFileName);
}





//! @brief Verifies that a system of equations is solveable (number of equations = number of unknowns etc)
bool HopsanGenerator::verifyEquationSystem(QList<Expression> equations, QList<Expression> stateVars)
{
    bool retval = true;

    if(equations.size() != stateVars.size())
    {
        QStringList equationList;
        for(int s=0; s<equations.size(); ++s)
        {
            equationList.append(equations[s].toString());
        }
        qDebug() << "Equations: " << equationList;

        QStringList stateVarList;
        for(int s=0; s<stateVars.size(); ++s)
        {
            stateVarList.append(stateVars[s].toString());
        }
        qDebug() << "State vars: " << stateVarList;

        printErrorMessage("Number of equations = " + QString::number(equations.size()) + ", number of state variables = " + QString::number(stateVars.size()));
        retval = false;
    }

    return retval;
}


//! @brief Verifies that a list of parameter specifications is correct
//! @param parameters List of parameter specifications
bool HopsanGenerator::verifyParameteres(QList<ParameterSpecification> parameters)
{
    for(int i=0; i<parameters.size(); ++i)
    {
        if(parameters.at(i).name.isEmpty())
        {
            printErrorMessage("Parameter " + QString::number(i+1) + " has no name specified.");
            return false;
        }
        if(parameters.at(i).init.isEmpty())
        {
            printErrorMessage("Parameter " + QString::number(i+1) + " has no initial value specified.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of ports specifications is correct
//! @param ports List of ports specifications
bool HopsanGenerator::verifyPorts(QList<PortSpecification> ports)
{
    for(int i=0; i<ports.size(); ++i)
    {
        if(ports.at(i).name.isEmpty())
        {
            printErrorMessage("Port " + QString::number(i+1) + " has no name specified.");
            return false;
        }
        if(ports.at(i).notrequired && ports.at(i).defaultvalue.isEmpty())
        {
            printErrorMessage("Port \"" + ports.at(i).name + " is not required but has no default value.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of utilities specifications is correct
//! @param utilities List of utilities specifications
bool HopsanGenerator::verifyUtilities(QList<UtilitySpecification> utilities)
{
    for(int i=0; i<utilities.size(); ++i)
    {
        if(utilities.at(i).name.isEmpty())
        {
            printErrorMessage("Utility " + QString::number(i+1) + " has no name specified.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of variables specifications is correct
//! @param variables List of variables specifications
bool HopsanGenerator::verifyStaticVariables(QList<StaticVariableSpecification> variables)
{
    for(int i=0; i<variables.size(); ++i)
    {
        if(variables.at(i).name.isEmpty())
        {
            printErrorMessage("Static variable " + QString::number(i+1) + " has no name specified.");
            return false;
        }
    }
    return true;
}





//! @note First and last q-type variable must represent intensity and flow
QStringList HopsanGenerator::getQVariables(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "F" << "x" <<  "me" << "v";
    }
    if(nodeType == "NodeMechanicRotational")
    {
        retval << "T" << "th" << "w";
    }
    if(nodeType == "NodeHydraulic")
    {
        retval << "p" << "q";
    }
    if(nodeType == "NodePneumatic")
    {
        retval << "p" << "qm" << "qe";
    }
    if(nodeType == "NodeElectric")
    {
        retval << "U" << "i";
    }
    return retval;
}


//! @note c must come first and Zc last
QStringList HopsanGenerator::getCVariables(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodeMechanicRotational")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodeHydraulic")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodePneumatic")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodeElectric")
    {
        retval << "c" << "Zc";
    }
    return retval;
}


//! @brief Returns list of variable enum names for specified node type
//! @param nodeType Node type to use
//! @note c must come first and Zc last
QStringList HopsanGenerator::getVariableLabels(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "FORCE" << "POSITION" << "EQMASS"  << "VELOCITY"<< "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeMechanicRotational")
    {
        retval << "TORQUE" << "ANGLE" << "ANGULARVELOCITY" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeHydraulic")
    {
        retval << "PRESSURE" << "FLOW" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodePneumatic")
    {
        retval << "PRESSURE" << "MASSFLOW" << "ENERGYFLOW" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeElectric")
    {
        retval << "VOLTAGE" << "CURRENT" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeSignal")
    {
        retval << "VALUE";
    }
    return retval;
}



//! @brief Function for loading an XML DOM Documunt from file
//! @param[in] rFile The file to load from
//! @param[in] rDomDocument The DOM Document to load into
//! @param[in] rootTagName The expected root tag name to extract from the Dom Document
//! @returns The extracted DOM root element from the loaded DOM document
QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName)
{
    QString errorStr;
    int errorLine, errorColumn;
    if (!rDomDocument.setContent(&rFile, false, &errorStr, &errorLine, &errorColumn))
    {
        //! @todo Error message somehow
    }
    else
    {
        QDomElement xmlRoot = rDomDocument.documentElement();
        if (xmlRoot.tagName() != rootTagName)
        {
            //! @todo Error message somehow
        }
        else
        {
            return xmlRoot;
        }
    }
    return QDomElement(); //NULL
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


//! @brief Copy a directory with contents
//! @param [in] fromPath The absolute path to the directory to copy
//! @param [in] toPath The absolute path to the destination (including resulting dir name)
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
void copyDir(const QString fromPath, QString toPath)
{
    QDir toDir(toPath);
    toDir.mkpath(toPath);
    if (toPath.endsWith('/'))
    {
        toPath.chop(1);
    }

    QDir fromDir(fromPath);
    foreach(QFileInfo info, fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            copyDir(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
        else
        {
            QFile::copy(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
    }
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
void copyIncludeFilesToDir(QString path)
{
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("include");
    saveDir.cd("include");

    copyDir( QString("../HopsanCore/include"), saveDir.path() );
}


//! @brief Removes all illegal characters from the string, so that it can be used as a variable name.
//! @param org Original string
//! @returns String without illegal characters
inline QString HopsanGenerator::toVarName(const QString org)
{
    QString ret = org;
    while(!ret.isEmpty() && !ret[0].isLetter())
    {
        ret = ret.right(ret.size()-1);
    }
    for(int i=1; i<ret.size(); ++i)
    {
        if(!ret[i].isLetterOrNumber())
        {
            ret.remove(i,1);
            i--;
        }
    }
    return ret;
}


QString HopsanGenerator::extractTaggedSection(QString str, QString tag)
{
    QString startStr = ">>>"+tag+">>>";
    QString endStr = "<<<"+tag+"<<<";
    if(!str.contains(startStr) || !str.contains(endStr))
    {
        return QString();
    }
    else
    {
        int i = str.indexOf(startStr)+startStr.size();
        int n = str.indexOf(endStr)-i;
        return str.mid(i, n);
    }
}


void HopsanGenerator::replaceTaggedSection(QString &str, QString tag, QString replacement)
{
    QString taggedSection = ">>>"+tag+">>>"+extractTaggedSection(str, tag)+"<<<"+tag+"<<<";
    str.replace(taggedSection, replacement);
}


QString HopsanGenerator::replaceTag(QString str, QString tag, QString replacement)
{
    QString retval = str;
    retval.replace("<<<"+tag+">>>", replacement);
    return retval;
}


QString HopsanGenerator::replaceTags(QString str, QStringList tags, QStringList replacements)
{
    QString retval = str;
    for(int i=0; i<tags.size(); ++i)
    {
        retval.replace("<<<"+tags[i]+">>>", replacements[i]);
    }
    return retval;
}


// Operators
QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}
