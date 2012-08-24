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
//! @file   ComponentPropertiesDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id$

#include <QtGui>
#include <QDebug>

#include "ComponentPropertiesDialog.h"

#include "MainWindow.h"
#include "Configuration.h"

#include "UndoStack.h"
#include "GUIPort.h"

#include "Widgets/MessageWidget.h"

#include "Widgets/SystemParametersWidget.h"
#include "Widgets/LibraryWidget.h"

#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"

#include "Utilities/GUIUtilities.h"
#include "Dialogs/MovePortsDialog.h"
#include "Dialogs/ParameterSettingsLayout.h"
#include "Dialogs/ComponentGeneratorDialog.h"

#include "CoreAccess.h"

//! @class ComponentPropertiesDialog
//! @brief The ComponentPropertiesDialog class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog::ComponentPropertiesDialog(Component *pComponent, MainWindow *pParent)
    : ModelObjectPropertiesDialog(pComponent, pParent)
{
    mpComponent = pComponent;
    this->setPalette(gConfig.getPalette());
    if(mpComponent->getTypeName().startsWith("CppComponent"))
    {
        createCppEditStuff();
    }
    else if(mpComponent->getTypeName().startsWith("ModelicaComponent"))
    {
        createModelicaEditStuff();
    }
    else
    {
        createEditStuff();
    }
}


//! @brief Check if the parameter is a start value
//! @param [in,out] parameterDescription The description of the parameter/startvalue
//! @returns true if it is a startvalue, otherwise false
//!
//! This method is used to determine whether or not a parameter should be interpretted
//! as a start value by the GUI. In HOPSANcore there is no difference between parameters
//! and start values. The start values are registred and stored in the same container.
//! But, a start value is taged by "startvalue:" in the description.
bool ComponentPropertiesDialog::interpretedAsStartValue(QString &parameterDescription)
{    
    QString startValueString = "startvalue:";
    bool res=false;
    if(parameterDescription.contains(startValueString, Qt::CaseInsensitive))
    {
        parameterDescription.remove(startValueString, Qt::CaseInsensitive);
        res = true;
    }
    return res;
}


