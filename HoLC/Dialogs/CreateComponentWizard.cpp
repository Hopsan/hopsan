#include <QGroupBox>

#include "CreateComponentWizard.h"
#include "Handlers/FileHandler.h"
#include "Handlers/MessageHandler.h"
#include "Utilities/StringUtilities.h"
#include <QScrollArea>

NodeInfo::NodeInfo(QString nodeType)
{
    //! @todo This should not be hard-coded!
    if(nodeType == "NodeSignal")
    {
        niceName = "signal";
        shortNames << "y";
        variableLabels << "Value";
    }
    else if(nodeType == "NodeMechanic")
    {
        niceName = "mechanic";
        qVariables << "x" << "v" << "F" << "me";
        cVariables << "c" << "Zc";
        intensity = "F";
        flow = "v";
        shortNames << qVariables << cVariables;
        variableLabels << "Position" << "Velocity" << "Force" << "EquivalentMass" << "WaveVariable" << "CharImpedance";
    }
    else if(nodeType == "NodeMechanicRotational")
    {
        niceName = "mechanicrotational";
        qVariables << "a" << "w" << "T" << "Je";
        cVariables << "c" << "Zc";
        intensity = "T";
        flow = "w";
        shortNames << qVariables << cVariables;
        variableLabels << "Angle" << "AngularVelocity" << "Torque" << "EquivalentInertia" << "WaveVariable" << "CharImpedance";
    }
    else if(nodeType == "NodeHydraulic")
    {
        niceName = "hydraulic";
        qVariables << "q" << "p";
        cVariables << "c" << "Zc";
        intensity = "p";
        flow = "q";
        shortNames << qVariables << cVariables;
        variableLabels << "Flow" << "Pressure" << "WaveVariable" << "CharImpedance";
    }
}

void NodeInfo::getNodeTypes(QStringList &nodeTypes)
{
    //! @todo Support pneumatic and electic as well
    nodeTypes << "NodeMechanic" << "NodeMechanicRotational" << "NodeHydraulic";// << "NodePneumatic" << "NodeElectric";
}

