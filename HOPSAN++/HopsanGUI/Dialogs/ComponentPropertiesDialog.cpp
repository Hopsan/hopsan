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
#include "../Widgets/LibraryWidget.h"
#include "../Configuration.h"
#include "../Utilities/GUIUtilities.h"


//! @class ComponentPropertiesDialog
//! @brief The ComponentPropertiesDialog class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog::ComponentPropertiesDialog(GUIComponent *pGUIComponent, MainWindow *parent)
    : QDialog(parent)
{
    mpGUIComponent = pGUIComponent;
    this->setPalette(gConfig.getPalette());
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

    QLabel *pHelpPicture = new QLabel();
    QPixmap helpPixMap;
    helpPixMap.load(mpGUIComponent->getAppearanceData()->getBasePath() + mpGUIComponent->getHelpPicture());
    pHelpPicture->setPixmap(helpPixMap);

    QLabel *pHelpHeading = new QLabel(gpMainWindow->mpLibrary->getAppearanceData(mpGUIComponent->getTypeName())->getName(), this);
    pHelpHeading->setAlignment(Qt::AlignCenter);
    QFont tempFont = pHelpHeading->font();
    tempFont.setPixelSize(16);
    tempFont.setBold(true);
    pHelpHeading->setFont(tempFont);
    QLabel *pHelpText = new QLabel(mpGUIComponent->getHelpText(), this);
    pHelpText->setWordWrap(true);
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
        if((!ok) && !(mpGUIComponent->getParentContainerObject()->getCoreSystemAccessPtr()->hasSystemParameter(valueTxt)))
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

        //Adjust sizes of labels, to make sure that all text is visible and that the spacing is not too big between them
    int descriptionSize=30;
    int nameSize = 10;
    for(int i=0; i<mvParameterLayout.size(); ++i)
    {
        descriptionSize = std::max(descriptionSize, mvParameterLayout.at(i)->mDescriptionNameLabel.width());
        nameSize = std::max(nameSize, mvParameterLayout.at(i)->mDataNameLabel.width());
    }
    for(int i=0; i<mvParameterLayout.size(); ++i)
    {
        mvParameterLayout.at(i)->mDescriptionNameLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
        mvParameterLayout.at(i)->mDataNameLabel.setFixedWidth(nameSize+10);
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
//            QString portName("Port, ");
            QString portName;
            portName.append((*portIt)->getName());
//            QLabel *portLabelName = new QLabel(portName, this);
//            portLabelName->setFont(fontH2);
//            startValueLayout->addWidget(portLabelName, sr, 0);
//            ++sr;

            //mvStartValueLayout[j].resize(startDataNamesStr[j].size());
            for(int i=0; i < startDataNamesStr[j].size(); ++i)
            {
                if(!startDataNamesStr[j][i].isEmpty())
                {

//                    mvStartValueLayout[j][i]= new ParameterLayout(startDataNamesStr[j][i],
//                                                                  portName,
//                                                                  startDataValuesTxt[j][i],
//                                                                  "["+startDataUnitsStr[j][i]+"]",
//                                                                  mpGUIComponent);

                    mvStartValueLayout[j].append(new ParameterLayout(startDataNamesStr[j][i],
                                                                  portName,
                                                                  startDataValuesTxt[j][i],
                                                                  "["+startDataUnitsStr[j][i]+"]",
                                                                  mpGUIComponent));

                      startValueLayout->addLayout(mvStartValueLayout[j].last(), sr, 0);
                }
                ++sr;
            }
            ++j;
        }     
    }

    descriptionSize=30;
    nameSize = 10;
    for(int j=0; j<mvStartValueLayout.size(); ++j)
    {
        for(int i=0; i<mvStartValueLayout.at(j).size(); ++i)
        {
            descriptionSize = std::max(descriptionSize, mvStartValueLayout.at(j).at(i)->mDescriptionNameLabel.width());
            nameSize = std::max(nameSize, mvStartValueLayout.at(j).at(i)->mDataNameLabel.width());
        }
    }
    for(int j=0; j<mvStartValueLayout.size(); ++j)
    {
        for(int i=0; i<mvStartValueLayout.at(j).size(); ++i)
        {
            mvStartValueLayout.at(j).at(i)->mDescriptionNameLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
            mvStartValueLayout.at(j).at(i)->mDataNameLabel.setFixedWidth(nameSize+10);
        }
    }


    //qDebug() << "after parnames";

    cancelButton = new QPushButton(tr("&Cancel"), this);
    okButton = new QPushButton(tr("&Ok"), this);
    okButton->setDefault(true);

    buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    connect(okButton, SIGNAL(clicked()), SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(close()));

    QGroupBox *pHelpGroupBox = new QGroupBox();
    QVBoxLayout *pHelpLayout = new QVBoxLayout();
    pHelpPicture->setAlignment(Qt::AlignCenter);
    pHelpLayout->addWidget(pHelpHeading);
    if(!mpGUIComponent->getHelpPicture().isNull())
        pHelpLayout->addWidget(pHelpPicture);
    if(!mpGUIComponent->getHelpText().isNull())
        pHelpLayout->addWidget(pHelpText);
    pHelpGroupBox->setStyleSheet(QString::fromUtf8("QGroupBox {background-color: white; border: 2px solid gray; border-radius: 5px; margin-top: 1ex;}"));
    pHelpGroupBox->setLayout(pHelpLayout);

    QHBoxLayout *pNameLayout = new QHBoxLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    if(!mpGUIComponent->getHelpText().isNull() || !mpGUIComponent->getHelpPicture().isNull())
    {
        mainLayout->addWidget(pHelpGroupBox, lr, 0, 1, 2);
    }

    ++lr;

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
    else
    {
        pParameterLabel->hide();
    }
    if(!(mvStartValueLayout[0].isEmpty()))
    {
        mainLayout->addWidget(pStartValueLabel,lr, 0);
        ++lr;
        mainLayout->addLayout(startValueLayout, lr, 0);
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
    pPrimaryLayout->addWidget(pScrollArea);
    setLayout(pPrimaryLayout);

    pPrimaryWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
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
    pScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWindowTitle(tr("Parameters"));
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog::okPressed()
{
    mpGUIComponent->getParentContainerObject()->renameGUIModelObject(mpGUIComponent->getName(), mpNameEdit->text());
    //qDebug() << mpNameEdit->text();

    setParametersAndStartValues();
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
void ComponentPropertiesDialog::setParametersAndStartValues()
{
    //! @todo Maybe only use strings as value to parameters and start values and interpret it in core, this opens up for aritmetric expressions as well

    //Parameters
    bool addedUndoPost = false;
    for (int i=0 ; i < mvParameterLayout.size(); ++i )
    {
        QString valueTxt = mvParameterLayout[i]->getDataValueTxt();

        //Get the old value to se if it changed
        QString oldValueTxt = mpGUIComponent->getParameterValueTxt(mvParameterLayout[i]->getDataName());
        //Parameter has changed, add to undo stack
        if(oldValueTxt != valueTxt)
        {
            if(!addedUndoPost)
            {
                this->mpGUIComponent->getParentContainerObject()->mUndoStack->newPost("changedparameters");
                addedUndoPost = true;
            }
            this->mpGUIComponent->getParentContainerObject()->mUndoStack->registerChangedParameter(mpGUIComponent->getName(),
                                                                                                mvParameterLayout[i]->getDataName(),
                                                                                                oldValueTxt,
                                                                                                valueTxt);
            mpGUIComponent->getParentContainerObject()->mpParentProjectTab->hasChanged();
        }
        //Set the parameter
        if(!mpGUIComponent->setParameterValue(mvParameterLayout[i]->getDataName(), valueTxt))
        {
            QMessageBox::critical(0, "Hopsan GUI",
                                  QString("'%1' is an invalid value for parameter '%2'.")
                                  .arg(valueTxt)
                                  .arg(mvParameterLayout[i]->getDataName()));
            mvParameterLayout[i]->setDataValueTxt(oldValueTxt);
            return;
        }
    }

    //StartValues
//    QList<GUIPort*> ports = mpGUIComponent->getPortListPtrs();
//    QList<GUIPort*>::iterator portIt;
    for(int j = 0; j < mvStartValueLayout.size(); ++j)
//        for(portIt=ports.begin(); portIt!=ports.end(); ++portIt)
    {
        for(int i=0; i < mvStartValueLayout[j].size(); ++i)
        {
            QString valueTxt = mvStartValueLayout[j][i]->getDataValueTxt();

            //Get the old value to se if it changed
            QString oldValueTxt = mpGUIComponent->getStartValueTxt(mvStartValueLayout[j][i]->getDescriptionName(), mvStartValueLayout[j][i]->getDataName());
            //Parameter has changed, add to undo stack
            if(oldValueTxt != valueTxt)
            {
                if(!addedUndoPost)
                {
                    this->mpGUIComponent->getParentContainerObject()->mUndoStack->newPost("changedparameters");
                    addedUndoPost = true;
                }
                this->mpGUIComponent->getParentContainerObject()->mUndoStack->registerChangedStartValue(mpGUIComponent->getName(),
                                                                                                     mvStartValueLayout[j][i]->getDescriptionName(),
                                                                                                     mvStartValueLayout[j][i]->getDataName(),
                                                                                                     oldValueTxt,
                                                                                                     valueTxt);
                mpGUIComponent->getParentContainerObject()->mpParentProjectTab->hasChanged();
            }
            //Set the start value
            if(!mpGUIComponent->setStartValue(mvStartValueLayout[j][i]->getDescriptionName(), mvStartValueLayout[j][i]->getDataName(), valueTxt))
            {
                QMessageBox::critical(0, "Hopsan GUI",
                                      QString("'%1' is an invalid value for start value '%2'.")
                                      .arg(valueTxt)
                                      .arg(mvStartValueLayout[j][i]->getDataName()));
                mvStartValueLayout[j][i]->setDataValueTxt(oldValueTxt);
                return;
            }
        }
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
    mDataName = dataName;

    mpGUIModelObject = pGUIModelObject;

    mDescriptionNameLabel.setMinimumWidth(30);
    mDescriptionNameLabel.setMaximumWidth(200);
    mDataNameLabel.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mDataNameLabel.setMinimumWidth(10);
    mDataNameLabel.setMaximumWidth(100);
    mDataValuesLineEdit.setMinimumWidth(100);
    mDataValuesLineEdit.setMaximumWidth(100);
    mUnitNameLabel.setMinimumWidth(50);
    mUnitNameLabel.setMaximumWidth(50);

    mResetDefaultToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
    mResetDefaultToolButton.setToolTip("Reset Default Value");

    mSystemParameterToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
    mSystemParameterToolButton.setToolTip("Map To System Parameter");

    mDataNameLabel.setText(parseVariableDescription(dataName));
    mDataNameLabel.adjustSize();
    mDescriptionNameLabel.setText(descriptionName);
    mDescriptionNameLabel.adjustSize();
    mDataValuesLineEdit.setText(dataValue);
    mUnitNameLabel.setText(parseVariableUnit(unitName));

    addWidget(&mDescriptionNameLabel, 0, 0);
    addWidget(&mDataNameLabel, 0, 1);
    addWidget(&mDataValuesLineEdit, 0, 2);
    addWidget(&mUnitNameLabel, 0, 3);
    addWidget(&mResetDefaultToolButton, 0, 4);
    addWidget(&mSystemParameterToolButton, 0, 5);

    QPalette palette( mDataValuesLineEdit.palette() );
    palette.setColor( QPalette::Text, QColor("gray") );
    mDataValuesLineEdit.setPalette(palette);

    pickColor();

    connect(&mResetDefaultToolButton, SIGNAL(clicked()), this, SLOT(setDefaultValue()));
    connect(&mSystemParameterToolButton, SIGNAL(clicked()), this, SLOT(showListOfSystemParameters()));
    connect(&mDataValuesLineEdit, SIGNAL(editingFinished()), this, SLOT(pickColor()));
}


QString ParameterLayout::getDescriptionName()
{
    return mDescriptionNameLabel.text();
}


QString ParameterLayout::getDataName()
{
    return mDataName;
}


double ParameterLayout::getDataValue()
{
    return mDataValuesLineEdit.text().toDouble();
}

QString ParameterLayout::getDataValueTxt()
{
    return mDataValuesLineEdit.text();
}


void ParameterLayout::setDataValueTxt(QString valueTxt)
{
    mDataValuesLineEdit.setText(valueTxt);
}


//! @brief Sets the value in the text field to the default parameter value
void ParameterLayout::setDefaultValue()
{
    QString tempText;
    tempText.setNum(mpGUIModelObject->mpParentContainerObject->getCoreSystemAccessPtr()->getDefaultParameterValue(mpGUIModelObject->getName(), this->mDescriptionNameLabel.text() + this->mDataName));
    mDataValuesLineEdit.setText(tempText);
    pickColor();
}


void ParameterLayout::showListOfSystemParameters()
{
    //mSystemParameterToolButton.setDown(false);

    QMenu menu;

    QMap<std::string, double> SystemMap = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getCoreSystemAccessPtr()->getSystemParametersMap();
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


void ParameterLayout::pickColor()
{
    if(mDataValuesLineEdit.text().toDouble() == mpGUIModelObject->mpParentContainerObject->getCoreSystemAccessPtr()->getDefaultParameterValue(mpGUIModelObject->getName(), this->mDescriptionNameLabel.text() + this->mDataName))
    {
        QPalette palette( mDataValuesLineEdit.palette() );
        palette.setColor( QPalette::Text, QColor("gray") );
        mDataValuesLineEdit.setPalette(palette);
    }
    else
    {
        QPalette palette( mDataValuesLineEdit.palette() );
        palette.setColor( QPalette::Text, QColor("black") );
        mDataValuesLineEdit.setPalette(palette);
    }
}
