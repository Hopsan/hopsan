//!
//! @file   ComponentPropertiesDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id$

#include <QtGui>
#include <cassert>
#include <iostream>

#include "ComponentPropertiesDialog.h"
#include "MainWindow.h"
#include "GUIPort.h"
#include "MessageWidget.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "UndoStack.h"
#include "ProjectTabWidget.h"

#include <QToolButton>


//! @class ComponentPropertiesDialog
//! @brief The ComponentPropertiesDialog class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog::ComponentPropertiesDialog(GUIComponent *pGUIComponent, QWidget *parent)
    : QDialog(parent)
{
    mpGUIComponent = pGUIComponent;
    createEditStuff();
}


//! @brief Creates the contents in the parameter dialog
void ComponentPropertiesDialog::createEditStuff()
{
    mpNameEdit = new QLineEdit(mpGUIComponent->getName());

    QFont fontH1;
    fontH1.setBold(true);

    QFont fontH2;
    fontH2.setBold(true);
    fontH2.setItalic(true);

    QLabel *pParameterLabel = new QLabel("Parameters");
    pParameterLabel->setFont(fontH1);

    QVector<QString> parnames = mpGUIComponent->getParameterNames();
    QVector<QString>::iterator pit;
    for ( pit=parnames.begin(); pit!=parnames.end(); ++pit )
    {
        mParameterVarVector.push_back(new QLabel(*pit));
        mParameterVarVector.back()->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        mParameterDescriptionVector.push_back(new QLabel(mpGUIComponent->getParameterDescription(*pit).append(", ")));
        mParameterUnitVector.push_back(new QLabel(mpGUIComponent->getParameterUnit(*pit)));

        mParameterValueVector.push_back(new QLineEdit());


        QToolButton *pGlobalButton = new QToolButton();
        pGlobalButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
        connect(pGlobalButton, SIGNAL(pressed()), this, SLOT(showListOfSystemParameters()));

        mSystemParameterButtons.append(pGlobalButton);
        mSystemParameterVector.push_back(pGlobalButton);
        //mValueVector.back()->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mValueVector.back()));

        if(mpGUIComponent->isParameterMappedToSystemParameter(*pit))
        {
            if(mpGUIComponent->mpParentContainerObject->getCoreSystemAccessPtr()->hasSystemParameter(mpGUIComponent->getSystemParameterKey(*pit)))
            {
                mParameterValueVector.back()->setText(mpGUIComponent->getSystemParameterKey(*pit));
            }
            else
            {
                mpGUIComponent->forgetSystemParameterMapping(*pit);
                QString valueTxt;
                valueTxt.setNum(mpGUIComponent->getParameterValue(*pit), 'g', 6 );
                mParameterValueVector.back()->setText(valueTxt);
                gpMainWindow->mpMessageWidget->printGUIWarningMessage(tr("Warning: Global parameter no longer exists, replacing with last used value."));
            }
        }
        else
        {
            QString valueTxt;
            valueTxt.setNum(mpGUIComponent->getParameterValue(*pit), 'g', 6 );
            mParameterValueVector.back()->setText(valueTxt);
        }
        mParameterVarVector.back()->setBuddy(mParameterValueVector.back());

    }


    QGridLayout *startValueLayout = new QGridLayout;
    size_t sr=0;
    QLabel *pStartValueLabel = new QLabel("Start Values");
    pStartValueLabel->setFont(fontH1);

    QList<GUIPort*> ports = mpGUIComponent->getPortListPtrs();
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

    QVBoxLayout *parameterDescriptionLayput = new QVBoxLayout;
    QVBoxLayout *parameterVarLayout = new QVBoxLayout;
    QVBoxLayout *parameterValueLayout = new QVBoxLayout;
    mpParameterGlobalLayout = new QVBoxLayout;
    QVBoxLayout *parameterUnitLayout = new QVBoxLayout;
    for (size_t i=0 ; i <mParameterVarVector.size(); ++i )
    {
        parameterDescriptionLayput->addWidget(mParameterDescriptionVector[i]);
        parameterVarLayout->addWidget(mParameterVarVector[i]);
        parameterValueLayout->addWidget(mParameterValueVector[i]);
        mpParameterGlobalLayout->addWidget(mSystemParameterVector[i]);
        parameterUnitLayout->addWidget(mParameterUnitVector[i]);
    }

    QHBoxLayout *parameterLayout = new QHBoxLayout;
    parameterLayout->addLayout(parameterDescriptionLayput);
    parameterLayout->addLayout(parameterVarLayout);
    parameterLayout->addLayout(parameterValueLayout);
    parameterLayout->addLayout(mpParameterGlobalLayout);
    parameterLayout->addLayout(parameterUnitLayout);
    parameterLayout->addStretch(1);


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(buttonBox, lr, 1);
    ++lr;

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
void ComponentPropertiesDialog::okPressed()
{
    mpGUIComponent->mpParentContainerObject->renameGUIModelObject(mpGUIComponent->getName(), mpNameEdit->text());
    //qDebug() << mpNameEdit->text();

    setParameters();
    setStartValues();
}


//! Sets the parameters in the core component. Read the values from the dialog and write them into the core component.
void ComponentPropertiesDialog::setParameters()
{
    bool addedUndoPost = false;
    for (size_t i=0 ; i < mParameterValueVector.size(); ++i )
    {

        //! @test This is just a preliminary check for how global parameters can be implemented
        qDebug() << "Checking " << mParameterVarVector[i]->text();


        QString requestedParameter = mParameterValueVector[i]->text();
        bool ok;
        double newValue = requestedParameter.toDouble(&ok);

        if(!ok)     //Global parameter
        {
            if(mpGUIComponent->mpParentContainerObject->getCoreSystemAccessPtr()->hasSystemParameter(requestedParameter))
            {
                mpGUIComponent->mapParameterToSystemParameter(mParameterVarVector[i]->text(), requestedParameter);
            }
            else    //User has written something illegal
            {
                //! @todo Make something better, like showing a warning box, if parameter is not ok. Maybe check all parameters before setting any of them.
                MessageWidget *messageWidget = gpMainWindow->mpMessageWidget;//qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent()->parent())->mpMessageWidget;
                messageWidget->printGUIMessage(QString("ComponentPropertiesDialog::setParameters(): You must give a correct value for '").append(mParameterVarVector[i]->text()).append(QString("', putz. Try again!")));
                qDebug() << "Inte okej!";
                return;
            }
        }
        else
        {
            if(mpGUIComponent->getParameterValue(mParameterVarVector[i]->text()) != newValue)     //Normal parameter (a double value)
            {
                if(!addedUndoPost)
                {
                    this->mpGUIComponent->mpParentContainerObject->mUndoStack->newPost("changedparameters");
                    addedUndoPost = true;
                }
                this->mpGUIComponent->mpParentContainerObject->mUndoStack->registerChangedParameter(mpGUIComponent->getName(), mParameterVarVector[i]->text(), mpGUIComponent->getParameterValue(mParameterVarVector[i]->text()), newValue);
            }
            mpGUIComponent->setParameterValue(mParameterVarVector[i]->text(), newValue);
        }
    }

    std::cout << "Parameters updated." << std::endl;
    this->close();
}


void ComponentPropertiesDialog::setStartValues()
{
    QList<GUIPort*> ports = mpGUIComponent->getPortListPtrs();
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



void ComponentPropertiesDialog::showListOfSystemParameters()
{
    QMenu menu;

    QMap<std::string, double> globalMap = gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getSystemParametersMap();
    QMap<std::string, double>::iterator it;
    for(it=globalMap.begin(); it!=globalMap.end(); ++it)
    {
        QString valueString;
        valueString.setNum(it.value());
        QAction *tempAction = menu.addAction(QString(it.key().c_str()));
        tempAction->setIconVisibleInMenu(false);
    }

    size_t i;
    for(i=0; i<mSystemParameterButtons.size(); ++i)
    {
        if(mSystemParameterButtons.at(i)->underMouse())
        {
            qDebug() << "Clicked on number " << i;
            break;
        }
    }

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());

    if(selectedAction != 0)
    {
        this->mParameterValueVector[i]->setText(selectedAction->text());
    }
}
