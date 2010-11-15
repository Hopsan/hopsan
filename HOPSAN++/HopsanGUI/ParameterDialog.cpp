//!
//! @file   ParameterDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#include <QtGui>
#include <cassert>
#include <iostream>

#include "ParameterDialog.h"
#include "MainWindow.h"
//#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "MessageWidget.h"
#include "GUIObjects/GUIComponent.h"


//! @class ParameterDialog
//! @brief The ParameterDialog class is a Widget used to interact with component parameters.
//!
//! It read and write parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ParameterDialog::ParameterDialog(GUIComponent *pGUIComponent, QWidget *parent)
    : QDialog(parent)
{
    mpGUIModelObject = pGUIComponent;
    isGUISubsystem = false;

    createEditStuff();
}


//! @brief Constructor for the parameter dialog for a subsystem
//! @param pGUISubsystem Pointer to the subsystem
//! @param parent Pointer to the parent widget
ParameterDialog::ParameterDialog(GUISystem *pGUISubsystem, QWidget *parent)     : QDialog(parent)
{
    mpGUIModelObject = pGUISubsystem;
    isGUISubsystem = true;

    createEditStuff();
}


//! @brief Creates the contents in the parameter dialog
void ParameterDialog::createEditStuff()
{
    mpNameEdit = new QLineEdit(mpGUIModelObject->getName());

    QFont fontH1;
    fontH1.setBold(true);

    QFont fontH2;
    fontH2.setBold(true);
    fontH2.setItalic(true);

    QLabel *pParameterLabel = new QLabel("Parameters");
    pParameterLabel->setFont(fontH1);

    //qDebug() << "before parnames";
    QVector<QString> parnames = mpGUIModelObject->getParameterNames();
    //qDebug() << "parnames.size: " << parnames.size();
    QVector<QString>::iterator pit;
    for ( pit=parnames.begin(); pit!=parnames.end(); ++pit )
    {
        mParameterVarVector.push_back(new QLabel(*pit));
        mParameterVarVector.back()->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        mParameterDescriptionVector.push_back(new QLabel(mpGUIModelObject->getParameterDescription(*pit).append(", ")));
        mParameterUnitVector.push_back(new QLabel(mpGUIModelObject->getParameterUnit(*pit)));

        mParameterValueVector.push_back(new QLineEdit());
        //mValueVector.back()->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mValueVector.back()));

        QString valueTxt;
        valueTxt.setNum(mpGUIModelObject->getParameterValue(*pit), 'g', 6 );
        mParameterValueVector.back()->setText(valueTxt);

        mParameterVarVector.back()->setBuddy(mParameterValueVector.back());

    }


    QGridLayout *startValueLayout = new QGridLayout;
    size_t sr=0;
    QLabel *pStartValueLabel = new QLabel("Start Values");
    pStartValueLabel->setFont(fontH1);

    QList<GUIPort*> ports = mpGUIModelObject->getPortListPtrs();
    QList<GUIPort*>::iterator portIt;
    double j=0;
    QVector<QVector<QString> > startDataNamesStr, startDataUnitsStr;
    QVector<QVector<double> > startDataValuesStr;
    startDataNamesStr.resize(ports.size());
    startDataValuesStr.resize(ports.size());
    startDataUnitsStr.resize(ports.size());
    mStartDataNames.resize(ports.size());
    mStartDataValues.resize(ports.size());
    mStartDataUnits.resize(ports.size());
    for ( portIt=ports.begin(); portIt!=ports.end(); ++portIt )
    {
        (*portIt)->getStartValueDataNamesValuesAndUnits(startDataNamesStr[j], startDataValuesStr[j], startDataUnitsStr[j]);
        if(!(startDataNamesStr[j].isEmpty()))
        {
//            (*portIt)->getStartValueDataNamesValuesAndUnits(startDataNamesStr[j], startDataValuesStr[j], startDataUnitsStr[j]);

            QString portName("Port, ");
            portName.append((*portIt)->getName());
            QLabel *portLabelName = new QLabel(portName);
            portLabelName->setFont(fontH2);
            startValueLayout->addWidget(portLabelName, sr, 0);
            ++sr;

            mStartDataNames[j].resize(startDataNamesStr[j].size());
            mStartDataValues[j].resize(startDataNamesStr[j].size());
            mStartDataUnits[j].resize(startDataNamesStr[j].size());
            for(size_t i=0; i < startDataNamesStr[j].size(); ++i)
            {
                QString tmpText;
                tmpText.append(startDataNamesStr[j][i]);
                mStartDataNames[j][i] = new QLabel(tmpText);
                mStartDataNames[j][i]->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                startValueLayout->addWidget(mStartDataNames[j][i], sr, 0);

                mStartDataValues[j][i] = new QLineEdit();
                QString valueTxt;
                valueTxt.setNum(startDataValuesStr[j][i], 'g', 6 );
                mStartDataValues[j][i]->setText(valueTxt);
                startValueLayout->addWidget(mStartDataValues[j][i], sr, 1);
                mStartDataNames[j][i]->setBuddy(mStartDataValues[j][i]);

                tmpText.clear();
                tmpText.append(startDataUnitsStr[j][i]);
                mStartDataUnits[j][i] = new QLabel(tmpText);
                startValueLayout->addWidget(mStartDataUnits[j][i], sr, 2);

                ++sr;
            }
            ++j;
        }
    }


    //qDebug() << "after parnames";

    okButton = new QPushButton(tr("&Ok"));
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setDefault(false);

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    connect(okButton, SIGNAL(pressed()), SLOT(okPressed()));
    connect(cancelButton, SIGNAL(pressed()), SLOT(close()));

    QHBoxLayout *pNameLayout = new QHBoxLayout;
    QLabel *pNameLabel = new QLabel("Name: ");
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);

    QHBoxLayout *pCQSLayout;
    if (isGUISubsystem)
    {
        pCQSLayout = new QHBoxLayout;
        //This is very hopsan specific (or actually TLM specific)
        mpCQSEdit = new QLineEdit(mpGUIModelObject->getTypeCQS());
        QLabel *pCQSLabel = new QLabel("CQS: ");

        pCQSLayout->addWidget(pCQSLabel);
        pCQSLayout->addWidget(mpCQSEdit);
    }

    QVBoxLayout *parameterDescriptionLayput = new QVBoxLayout;
    QVBoxLayout *parameterVarLayout = new QVBoxLayout;
    QVBoxLayout *parameterValueLayout = new QVBoxLayout;
    QVBoxLayout *parameterUnitLayout = new QVBoxLayout;
    for (size_t i=0 ; i <mParameterVarVector.size(); ++i )
    {
        parameterDescriptionLayput->addWidget(mParameterDescriptionVector[i]);
        parameterVarLayout->addWidget(mParameterVarVector[i]);
        parameterValueLayout->addWidget(mParameterValueVector[i]);
        parameterUnitLayout->addWidget(mParameterUnitVector[i]);
    }

    QHBoxLayout *parameterLayout = new QHBoxLayout;
    parameterLayout->addLayout(parameterDescriptionLayput);
    parameterLayout->addLayout(parameterVarLayout);
    parameterLayout->addLayout(parameterValueLayout);
    parameterLayout->addLayout(parameterUnitLayout);
    parameterLayout->addStretch(1);


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(buttonBox, lr, 1);
    ++lr;
    if (isGUISubsystem)
    {
        mainLayout->addLayout(pCQSLayout, lr, 0);
        ++lr;
    }
    if(!(mParameterVarVector.empty()))
    {
        mainLayout->addWidget(pParameterLabel, lr, 0);
        ++lr;
        mainLayout->addLayout(parameterLayout, lr, 0);
        ++lr;
    }
    if(!(mStartDataNames[0].isEmpty()))
    {
        mainLayout->addWidget(pStartValueLabel,lr, 0);
        ++lr;
        mainLayout->addLayout(startValueLayout, lr, 0);
    }
    setLayout(mainLayout);

    setWindowTitle(tr("Parameters"));
}