//! @brief Creates the contents in the parameter dialog
void ComponentPropertiesDialog::createEditStuff()
{
    mpNameEdit = new QLineEdit(mpComponent->getName(), this);

    QFont fontH1;
    fontH1.setBold(true);

//    QFont fontH2;
//    fontH2.setBold(true);
//    fontH2.setItalic(true);

    QLabel *pHelpPicture = new QLabel();
    QPixmap helpPixMap;
    helpPixMap.load(mpComponent->getAppearanceData()->getBasePath() + mpComponent->getHelpPicture());
    pHelpPicture->setPixmap(helpPixMap);

    QLabel *pHelpHeading = new QLabel(gpMainWindow->mpLibrary->getAppearanceData(mpComponent->getTypeName())->getDisplayName(), this);
    pHelpHeading->setAlignment(Qt::AlignCenter);
    QFont tempFont = pHelpHeading->font();
    tempFont.setPixelSize(16);
    tempFont.setBold(true);
    pHelpHeading->setFont(tempFont);
    QLabel *pHelpText = new QLabel(mpComponent->getHelpText(), this);
    pHelpText->setWordWrap(true);
    QLabel *pParameterLabel = new QLabel("Parameters", this);
    pParameterLabel->setFont(fontH1);
    QLabel *pStartValueLabel = new QLabel("Start Values", this);
    pStartValueLabel->setFont(fontH1);

    QGridLayout *parameterLayout = new QGridLayout();
    QGridLayout *startValueLayout = new QGridLayout();

    //QVector<QString> qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes;
    //mpGUIComponent->getParameters(qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes);

    QVector<CoreParameterData> paramDataVector;
    mpComponent->getParameters(paramDataVector);

    size_t nParam=0;
    size_t nStV=0;
    for(int i=0; i<paramDataVector.size(); ++i)
    {
        if(interpretedAsStartValue(paramDataVector[i].mDescription))
        {
            //QString unit = gConfig.getDefaultUnit(qParameterNames[i].section("::", 1, 1));
            paramDataVector[i].mUnit.prepend("[").append("]");
            mvStartValueLayout.push_back(new ParameterSettingsLayout(paramDataVector[i],
                                                                     mpComponent));
            startValueLayout->addLayout(mvStartValueLayout.back(), nParam, 0);
            ++nParam;
        }
        else
        {
            mvParameterLayout.push_back(new ParameterSettingsLayout(paramDataVector[i],
                                                                    mpComponent));
            parameterLayout->addLayout(mvParameterLayout.back(), nStV, 0);
            ++nStV;
        }
    }

    //Adjust sizes of labels, to make sure that all text is visible and that the spacing is not too big between them
    int descriptionSize=30;
    int nameSize = 10;
    //Paramters
    for(int i=0; i<mvParameterLayout.size(); ++i)
    {
        descriptionSize = std::max(descriptionSize, mvParameterLayout.at(i)->mDescriptionLabel.width());
        nameSize = std::max(nameSize, mvParameterLayout.at(i)->mNameLabel.width());
    }
    //Start values
    for(int i=0; i<mvStartValueLayout.size(); ++i)
    {
        descriptionSize = std::max(descriptionSize, mvStartValueLayout.at(i)->mDescriptionLabel.width());
        nameSize = std::max(nameSize, mvStartValueLayout.at(i)->mNameLabel.width());
    }
    //Paramters
    for(int i=0; i<mvParameterLayout.size(); ++i)
    {
        mvParameterLayout.at(i)->mDescriptionLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
        mvParameterLayout.at(i)->mNameLabel.setFixedWidth(nameSize+10);
    }
    //Start values
    for(int i=0; i<mvStartValueLayout.size(); ++i)
    {
        mvStartValueLayout.at(i)->mDescriptionLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
        mvStartValueLayout.at(i)->mNameLabel.setFixedWidth(nameSize+10);
    }

    //qDebug() << "after parnames";
    mpEditPortPos = new QPushButton(tr("&Move ports"), this);
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpOkButton = new QPushButton(tr("&Ok"), this);
    mpOkButton->setDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpEditPortPos, QDialogButtonBox::ActionRole);

    connect(mpOkButton, SIGNAL(clicked()), SLOT(okPressed()));
    connect(mpCancelButton, SIGNAL(clicked()), SLOT(close()));
    connect(mpEditPortPos, SIGNAL(clicked()), SLOT(editPortPos()));

    QGroupBox *pHelpGroupBox = new QGroupBox();
    QVBoxLayout *pHelpLayout = new QVBoxLayout();
    pHelpPicture->setAlignment(Qt::AlignCenter);
    pHelpLayout->addWidget(pHelpHeading);
    if(!mpComponent->getHelpPicture().isNull())
        pHelpLayout->addWidget(pHelpPicture);
    if(!mpComponent->getHelpText().isNull())
        pHelpLayout->addWidget(pHelpText);
    pHelpGroupBox->setStyleSheet(QString::fromUtf8("QGroupBox {background-color: white; border: 2px solid gray; border-radius: 5px; margin-top: 1ex;}"));
    pHelpGroupBox->setLayout(pHelpLayout);

    QGridLayout *pNameLayout = new QGridLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    QLabel *pTypeNameLabel = new QLabel("Type Name: \"" + mpComponent->getTypeName() + "\"", this);
    pNameLayout->addWidget(pNameLabel,0,0);
    pNameLayout->addWidget(mpNameEdit,0,1);
    pNameLayout->addWidget(pTypeNameLabel,1,0,1,2);
    if (!mpComponent->getSubTypeName().isEmpty())
    {
        QLabel *pSubTypeNameLabel = new QLabel("SubType Name: \"" + mpComponent->getSubTypeName() + "\"", this);
        pNameLayout->addWidget(pSubTypeNameLabel,2,0,1,2);
    }

    QGridLayout *mainLayout = new QGridLayout();
    //mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    if(!mpComponent->getHelpText().isNull() || !mpComponent->getHelpPicture().isNull())
    {
        mainLayout->addWidget(pHelpGroupBox, lr, 0, 1, 2);
    }

    ++lr;

    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(mpButtonBox, lr, 1);

    ++lr;

    if(!(mvParameterLayout.empty()))
    {
        mainLayout->addWidget(pParameterLabel, lr, 0, 1, 2);
        ++lr;
        mainLayout->addLayout(parameterLayout, lr, 0, 1, 2);
        ++lr;
    }
    else
    {
        pParameterLabel->hide();
    }
    if(!(mvStartValueLayout.isEmpty()))
    {
        mainLayout->addWidget(pStartValueLabel,lr, 0, 1, 2);
        ++lr;
        mainLayout->addLayout(startValueLayout, lr, 0, 1, 2);
    }
    else
    {
        pStartValueLabel->hide();
    }

    QWidget *pPrimaryWidget = new QWidget(this);
    pPrimaryWidget->setLayout(mainLayout);
    pPrimaryWidget->setPalette(gConfig.getPalette());

    QScrollArea *pScrollArea = new QScrollArea(this);
    pScrollArea->setWidget(pPrimaryWidget);
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QGridLayout *pPrimaryLayout = new QGridLayout(this);

    QString filePath = mpComponent->getAppearanceData()->getSourceCodeFile();
    if(!filePath.isEmpty())
    {
        filePath.prepend(mpComponent->getAppearanceData()->getBasePath());
        QFile file(filePath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString code;
        QTextStream t(&file);
        code = t.readAll();
        file.close();


        QTextEdit *pSourceCodeTextEdit = new QTextEdit(this);
        pSourceCodeTextEdit->setReadOnly(true);
        pSourceCodeTextEdit->setText(code);
        if(filePath.endsWith(".hpp"))
        {
            CppHighlighter *pHighLighter = new CppHighlighter(pSourceCodeTextEdit->document());
        }
        else if(filePath.endsWith(".mo"))
        {
            ModelicaHighlighter *pHighLighter = new ModelicaHighlighter(pSourceCodeTextEdit->document());
        }

        QVBoxLayout *pSourceCodeLayout = new QVBoxLayout(this);
        pSourceCodeLayout->addWidget(pSourceCodeTextEdit);

        QWidget *pSourceCodeWidget = new QWidget(this);
        pSourceCodeWidget->setLayout(pSourceCodeLayout);

        QTabWidget *pTabWidget = new QTabWidget(this);
        pTabWidget->addTab(pScrollArea, "Parameters");
        pTabWidget->addTab(pSourceCodeWidget, "Source Code");
        pPrimaryLayout->addWidget(pTabWidget);
    }
    else
    {
        pPrimaryLayout->addWidget(pScrollArea);
    }

    setLayout(pPrimaryLayout);

    pPrimaryWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pPrimaryLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
    pScrollArea->setFixedHeight(std::min(pPrimaryWidget->height()+3, maxHeight));
    if(pScrollArea->minimumHeight() == maxHeight)
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+19);
    }
    else
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+3);
    }
    pScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWindowTitle(tr("Component Properties"));
}


