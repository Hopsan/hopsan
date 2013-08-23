/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 rediibuting any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   EditComponentDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the component generator dialog
//!
//$Id$

//C++ includes
#include <cassert>

//Qt includes
#include <QFont>

//Hopsan includes
#include "common.h"
#include "Configuration.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "Dialogs/ComponentGeneratorDialog.h"
#include "Dialogs/EditComponentDialog.h"
#include "Dialogs/MovePortsDialog.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "GUIPort.h"
#include "Utilities/ComponentGeneratorUtilities.h"
#include "Utilities/HighlightingUtilities.h"
#include "Utilities/XMLUtilities.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/PyDockWidget.h"


EditComponentDialog::EditComponentDialog(QString code, SourceCodeEnumT language)
{
    if (this->objectName().isEmpty())
        this->setObjectName(QString::fromUtf8("EditComponentDialog"));
    this->resize(640, 480);
    mpVerticalLayout = new QVBoxLayout(this);
    mpVerticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    mpCodeTextEdit = new QTextEdit(this);
    mpCodeTextEdit->setObjectName(QString::fromUtf8("mpCodeTextEdit"));
    mpCodeTextEdit->setTabStopWidth(4);

    mpVerticalLayout->addWidget(mpCodeTextEdit);

    mpButtonBox = new QDialogButtonBox(this);
    mpButtonBox->setObjectName(QString::fromUtf8("mpButtonBox"));
    mpButtonBox->setOrientation(Qt::Horizontal);
    mpButtonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    mpButtonBox->setCenterButtons(false);

    mpVerticalLayout->addWidget(mpButtonBox);

    retranslateUi();
    QObject::connect(mpButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(mpButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QMetaObject::connectSlotsByName(this);

    mpCodeTextEdit->setPlainText(code);
    setHighlighter(language);

    if(code.isEmpty())
    {
        openCreateComponentWizard(language);
    }
}



void EditComponentDialog::retranslateUi()
{
    this->setWindowTitle(QApplication::translate("EditComponentDialog", "Dialog", 0, QApplication::UnicodeUTF8));
}


QString EditComponentDialog::getCode()
{
    return mpCodeTextEdit->toPlainText();
}


void EditComponentDialog::setHighlighter(SourceCodeEnumT language)
{
    if(language == Modelica)
    {
        ModelicaHighlighter *pEquationHighLighter = new ModelicaHighlighter(mpCodeTextEdit->document());
        Q_UNUSED(pEquationHighLighter);
    }
    else
    {
        CppHighlighter *pEquationHighLighter = new CppHighlighter(mpCodeTextEdit->document());
        Q_UNUSED(pEquationHighLighter);
    }
}




void EditComponentDialog::openCreateComponentWizard(SourceCodeEnumT language)
{
    CreateComponentWizard *pWizard = new CreateComponentWizard(language, this);
    pWizard->show();
}

CreateComponentWizard::CreateComponentWizard(EditComponentDialog::SourceCodeEnumT language, EditComponentDialog *parent)
    : QWizard(parent)
{
    mLanguage = language;

    mpParent = parent;

    mpFirstPage = new QWizardPage(this);
    mpFirstPage->setTitle("General settings");
    mpFirstPageLayout = new QGridLayout(this);
    mpFirstPage->setLayout(mpFirstPageLayout);
    mpNumberOfPortsLabel = new QLabel("Number of ports:");
    mpNumberOfPortsSpinBox = new QSpinBox(this);
    mpNumberOfPortsSpinBox->setValue(2);
    mpNumberOfParametersLabel = new QLabel("Number of parameters:");
    mpNumberOfParametersSpinBox = new QSpinBox(this);
    mpNumberOfParametersSpinBox->setValue(0);
    mpTypeNameLabel = new QLabel("Type name:", this);
    mpTypeNameLineEdit = new QLineEdit(this);
    mpDisplayNameLabel = new QLabel("Display name: ", this);
    mpDisplayNameLineEdit = new QLineEdit(this);
    mpCqsTypeLabel = new QLabel("CQS type: ", this);
    mpCqsTypeComboBox = new QComboBox(this);
    mpCqsTypeComboBox->addItems(QStringList() << "C" << "Q" << "S");
    mpFirstPageLayout->addWidget(mpTypeNameLabel,0,0);
    mpFirstPageLayout->addWidget(mpTypeNameLineEdit,0,1);
    mpFirstPageLayout->addWidget(mpDisplayNameLabel,1,0);
    mpFirstPageLayout->addWidget(mpDisplayNameLineEdit,1,1);
    mpFirstPageLayout->addWidget(mpCqsTypeLabel,2,0);
    mpFirstPageLayout->addWidget(mpCqsTypeComboBox,2,1);
    mpFirstPageLayout->addWidget(mpNumberOfPortsLabel,3,0);
    mpFirstPageLayout->addWidget(mpNumberOfPortsSpinBox,3,1);
    mpFirstPageLayout->addWidget(mpNumberOfParametersLabel,4,0);
    mpFirstPageLayout->addWidget(mpNumberOfParametersSpinBox,4,1);

    mpSecondPage = new QWizardPage(this);
    mpSecondPage->setTitle("Port settings");
    mpSecondPageLayout = new QGridLayout(this);
    mpSecondPage->setLayout(mpSecondPageLayout);
    mpPortIdTitle = new QLabel("Id");
    mpPortNameTitle = new QLabel("Name");
    mpPortTypeTitle = new QLabel("Port type");
    mpNodeTypeTitle = new QLabel("Node type");
    mpDefaultValueTitle = new QLabel("Default value");
    mpSecondPageLayout->addWidget(mpPortIdTitle,        0,0);
    mpSecondPageLayout->addWidget(mpPortNameTitle,      0,1);
    mpSecondPageLayout->addWidget(mpPortTypeTitle,      0,2);
    mpSecondPageLayout->addWidget(mpDefaultValueTitle,  0,3);

    mpThirdPage = new QWizardPage(this);
    mpThirdPage->setTitle("Parameter settings!");
    mpThirdPage->setFinalPage(true);
    mpThirdPageLayout = new QGridLayout(this);
    mpThirdPage->setLayout(mpThirdPageLayout);
    mpParameterNameTitle = new QLabel("Name");
    mpParameterUnitTitle = new QLabel("Unit");
    mpParameterDescriptionTitle = new QLabel("Description");
    mpParameterValueTitle = new QLabel("Default value");
    mpThirdPageLayout->addWidget(mpParameterNameTitle,        0,0);
    mpThirdPageLayout->addWidget(mpParameterUnitTitle,      0,1);
    mpThirdPageLayout->addWidget(mpParameterDescriptionTitle,      0,2);
    mpThirdPageLayout->addWidget(mpParameterValueTitle,      0,3);


    this->addPage(mpFirstPage);
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(updatePage(int)));
    connect(this, SIGNAL(accepted()), this, SLOT(generate()));
    this->addPage(mpSecondPage);
    this->addPage(mpThirdPage);
}


void CreateComponentWizard::updatePage(int i)
{
    if(i == 1)      //Ports page
    {
        while(mPortIdPtrs.size() > mpNumberOfPortsSpinBox->value())
        {
            mpSecondPageLayout->removeWidget(mPortIdPtrs.last());
            delete(mPortIdPtrs.last());
            mPortIdPtrs.removeLast();
            mpSecondPageLayout->removeWidget(mPortNameLineEditPtrs.last());
            delete(mPortNameLineEditPtrs.last());
            mPortNameLineEditPtrs.removeLast();
            mpSecondPageLayout->removeWidget(mPortTypeComboBoxPtrs.last());
            delete(mPortTypeComboBoxPtrs.last());
            mPortTypeComboBoxPtrs.removeLast();
            mpSecondPageLayout->removeWidget(mPortDefaultSpinBoxPtrs.last());
            delete(mPortDefaultSpinBoxPtrs.last());
            mPortDefaultSpinBoxPtrs.removeLast();
        }

        int portId = mPortIdPtrs.size()+1;
        while(mPortIdPtrs.size() < mpNumberOfPortsSpinBox->value())
        {
            mPortIdPtrs.append(new QLabel(QString::number(portId), this));
            mPortNameLineEditPtrs.append(new QLineEdit("P"+QString::number(portId), this));
            mPortTypeComboBoxPtrs.append(new QComboBox(this));
            mPortTypeComboBoxPtrs.last()->addItems(QStringList() << "NodeSignalIn" << "NodeSignalOut");
            QStringList nodeTypes;
            NodeInfo::getNodeTypes(nodeTypes);
            Q_FOREACH(const QString &type, nodeTypes)
            {
                QString name = NodeInfo(type).niceName;
                name.replace(0, 1, name[0].toUpper());
                mPortTypeComboBoxPtrs.last()->addItem(type);
            }
            mPortDefaultSpinBoxPtrs.append(new QDoubleSpinBox(this));
            mPortDefaultSpinBoxPtrs.last()->setValue(0);

            mpSecondPageLayout->addWidget(mPortIdPtrs.last(),               portId,0);
            mpSecondPageLayout->addWidget(mPortNameLineEditPtrs.last(),     portId,1);
            mpSecondPageLayout->addWidget(mPortTypeComboBoxPtrs.last(),     portId,2);
            mpSecondPageLayout->addWidget(mPortDefaultSpinBoxPtrs.last(),   portId,3);
            ++portId;
        }
    }
    else if(i == 2)     //Parameteres page
    {
        while(mParameterNameLineEditPtrs.size() > mpNumberOfParametersSpinBox->value())
        {
            mpThirdPageLayout->removeWidget(mParameterNameLineEditPtrs.last());
            delete(mParameterNameLineEditPtrs.last());
            mParameterNameLineEditPtrs.removeLast();
            mpThirdPageLayout->removeWidget(mParameterUnitLineEditPtrs.last());
            delete(mParameterUnitLineEditPtrs.last());
            mParameterUnitLineEditPtrs.removeLast();
            mpThirdPageLayout->removeWidget(mParameterDescriptionLineEditPtrs.last());
            delete(mParameterDescriptionLineEditPtrs.last());
            mParameterDescriptionLineEditPtrs.removeLast();
            mpThirdPageLayout->removeWidget(mParameterValueLineEditPtrs.last());
            delete(mParameterValueLineEditPtrs.last());
            mParameterValueLineEditPtrs.removeLast();
        }

        int parameterId = mParameterNameLineEditPtrs.size()+1;
        while(mParameterNameLineEditPtrs.size() < mpNumberOfParametersSpinBox->value())
        {
            mParameterNameLineEditPtrs.append(new QLineEdit(this));
            mParameterUnitLineEditPtrs.append(new QLineEdit(this));
            mParameterDescriptionLineEditPtrs.append(new QLineEdit(this));
            mParameterValueLineEditPtrs.append(new QLineEdit("0", this));

            mpThirdPageLayout->addWidget(mParameterNameLineEditPtrs.last(),         parameterId,0);
            mpThirdPageLayout->addWidget(mParameterUnitLineEditPtrs.last(),         parameterId,1);
            mpThirdPageLayout->addWidget(mParameterDescriptionLineEditPtrs.last(),  parameterId,2);
            mpThirdPageLayout->addWidget(mParameterValueLineEditPtrs.last(),        parameterId,3);
            ++parameterId;
        }
    }
}


void CreateComponentWizard::generate()
{
    //! @todo Add verifications!

    QString output;

    QString typeName = mpTypeNameLineEdit->text();
    QString displayName = mpDisplayNameLineEdit->text();
    QString cqsType = mpCqsTypeComboBox->currentText();
    QString cqsTypeLong = mpCqsTypeComboBox->currentText();
    if(cqsType == "S")
    {
        cqsTypeLong.append("ignal");
    }

    //Ports
    QMap<QString, QStringList> nodeToPortMap;
    for(int p=0; p<mPortIdPtrs.size(); ++p)
    {
        QString portType = mPortTypeComboBoxPtrs[p]->currentText();
        if(portType      == "NodeSignalIn")             { portType = "NodeSignalIn"; }
        else if(portType == "NodeSignalOut")            { portType = "NodeSignalOut"; }
        QStringList nodeTypes;
        NodeInfo::getNodeTypes(nodeTypes);
//        Q_FOREACH(const QString &type, nodeTypes)
//        {
//            if(portType.toLower() == NodeInfo(type).niceName)
//                portType = type;
//        }

        if(nodeToPortMap.contains(portType))
        {
            QStringList list = nodeToPortMap.find(portType).value();
            list.append(mPortNameLineEditPtrs[p]->text());
            nodeToPortMap.insert(portType, list);
        }
        else
        {
            nodeToPortMap.insert(portType, QStringList() << mPortNameLineEditPtrs[p]->text());
        }
    }


    if(mLanguage==EditComponentDialog::Modelica)
    {        //Initial declaration

        output.append("model "+typeName+" \""+displayName+"\"\n");
        output.append("  annotation(hopsanCqsType = \""+cqsType+"\");\n\n");

        QMap<QString, QStringList>::iterator pit;
        for(pit=nodeToPortMap.begin(); pit!=nodeToPortMap.end(); ++pit)
        {
            output.append("  "+pit.key()+" ");
            for(int p=0; p<pit.value().size(); ++p)
            {
                output.append(pit.value()[p]+", ");
            }
            output.chop(2);
            output.append(";\n");
        }
        output.append("\n");

        //Parameters
        for(int p=0; p<mParameterNameLineEditPtrs.size(); ++p)
        {
            QString name = mParameterNameLineEditPtrs[p]->text();
            QString unit = mParameterUnitLineEditPtrs[p]->text();
            QString desc = mParameterDescriptionLineEditPtrs[p]->text();
            QString value = mParameterValueLineEditPtrs[p]->text();

            output.append("  parameter Real "+name+"(unit=\""+unit+"\")="+value+" \""+desc+"\";\n");
        }
        if(mParameterNameLineEditPtrs.size() > 0)
        {
            output.append("\n");
        }

        //The rest
        output.append("algorithm\n  //Place initial algorithms here\n\n");
        output.append("equation\n  //Place equations here\n\n");
        output.append("algorithm\n  //Place final algorithms here\n\n");
        output.append("end "+mpTypeNameLineEdit->text()+";\n");
    }
    else
    {
        output.append("#include <math.h>\n");
        output.append("#include \"ComponentEssentials.h\"\n");
        output.append("#include \"ComponentUtilities.h\"\n\n");
        output.append("using namespace std;\n\n");
        output.append("namespace hopsan {\n\n");


        output.append("class "+typeName+" : public Component"+cqsTypeLong+"\n{\n");
        output.append("private:\n");

        //Port local variables
        output.append("    double ");
        //QMap<QString, QStringList>::iterator pit;
        //for(pit=nodeToPortMap.begin(); pit!=nodeToPortMap.end(); ++pit)
        //{
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            QString numStr;
            QString portName = mPortNameLineEditPtrs[p]->text();
            QString portType = mPortTypeComboBoxPtrs[p]->currentText();
            varNames << NodeInfo(portType).qVariables << NodeInfo(portType).cVariables;
            numStr = QString::number(p+1);
            if(portType == "NodeSignalIn" || portType == "NodeSignalOut")
            {
                varNames.clear();
                varNames << portName;
                numStr = "";
            }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append(varNames[v]+numStr+", ");
            }
        }
        output.chop(2);
        output.append(";\n");

        //Parameter variables
        if(!mParameterNameLineEditPtrs.isEmpty())
        {
            output.append("    double ");
            for(int p=0; p<mParameterNameLineEditPtrs.size(); ++p)
            {
                output.append(mParameterNameLineEditPtrs[p]->text()+", ");
            }
            output.chop(2);
            output.append(";\n");
        }

        //Node data pointers
        output.append("    double ");
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            QString portName = mPortNameLineEditPtrs[p]->text();
            QString portType = mPortTypeComboBoxPtrs[p]->currentText();
            varNames << NodeInfo(portType).qVariables << NodeInfo(portType).cVariables;
            QString numStr;

            if(portType == "NodeSignalIn" || portType == "NodeSignalOut")
            {
                varNames.clear();
                varNames << portName;
            }
            if(portType != "NodeSignalIn" && portType != "NodeSignalOut")
            {
                numStr = QString::number(p+1);
            }
            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("*mpND_"+varNames[v]+numStr+", ");
            }
        }
        output.chop(2);
        output.append(";\n");

        //Port pointers
        output.append("    Port ");
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            output.append("*mp"+mPortNameLineEditPtrs[p]->text()+", ");
        }
        output.chop(2);
        output.append(";\n\n");

        //Creator function
        output.append("public:\n");
        output.append("    static Component *Creator()\n");
        output.append("    {\n");
        output.append("        return new "+typeName+"();\n");
        output.append("    }\n\n");

        //Contructor
        output.append("    void configure()\n");
        output.append("    {\n");

        //Register parameters
        for(int p=0; p<mParameterNameLineEditPtrs.size(); ++p)
        {
            QString name = mParameterNameLineEditPtrs[p]->text();
            QString desc = mParameterDescriptionLineEditPtrs[p]->text();
            QString unit = mParameterUnitLineEditPtrs[p]->text();
            QString value = mParameterValueLineEditPtrs[p]->text();
            output.append("        addConstant(\""+name+"\", \""+desc+"\", \""+unit+"\", "+value+", "+name+");\n");
        }
        if(!mParameterNameLineEditPtrs.isEmpty()) { output.append("\n"); }

        QStringList portNames, portTypes, nodeTypes;

        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            portNames << mPortNameLineEditPtrs[p]->text();
            QString type = mPortTypeComboBoxPtrs[p]->currentText();

            if(type      == "NodeSignalIn")             { portTypes << "ReadPort"; }
            else if(type == "NodeSignalOut")            { portTypes << "WritePort"; }
            else                                        { portTypes << "PowerPort"; }

            nodeTypes << type;

            QString actualNodeType = nodeTypes[p];
            if(actualNodeType.startsWith("NodeSignal"))
                actualNodeType = "NodeSignal";

            output.append("        mp"+portNames[p]+" = add"+portTypes[p]+"(\""+portNames[p]+"\", \""+actualNodeType+"\");\n\n");


            QStringList varNames;
            if(nodeTypes[p] != "NodeSignal")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables << NodeInfo(nodeTypes[p]).cVariables;
            }
            else
            {
                if(portTypes[p] == "ReadPort") { varNames << portNames[p]; }
                if(portTypes[p] == "WritePort") { varNames << portNames[p]; }
            }

            QStringList varLabels = NodeInfo(nodeTypes[p]).variableLabels;
            QString numStr, defaultValue;
            if(portTypes[p] != "ReadPort" && portTypes[p] != "WritePort")
            {
                numStr = QString::number(p+1);
            }
            else
            {
                defaultValue = ", "+mPortDefaultSpinBoxPtrs[p]->text();
            }

            for(int v=0; v<varNames.size(); ++v)
            {
                if(!defaultValue.isEmpty())
                    output.append("        setDefaultStartValue(mp"+portNames[p]+", "+nodeTypes[p]+"::"+varLabels[v]+defaultValue+");\n");
            }
        }
        output.append("    }\n\n");

        //Initialize
        output.append("    void initialize()\n");
        output.append("    {\n");

        //Assign node data pointers
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(nodeTypes[p] != "NodeSignalIn" && nodeTypes[p] != "NodeSignalOut")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables << NodeInfo(nodeTypes[p]).cVariables;
            }
            else
            {
                if(portTypes[p] == "ReadPort") { varNames << portNames[p]; }
                if(portTypes[p] == "WritePort") { varNames << portNames[p]; }
            }

            QStringList varLabels = NodeInfo(nodeTypes[p]).variableLabels;
            if(nodeTypes[p].startsWith("NodeSignal"))
                varLabels = NodeInfo("NodeSignal").variableLabels;
            QString numStr, defaultValue;
            if(portTypes[p] != "ReadPort" && portTypes[p] != "WritePort")
            {
                numStr = QString::number(p+1);
            }
            else
            {
                defaultValue = ", "+mPortDefaultSpinBoxPtrs[p]->text();
            }

            for(int v=0; v<varNames.size(); ++v)
            {
                QString actualNodeType = nodeTypes[p];
                if(actualNodeType.startsWith("NodeSignal"))
                    actualNodeType = "NodeSignal";
                output.append("        mpND_"+varNames[v]+numStr+" = getSafeNodeDataPtr(mp"+portNames[p]+", "+actualNodeType+"::"+varLabels[v]+");\n");
            }
        }
        output.append("\n");

        //Create local port variables
        //Create local input port variables
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(cqsType == "Q")
            {
                varNames << NodeInfo(nodeTypes[p]).cVariables;
            }
            else if(cqsType == "C")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables;
            }
            if(portTypes[p] == "ReadPort") { varNames << portNames[p]; }
            QString numStr;
            if(portTypes[p] != "ReadPort") { numStr = QString::number(p+1); }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        "+varNames[v]+numStr+" = (*mpND_"+varNames[v]+numStr+");\n");
            }
        }

        output.append("\n");
        output.append("        //WRITE INITIALIZATION HERE\n");
        output.append("\n");

        //Create local output port variables
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(cqsType == "C")
            {
                varNames << NodeInfo(nodeTypes[p]).cVariables;
            }
            else if(cqsType == "Q")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables;
            }
            if(portTypes[p] == "WritePort") { varNames << portNames[p]; }
            QString numStr;
            if(portTypes[p] != "WritePort") { numStr = QString::number(p+1); }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        (*mpND_"+varNames[v]+numStr+") = "+varNames[v]+numStr+";\n");
            }
        }
        output.append("    }\n\n");


        //SimulateOneTimestep
        output.append("    void simulateOneTimestep()\n");
        output.append("    {\n");

        //Create local input port variables
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(cqsType == "Q")
            {
                varNames << NodeInfo(nodeTypes[p]).cVariables;
            }
            else if(cqsType == "C")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables;
            }
            if(portTypes[p] == "ReadPort") { varNames << portNames[p]; }
            QString numStr;
            if(portTypes[p] != "ReadPort") { numStr = QString::number(p+1); }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        "+varNames[v]+numStr+" = (*mpND_"+varNames[v]+numStr+");\n");
            }
        }
        output.append("\n");

        output.append("        //WRITE SIMULATION CODE HERE\n\n");

        //Create local output port variables
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(cqsType == "C")
            {
                varNames << NodeInfo(nodeTypes[p]).cVariables;
            }
            else if(cqsType == "Q")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables;
            }
            if(portTypes[p] == "WritePort") { varNames << portNames[p]; }
            QString numStr;
            if(portTypes[p] != "WritePort") { numStr = QString::number(p+1); }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        (*mpND_"+varNames[v]+numStr+") = "+varNames[v]+numStr+";\n");
            }
        }
        output.append("\n");

        output.append("    }\n\n");

        //Finalize
        output.append("    void finalize()\n");
        output.append("    {\n");
        output.append("        //WRITE FINALIZE CODE HERE\n");
        output.append("    }\n");

        output.append("};\n");
        output.append("}\n");
    }

    mpParent->mpCodeTextEdit->setPlainText(output);
    this->deleteLater();
}
