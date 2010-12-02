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
#include "../MainWindow.h"
#include "../GUIPort.h"
#include "../Widgets/MessageWidget.h"
#include "../GUIObjects/GUIComponent.h"
#include "../GUIObjects/GUIContainerObject.h"
#include "../GUIObjects/GUISystem.h"
#include "../UndoStack.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../Widgets/SystemParametersWidget.h"


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
    mpNameEdit = new QLineEdit(mpGUIComponent->getName(), this);

    QFont fontH1;
    fontH1.setBold(true);

    QFont fontH2;
    fontH2.setBold(true);
    fontH2.setItalic(true);

    QLabel *pParameterLabel = new QLabel("Parameters", this);
    pParameterLabel->setFont(fontH1);

    QGridLayout *parameterLayout = new QGridLayout();

    QVector<QString> parnames = mpGUIComponent->getParameterNames();
    QVector<QString>::iterator pit;
    size_t n = 0;
    for ( pit=parnames.begin(); pit!=parnames.end(); ++pit )
    {
        QString valueTxt = mpGUIComponent->getParameterValueTxt(*pit);
        bool ok;
        valueTxt.toDouble(&ok);
        if((!ok) && !(mpGUIComponent->mpParentContainerObject->getCoreSystemAccessPtr()->hasSystemParameter(valueTxt)))
        {
            gpMainWindow->mpMessageWidget->printGUIWarningMessage(tr("Global parameter no longer exists, replacing with last used value."));
        }

        mvParameterLayout.push_back(new ParameterLayout(*pit,
                                                        mpGUIComponent->getParameterDescription(*pit),
                                                        valueTxt,
                                                        mpGUIComponent->getParameterUnit(*pit),
                                                        mpGUIComponent));

        parameterLayout->addLayout(mvParameterLayout.back(), n, 0);
        ++n;
    }


    QGridLayout *startValueLayout = new QGridLayout();
    size_t sr=0;
    QLabel *pStartValueLabel = new QLabel("Start Values", this);
    pStartValueLabel->setFont(fontH1);

    QList<GUIPort*> ports = mpGUIComponent->getPortListPtrs();
    QList<GUIPort*>::iterator portIt;
    double j=0;
    QVector<QVector<QString> > startDataNamesStr, startDataUnitsStr;
    QVector<QVector<double> > startDataValuesDbl;
    QVector<QVector<QString> > startDataValuesTxt;
    startDataNamesStr.resize(ports.size());
    startDataValuesDbl.resize(ports.size());
    startDataValuesTxt.resize(ports.size());
    startDataUnitsStr.resize(ports.size());
    mvStartValueLayout.resize(ports.size());
    for ( portIt=ports.begin(); portIt!=ports.end(); ++portIt )
    {
        (*portIt)->getStartValueDataNamesValuesAndUnits(startDataNamesStr[j], startDataValuesTxt[j], startDataUnitsStr[j]);
        if(!(startDataNamesStr[j].isEmpty()))
        {
            QString portName("Port, ");
            portName.append((*portIt)->getName());
            QLabel *portLabelName = new QLabel(portName, this);
            portLabelName->setFont(fontH2);
            startValueLayout->addWidget(portLabelName, sr, 0);
            ++sr;

            mvStartValueLayout[j].resize(startDataNamesStr[j].size());
            for(int i=0; i < startDataNamesStr[j].size(); ++i)
            {
                mvStartValueLayout[j][i]= new ParameterLayout("",
                                                              startDataNamesStr[j][i],
                                                              startDataValuesTxt[j][i],
                                                              startDataUnitsStr[j][i],
                                                              mpGUIComponent);
                startValueLayout->addLayout(mvStartValueLayout[j][i], sr, 0);

                ++sr;
            }
            ++j;
        }
    }


    //qDebug() << "after parnames";

    okButton = new QPushButton(tr("&Ok"), this);
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("&Cancel"), this);
    cancelButton->setDefault(false);

    buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    connect(okButton, SIGNAL(pressed()), SLOT(okPressed()));
    connect(cancelButton, SIGNAL(pressed()), SLOT(close()));

    QHBoxLayout *pNameLayout = new QHBoxLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(buttonBox, lr, 1);
    ++lr;

    if(!(mvParameterLayout.empty()))
    {
        mainLayout->addWidget(pParameterLabel, lr, 0);
        ++lr;
        mainLayout->addLayout(parameterLayout, lr, 0);
        ++lr;
    }
    if(!(mvStartValueLayout[0].isEmpty()))
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

    setParametersAndStartValues();
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
void ComponentPropertiesDialog::setParametersAndStartValues()
{
    bool addedUndoPost = false;
    for (int i=0 ; i < mvParameterLayout.size(); ++i )
    {
        QString valueTxt = mvParameterLayout[i]->getDataValueTxt();

        //Set the parameter
        mpGUIComponent->setParameterValue(mvParameterLayout[i]->getDataName(), valueTxt);

        //Parameter has changed, add to undo stack FIX THIS!!!
        if(mpGUIComponent->getParameterValueTxt(mvParameterLayout[i]->getDataName()) != valueTxt)
        {
            if(!addedUndoPost)
            {
                this->mpGUIComponent->mpParentContainerObject->mUndoStack->newPost("changedparameters");
                addedUndoPost = true;
            }
            this->mpGUIComponent->mpParentContainerObject->mUndoStack->registerChangedParameter(mpGUIComponent->getName(), mvParameterLayout[i]->getDataName(), mpGUIComponent->getParameterValueTxt(mvParameterLayout[i]->getDataName()), valueTxt);
            mpGUIComponent->mpParentContainerObject->mpParentProjectTab->hasChanged();
        }
    }


//    std::cout << "Parameters updated." << std::endl;
//    this->close();
//}


////! @brief Sets the start values in the core component. Read the values from the dialog and write them into the core component.
//void ComponentPropertiesDialog::setStartValues()
//{
    //! @todo Maybe only use strings as value to parameters and start values and interpret it in core, this opens up for aritmetric expressions as well

    QList<GUIPort*> ports = mpGUIComponent->getPortListPtrs();
    QList<GUIPort*>::iterator portIt;
    //Used for plain values
    QVector<QString> startDataNamesStr;
    QVector<double> startDataValuesDbl;
    //Used for mapped to system parameters start values
    QVector<QString> startDataNamesStrSysPar;
    QVector<QString> startDataValuesTxtSysPar;

    size_t j=0;
    //This loop deal with plain values, not mapped system parameters
    for ( portIt=ports.begin(); portIt!=ports.end(); ++portIt )
    {
        startDataNamesStr.clear();
        startDataValuesDbl.clear();
        startDataNamesStrSysPar.clear();
        startDataValuesTxtSysPar.clear();
        //Go trough all start values in all ports
        for(int i=0; i < mvStartValueLayout[j].size(); ++i)
        {
            bool isDbl;
            //Check if the start value is convertible to a double, if so assume that it is just a plain value that should be used,
            //if not assume that it should be mapped to a System parameter
            mvStartValueLayout[j][i]->getDataValueTxt().toDouble(&isDbl);
            if(isDbl)
            {
                //Save the start values that should be set to just plain values (e.g. 17.0) into tmp vectors
                startDataNamesStr.append(mvStartValueLayout[j][i]->getDescriptionName());
                startDataValuesDbl.append(mvStartValueLayout[j][i]->getDataValue());
            }
            else
            {
                //Save the System parameter name (e.g. "myparameter") that should be mapped with the start value
                startDataNamesStrSysPar.append(mvStartValueLayout[j][i]->getDescriptionName());
                startDataValuesTxtSysPar.append(mvStartValueLayout[j][i]->getDataValueTxt());
            }
        }
        //Set this plain start values to the ports
        (*portIt)->setStartValueDataByNames(startDataNamesStr, startDataValuesDbl);
        //Set/map the start values to the system parameters for the ports
        (*portIt)->setStartValueDataByNames(startDataNamesStrSysPar, startDataValuesTxtSysPar);
        ++j;
    }

    std::cout << "Parameters and start values updated." << std::endl;
    this->close();
}