void ComponentPropertiesDialog::createCppEditStuff()
{
    mpNameEdit = new QLineEdit(mpComponent->getName(), this);

    QFont fontH1;
    fontH1.setBold(true);

    mpEditPortPos = new QPushButton(tr("&Move ports"), this);
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpOkButton = new QPushButton(tr("&Ok"), this);
    mpOkButton->setDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpEditPortPos, QDialogButtonBox::ActionRole);

    connect(mpOkButton, SIGNAL(clicked()), SLOT(okPressed()));
    connect(mpCancelButton, SIGNAL(clicked()), SLOT(close()));
    connect(mpEditPortPos, SIGNAL(clicked()), SLOT(editPortPos()));

    QLabel *pInputPortsLabel = new QLabel("Input ports:", this);
    QLabel *pOutputPortsLabel = new QLabel("Output ports:", this);

    mpInputPortsSpinBox = new QSpinBox(this);
    mpInputPortsSpinBox->setValue(mpComponent->getCppInputs());
    mpInputPortsSpinBox->setSingleStep(1);

    mpOutputPortsSpinBox = new QSpinBox(this);
    mpOutputPortsSpinBox->setValue(mpComponent->getCppOutputs());
    mpOutputPortsSpinBox->setSingleStep(1);

    QHBoxLayout *pPortsLayout = new QHBoxLayout();
    pPortsLayout->addWidget(pInputPortsLabel);
    pPortsLayout->addWidget(mpInputPortsSpinBox);
    pPortsLayout->addWidget(pOutputPortsLabel);
    pPortsLayout->addWidget(mpOutputPortsSpinBox);

    mpTextEdit = new QTextEdit(this);
    mpTextEdit->setPlainText(mpComponent->getCppCode());
    CppHighlighter *pTextEditHighlighter = new CppHighlighter(mpTextEdit->document());

    QGridLayout *pNameLayout = new QGridLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    QLabel *pTypeNameLabel = new QLabel("Type Name: \"" + mpComponent->getTypeName() + "\"", this);
    pNameLayout->addWidget(pNameLabel,0,0);
    pNameLayout->addWidget(mpNameEdit,0,1);
    pNameLayout->addWidget(pTypeNameLabel,1,0,1,2);
    if (!mpComponent->getSubTypeName().isEmpty())
    {
        QLabel *pSubTypeNameLabel = new QLabel("SubType Name: \"" + mpComponent->getSubTypeName() + "\"", this);
        pNameLayout->addWidget(pSubTypeNameLabel,2,0,1,2);
    }

    QGridLayout *mainLayout = new QGridLayout();
    //mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row

    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(mpButtonBox, lr, 1);

    ++lr;

    mainLayout->addLayout(pPortsLayout, lr, 0, 1, 2);

    ++lr;

    mainLayout->addWidget(mpTextEdit, lr, 0, 1, 2);

    QWidget *pPrimaryWidget = new QWidget(this);
    pPrimaryWidget->setLayout(mainLayout);
    pPrimaryWidget->setPalette(gConfig.getPalette());

    QScrollArea *pScrollArea = new QScrollArea(this);
    pScrollArea->setWidget(pPrimaryWidget);
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QGridLayout *pPrimaryLayout = new QGridLayout(this);

    pPrimaryLayout->addWidget(pScrollArea);

    setLayout(pPrimaryLayout);

    pPrimaryWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pPrimaryLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
    pScrollArea->setFixedHeight(std::min(pPrimaryWidget->height()+3, maxHeight));
    if(pScrollArea->minimumHeight() == maxHeight)
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+19);
    }
    else
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+3);
    }
    pScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWindowTitle(tr("C++ Component Properties"));
}