CreateComponentWizard::CreateComponentWizard(FileHandler *pFileHandler, MessageHandler *pMessageHandler, QWidget *parent)
    : QWizard(parent)
{
    resize(800,600);

    //mLanguage = language;

    mpParent = parent;
    mpFileHandler = pFileHandler;
    mpMessageHandler = pMessageHandler;
    this->setStyleSheet(parent->styleSheet());
    this->setPalette(parent->palette());

    QWizardPage *pFirstPage = new QWizardPage(this);
    pFirstPage->setTitle("General settings");
    QGridLayout *pFirstPageLayout = new QGridLayout(this);
    pFirstPage->setLayout(pFirstPageLayout);
    QLabel *pNumberOfConstantsLabel = new QLabel("Number of constants:");
    mpNumberOfConstantsSpinBox = new QSpinBox(this);
    mpNumberOfConstantsSpinBox->setValue(0);
    QLabel *pNumberOfInputsLabel = new QLabel("Number of input variables:");
    mpNumberOfInputsSpinBox = new QSpinBox(this);
    mpNumberOfInputsSpinBox->setValue(0);
    QLabel*pNumberOfOutputsLabel = new QLabel("Number of output variables:");
    mpNumberOfOutputsSpinBox = new QSpinBox(this);
    mpNumberOfOutputsSpinBox->setValue(0);
    QLabel *pNumberOfPortsLabel = new QLabel("Number of power ports:");
    mpNumberOfPortsSpinBox = new QSpinBox(this);
    mpNumberOfPortsSpinBox->setValue(0);
    QLabel *pTypeNameLabel = new QLabel("Type name:", this);
    mpTypeNameLineEdit = new QLineEdit(this);
    QLabel *pDisplayNameLabel = new QLabel("Display name: ", this);
    mpDisplayNameLineEdit = new QLineEdit(this);
    QLabel *pCqsTypeLabel = new QLabel("CQS type: ", this);
    mpCqsTypeComboBox = new QComboBox(this);
    mpCqsTypeComboBox->addItems(QStringList() << "C" << "Q" << "S");

    int row=-1;
    pFirstPageLayout->addWidget(pTypeNameLabel,             ++row,  0);
    pFirstPageLayout->addWidget(mpTypeNameLineEdit,         row,    1);
    pFirstPageLayout->addWidget(pDisplayNameLabel,          ++row,  0);
    pFirstPageLayout->addWidget(mpDisplayNameLineEdit,      row,    1);
    pFirstPageLayout->addWidget(pCqsTypeLabel,              ++row,  0);
    pFirstPageLayout->addWidget(mpCqsTypeComboBox,          row,    1);
    pFirstPageLayout->addWidget(pNumberOfConstantsLabel,    ++row,  0);
    pFirstPageLayout->addWidget(mpNumberOfConstantsSpinBox, row,    1);
    pFirstPageLayout->addWidget(pNumberOfInputsLabel,    ++row,  0);
    pFirstPageLayout->addWidget(mpNumberOfInputsSpinBox, row,    1);
    pFirstPageLayout->addWidget(pNumberOfOutputsLabel,    ++row,  0);
    pFirstPageLayout->addWidget(mpNumberOfOutputsSpinBox, row,    1);
    pFirstPageLayout->addWidget(pNumberOfPortsLabel,        ++row,  0);
    pFirstPageLayout->addWidget(mpNumberOfPortsSpinBox,     row,    1);


    QWizardPage *pSecondPage = new QWizardPage(this);
    pSecondPage->setTitle("Port settings");


    QWidget *pScrollWidget = new QWidget(this);
    QVBoxLayout *pScrollLayout = new QVBoxLayout(pScrollWidget);
    pScrollWidget->setLayout(pScrollLayout);
    pScrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QScrollArea *pScrollArea = new QScrollArea(this);
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    pScrollArea->setWidget(pScrollWidget);
    pScrollArea->setWidgetResizable(true);


    QVBoxLayout *pSecondPageLayout = new QVBoxLayout(this);
    pSecondPageLayout->addWidget(pScrollArea);
    pSecondPage->setLayout(pSecondPageLayout);

    QGroupBox *pConstantsBox = new QGroupBox("Constants", this);
    QGroupBox *pInputsBox = new QGroupBox("Input Variables", this);
    QGroupBox *pOutputsBox = new QGroupBox("Output Variables", this);
    QGroupBox *pPortsBox = new QGroupBox("Power Ports", this);
    pScrollLayout->addWidget(pConstantsBox);
    pScrollLayout->addWidget(pInputsBox);
    pScrollLayout->addWidget(pOutputsBox);
    pScrollLayout->addWidget(pPortsBox);
    mpConstantsLayout = new QGridLayout(pConstantsBox);
    mpInputsLayout = new QGridLayout(pInputsBox);
    mpOutputsLayout = new QGridLayout(pOutputsBox);
    mpPortsLayout = new QGridLayout(pPortsBox);

    //Constants on page 2
    QLabel *pConstantsNameTitle = new QLabel("Name");
    QLabel *pConstantsUnitTitle = new QLabel("Unit");
    QLabel *pConstantsDescriptionTitle = new QLabel("Description");
    QLabel *pConstantsValueTitle = new QLabel("Default value");
    mpConstantsLayout->addWidget(pConstantsNameTitle,        0,0);
    mpConstantsLayout->addWidget(pConstantsUnitTitle,      0,1);
    mpConstantsLayout->addWidget(pConstantsDescriptionTitle,      0,2);
    mpConstantsLayout->addWidget(pConstantsValueTitle,      0,3);

    //Inputs on page 2
    QLabel *pInputsNameTitle = new QLabel("Name");
    QLabel *pInputsUnitTitle = new QLabel("Unit");
    QLabel *pInputsDescriptionTitle = new QLabel("Description");
    QLabel *pInputsValueTitle = new QLabel("Default value");
    mpInputsLayout->addWidget(pInputsNameTitle,        0,0);
    mpInputsLayout->addWidget(pInputsUnitTitle,      0,1);
    mpInputsLayout->addWidget(pInputsDescriptionTitle,      0,2);
    mpInputsLayout->addWidget(pInputsValueTitle,      0,3);

    //Outputs on page 2
    QLabel *pOutputsNameTitle = new QLabel("Name");
    QLabel *pOutputsUnitTitle = new QLabel("Unit");
    QLabel *pOutputsDescriptionTitle = new QLabel("Description");
    QLabel *pOutputsValueTitle = new QLabel("Default value");
    mpOutputsLayout->addWidget(pOutputsNameTitle,        0,0);
    mpOutputsLayout->addWidget(pOutputsUnitTitle,      0,1);
    mpOutputsLayout->addWidget(pOutputsDescriptionTitle,      0,2);
    mpOutputsLayout->addWidget(pOutputsValueTitle,      0,3);

    //Power ports on page 2
    QLabel *pPortIdTitle = new QLabel("Id");
    QLabel *pPortNameTitle = new QLabel("Name");
    QLabel *pPortDescriptionTitle = new QLabel("Description");
    QLabel *pPortTypeTitle = new QLabel("Port type");
    QLabel *pDefaultValueTitle = new QLabel("Default value");
    mpPortsLayout->addWidget(pPortIdTitle,          0,0);
    mpPortsLayout->addWidget(pPortNameTitle,        0,1);
    mpPortsLayout->addWidget(pPortDescriptionTitle, 0,2);
    mpPortsLayout->addWidget(pPortTypeTitle,        0,3);
    mpPortsLayout->addWidget(pDefaultValueTitle,    0,4);

    this->addPage(pFirstPage);
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(updatePage(int)));
    connect(this, SIGNAL(accepted()), this, SLOT(generate()));
    this->addPage(pSecondPage);
}