ParameterLayout::ParameterLayout(QString dataName, QString descriptionName, double dataValue, QString unitName, GUIModelObject *pGUIModelObject, QWidget *parent)
    : QGridLayout(parent)
{
    QString dataValueStr;
    dataValueStr.setNum(dataValue);
    commonConstructorCode(dataName, descriptionName, dataValueStr, unitName, pGUIModelObject);
}


ParameterLayout::ParameterLayout(QString dataName, QString descriptionName, QString dataValue, QString unitName, GUIModelObject *pGUIModelObject, QWidget *parent)
    : QGridLayout(parent)
{
    commonConstructorCode(dataName, descriptionName, dataValue, unitName, pGUIModelObject);
}


void ParameterLayout::commonConstructorCode(QString dataName, QString descriptionName, QString dataValue, QString unitName, GUIModelObject *pGUIModelObject)
{
    mpGUIModelObject = pGUIModelObject;

    mDescriptionNameLabel.setMinimumWidth(100);
    mDescriptionNameLabel.setMaximumWidth(100);
    mDataNameLabel.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mDataNameLabel.setMinimumWidth(50);
    mDataNameLabel.setMaximumWidth(50);
    mDataValuesLineEdit.setMinimumWidth(100);
    mDataValuesLineEdit.setMaximumWidth(100);
    mUnitNameLabel.setMinimumWidth(50);
    mUnitNameLabel.setMaximumWidth(50);

    mSystemParameterToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));

    mDataNameLabel.setText(dataName);
    mDescriptionNameLabel.setText(descriptionName);
    mDataValuesLineEdit.setText(dataValue);
    mUnitNameLabel.setText(unitName);

    addWidget(&mDescriptionNameLabel, 0, 0);
    addWidget(&mDataNameLabel, 0, 1);
    addWidget(&mDataValuesLineEdit, 0, 2);
    addWidget(&mSystemParameterToolButton, 0, 3);
    addWidget(&mUnitNameLabel, 0, 4);

    connect(&mSystemParameterToolButton, SIGNAL(pressed()), this, SLOT(showListOfSystemParameters()));
}


QString ParameterLayout::getDescriptionName()
{
    return mDescriptionNameLabel.text();
}


QString ParameterLayout::getDataName()
{
    return mDataNameLabel.text();
}


double ParameterLayout::getDataValue()
{
    return mDataValuesLineEdit.text().toDouble();
}

QString ParameterLayout::getDataValueTxt()
{
    return mDataValuesLineEdit.text();
}


void ParameterLayout::showListOfSystemParameters()
{
    //mSystemParameterToolButton.setDown(false);

    QMenu menu;

    QMap<std::string, double> SystemMap = gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getSystemParametersMap();
    QMap<std::string, double>::iterator it;
    for(it=SystemMap.begin(); it!=SystemMap.end(); ++it)
    {
        QString valueString;
        valueString.setNum(it.value());
        QAction *tempAction = menu.addAction(QString(it.key().c_str()));
        tempAction->setIconVisibleInMenu(false);
    }

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());
    if(selectedAction != 0)
    {
        mDataValuesLineEdit.setText(selectedAction->text());
    }
}
