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

#ifdef WIN32
#include <windows.h>
#endif

#include "HopsanComponentGenerator.h"
#include "SymHop.h"


using namespace std;
using namespace SymHop;


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



HopsanComponentGenerator::HopsanComponentGenerator(QString coreIncludePath, QString binPath, bool showDialog)
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


void HopsanComponentGenerator::printMessage(QString msg)
{
    if(mShowDialog)
    {
        mpTextEdit->setTextColor("BLACK");
        mpTextEdit->append(msg);
        QApplication::processEvents();
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }
}


void HopsanComponentGenerator::printErrorMessage(QString msg)
{
    if(mShowDialog)
    {
        mpTextEdit->setTextColor("RED");
        mpTextEdit->append(msg);
        QApplication::processEvents();
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }
}


void HopsanComponentGenerator::generateFromModelica(QString code)
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


void HopsanComponentGenerator::generateFromFmu(QString path)
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

    //Move all binary files to FMU directory
    QDir win32Dir = QDir::cleanPath(fmuDir.path() + "/binaries/win32");
    if(!win32Dir.exists())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: Unable to unpack files");
        return;
    }
    QFileInfoList binaryFiles = win32Dir.entryInfoList(QDir::Files);
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

    printMessage("Parsing XML file");

    //Load XML data from ModelDescription.xml
    //Copy xml-file to this directory
    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(fmuDir.path() + "/ModelDescription.xml");
    if(!win32Dir.exists())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: ModelDescription.xml not found.");
        return;
    }
    QDomDocument fmuDomDocument;
    QDomElement fmuRoot = loadXMLDomDocument(modelDescriptionFile, fmuDomDocument, "fmiModelDescription");
    modelDescriptionFile.close();

    if(fmuRoot == QDomElement())
    {
        removeDir(fmuDir.path());
        printErrorMessage("Import of FMU failed: Could not parse ModelDescription.xml.");
        return;
    }

    printMessage("Writing fmuLib.cc");

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

    QTextStream fmuLibStream(&fmuLibFile);
    fmuLibStream << "#include \"component_code/"+fmuName+".hpp\"\n";
    fmuLibStream << "#include \""+mCoreIncludePath+"ComponentEssentials.h\"\n";
    fmuLibStream << "using namespace hopsan;\n\n";
    fmuLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr)\n";
    fmuLibStream << "{\n";
    fmuLibStream << "    cfact_ptr->registerCreatorFunction(\"" + fmuName + "\", " + fmuName + "::Creator);\n";
    fmuLibStream << "}\n\n";
    fmuLibStream << "extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n";
    fmuLibStream << "{\n";
    fmuLibStream << "    pHopsanExternalLibInfo->libName = (char*)\"HopsanFMULibrary_power\";\n";
    fmuLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    fmuLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    fmuLibStream << "}\n";
    fmuLibFile.close();

    printMessage("Writing " + fmuName + ".hpp");

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

    QTextStream fmuComponentHppStream(&fmuComponentHppFile);
    fmuComponentHppStream << "#ifndef "+fmuName+"_H\n";
    fmuComponentHppStream << "#define "+fmuName+"_H\n\n";
    fmuComponentHppStream << "#define BUFSIZE 4096\n\n";
    fmuComponentHppStream << "#define _WIN32_WINNT 0x0502\n";
    fmuComponentHppStream << "#include \"../fmi_me.h\"\n";
    fmuComponentHppStream << "#include \"../xml_parser.h\"\n";
    fmuComponentHppStream << "#include \""+mCoreIncludePath+"ComponentEssentials.h\"\n\n";
    fmuComponentHppStream << "#include <sstream>\n";
    fmuComponentHppStream << "#include <string>\n";
    fmuComponentHppStream << "#include <stdio.h>\n";
    fmuComponentHppStream << "#include <stdlib.h>\n";
    fmuComponentHppStream << "#include <string.h>\n";
    fmuComponentHppStream << "#include <assert.h>\n";
    fmuComponentHppStream << "#ifdef WIN32\n";
    fmuComponentHppStream << "#include <windows.h>\n";
    fmuComponentHppStream << "#endif\n\n";
    fmuComponentHppStream << "void fmuLogger(fmiComponent c, fmiString instanceName, fmiStatus status,\n";
    fmuComponentHppStream << "               fmiString category, fmiString message, ...){}\n\n";
    fmuComponentHppStream << "namespace hopsan {\n\n";
    fmuComponentHppStream << "    class "+fmuName+" : public ComponentQ\n";
    fmuComponentHppStream << "    {\n";
    fmuComponentHppStream << "    private:\n";
    fmuComponentHppStream << "        FMU mFMU;\n";
    fmuComponentHppStream << "        FILE* mFile;\n";
    fmuComponentHppStream << "        fmiComponent c;                  // instance of the fmu\n";
    fmuComponentHppStream << "        fmiEventInfo eventInfo;          // updated by calls to initialize and eventUpdate\n";
    fmuComponentHppStream << "        int nx;                          // number of state variables\n";
    fmuComponentHppStream << "        int nz;                          // number of state event indicators\n";
    fmuComponentHppStream << "        double *x;                       // continuous states\n";
    fmuComponentHppStream << "        double *xdot;                    // the crresponding derivatives in same order\n";
    fmuComponentHppStream << "        double *z;                // state event indicators\n";
    fmuComponentHppStream << "        double *prez;             // previous values of state event indicators\n\n";
    QDomElement variablesElement = fmuRoot.firstChildElement("ModelVariables");
    QDomElement varElement = variablesElement.firstChildElement("ScalarVariable");
    int i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality"))
        {
            fmuComponentHppStream << "        Port *mpIn"+numStr+";\n";
            fmuComponentHppStream << "        Port *mpOut"+numStr+";\n";
        }
        else if(varElement.attribute("causality") == "input")
            fmuComponentHppStream << "        Port *mpIn"+numStr+";\n";
        else if(varElement.attribute("causality") == "output")
            fmuComponentHppStream << "        Port *mpOut"+numStr+";\n";
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    //fmuComponentHppStream << "        Port *mpP1;\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "input")
            fmuComponentHppStream << "        double *mpND_in" + numStr + ";\n";
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "output")
            fmuComponentHppStream << "        double *mpND_out" + numStr + ";\n";
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            fmuComponentHppStream << "        double par"+QString::number(i)+";\n";
            ++i;
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "\n";
    fmuComponentHppStream << "    public:\n";
    fmuComponentHppStream << "        static Component *Creator()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            return new "+fmuName+"();\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        void configure()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            mFMU.modelDescription = parse(\""+fmuDir.path()+"/ModelDescription.xml\");\n";
    fmuComponentHppStream << "            assert(mFMU.modelDescription);\n";
    fmuComponentHppStream << "            assert(loadDll(\""+fmuDir.path()+"/"+fmuName+".dll\"));\n";
    fmuComponentHppStream << "            addInfoMessage(getString(mFMU.modelDescription, att_modelIdentifier));\n\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality"))
        {
            fmuComponentHppStream << "            mpIn"+numStr+" = addReadPort(\""+varElement.attribute("name")+"In\", \"NodeSignal\", Port::NOTREQUIRED);\n";
            fmuComponentHppStream << "            mpOut"+numStr+" = addWritePort(\""+varElement.attribute("name")+"Out\", \"NodeSignal\", Port::NOTREQUIRED);\n";
        }
        else if(varElement.attribute("causality") == "input")
            fmuComponentHppStream << "            mpIn"+numStr+" = addReadPort(\""+varElement.attribute("name")+"\", \"NodeSignal\", Port::NOTREQUIRED);\n";
        else if(varElement.attribute("causality") == "output")
            fmuComponentHppStream << "            mpOut"+numStr+" = addWritePort(\""+varElement.attribute("name")+"\", \"NodeSignal\", Port::NOTREQUIRED);\n";
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    //fmuComponentHppStream << "            mpP1 = addWritePort(\"out\", \"NodeSignal\");\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            fmuComponentHppStream << "            par"+QString::number(i)+" = "+varElement.firstChildElement("Real").attribute("start")+";\n";
            fmuComponentHppStream << "            registerParameter(\""+varElement.attribute("name")+"\", \""+varElement.attribute("description")+"\", \"-\", par"+QString::number(i)+");\n";
            ++i;
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "       void initialize()\n";
    fmuComponentHppStream << "       {\n";
    fmuComponentHppStream << "           if (!mFMU.modelDescription)\n";
    fmuComponentHppStream << "           {\n";
    fmuComponentHppStream << "               addErrorMessage(\"Missing FMU model description\");\n";
    fmuComponentHppStream << "               stopSimulation();\n";
    fmuComponentHppStream << "           }\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    int nInputs=0;
    int nOutputs=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "input")
        {
            fmuComponentHppStream << "          mpND_in"+numStr+" = getSafeNodeDataPtr(mpIn"+numStr+", NodeSignal::VALUE);\n\n";
            ++nInputs;
        }
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "output")
        {
            fmuComponentHppStream << "          mpND_out"+numStr+" = getSafeNodeDataPtr(mpOut"+numStr+", NodeSignal::VALUE);\n\n";
            ++nOutputs;
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "            //Initialize FMU\n";
    fmuComponentHppStream << "            ModelDescription* md;            // handle to the parsed XML file\n";
    fmuComponentHppStream << "            const char* guid;                // global unique id of the fmu\n";
    fmuComponentHppStream << "            fmiCallbackFunctions callbacks;  // called by the model during simulation\n";
    fmuComponentHppStream << "            fmiStatus fmiFlag;               // return code of the fmu functions\n";
    fmuComponentHppStream << "            fmiReal t0 = 0;                  // start time\n";
    fmuComponentHppStream << "            fmiBoolean toleranceControlled = fmiFalse;\n";
    fmuComponentHppStream << "            int loggingOn = 0;\n\n";
    fmuComponentHppStream << "            // instantiate the fmu\n";
    fmuComponentHppStream << "            md = mFMU.modelDescription;\n";
    fmuComponentHppStream << "            guid = getString(md, att_guid);\n";
    fmuComponentHppStream << "            callbacks.logger = fmuLogger;\n";
    fmuComponentHppStream << "            callbacks.allocateMemory = calloc;\n";
    fmuComponentHppStream << "            callbacks.freeMemory = free;\n";
    fmuComponentHppStream << "            c = mFMU.instantiateModel(getModelIdentifier(md), guid, callbacks, loggingOn);\n\n";
    fmuComponentHppStream << "            // allocate memory\n";
    fmuComponentHppStream << "            nx = getNumberOfStates(md);\n";
    fmuComponentHppStream << "            nz = getNumberOfEventIndicators(md);\n";
    fmuComponentHppStream << "            x    = (double *) calloc(nx, sizeof(double));\n";
    fmuComponentHppStream << "            xdot = (double *) calloc(nx, sizeof(double));\n";
    fmuComponentHppStream << "            if (nz>0)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                z    =  (double *) calloc(nz, sizeof(double));\n";
    fmuComponentHppStream << "                prez =  (double *) calloc(nz, sizeof(double));\n";
    fmuComponentHppStream << "            }\n\n";
    fmuComponentHppStream << "            // set the start time and initialize\n";
    fmuComponentHppStream << "            fmiFlag =  mFMU.setTime(c, t0);\n";
    fmuComponentHppStream << "            fmiFlag =  mFMU.initialize(c, toleranceControlled, t0, &eventInfo);\n\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        void simulateOneTimestep()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            ScalarVariable** vars = mFMU.modelDescription->modelVariables;\n";
    fmuComponentHppStream << "            double value;\n";
    fmuComponentHppStream << "            ScalarVariable* sv;\n";
    fmuComponentHppStream << "            fmiValueReference vr;\n\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        if(varElement.attribute("variability") == "parameter")
        {
            fmuComponentHppStream << "            sv = vars["+QString(varElement.attribute("valueReference"))+"];\n";
            fmuComponentHppStream << "            vr = getValueReference(sv);\n";
            fmuComponentHppStream << "            value=par"+QString::number(i)+";\n";
            fmuComponentHppStream << "            mFMU.setReal(c, &vr, 1, &value);\n";
            ++i;
        }
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "\n";
    fmuComponentHppStream << "            //write input values\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "input")
        {
            fmuComponentHppStream << "            if(mpIn"+numStr+"->isConnected())\n";
            fmuComponentHppStream << "            {\n";
            fmuComponentHppStream << "                sv = vars["+numStr+"];\n";
            fmuComponentHppStream << "                vr = getValueReference(sv);\n";
            fmuComponentHppStream << "                value = (*mpND_in"+numStr+");\n";
            fmuComponentHppStream << "                mFMU.setReal(c, &vr, 1, &value);\n\n";
            fmuComponentHppStream << "            }\n";
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "            //run simulation\n";
    fmuComponentHppStream << "            simulateFMU();\n\n";
    fmuComponentHppStream << "            //write back output values\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "output")
        {

            fmuComponentHppStream << "            sv = vars["+numStr+"];\n";
            fmuComponentHppStream << "            vr = getValueReference(sv);\n";
            fmuComponentHppStream << "            mFMU.getReal(c, &vr, 1, &value);\n";
            fmuComponentHppStream << "            (*mpND_out"+numStr+") = value;\n\n";
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "        }\n";
    fmuComponentHppStream << "        void finalize()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            //cleanup\n";
    fmuComponentHppStream << "            mFMU.terminate(c);\n";
    fmuComponentHppStream << "            //mFMU.freeModelInstance(c);\n";
    fmuComponentHppStream << "            if (x!=NULL) free(x);\n";
    fmuComponentHppStream << "            if (xdot!= NULL) free(xdot);\n";
    fmuComponentHppStream << "            if (z!= NULL) free(z);\n";
    fmuComponentHppStream << "            if (prez!= NULL) free(prez);\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        bool loadDll(std::string dllPath)\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            bool success = true;\n";
    fmuComponentHppStream << "            HANDLE h;\n";
    fmuComponentHppStream << "            std::string libdir = dllPath;\n";
    fmuComponentHppStream << "            while(libdir.at(libdir.size()-1) != '/')\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "            libdir.erase(libdir.size()-1, 1);\n";
    fmuComponentHppStream << "            }\n";
    fmuComponentHppStream << "            SetDllDirectoryA(libdir.c_str());       //Set search path for dependencies\n";
    fmuComponentHppStream << "            h = LoadLibraryA(dllPath.c_str());\n";
    fmuComponentHppStream << "            if (!h)\n";
    fmuComponentHppStream << "            {\n";
    //fmuComponentHppStream << "                qDebug() << QString(\"error: Could not load dll\\n\");\n";
    fmuComponentHppStream << "                success = false; // failure\n";
    fmuComponentHppStream << "                return success;\n";
    fmuComponentHppStream << "            }\n";
    fmuComponentHppStream << "            mFMU.dllHandle = h;\n\n";
    fmuComponentHppStream << "            mFMU.getModelTypesPlatform   = (fGetModelTypesPlatform) getAdr(&success, \"fmiGetModelTypesPlatform\");\n";
    fmuComponentHppStream << "            mFMU.instantiateModel        = (fInstantiateModel)   getAdr(&success, \"fmiInstantiateModel\");\n";
    fmuComponentHppStream << "            mFMU.freeModelInstance       = (fFreeModelInstance)  getAdr(&success, \"fmiFreeModelInstance\");\n";
    fmuComponentHppStream << "            mFMU.setTime                 = (fSetTime)            getAdr(&success, \"fmiSetTime\");\n";
    fmuComponentHppStream << "            mFMU.setContinuousStates     = (fSetContinuousStates)getAdr(&success, \"fmiSetContinuousStates\");\n";
    fmuComponentHppStream << "            mFMU.completedIntegratorStep = (fCompletedIntegratorStep)getAdr(&success, \"fmiCompletedIntegratorStep\");\n";
    fmuComponentHppStream << "            mFMU.initialize              = (fInitialize)         getAdr(&success, \"fmiInitialize\");\n";
    fmuComponentHppStream << "            mFMU.getDerivatives          = (fGetDerivatives)     getAdr(&success, \"fmiGetDerivatives\");\n";
    fmuComponentHppStream << "            mFMU.getEventIndicators      = (fGetEventIndicators) getAdr(&success, \"fmiGetEventIndicators\");\n";
    fmuComponentHppStream << "            mFMU.eventUpdate             = (fEventUpdate)        getAdr(&success, \"fmiEventUpdate\");\n";
    fmuComponentHppStream << "            mFMU.getContinuousStates     = (fGetContinuousStates)getAdr(&success, \"fmiGetContinuousStates\");\n";
    fmuComponentHppStream << "            mFMU.getNominalContinuousStates = (fGetNominalContinuousStates)getAdr(&success, \"fmiGetNominalContinuousStates\");\n";
    fmuComponentHppStream << "            mFMU.getStateValueReferences = (fGetStateValueReferences)getAdr(&success, \"fmiGetStateValueReferences\");\n";
    fmuComponentHppStream << "            mFMU.terminate               = (fTerminate)          getAdr(&success, \"fmiTerminate\");\n\n";
    fmuComponentHppStream << "            mFMU.getVersion              = (fGetVersion)         getAdr(&success, \"fmiGetVersion\");\n";
    fmuComponentHppStream << "            mFMU.setDebugLogging         = (fSetDebugLogging)    getAdr(&success, \"fmiSetDebugLogging\");\n";
    fmuComponentHppStream << "            mFMU.setReal                 = (fSetReal)            getAdr(&success, \"fmiSetReal\");\n";
    fmuComponentHppStream << "            mFMU.setInteger              = (fSetInteger)         getAdr(&success, \"fmiSetInteger\");\n";
    fmuComponentHppStream << "            mFMU.setBoolean              = (fSetBoolean)         getAdr(&success, \"fmiSetBoolean\");\n";
    fmuComponentHppStream << "            mFMU.setString               = (fSetString)          getAdr(&success, \"fmiSetString\");\n";
    fmuComponentHppStream << "            mFMU.getReal                 = (fGetReal)            getAdr(&success, \"fmiGetReal\");\n";
    fmuComponentHppStream << "            mFMU.getInteger              = (fGetInteger)         getAdr(&success, \"fmiGetInteger\");\n";
    fmuComponentHppStream << "            mFMU.getBoolean              = (fGetBoolean)         getAdr(&success, \"fmiGetBoolean\");\n";
    fmuComponentHppStream << "            mFMU.getString               = (fGetString)          getAdr(&success, \"fmiGetString\");\n";
    fmuComponentHppStream << "            return success;\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        void* getAdr(bool* success, const char* functionName)\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            char name[BUFSIZE];\n";
    fmuComponentHppStream << "            void* fp;\n";
    fmuComponentHppStream << "            ModelDescription *me = mFMU.modelDescription;\n";
    fmuComponentHppStream << "            sprintf(name, \"%s_%s\", getModelIdentifier(me), functionName);\n";
    fmuComponentHppStream << "            fp = (void*)GetProcAddress((HINSTANCE__*)mFMU.dllHandle, name);\n";
    fmuComponentHppStream << "            //fp = (void*)GetProcAddress((HINSTANCE)fmu->dllHandle, name);        //CASTINGS MAY NOT WORK!!!\n";
    fmuComponentHppStream << "            if (!fp) {\n";
    fmuComponentHppStream << "                *success = false; // mark dll load as 'failed'\n";
    fmuComponentHppStream << "            }\n";
    fmuComponentHppStream << "            return fp;\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        void simulateFMU()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            int i;                          // For loop index\n";
    fmuComponentHppStream << "            fmiBoolean timeEvent, stateEvent, stepEvent;\n";
    fmuComponentHppStream << "            fmiStatus fmiFlag;               // return code of the fmu functions\n\n";
    fmuComponentHppStream << "            if (eventInfo.terminateSimulation)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                stopSimulation();\n";
    fmuComponentHppStream << "            }\n\n";
    fmuComponentHppStream << "            //Simulate one step\n\n";
    fmuComponentHppStream << "            // get current state and derivatives\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.getContinuousStates(c, x, nx);\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.getDerivatives(c, xdot, nx);\n\n";
    fmuComponentHppStream << "            // advance time\n";
    fmuComponentHppStream << "            timeEvent = eventInfo.upcomingTimeEvent && eventInfo.nextEventTime < mTime;\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.setTime(c, mTime);\n\n";
    fmuComponentHppStream << "            // perform one step\n";
    fmuComponentHppStream << "            for (i=0; i<nx; i++) x[i] += mTimestep*xdot[i]; // forward Euler method\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.setContinuousStates(c, x, nx);\n\n";
    fmuComponentHppStream << "            // Check for step event, e.g. dynamic state selection\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.completedIntegratorStep(c, &stepEvent);\n\n";
    fmuComponentHppStream << "            // Check for state event\n";
    fmuComponentHppStream << "            for (i=0; i<nz; i++) prez[i] = z[i];\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.getEventIndicators(c, z, nz);\n";
    fmuComponentHppStream << "            stateEvent = FALSE;\n";
    fmuComponentHppStream << "            for (i=0; i<nz; i++)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                stateEvent = stateEvent || (prez[i] * z[i] < 0);\n";
    fmuComponentHppStream << "            }\n\n";
    fmuComponentHppStream << "            //! @todo Event criteria are disabled for now, so there will be a time event every time step no matter what.\n\n";
    fmuComponentHppStream << "            // handle events\n";
    fmuComponentHppStream << "            if (timeEvent || stateEvent || stepEvent)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                // event iteration in one step, ignoring intermediate results\n";
    fmuComponentHppStream << "                fmiFlag = mFMU.eventUpdate(c, fmiFalse, &eventInfo);\n";
    fmuComponentHppStream << "                // terminate simulation, if requested by the model\n";
    fmuComponentHppStream << "                if (eventInfo.terminateSimulation)\n";
    fmuComponentHppStream << "                {\n";
    fmuComponentHppStream << "                    stopSimulation();\n";
    fmuComponentHppStream << "                }\n";
    fmuComponentHppStream << "            } // if event\n";
    fmuComponentHppStream << "        }\n";
    fmuComponentHppStream << "    };\n";
    fmuComponentHppStream << "}\n\n";
    fmuComponentHppStream << "#endif // "+fmuName+"_H\n";
    fmuComponentHppFile.close();

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
    double inputPosStep=1.0/(nInputs+1.0);      //These 4 variables are used for port positioning
    double outputPosStep=1.0/(nOutputs+1.0);
    double inputPos=0;
    double outputPos=0;
    while (!varElement.isNull())
    {
        QString numStr, numStr2;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality"))
        {
            inputPos += inputPosStep;
            numStr2.setNum(inputPos);
            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
            outputPos += outputPosStep;
            numStr2.setNum(outputPos);
            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
        }
        else if(varElement.attribute("causality") == "input")
        {
            inputPos += inputPosStep;
            numStr2.setNum(inputPos);
            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
        }
        else if(varElement.attribute("causality") == "output")
        {
            outputPos += outputPosStep;
            numStr2.setNum(outputPos);
            fmuXmlStream << "            <port name=\""+varElement.attribute("name")+"\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuXmlStream << "        </ports>\n";
    fmuXmlStream << "    </modelobject>\n";
    fmuXmlStream << "</hopsanobjectappearance>\n";
    fmuXmlFile.close();

    //Move FMI source files to compile directory
    QFile simSupportSourceFile;
    simSupportSourceFile.setFileName(gExecPath + "../ThirdParty/fmi/sim_support.c");
    if(simSupportSourceFile.copy(fmuDir.path() + "/sim_support.c"))
    {
        printMessage("Copying sim_support.c");
        printMessage("Copying " + simSupportSourceFile.fileName() + " to " + fmuDir.path() + "/sim_support.c");
    }

    QFile stackSourceFile;
    stackSourceFile.setFileName(gExecPath + "../ThirdParty/fmi/stack.cc");
    if(stackSourceFile.copy(fmuDir.path() + "/stack.cc"))
    {
        printMessage("Copying stack.cc");
        printMessage("Copying " + stackSourceFile.fileName() + " to " + fmuDir.path() + "/stack.cc");
    }

    QFile xmlParserSourceFile;
    xmlParserSourceFile.setFileName(gExecPath + "../ThirdParty/fmi/xml_parser.h");
    if(xmlParserSourceFile.copy(fmuDir.path() + "/xml_parser.h"))
    {
        printMessage("Copying xml_parser.h");
        printMessage("Copying " + xmlParserSourceFile.fileName() + " to " + fmuDir.path() + "/xml_parser.h");
    }

    QFile simSupportHeaderFile;
    simSupportHeaderFile.setFileName(gExecPath + "../ThirdParty/fmi/sim_support.h");
    if(simSupportHeaderFile.copy(fmuDir.path() + "/sim_support.h"))
    {
        printMessage("Copying sim_support.h");
        printMessage("Copying " + simSupportHeaderFile.fileName() + " to " + fmuDir.path() + "/sim_support.h");
    }

    QFile stackHeaderFile;
    stackHeaderFile.setFileName(gExecPath + "../ThirdParty/fmi/stack.h");
    if(stackHeaderFile.copy(fmuDir.path() + "/stack.h"))
    {
        printMessage("Copying stack.h");
        printMessage("Copying " + stackHeaderFile.fileName() + " to " + fmuDir.path() + "/stack.h");
    }

    QFile xmlParserHeaderFile;
    xmlParserHeaderFile.setFileName(gExecPath + "../ThirdParty/fmi/xml_parser.cc");
    if(xmlParserHeaderFile.copy(fmuDir.path() + "/xml_parser.cc"))
    {
        printMessage("Copying xml_parser.cc");
        printMessage("Copying " + xmlParserHeaderFile.fileName() + " to " + fmuDir.path() + "/xml_parser.cc");
    }

    QFile expatFile;
    expatFile.setFileName(gExecPath + "../ThirdParty/fmi/expat.h");
    if(expatFile.copy(fmuDir.path() + "/expat.h"))
    {
        printMessage("Copying expat.h");
        printMessage("Copying " + expatFile.fileName() + " to " + fmuDir.path() + "/expat.h");
    }

    QFile expatExternalFile;
    expatExternalFile.setFileName(gExecPath + "../ThirdParty/fmi/expat_external.h");
    if(expatExternalFile.copy(fmuDir.path() + "/expat_external.h"))
    {
        printMessage("Copying expat_external.h");
        printMessage("Copying " + expatExternalFile.fileName() + " to " + fmuDir.path() + "/expat_external.h");
    }

    QFile libExpatAFile;
    libExpatAFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpat.a");
    if(libExpatAFile.copy(fmuDir.path() + "/libexpat.a"))
    {
        printMessage("Copying libexpat.a");
        printMessage("Copying " + libExpatAFile.fileName() + " to " + fmuDir.path() + "/libexpat.a");
    }

    QFile libExpatDllFile;
    libExpatDllFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpat.dll");
    if(libExpatDllFile.copy(fmuDir.path() + "/libexpat.dll"))
    {
        printMessage("Copying libexpat.dll");
        printMessage("Copying " + libExpatDllFile.fileName() + " to " + fmuDir.path() + "/libexpat.dll");
    }

    QFile libExpatwAFile;
    libExpatwAFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpatw.a");
    if(libExpatwAFile.copy(fmuDir.path() + "/libexpatw.a"))
    {
        printMessage("Copying libexpatw.a");
        printMessage("Copying " + libExpatwAFile.fileName() + " to " + fmuDir.path() + "/libexpatw.a");
    }

    QFile libExpatwDllFile;
    libExpatwDllFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpatw.dll");
    if(libExpatwDllFile.copy(fmuDir.path() + "/libexpatw.dll"))
    {
        printMessage("Copying libexpatw.dll");
        printMessage("Copying " + libExpatwDllFile.fileName() + " to " + fmuDir.path() + "/libexpatw.dll");
    }

    QFile fmiMeFile;
    fmiMeFile.setFileName(gExecPath + "../ThirdParty/fmi/fmi_me.h");
    if(fmiMeFile.copy(fmuDir.path() + "/fmi_me.h"))
    {
        printMessage("Copying fmi_me.h");
        printMessage("Copying " + fmiMeFile.fileName() + " to " + fmuDir.path() + "/fmi_me.h");
    }

    QFile fmiModelFunctionsFile;
    fmiModelFunctionsFile.setFileName(gExecPath + "../ThirdParty/fmi/fmiModelFunctions.h");
    if(fmiModelFunctionsFile.copy(fmuDir.path() + "/fmiModelFunctions.h"))
    {
        printMessage("Copying fmiModelFunctions.h");
        printMessage("Copying " + fmiModelFunctionsFile.fileName() + " to " + fmuDir.path() + "/fmiModelFunctions.h");
    }

    QFile fmiModelTypesFile;
    fmiModelTypesFile.setFileName(gExecPath + "../ThirdParty/fmi/fmiModelTypes.h");
    if(fmiModelTypesFile.copy(fmuDir.path() + "/fmiModelTypes.h"))
    {
        printMessage("Copying fmiModelTypes.h");
        printMessage("Copying " + fmiModelTypesFile.fileName() + " to " + fmuDir.path() + "/fmiModelTypes.h");
    }

    printMessage("Writing compilation script");

    //Create compilation script file
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


    printMessage("Compiling " + fmuName + ".dll");


    //Call compilation script file
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

    if(!fmuDir.exists(fmuName + ".dll"))
    {
        printErrorMessage("Import of FMU failed: Compilation error.");
        removeDir(fmuDir.path());
        return;
    }


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



//! @brief Parses a modelica model code to Hopsan classes
//! @param code Input Modelica code
//! @param typeName Type name of new component
//! @param displayName Display name of new component
//! @param initAlgorithms Initial algorithms for new component
//! @param equations Equations for new component
//! @param portList List of port specifications for new component
//! @param parametersList List of parameter specifications for new component
void HopsanComponentGenerator::parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations,
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
                        initAlgorithms.last().insert(idx2, QString().setNum(i+1));
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
                        equations.last().insert(idx2, QString().setNum(i+1));
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
                        finalAlgorithms.last().insert(idx2, QString().setNum(i+1));
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
void HopsanComponentGenerator::generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &plainInitAlgorithms, QStringList &plainEquations, QStringList &plainFinalAlgorithms, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters)
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
        QString num = QString().setNum(i+1);
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
        QString num = QString().setNum(i+1);
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
        comp.utilityNames << "mDelay"+QString().setNum(i);
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
        comp.varNames << "order["+QString().setNum(stateVars.size())+"]" << "jacobianMatrix" << "systemEquations" << "stateVariables" << "mpSolver";
        comp.varTypes << "double" << "Matrix" << "Vec" << "Vec" << "EquationSystemSolver*";

        comp.initEquations << "jacobianMatrix.create("+QString().setNum(equations.size())+","+QString().setNum(stateVars.size())+");";
        comp.initEquations << "systemEquations.create("+QString().setNum(equations.size())+");";
        comp.initEquations << "stateVariables.create("+QString().setNum(equations.size())+");";
        comp.initEquations << "";
    }

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.initEquations << "mDelay"+QString().setNum(i)+".initialize("+QString().setNum(delaySteps.at(i).toInt())+", "+delayTerms[i].toString()+");";
    }

    if(!jacobian.isEmpty())
    {
        comp.initEquations << "";
        //comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString().setNum(sysEquations.size())+");";
        comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString().setNum(equations.size())+", &jacobianMatrix, &systemEquations, &stateVariables);";
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
            comp.simEquations << "stateVariables["+QString().setNum(i)+"] = "+stateVars[i].toString()+";";
        }

        comp.simEquations << "";
        comp.simEquations << "    //System Equations";
        for(int i=0; i<equations.size(); ++i)
        {
            comp.simEquations << "    systemEquations["+QString().setNum(i)+"] = "+equations[i].toString()+";";
   //         comp.simEquations << "    "+stateVars[i]+" = " + resEquations[i]+";";
        }
        comp.simEquations << "";
        comp.simEquations << "    //Jacobian Matrix";
        for(int i=0; i<equations.size(); ++i)
        {
            for(int j=0; j<stateVars.size(); ++j)
            {
                comp.simEquations << "    jacobianMatrix["+QString().setNum(i)+"]["+QString().setNum(j)+"] = "+jacobian[i][j].toString()+";";
            }
        }

        comp.simEquations << "";
        comp.simEquations << "    //Solving equation using LU-faktorisation";
        comp.simEquations << "    mpSolver->solve();";
        comp.simEquations << "";
        for(int i=0; i<stateVars.size(); ++i)
        {
            comp.simEquations << "    "+stateVars[i].toString()+"=stateVariables["+QString().setNum(i)+"];";
        }
    }

    //Update delays
    comp.simEquations << "";
    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.simEquations << "mDelay"+QString().setNum(i)+".update("+delayTerms[i].toString()+");";
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



QString HopsanComponentGenerator::generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues)
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
                varName = varNames[v] + QString().setNum(portId);
            codeStream << varName << ", ";
        }
        QString varName;
        if(comp.portNodeTypes[i] == "NodeSignal")
            varName = varNames.last();
        else
            varName = varNames.last() + QString().setNum(portId);
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
        QString id = QString().setNum(portId);
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
                varName = varNames[v]+QString().setNum(portId);
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
                    varName = varNames[v] + QString().setNum(portId);
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
                        varName = varNames[v] + QString().setNum(portId);
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
                varName = varNames[v] + QString().setNum(portId);
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
                varName = varNames[v] + QString().setNum(portId);
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
void HopsanComponentGenerator::compileFromComponentObject(QString outputFile, ComponentSpecification comp, /*ModelObjectAppearance appearance,*/ bool overwriteStartValues)
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
bool HopsanComponentGenerator::verifyEquationSystem(QList<Expression> equations, QList<Expression> stateVars)
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

        printErrorMessage("Number of equations = " + QString().setNum(equations.size()) + ", number of state variables = " + QString().setNum(stateVars.size()));
        retval = false;
    }

    return retval;
}