void CreateComponentWizard::updatePage(int i)
{
    //! @todo Check that type name and displayname are not empty
    //! @todo Why do we have a display name?!

    if(i == 1)      //Second page
    {
        //Constants
        while(mConstantsNameLineEditPtrs.size() > mpNumberOfConstantsSpinBox->value())
        {
            mpConstantsLayout->removeWidget(mConstantsNameLineEditPtrs.last());
            delete(mConstantsNameLineEditPtrs.last());
            mConstantsNameLineEditPtrs.removeLast();
            mpConstantsLayout->removeWidget(mConstantsUnitLineEditPtrs.last());
            delete(mConstantsUnitLineEditPtrs.last());
            mConstantsUnitLineEditPtrs.removeLast();
            mpConstantsLayout->removeWidget(mConstantsDescriptionLineEditPtrs.last());
            delete(mConstantsDescriptionLineEditPtrs.last());
            mConstantsDescriptionLineEditPtrs.removeLast();
            mpConstantsLayout->removeWidget(mConstantsValueLineEditPtrs.last());
            delete(mConstantsValueLineEditPtrs.last());
            mConstantsValueLineEditPtrs.removeLast();
        }

        int constantId = mConstantsNameLineEditPtrs.size()+1;
        while(mConstantsNameLineEditPtrs.size() < mpNumberOfConstantsSpinBox->value())
        {
            mConstantsNameLineEditPtrs.append(new QLineEdit(this));
            mConstantsUnitLineEditPtrs.append(new QLineEdit(this));
            mConstantsDescriptionLineEditPtrs.append(new QLineEdit(this));
            mConstantsValueLineEditPtrs.append(new QLineEdit("0", this));

            mpConstantsLayout->addWidget(mConstantsNameLineEditPtrs.last(),         constantId,0);
            mpConstantsLayout->addWidget(mConstantsUnitLineEditPtrs.last(),         constantId,1);
            mpConstantsLayout->addWidget(mConstantsDescriptionLineEditPtrs.last(),  constantId,2);
            mpConstantsLayout->addWidget(mConstantsValueLineEditPtrs.last(),        constantId,3);
            ++constantId;
        }

        //Input variables
        while(mInputsNameLineEditPtrs.size() > mpNumberOfInputsSpinBox->value())
        {
            mpInputsLayout->removeWidget(mInputsNameLineEditPtrs.last());
            delete(mInputsNameLineEditPtrs.last());
            mInputsNameLineEditPtrs.removeLast();
            mpInputsLayout->removeWidget(mInputsUnitLineEditPtrs.last());
            delete(mInputsUnitLineEditPtrs.last());
            mInputsUnitLineEditPtrs.removeLast();
            mpInputsLayout->removeWidget(mInputsDescriptionLineEditPtrs.last());
            delete(mInputsDescriptionLineEditPtrs.last());
            mInputsDescriptionLineEditPtrs.removeLast();
            mpInputsLayout->removeWidget(mInputsValueLineEditPtrs.last());
            delete(mInputsValueLineEditPtrs.last());
            mInputsValueLineEditPtrs.removeLast();
        }

        int inputId = mInputsNameLineEditPtrs.size()+1;
        while(mInputsNameLineEditPtrs.size() < mpNumberOfInputsSpinBox->value())
        {
            mInputsNameLineEditPtrs.append(new QLineEdit(this));
            mInputsUnitLineEditPtrs.append(new QLineEdit(this));
            mInputsDescriptionLineEditPtrs.append(new QLineEdit(this));
            mInputsValueLineEditPtrs.append(new QLineEdit("0", this));

            mpInputsLayout->addWidget(mInputsNameLineEditPtrs.last(),         inputId,0);
            mpInputsLayout->addWidget(mInputsUnitLineEditPtrs.last(),         inputId,1);
            mpInputsLayout->addWidget(mInputsDescriptionLineEditPtrs.last(),  inputId,2);
            mpInputsLayout->addWidget(mInputsValueLineEditPtrs.last(),        inputId,3);
            ++inputId;
        }

        //Output variables
        while(mOutputsNameLineEditPtrs.size() > mpNumberOfOutputsSpinBox->value())
        {
            mpOutputsLayout->removeWidget(mOutputsNameLineEditPtrs.last());
            delete(mOutputsNameLineEditPtrs.last());
            mOutputsNameLineEditPtrs.removeLast();
            mpOutputsLayout->removeWidget(mOutputsUnitLineEditPtrs.last());
            delete(mOutputsUnitLineEditPtrs.last());
            mOutputsUnitLineEditPtrs.removeLast();
            mpOutputsLayout->removeWidget(mOutputsDescriptionLineEditPtrs.last());
            delete(mOutputsDescriptionLineEditPtrs.last());
            mOutputsDescriptionLineEditPtrs.removeLast();
            mpOutputsLayout->removeWidget(mOutputsValueLineEditPtrs.last());
            delete(mOutputsValueLineEditPtrs.last());
            mOutputsValueLineEditPtrs.removeLast();
        }

        int outputId = mOutputsNameLineEditPtrs.size()+1;
        while(mOutputsNameLineEditPtrs.size() < mpNumberOfOutputsSpinBox->value())
        {
            mOutputsNameLineEditPtrs.append(new QLineEdit(this));
            mOutputsUnitLineEditPtrs.append(new QLineEdit(this));
            mOutputsDescriptionLineEditPtrs.append(new QLineEdit(this));
            mOutputsValueLineEditPtrs.append(new QLineEdit("0", this));

            mpOutputsLayout->addWidget(mOutputsNameLineEditPtrs.last(),         outputId,0);
            mpOutputsLayout->addWidget(mOutputsUnitLineEditPtrs.last(),         outputId,1);
            mpOutputsLayout->addWidget(mOutputsDescriptionLineEditPtrs.last(),  outputId,2);
            mpOutputsLayout->addWidget(mOutputsValueLineEditPtrs.last(),        outputId,3);
            ++outputId;
        }

        //Power ports
        while(mPortIdPtrs.size() > mpNumberOfPortsSpinBox->value())
        {
            mpPortsLayout->removeWidget(mPortIdPtrs.last());
            delete(mPortIdPtrs.last());
            mPortIdPtrs.removeLast();
            mpPortsLayout->removeWidget(mPortNameLineEditPtrs.last());
            delete(mPortNameLineEditPtrs.last());
            mPortNameLineEditPtrs.removeLast();
            mpPortsLayout->removeWidget(mPortDescriptionLineEditPtrs.last());
            delete(mPortDescriptionLineEditPtrs.last());
            mPortDescriptionLineEditPtrs.removeLast();
            mpPortsLayout->removeWidget(mPortTypeComboBoxPtrs.last());
            delete(mPortTypeComboBoxPtrs.last());
            mPortTypeComboBoxPtrs.removeLast();
            mpPortsLayout->removeWidget(mPortDefaultSpinBoxPtrs.last());
            delete(mPortDefaultSpinBoxPtrs.last());
            mPortDefaultSpinBoxPtrs.removeLast();
        }

        int portId = mPortIdPtrs.size()+1;
        while(mPortIdPtrs.size() < mpNumberOfPortsSpinBox->value())
        {
            mPortIdPtrs.append(new QLabel(QString::number(portId), this));
            mPortNameLineEditPtrs.append(new QLineEdit("P"+QString::number(portId), this));
            mPortDescriptionLineEditPtrs.append(new QLineEdit(this));
            mPortTypeComboBoxPtrs.append(new QComboBox(this));
            QStringList nodeTypes;
            NodeInfo::getNodeTypes(nodeTypes);
            //nodeTypes << "NodeMechanic" << "NodeMechanicRotational" << "NodeHydraulic" << "NodePneumatic" << "NodeElectric";
            Q_FOREACH(const QString &type, nodeTypes)
            {
                QString name = NodeInfo(type).niceName;
                name.replace(0, 1, name[0].toUpper());
                mPortTypeComboBoxPtrs.last()->addItem(type);
            }
            mPortDefaultSpinBoxPtrs.append(new QDoubleSpinBox(this));
            mPortDefaultSpinBoxPtrs.last()->setValue(0);

            mpPortsLayout->addWidget(mPortIdPtrs.last(),                    portId,0);
            mpPortsLayout->addWidget(mPortNameLineEditPtrs.last(),          portId,1);
            mpPortsLayout->addWidget(mPortDescriptionLineEditPtrs.last(),   portId,2);
            mpPortsLayout->addWidget(mPortTypeComboBoxPtrs.last(),          portId,3);
            mpPortsLayout->addWidget(mPortDefaultSpinBoxPtrs.last(),        portId,4);
            ++portId;
        }
        //! @todo I think that it would be better if each portedid line (actaully same for constants in/out variables) would be a widget by itself then all of this code would be easier to manage
    }
}