void ComponentPropertiesDialog::createModelicaEditStuff()
{
    mpNameEdit = new QLineEdit(mpComponent->getName(), this);

    QFont fontH1;
    fontH1.setBold(true);

    mpEditPortPos = new QPushButton(tr("&Move ports"), this);
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpOkButton = new QPushButton(tr("&Ok"), this);
    mpOkButton->setDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpEditPortPos, QDialogButtonBox::ActionRole);

    connect(mpOkButton, SIGNAL(clicked()), SLOT(okPressed()));
    connect(mpCancelButton, SIGNAL(clicked()), SLOT(close()));
    connect(mpEditPortPos, SIGNAL(clicked()), SLOT(editPortPos()));

    mpTextEdit = new QTextEdit(this);
    mpTextEdit->setPlainText(mpComponent->getModelicaCode());
    mpTextEdit->setMinimumWidth(640);
    ModelicaHighlighter *pTextEditHighlighter = new ModelicaHighlighter(mpTextEdit->document());

    QGridLayout *pNameLayout = new QGridLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    QLabel *pTypeNameLabel = new QLabel("Type Name: \"" + mpComponent->getTypeName() + "\"", this);
    pNameLayout->addWidget(pNameLabel,0,0);
    pNameLayout->addWidget(mpNameEdit,0,1);
    pNameLayout->addWidget(pTypeNameLabel,1,0,1,2);
    if (!mpComponent->getSubTypeName().isEmpty())
    {
        QLabel *pSubTypeNameLabel = new QLabel("SubType Name: \"" + mpComponent->getSubTypeName() + "\"", this);
        pNameLayout->addWidget(pSubTypeNameLabel,2,0,1,2);
    }

    QGridLayout *mainLayout = new QGridLayout();
    //mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row

    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(mpButtonBox, lr, 1);

    ++lr;

    mainLayout->addWidget(mpTextEdit, lr, 0, 1, 2);

    QWidget *pPrimaryWidget = new QWidget(this);
    pPrimaryWidget->setLayout(mainLayout);
    pPrimaryWidget->setPalette(gConfig.getPalette());

    QScrollArea *pScrollArea = new QScrollArea(this);
    pScrollArea->setWidget(pPrimaryWidget);
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QGridLayout *pPrimaryLayout = new QGridLayout(this);

    pPrimaryLayout->addWidget(pScrollArea);

    setLayout(pPrimaryLayout);

    pPrimaryWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pPrimaryLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
    pScrollArea->setFixedHeight(std::min(pPrimaryWidget->height()+3, maxHeight));
    if(pScrollArea->minimumHeight() == maxHeight)
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+19);
    }
    else
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+3);
    }
    //pScrollArea->setMinimumWidth(640);
    pScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWindowTitle(tr("Modelica Component Properties"));
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog::okPressed()
{
    mpComponent->getParentContainerObject()->renameModelObject(mpComponent->getName(), mpNameEdit->text());
    //qDebug() << mpNameEdit->text();

    if(mpComponent->getTypeName().startsWith("CppComponent"))
    {
        recompileCppFromDialog();
    }
    else
    {
        setParametersAndStartValues();
    }
}