//! @brief Verifies that a list of parameter specifications is correct
//! @param parameters List of parameter specifications
bool HopsanComponentGenerator::verifyParameteres(QList<ParameterSpecification> parameters)
{
    for(int i=0; i<parameters.size(); ++i)
    {
        if(parameters.at(i).name.isEmpty())
        {
            printErrorMessage("Parameter " + QString().setNum(i+1) + " has no name specified.");
            return false;
        }
        if(parameters.at(i).init.isEmpty())
        {
            printErrorMessage("Parameter " + QString().setNum(i+1) + " has no initial value specified.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of ports specifications is correct
//! @param ports List of ports specifications
bool HopsanComponentGenerator::verifyPorts(QList<PortSpecification> ports)
{
    for(int i=0; i<ports.size(); ++i)
    {
        if(ports.at(i).name.isEmpty())
        {
            printErrorMessage("Port " + QString().setNum(i+1) + " has no name specified.");
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
bool HopsanComponentGenerator::verifyUtilities(QList<UtilitySpecification> utilities)
{
    for(int i=0; i<utilities.size(); ++i)
    {
        if(utilities.at(i).name.isEmpty())
        {
            printErrorMessage("Utility " + QString().setNum(i+1) + " has no name specified.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of variables specifications is correct
//! @param variables List of variables specifications
bool HopsanComponentGenerator::verifyStaticVariables(QList<StaticVariableSpecification> variables)
{
    for(int i=0; i<variables.size(); ++i)
    {
        if(variables.at(i).name.isEmpty())
        {
            printErrorMessage("Static variable " + QString().setNum(i+1) + " has no name specified.");
            return false;
        }
    }
    return true;
}





//! @note First and last q-type variable must represent intensity and flow
QStringList HopsanComponentGenerator::getQVariables(QString nodeType)
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
QStringList HopsanComponentGenerator::getCVariables(QString nodeType)
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
QStringList HopsanComponentGenerator::getVariableLabels(QString nodeType)
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