void CreateComponentWizard::generate()
{
    //! @todo Add verifications!
    //! @todo Make sure file does not already exist! (both in file system and in project)

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
        QStringList nodeTypes;
        NodeInfo::getNodeTypes(nodeTypes);
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



    //Begin generating
    //! @todo Should maybe be in file handler instead

    //Read template file
    QFile compTemplateFile(":Templates/componentTemplate.hpp");
    if(!compTemplateFile.open(QFile::ReadOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Unable to open file for reading: "+compTemplateFile.fileName());
        return;
    }
    QString code = compTemplateFile.readAll();
    compTemplateFile.close();


    QString headerGuard = typeName.toUpper()+"_HPP_INCLUDED";
    QString memberVariableCode;
    QString localVariableCode;
    QString addConstantsCode;
    QString addInputsCode;
    QString addOutputsCode;
    QString addPortsCode;
    QString setDefaultStartValuesCode;
    QString getDataPtrsCode;
    QString readFromNodesCode;
    QString writeToNodesCode;

    //Parameter variables
    if(!mConstantsNameLineEditPtrs.isEmpty())
    {
        memberVariableCode.append(spaces(8)+"//Constants\n");
        memberVariableCode.append(spaces(8)+"double ");
        for(int p=0; p<mConstantsNameLineEditPtrs.size(); ++p)
        {
            //memberVariableCode.append(mConstantsNameLineEditPtrs[p]->text()+", ");
            memberVariableCode.append(QString("m%1, ").arg(firstCapital(mConstantsNameLineEditPtrs[p]->text())));
        }
        memberVariableCode.chop(2);
        memberVariableCode.append(";\n");
    }

    //Input variables
    if(!mInputsNameLineEditPtrs.isEmpty())
    {
        memberVariableCode.append(spaces(8)+"//Input variable node data pointers\n");
        memberVariableCode.append(spaces(8)+"double ");
        for(int p=0; p<mInputsNameLineEditPtrs.size(); ++p)
        {
            //memberVariableCode.append("*mp"+firstCapital(mInputsNameLineEditPtrs[p]->text())+", ");
            memberVariableCode.append(QString("*mp%1, ").arg(firstCapital(mInputsNameLineEditPtrs[p]->text())));
        }
        memberVariableCode.chop(2);
        memberVariableCode.append(";\n");
    }

    //Output variables
    if(!mOutputsNameLineEditPtrs.isEmpty())
    {
        memberVariableCode.append(spaces(8)+"//Output variable node data pointers\n");
        memberVariableCode.append(spaces(8)+"double ");
        for(int p=0; p<mOutputsNameLineEditPtrs.size(); ++p)
        {
            //memberVariableCode.append("*mp"+firstCapital(mOutputsNameLineEditPtrs[p]->text())+", ");
            memberVariableCode.append(QString("*mp%1, ").arg(firstCapital(mOutputsNameLineEditPtrs[p]->text())));
        }
        memberVariableCode.chop(2);
        memberVariableCode.append(";\n");
    }

    //Port pointers
    if(!mPortNameLineEditPtrs.isEmpty())
    {
        memberVariableCode.append(spaces(8)+"//Power port pointers\n");
        memberVariableCode.append(spaces(8)+"Port ");
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            //memberVariableCode.append("*mp"+firstCapital(mPortNameLineEditPtrs[p]->text())+", ");
            memberVariableCode.append(QString("*mp%1, ").arg(firstCapital(mPortNameLineEditPtrs[p]->text())));
        }
        memberVariableCode.chop(2);
        memberVariableCode.append(";\n");
    }

    //Node data pointers
    if(!mPortNameLineEditPtrs.isEmpty())
    {
        memberVariableCode.append(spaces(8)+"//Power port node data pointers\n");
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QString portName = mPortNameLineEditPtrs[p]->text();
            QStringList varNames;
            QString portType = mPortTypeComboBoxPtrs[p]->currentText();
            varNames << NodeInfo(portType).qVariables << NodeInfo(portType).cVariables;
//            QString numStr;
//            numStr = QString::number(p+1);

            memberVariableCode.append(spaces(8)+"double ");
            for(int v=0; v<varNames.size(); ++v)
            {
                //memberVariableCode.append("*mp"+portName+"_"+varNames[v]+numStr+", ");
                memberVariableCode.append(QString("*mp%1_%2, ").arg(firstCapital(portName),varNames[v]));
            }
            memberVariableCode.chop(2);
            memberVariableCode.append(";\n");
        }
    }



    //Add constants
    for(int p=0; p<mConstantsNameLineEditPtrs.size(); ++p)
    {
        QString name = mConstantsNameLineEditPtrs[p]->text();
        QString desc = mConstantsDescriptionLineEditPtrs[p]->text();
        QString unit = mConstantsUnitLineEditPtrs[p]->text();
        QString value = mConstantsValueLineEditPtrs[p]->text();
        //addConstantsCode.append(QString("\n            addConstant(\""+name+"\", \""+desc+"\", \""+unit+"\", "+value+", "+name+");");
        addConstantsCode.append(spaces(12)+QString("addConstant(\"%1\", \"%2\", \"%3\", %4, m%5);\n").arg(name, desc, unit, value, firstCapital((name))));
    }