//! @brief Reads the values from the dialog and writes them into the core component
void ParameterDialog::okPressed()
{
    mpGUIModelObject->mpParentContainerObject->renameGUIModelObject(mpGUIModelObject->getName(), mpNameEdit->text());
    //qDebug() << mpNameEdit->text();

    setParameters();
    setStartValues();

    if (isGUISubsystem)
    {
        qDebug() << "Setting CQS type to: " << this->mpCQSEdit->displayText();
        mpGUIModelObject->setTypeCQS(this->mpCQSEdit->displayText());
    }
}


//! Sets the parameters in the core component. Read the values from the dialog and write them into the core component.
void ParameterDialog::setParameters()
{
    for (size_t i=0 ; i < mParameterValueVector.size(); ++i )
    {
        qDebug() << "Checking " << mParameterVarVector[i]->text();
        if(mParameterValueVector[i]->text().startsWith("<") && mParameterValueVector[i]->text().endsWith(">")) //! @todo Break out global parameter stuff to own method so it can be used, for example, in start value method too
        {
            QString requestedParameter = mParameterValueVector[i]->text().mid(1, mParameterValueVector[i]->text().size()-2);
            qDebug() << "Found global parameter \"" << requestedParameter << "\"";
        }
        bool ok;
        double newValue = mParameterValueVector[i]->text().toDouble(&ok);
        if (!ok)
        {

            MessageWidget *messageWidget = this->mpGUIModelObject->mpParentContainerObject->mpMainWindow->mpMessageWidget;//qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent()->parent())->mpMessageWidget;
            messageWidget->printGUIMessage(QString("ParameterDialog::setParameters(): You must give a correct value for '").append(mParameterVarVector[i]->text()).append(QString("', putz. Try again!")));
            qDebug() << "Inte okej!";
            return;
        }
        mpGUIModelObject->setParameterValue(mParameterVarVector[i]->text(), newValue);
    }


    std::cout << "Parameters updated." << std::endl;
    this->close();
}


void ParameterDialog::setStartValues()
{
    QList<GUIPort*> ports = mpGUIModelObject->getPortListPtrs();
    QList<GUIPort*>::iterator portIt;
    QVector<QString> startDataNamesStr, startDataUnitsStr;
    QVector<double> startDataValuesStr;
    size_t j=0;
    for ( portIt=ports.begin(); portIt!=ports.end(); ++portIt )
    {
        startDataNamesStr.resize(mStartDataNames[j].size());
        startDataValuesStr.resize(mStartDataNames[j].size());
        startDataUnitsStr.resize(mStartDataNames[j].size());
        for(size_t i=0; i < mStartDataNames[j].size(); ++i)
        {
            startDataNamesStr[i] = mStartDataNames[j][i]->text();
            startDataValuesStr[i] = mStartDataValues[j][i]->text().toDouble();
     //       qDebug() << "startDataNamesStr[i]: " << startDataNamesStr[i] << "startDataValuesStr[i]: " << startDataValuesStr[i];
        }

        (*portIt)->setStartValueDataByNames(startDataNamesStr, startDataValuesStr);
        ++j;
    }
    std::cout << "Start values updated." << std::endl;
    this->close();
}