void ComponentPropertiesDialog::editPortPos()
{
    //! @todo who owns the dialog, is it ever removed?
    MovePortsDialog *dialog = new MovePortsDialog(mpComponent->getAppearanceData(), mpComponent->getParentContainerObject()->getGfxType());
    connect(dialog, SIGNAL(finished()), mpComponent, SLOT(refreshExternalPortsAppearanceAndPosition()), Qt::UniqueConnection);
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
//! @see setParametersAndStartValues(QVector<ParameterLayout *> vParLayout)
void ComponentPropertiesDialog::setParametersAndStartValues()
{
    if(setParameterValues(mvParameterLayout) && setParameterValues(mvStartValueLayout))
    {
        qDebug() << "Parameters and start values updated.";
        this->close();
    }
}



void ComponentPropertiesDialog::recompileCppFromDialog()
{
    //Generate type name

    QDateTime time = QDateTime();
    uint t = time.currentDateTime().toTime_t();     //Number of milliseconds since 1970

    double rd = rand() / (double)RAND_MAX;
    int r = int(rd*1000000.0);                      //Random number between 0 and 1000000

    QString typeName = "CppComponent_"+QString::number(t)+QString::number(r);

    qDebug() << typeName;


    //Generate code
    int nInputs = mpInputPortsSpinBox->value();
    int nOutputs = mpOutputPortsSpinBox->value();

    if(nOutputs == 0)
    {
        this->close();
        return;
    }

    QString plainCode = mpTextEdit->toPlainText();
    QString codeFromDialog=plainCode;
    plainCode.prepend("\n");
    plainCode.replace("\n", "\n            ");      //Add extra line spacing to code

    QString code;
    QTextStream codeStream(&code);

   // codeStream << "#ifndef "+typeName.toUpper()+"_HPP_INCLUDED\n";
   // codeStream << "#define "+typeName.toUpper()+"_HPP_INCLUDED\n\n";
    codeStream << "#include \"ComponentEssentials.h\"\n\n";
    codeStream << "namespace hopsan {\n\n";
    codeStream << "    class "+typeName+" : public ComponentSignal\n";
    codeStream << "    {\n\n";
    codeStream << "    private:\n";
    codeStream << "        Port ";
    for(int i=0; i<nInputs; ++i)
    {
        codeStream << "*mpIn"+QString::number(i);
        if(i != nInputs-1 || nOutputs>0) { codeStream << ", "; }
    }
    for(int o=0; o<nOutputs; ++o)
    {
        codeStream << "*mpOut"+QString::number(o);
        if(o != nOutputs-1) { codeStream << ", "; }
    }
    codeStream << ";\n";
    codeStream << "        double ";
    for(int i=0; i<nInputs; ++i)
    {
        codeStream << "*mpND_in"+QString::number(i);
        if(i != nInputs-1 || nOutputs>0) { codeStream << ", "; }
    }
    for(int o=0; o<nOutputs; ++o)
    {
        codeStream << "*mpND_out"+QString::number(o);
        if(o != nOutputs-1) { codeStream << ", "; }
    }
    codeStream << ";\n\n";
    codeStream << "    public:\n";
    codeStream << "        static Component *Creator()\n";
    codeStream << "        {\n";
    codeStream << "            return new "+typeName+"();\n";
    codeStream << "        }\n\n";
    codeStream << "        void configure()\n";
    codeStream << "        {\n\n";
    for(int i=0; i<nInputs; ++i)
    {
        codeStream << "            mpIn"+QString::number(i)+" = addReadPort(\"in"+QString::number(i)+"\", \"NodeSignal\", Port::NOTREQUIRED);\n";
    }
    for(int o=0; o<nOutputs; ++o)
    {
        codeStream << "            mpOut"+QString::number(o)+" = addWritePort(\"out"+QString::number(o)+"\", \"NodeSignal\", Port::NOTREQUIRED);\n";
    }
    codeStream << "        }\n\n";
    codeStream << "        void initialize()\n";
    codeStream << "        {\n";
    for(int i=0; i<nInputs; ++i)
    {
        codeStream << "            mpND_in"+QString::number(i)+" = getSafeNodeDataPtr(mpIn"+QString::number(i)+", NodeSignal::VALUE, 0);\n";
    }
    for(int o=0; o<nOutputs; ++o)
    {
        codeStream << "            mpND_out"+QString::number(o)+" = getSafeNodeDataPtr(mpOut"+QString::number(o)+", NodeSignal::VALUE);\n";
    }
    codeStream << "        }\n\n";
    codeStream << "        void simulateOneTimestep()\n";
    codeStream << "        {\n";
    for(int i=0; i<nInputs; ++i)
    {
        codeStream << "            double in"+QString::number(i)+" = *mpND_in"+QString::number(i)+";\n";
    }
    for(int o=0; o<nOutputs; ++o)
    {
        codeStream << "            double out"+QString::number(o)+" = *mpND_out"+QString::number(o)+";\n";
    }
    codeStream << "\n"+plainCode+"\n\n";
    for(int o=0; o<nOutputs; ++o)
    {
        codeStream << "            *mpND_out"+QString::number(o)+" = out"+QString::number(o)+";\n";
    }
    codeStream << "        }\n";
    codeStream << "    };\n";
    codeStream << "}\n\n";
    //codeStream << "#endif // "+typeName.toUpper()+"_HPP_INCLUDED\n";

    qDebug() << "Code: " << code;


    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateFromCpp(code, false);
    delete(pCoreAccess);
    QDir libDir = QDir();
    libDir.mkpath(gExecPath+"../componentLibraries/cppLibrary");

    QFile libFile;

#ifdef WIN32
    libFile.setFileName(gExecPath+"output/"+typeName+".dll");
    libFile.copy(gExecPath+"../componentLibraries/cppLibrary/"+typeName+".dll");
#else
    libFile.setFileName(gExecPath+"output/"+typeName+".so");
    libFile.copy(gExecPath+"../componentLibraries/cppLibrary/"+typeName+".so");
#endif

    QFile xmlFile;
    xmlFile.setFileName(gExecPath+"output/"+typeName+".xml");
    xmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString xmlCode = xmlFile.readAll();
    xmlFile.close();
    xmlCode.replace("    <icons/>", "    <icons>\n      <icon scale=\"1\" path=\":graphics/objecticons/cppcomponent.svg\" iconrotation=\"OFF\" type=\"user\"/>\n    </icons>");
    xmlFile.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlFile.write((const char *)xmlCode.toAscii().data());
    xmlFile.close();
    xmlFile.copy(gExecPath+"../componentLibraries/cppLibrary/"+typeName+".xml");

    gpMainWindow->mpLibrary->unloadExternalLibrary(gExecPath+"../componentLibraries/cppLibrary");
    gpMainWindow->mpLibrary->loadHiddenSecretDir(gExecPath+"../componentLibraries/cppLibrary");

    ContainerObject *pContainer = mpComponent->getParentContainerObject();
    QString name = mpComponent->getName();
    pContainer->replaceComponent(name, typeName);

    mpComponent = static_cast<Component*>(pContainer->getModelObject(name));
    mpComponent->setCppCode(codeFromDialog);
    mpComponent->setCppInputs(nInputs);
    mpComponent->setCppOutputs(nOutputs);

    this->close();
}