//    if(!addConstantsCode.isEmpty())
//    {
//        addConstantsCode.prepend("//Register constant parameters\n");
//    }

    //Add input variables
    for(int p=0; p<mInputsNameLineEditPtrs.size(); ++p)
    {
        QString name = mInputsNameLineEditPtrs[p]->text();
        QString desc = mInputsDescriptionLineEditPtrs[p]->text();
        QString unit = mInputsUnitLineEditPtrs[p]->text();
        QString value = mInputsValueLineEditPtrs[p]->text();
        //addInputsCode.append("\n            addInputVariable(\""+name+"\", \""+desc+"\", \""+unit+"\", "+value+", &mp"+name+");");
        addInputsCode.append(spaces(12)+QString("addInputVariable(\"%1\", \"%2\", \"%3\", %4, &mp%5);\n").arg(name, desc, unit, value, firstCapital(name)));
    }
//    if(!addInputsCode.isEmpty())
//    {
//        addInputsCode.prepend("//Register input variables\n");
//    }

    //Add output variables
    for(int p=0; p<mOutputsNameLineEditPtrs.size(); ++p)
    {
        QString name = mOutputsNameLineEditPtrs[p]->text();
        QString desc = mOutputsDescriptionLineEditPtrs[p]->text();
        QString unit = mOutputsUnitLineEditPtrs[p]->text();
        QString value = mOutputsValueLineEditPtrs[p]->text();
        //addOutputsCode.append("\n            addOutputVariable(\""+name+"\", \""+desc+"\", \""+unit+"\", "+value+", &mp"+name+");");
        addOutputsCode.append(spaces(12)+QString("addOutputVariable(\"%1\", \"%2\", \"%3\", %4, &mp%5);\n").arg(name, desc, unit, value, firstCapital(name)));
    }
//    if(!addOutputsCode.isEmpty())
//    {
//        addOutputsCode.prepend("//Register output variables\n");
//    }

    //Add power ports
    QStringList portNames, portDescriptions, nodeTypes;
    for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
    {
        portNames << mPortNameLineEditPtrs[p]->text();
        portDescriptions << mPortDescriptionLineEditPtrs[p]->text();
        QString type = mPortTypeComboBoxPtrs[p]->currentText();

        nodeTypes << type;

        QString actualNodeType = nodeTypes[p];
        if(actualNodeType.startsWith("NodeSignal"))
            actualNodeType = "NodeSignal";

        //addPortsCode.append("\n            mp"+portNames[p]+" = addPowerPort(\""+portNames[p]+"\", \""+actualNodeType+"\");");
        addPortsCode.append(spaces(12)+QString("mp%1 = addPowerPort(\"%2\", \"%3\", \"%4\");\n").arg(firstCapital(portNames[p]), portNames[p], actualNodeType, portDescriptions[p]));

        QStringList varNames;
        varNames << NodeInfo(nodeTypes[p]).qVariables << NodeInfo(nodeTypes[p]).cVariables;

        QStringList varLabels = NodeInfo(nodeTypes[p]).variableLabels;
        QString defaultValue;
//        QString numStr;
//        numStr = QString::number(p+1);

        for(int v=0; v<varNames.size(); ++v)
        {
            if(!defaultValue.isEmpty())
                //setDefaultStartValuesCode.append("\n            setDefaultStartValue(mp"+portNames[p]+", "+nodeTypes[p]+"::"+varLabels[v]+defaultValue+");");
                setDefaultStartValuesCode.append(spaces(12)+QString("setDefaultStartValue(mp%1, %2::%3, %4);\n").arg(firstCapital(portNames[p]), nodeTypes[p], varLabels[v], defaultValue));
        }
    }
//    if(!addPortsCode.isEmpty())
//    {
//        addPortsCode.prepend("//Add power ports\n");
//    }
//    if(!setDefaultStartValuesCode.isEmpty())
//    {
//        setDefaultStartValuesCode.prepend("//Set default start values\n");
//    }


    //Set node data pointers
    for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
    {
        QString portName = mPortNameLineEditPtrs[p]->text();
        QStringList varNames;
        varNames << NodeInfo(nodeTypes[p]).qVariables << NodeInfo(nodeTypes[p]).cVariables;

        QStringList varLabels = NodeInfo(nodeTypes[p]).variableLabels;
//        QString numStr, defaultValue;
//        numStr = QString::number(p+1);

        for(int v=0; v<varNames.size(); ++v)
        {
            //getDataPtrsCode.append("\n            mp"+portName+"_"+varNames[v]+numStr+" = getSafeNodeDataPtr(mp"+portNames[p]+", "+nodeTypes[p]+"::"+varLabels[v]+");");
            getDataPtrsCode.append(spaces(12)+QString("mp%1_%2 = getSafeNodeDataPtr(mp%1, %3::%4);\n").arg(firstCapital(portName), varNames[v], nodeTypes[p], varLabels[v]));
        }
    }
    if (!getDataPtrsCode.isEmpty())
    {
        getDataPtrsCode.prepend(spaces(12)+"//Get node data pointers from ports\n");
    }

    // Generate local variable declaration
    // Inputs
    if(!mInputsNameLineEditPtrs.isEmpty())
    {
        localVariableCode.append(spaces(12)+"double ");
        for(int p=0; p<mInputsNameLineEditPtrs.size(); ++p)
        {
            localVariableCode.append(mInputsNameLineEditPtrs[p]->text()+", ");
        }
        localVariableCode.chop(2);
        localVariableCode.append(";\n");
    }
    // Outputs
    if(!mOutputsNameLineEditPtrs.isEmpty())
    {
        localVariableCode.append(spaces(12)+"double ");
        for(int p=0; p<mOutputsNameLineEditPtrs.size(); ++p)
        {
            localVariableCode.append(mOutputsNameLineEditPtrs[p]->text()+", ");
        }
        localVariableCode.chop(2);
        localVariableCode.append(";\n");
    }
    // PowerPort local variables
    if(!mPortNameLineEditPtrs.isEmpty())
    {
        localVariableCode.append(spaces(12)+"double ");
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            QString portName = mPortNameLineEditPtrs[p]->text();
            QString portType = mPortTypeComboBoxPtrs[p]->currentText();
            varNames << NodeInfo(portType).qVariables << NodeInfo(portType).cVariables;
            for(int v=0; v<varNames.size(); ++v)
            {
                //localVariableCode.append(varNames[v]+QString::number(p+1)+", ");
                localVariableCode.append(QString("%1_%2, ").arg(portName,varNames[v]));
            }
        }
        localVariableCode.chop(2);
        localVariableCode.append(";\n");
    }
    if (!localVariableCode.isEmpty())
    {
        localVariableCode.prepend(spaces(12)+"//Declare local variables\n");
    }


    //Read from nodes
    for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
    {
        QString portName = mPortNameLineEditPtrs[p]->text();
        QStringList varNames;
        if(cqsType == "Q")
        {
            varNames << NodeInfo(nodeTypes[p]).cVariables;
        }
        else if(cqsType == "C")
        {
            varNames << NodeInfo(nodeTypes[p]).qVariables;
        }
//        QString numStr = QString::number(p+1);

        for(int v=0; v<varNames.size(); ++v)
        {
            //readFromNodesCode.append("\n            "+varNames[v]+numStr+" = (*mp"+portName+"_"+varNames[v]+numStr+");");
            readFromNodesCode.append(spaces(12)+QString("%1_%2 = (*mp%3_%2);\n").arg(portName, varNames[v], firstCapital(portName)));
        }
    }
    for(int i=0; i<mInputsNameLineEditPtrs.size(); ++i)
    {
        QString varName = mInputsNameLineEditPtrs[i]->text();
        //readFromNodesCode.append("\n            "+varName+" = (*mp"+varName+");");
        readFromNodesCode.append(spaces(12)+QString("%1 = (*mp%2);\n").arg(varName, firstCapital(varName)));
    }

    //Write to nodes
    for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
    {
        QString portName = mPortNameLineEditPtrs[p]->text();
        QStringList varNames;
        if(cqsType == "C")
        {
            varNames << NodeInfo(nodeTypes[p]).cVariables;
        }
        else if(cqsType == "Q")
        {
            varNames << NodeInfo(nodeTypes[p]).qVariables;
        }
//        QString numStr = QString::number(p+1);

        for(int v=0; v<varNames.size(); ++v)
        {
            //writeToNodesCode.append("\n            (*mp"+portName+"_"+varNames[v]+numStr+") = "+varNames[v]+numStr+";");
            writeToNodesCode.append(spaces(12)+QString("(*mp%1_%2) = %3_%2;\n").arg(firstCapital(portName),varNames[v], portName));
        }
    }
    for(int i=0; i<mOutputsNameLineEditPtrs.size(); ++i)
    {
        QString varName = mOutputsNameLineEditPtrs[i]->text();
        //writeToNodesCode.append("\n            (*mp"+varName+") = "+varName+";");
        writeToNodesCode.append(spaces(12)+QString("(*mp%1) = %2;\n").arg(firstCapital(varName),varName));
    }

    code.replace("<<<headerguard>>>", headerGuard);
    code.replace("<<<typename>>>", typeName);
    code.replace("<<<cqstype>>>", cqsTypeLong);
    replacePatternLine(code, "<<<membervariables>>>", memberVariableCode);
    replacePatternLine(code, "<<<localvariables>>>", localVariableCode);
    replacePatternLine(code, "<<<addconstants>>>", addConstantsCode);
    replacePatternLine(code, "<<<addinputs>>>", addInputsCode);
    replacePatternLine(code, "<<<addoutputs>>>", addOutputsCode);
    replacePatternLine(code, "<<<addports>>>", addPortsCode);
    replacePatternLine(code, "<<<setdefaultstartvalues>>>", setDefaultStartValuesCode);
    replacePatternLine(code, "<<<getdataptrs>>>", getDataPtrsCode);
    replacePatternLine(code, "<<<readfromnodes>>>", readFromNodesCode);
    replacePatternLine(code, "<<<writetonodes>>>", writeToNodesCode);

    mpFileHandler->addComponent(code, typeName);

    //Read template file
    QFile cafTemplateFile(":Templates/cafTemplate.xml");
    if(!cafTemplateFile.open(QFile::ReadOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Unable to open file for reading: "+cafTemplateFile.fileName());
        return;
    }
    QString cafCode = cafTemplateFile.readAll();
    cafTemplateFile.close();

    QString portsCode;

    //Add input variables
    int nInputs = mInputsNameLineEditPtrs.size();
    double dist = 1.0/(nInputs+1.0);
    double pos = 0.0;
    for(int p=0; p<nInputs; ++p)
    {
        pos += dist;
        QString name = mInputsNameLineEditPtrs[p]->text();
        QString x = "0.0";
        QString y = QString::number(pos);
        //portsCode.append("\n            <port x=\""+x+"\" y=\""+y+"\" a=\"180\" name=\""+name+"\" hidden=\"false\"/>");
        portsCode.append(spaces(12)+QString("<port name=\"%1\" x=\"%2\" y=\"%3\" a=\"%4\" visible=\"true\"/>\n").arg(name, x, y).arg(180));
    }

    //Add output variables
    int nOutputs = mOutputsNameLineEditPtrs.size();
    dist = 1.0/(nOutputs+1.0);
    pos = 0.0;
    for(int p=0; p<nOutputs; ++p)
    {
        pos += dist;
        QString name = mOutputsNameLineEditPtrs[p]->text();
        QString x = "1.0";
        QString y = QString::number(pos);
        //portsCode.append("\n            <port x=\""+x+"\" y=\""+y+"\" a=\"0\" name=\""+name+"\" hidden=\"false\"/>");
        portsCode.append(spaces(12)+QString("<port name=\"%1\" x=\"%2\" y=\"%3\" a=\"%4\" visible=\"true\"/>\n").arg(name, x, y).arg(0));
    }


    //Add power ports
    int nPowerPorts = mPortNameLineEditPtrs.size();
    dist = 1.0/(nPowerPorts+1.0);
    pos = 0.0;
    for(int p=0; p<nPowerPorts; ++p)
    {
        pos += dist;
        QString name = mPortNameLineEditPtrs[p]->text();
        QString x = QString::number(pos);
        QString y = "0.0";
        //portsCode.append("\n            <port x=\""+x+"\" y=\""+y+"\" a=\"270\" name=\""+name+"\"/>");
        portsCode.append(spaces(12)+QString("<port name=\"%1\" x=\"%2\" y=\"%3\" a=\"%4\"/>\n").arg(name, x, y).arg(270));
    }

    cafCode.replace("<<<hppfile>>>", typeName+".hpp");
    cafCode.replace("<<<typename>>>", typeName);
    cafCode.replace("<<<displayname>>>", displayName);
    replacePatternLine(cafCode, "<<<ports>>>", portsCode);

    mpFileHandler->addAppearanceFile(cafCode, typeName+".xml");
}

void CreateComponentWizard::open()
{
    this->restart();
    QWizard::open();
}

