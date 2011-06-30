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
//! @file   OptionsDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the options dialog
//!
//$Id$

#include <QtGui>
#include <QDebug>

#include "OptionsDialog.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../MainWindow.h"
#include "../GraphicsView.h"
#include "../Widgets/PlotWidget.h"
#include "../Configuration.h"

class ProjectTabWidget;


//! @class OptionsDialog
//! @brief A class for displaying a dialog window where user can change global program settings
//!
//! Settings are either stored in global config object or discarded, depending on user input.
//!

//! Constructor for the options dialog
//! @param parent Pointer to the main window
OptionsDialog::OptionsDialog(MainWindow *parent)
    : QDialog(parent)
{
    //mpParentMainWindow = parent;

        //Set the name and size of the main window
    this->setObjectName("OptionsDialog");
    this->resize(640,480);
    this->setWindowTitle("Options");
    this->setPalette(gConfig.getPalette());

        //Interface Options
    mpBackgroundColorLabel = new QLabel(tr("Work Area Background Color:"));
    mpBackgroundColorButton = new QToolButton();
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(gConfig.getBackgroundColor().red());
    greenString.setNum(gConfig.getBackgroundColor().green());
    blueString.setNum(gConfig.getBackgroundColor().blue());
    QString buttonStyle;
    buttonStyle.append("QToolButton			{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:pressed   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:checked		{ border: 1px solid gray;               border-style: inset;    border-radius: 5px;    	padding: 1px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:checked   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:unchecked		{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:unchecked   	{ border: 1px solid gray;               border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    mpBackgroundColorButton->setStyleSheet(buttonStyle);
    mpBackgroundColorButton->setAutoRaise(true);

    mpNativeStyleSheetCheckBox = new QCheckBox(tr("Use Native Style Sheet"));
    mpNativeStyleSheetCheckBox->setCheckable(true);
    mpNativeStyleSheetCheckBox->setChecked(gConfig.getUseNativeStyleSheet());

    mpShowWelcomeDialogCheckBox = new QCheckBox(tr("Show Welcome Dialog"));
    mpShowWelcomeDialogCheckBox->setCheckable(true);
    mpShowWelcomeDialogCheckBox->setChecked(gConfig.getShowWelcomeDialog());

    mpShowPopupHelpCheckBox = new QCheckBox(tr("Show Popup Help Messages"));
    mpShowPopupHelpCheckBox->setCheckable(true);
    mpShowPopupHelpCheckBox->setChecked(gConfig.getShowPopupHelp());

    mpAntiAliasingCheckBox = new QCheckBox(tr("Use Anti-Aliasing"));
    mpAntiAliasingCheckBox->setCheckable(true);
    mpAntiAliasingCheckBox->setChecked(gConfig.getAntiAliasing());

    mpInvertWheelCheckBox = new QCheckBox(tr("Invert Mouse Wheel"));
    mpInvertWheelCheckBox->setCheckable(true);
    mpInvertWheelCheckBox->setChecked(gConfig.getInvertWheel());

    mpSnappingCheckBox = new QCheckBox(tr("Auto Snap Components"));
    mpSnappingCheckBox->setCheckable(true);
    mpSnappingCheckBox->setChecked(gConfig.getSnapping());

    mpInterfaceGroupBox = new QGroupBox(tr("Interface"));
    mpInterfaceLayout = new QGridLayout;
    mpInterfaceLayout->addWidget(mpNativeStyleSheetCheckBox,    0, 0);
    mpInterfaceLayout->addWidget(mpShowWelcomeDialogCheckBox,   1, 0);
    mpInterfaceLayout->addWidget(mpShowPopupHelpCheckBox,       2, 0);
    mpInterfaceLayout->addWidget(mpInvertWheelCheckBox,         3, 0);
    mpInterfaceLayout->addWidget(mpAntiAliasingCheckBox,        4, 0);
    mpInterfaceLayout->addWidget(mpSnappingCheckBox,            5, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorLabel,        6, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorButton,       7, 1);
    mpInterfaceGroupBox->setLayout(mpInterfaceLayout);

        //Simulation Options
    mpEnableProgressBarCheckBox = new QCheckBox(tr("Enable Simulation Progress Bar"));
    mpEnableProgressBarCheckBox->setCheckable(true);
    mpEnableProgressBarCheckBox->setChecked(gConfig.getEnableProgressBar());

    mpProgressBarLabel = new QLabel(tr("Progress Bar Time Step [ms]"));
    mpProgressBarLabel->setEnabled(gConfig.getEnableProgressBar());
    mpProgressBarSpinBox = new QSpinBox();
    mpProgressBarSpinBox->setMinimum(1);
    mpProgressBarSpinBox->setMaximum(5000);
    mpProgressBarSpinBox->setSingleStep(10);
    mpProgressBarSpinBox->setValue(gConfig.getProgressBarStep());
    mpProgressBarSpinBox->setEnabled(gConfig.getEnableProgressBar());

    mpUseMulticoreCheckBox = new QCheckBox(tr("Use Multi-Threaded Simulation"));
    mpUseMulticoreCheckBox->setCheckable(true);
    mpUseMulticoreCheckBox->setChecked(gConfig.getUseMulticore());

    mpThreadsLabel = new QLabel(tr("Number of Simulation Threads \n(0 = Auto Detect)"));
    mpThreadsLabel->setEnabled(gConfig.getUseMulticore());
    mpThreadsSpinBox = new QSpinBox();
    mpThreadsSpinBox->setMinimum(0);
    mpThreadsSpinBox->setMaximum(1000000);
    mpThreadsSpinBox->setSingleStep(1);
    mpThreadsSpinBox->setValue(gConfig.getNumberOfThreads());
    mpThreadsSpinBox->setEnabled(gConfig.getUseMulticore());

    //mpThreadsWarningLabel = new QLabel(tr("Caution! Choosing more threads than the number of processor cores may be unstable on some systems."));
    //mpThreadsWarningLabel->setWordWrap(true);
    //QPalette palette = mpThreadsWarningLabel->palette();
    //palette.setColor(mpThreadsWarningLabel->backgroundRole(), Qt::darkRed);
    //palette.setColor(mpThreadsWarningLabel->foregroundRole(), Qt::darkRed);
    //mpThreadsWarningLabel->setPalette(palette);

    mpSimulationGroupBox = new QGroupBox(tr("Simulation"));
    mpSimulationLayout = new QGridLayout;
    mpSimulationLayout->addWidget(mpEnableProgressBarCheckBox, 0, 0);
    mpSimulationLayout->addWidget(mpProgressBarLabel, 1, 0);
    mpSimulationLayout->addWidget(mpProgressBarSpinBox, 1, 1);
    mpSimulationLayout->addWidget(mpUseMulticoreCheckBox, 2, 0, 1, 2);
    mpSimulationLayout->addWidget(mpThreadsLabel, 3, 0);
    mpSimulationLayout->addWidget(mpThreadsSpinBox, 3, 1);
    //mpSimulationLayout->addWidget(mpThreadsWarningLabel, 4, 0, 1, 2);
    mpSimulationGroupBox->setLayout(mpSimulationLayout);

    mpValueUnitLabel = new QLabel(tr("Default Value Unit"));
    mpValueUnitComboBox = new QComboBox();
    mpPressureUnitLabel = new QLabel(tr("Default Pressure Unit"));
    mpPressureUnitComboBox = new QComboBox();
    mpFlowUnitLabel = new QLabel(tr("Default Flow Unit"));
    mpFlowUnitComboBox = new QComboBox();
    mpForceUnitLabel = new QLabel(tr("Default Force Unit"));
    mpForceUnitComboBox = new QComboBox();
    mpPositionUnitLabel = new QLabel(tr("Default Position Unit"));
    mpPositionUnitComboBox = new QComboBox();
    mpVelocityUnitLabel = new QLabel(tr("Default Velocity Unit"));
    mpVelocityUnitComboBox = new QComboBox();
    mpTorqueUnitLabel = new QLabel(tr("Default Torque Unit"));
    mpTorqueUnitComboBox = new QComboBox();
    mpAngleUnitLabel = new QLabel(tr("Default Angle Unit"));
    mpAngleUnitComboBox = new QComboBox();
    mpAngularVelocityUnitLabel = new QLabel(tr("Default Angular Velocity Unit"));
    mpAngularVelocityUnitComboBox = new QComboBox();

    this->updateCustomUnits();

    mpAddValueUnitButton = new QPushButton("Add Custom Value Unit", this);
    mpAddPressureUnitButton = new QPushButton("Add Custom Pressure Unit", this);
    mpAddFlowUnitButton = new QPushButton("Add Custom Flow Unit", this);
    mpAddForceUnitButton = new QPushButton("Add Custom Force Unit", this);
    mpAddPositionUnitButton = new QPushButton("Add Custom Position Unit", this);
    mpAddVelocityUnitButton = new QPushButton("Add Custom Velocity Unit", this);
    mpAddTorqueUnitButton = new QPushButton("Add Custom Torque Unit", this);
    mpAddAngleUnitButton = new QPushButton("Add Custom Angle Unit", this);
    mpAddAngularVelocityUnitButton = new QPushButton("Add Custom Angular Velocity Unit", this);

    mpPlottingGroupBox = new QGroupBox(tr("Plotting"));
    mpPlottingLayout = new QGridLayout;
    mpPlottingLayout->addWidget(mpValueUnitLabel, 0, 0);
    mpPlottingLayout->addWidget(mpValueUnitComboBox, 0, 1);
    mpPlottingLayout->addWidget(mpAddValueUnitButton, 0, 2);
    mpPlottingLayout->addWidget(mpPressureUnitLabel, 1, 0);
    mpPlottingLayout->addWidget(mpPressureUnitComboBox, 1, 1);
    mpPlottingLayout->addWidget(mpAddPressureUnitButton, 1, 2);
    mpPlottingLayout->addWidget(mpFlowUnitLabel, 2, 0);
    mpPlottingLayout->addWidget(mpFlowUnitComboBox, 2, 1);
    mpPlottingLayout->addWidget(mpAddFlowUnitButton, 2, 2);
    mpPlottingLayout->addWidget(mpForceUnitLabel, 3, 0);
    mpPlottingLayout->addWidget(mpForceUnitComboBox, 3, 1);
    mpPlottingLayout->addWidget(mpAddForceUnitButton, 3, 2);
    mpPlottingLayout->addWidget(mpPositionUnitLabel, 4, 0);
    mpPlottingLayout->addWidget(mpPositionUnitComboBox, 4, 1);
    mpPlottingLayout->addWidget(mpAddPositionUnitButton, 4, 2);
    mpPlottingLayout->addWidget(mpVelocityUnitLabel, 5, 0);
    mpPlottingLayout->addWidget(mpVelocityUnitComboBox, 5, 1);
    mpPlottingLayout->addWidget(mpAddVelocityUnitButton, 5, 2);
    mpPlottingLayout->addWidget(mpTorqueUnitLabel, 6, 0);
    mpPlottingLayout->addWidget(mpTorqueUnitComboBox, 6, 1);
    mpPlottingLayout->addWidget(mpAddTorqueUnitButton, 6, 2);
    mpPlottingLayout->addWidget(mpAngleUnitLabel, 7, 0);
    mpPlottingLayout->addWidget(mpAngleUnitComboBox, 7, 1);
    mpPlottingLayout->addWidget(mpAddAngleUnitButton, 7, 2);
    mpPlottingLayout->addWidget(mpAngularVelocityUnitLabel, 8, 0);
    mpPlottingLayout->addWidget(mpAngularVelocityUnitComboBox, 8, 1);
    mpPlottingLayout->addWidget(mpAddAngularVelocityUnitButton, 8, 2);
    mpPlottingGroupBox->setLayout(mpPlottingLayout);

    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    mpOkButton = new QPushButton(tr("&Done"), this);
    mpOkButton->setDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);

    connect(gpMainWindow->mpOptionsAction,    SIGNAL(triggered()),    this,                   SLOT(show()));
    connect(mpEnableProgressBarCheckBox,    SIGNAL(toggled(bool)),  mpProgressBarLabel,     SLOT(setEnabled(bool)));
    connect(mpEnableProgressBarCheckBox,    SIGNAL(toggled(bool)),  mpProgressBarSpinBox,   SLOT(setEnabled(bool)));
    connect(mpBackgroundColorButton,        SIGNAL(clicked()),      this,                   SLOT(colorDialog()));
    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpOkButton,                     SIGNAL(clicked()),      this,                   SLOT(updateValues()));

    connect(mpAddValueUnitButton,           SIGNAL(clicked()),      this,                   SLOT(addValueUnit()));
    connect(mpAddPressureUnitButton,        SIGNAL(clicked()),      this,                   SLOT(addPressureUnit()));
    connect(mpAddFlowUnitButton,            SIGNAL(clicked()),      this,                   SLOT(addFlowUnit()));
    connect(mpAddForceUnitButton,           SIGNAL(clicked()),      this,                   SLOT(addForceUnit()));
    connect(mpAddPositionUnitButton,        SIGNAL(clicked()),      this,                   SLOT(addPositionUnit()));
    connect(mpAddVelocityUnitButton,        SIGNAL(clicked()),      this,                   SLOT(addVelocityUnit()));
    connect(mpAddTorqueUnitButton,          SIGNAL(clicked()),      this,                   SLOT(addTorqueUnit()));
    connect(mpAddAngleUnitButton,           SIGNAL(clicked()),      this,                   SLOT(addAngleUnit()));
    connect(mpAddAngularVelocityUnitButton, SIGNAL(clicked()),      this,                   SLOT(addAngularVelocityUnit()));

    connect(mpUseMulticoreCheckBox,         SIGNAL(toggled(bool)),  mpThreadsLabel,         SLOT(setEnabled(bool)));
    connect(mpUseMulticoreCheckBox,         SIGNAL(toggled(bool)),  mpThreadsSpinBox,       SLOT(setEnabled(bool)));

    QGridLayout *pLayout = new QGridLayout;
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(mpInterfaceGroupBox);
    pLayout->addWidget(mpSimulationGroupBox);
    pLayout->addWidget(mpPlottingGroupBox);
    pLayout->addWidget(mpButtonBox, 4, 0);
    setLayout(pLayout);
}


//! Slot that updates and saves the settings based on the choices made in the dialog box
void OptionsDialog::updateValues()
{
    gConfig.setShowWelcomeDialog(mpShowWelcomeDialogCheckBox->isChecked());
    gConfig.setShowPopupHelp(mpShowPopupHelpCheckBox->isChecked());
    gConfig.setUseNativeStyleSheet(mpNativeStyleSheetCheckBox->isChecked());

    if(gConfig.getUseNativeStyleSheet())
    {
        gpMainWindow->setStyleSheet((" "));
        QMainWindow dummy;
        gpMainWindow->setPalette(dummy.palette());
        this->setPalette(dummy.palette());
    }
    else
    {
        gpMainWindow->setStyleSheet(gConfig.getStyleSheet());
        gpMainWindow->setPalette(gConfig.getPalette());
        this->setPalette(gConfig.getPalette());
    }
    emit paletteChanged();
    gConfig.setInvertWheel(mpInvertWheelCheckBox->isChecked());
    gConfig.setAntiAliasing(mpAntiAliasingCheckBox->isChecked());
    gConfig.setSnapping(mpSnappingCheckBox->isChecked());
    for(int i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
    {
        gpMainWindow->mpProjectTabs->getTab(i)->getGraphicsView()->setRenderHint(QPainter::Antialiasing, gConfig.getAntiAliasing());
    }
    gConfig.setBackgroundColor(mPickedBackgroundColor);
    for(int i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
    {
        gpMainWindow->mpProjectTabs->getTab(i)->getGraphicsView()->updateViewPort();
    }
    gConfig.setEnableProgressBar(mpEnableProgressBarCheckBox->isChecked());
    gConfig.setProgressBarStep(mpProgressBarSpinBox->value());
    gConfig.setUseMultiCore(mpUseMulticoreCheckBox->isChecked());
    gConfig.setNumberOfThreads(mpThreadsSpinBox->value());
    gConfig.setDefaultUnit("Value", mpValueUnitComboBox->currentText());
    gConfig.setDefaultUnit("Pressure", mpPressureUnitComboBox->currentText());
    gConfig.setDefaultUnit("Flow", mpFlowUnitComboBox->currentText());
    gConfig.setDefaultUnit("Force", mpForceUnitComboBox->currentText());
    gConfig.setDefaultUnit("Position", mpPositionUnitComboBox->currentText());
    gConfig.setDefaultUnit("Velocity", mpVelocityUnitComboBox->currentText());
    gConfig.setDefaultUnit("Torque", mpTorqueUnitComboBox->currentText());
    gConfig.setDefaultUnit("Angle", mpAngleUnitComboBox->currentText());
    gConfig.setDefaultUnit("AngularVelocity", mpAngularVelocityUnitComboBox->currentText());
    gConfig.saveToXml();

    this->accept();
}


//! Slot that opens a color dialog where user can select a background color
void OptionsDialog::colorDialog()
{
    mPickedBackgroundColor = QColorDialog::getColor(gConfig.getBackgroundColor(), this);
    if (mPickedBackgroundColor.isValid())
    {
        QString redString;
        QString greenString;
        QString blueString;
        redString.setNum(mPickedBackgroundColor.red());
        greenString.setNum(mPickedBackgroundColor.green());
        blueString.setNum(mPickedBackgroundColor.blue());
        QString buttonStyle;
        buttonStyle.append("QToolButton                         { border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        buttonStyle.append("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        buttonStyle.append("QToolButton:hover:pressed   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        buttonStyle.append("QToolButton:hover                   { border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        buttonStyle.append("QToolButton:checked                 { border: 1px solid gray;               border-style: inset;    border-radius: 5px;    	padding: 1px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        buttonStyle.append("QToolButton:hover:checked   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        buttonStyle.append("QToolButton:unchecked		{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        buttonStyle.append("QToolButton:hover:unchecked   	{ border: 1px solid gray;               border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
        mpBackgroundColorButton->setStyleSheet(buttonStyle);
        mpBackgroundColorButton->setDown(false);
    }
    else
    {
        mPickedBackgroundColor = gConfig.getBackgroundColor();
    }
}


//! Reimplementation of show() slot. This is used to make sure that the background color button resets its color if the cancel button was pressed last time options were opened.
void OptionsDialog::show()
{
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(gConfig.getBackgroundColor().red());
    greenString.setNum(gConfig.getBackgroundColor().green());
    blueString.setNum(gConfig.getBackgroundColor().blue());
    QString buttonStyle;
    buttonStyle.append("QToolButton			{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:pressed   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:checked		{ border: 1px solid gray;               border-style: inset;    border-radius: 5px;    	padding: 1px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:checked   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:unchecked		{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:unchecked   	{ border: 1px solid gray;               border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    mpBackgroundColorButton->setStyleSheet(buttonStyle);
    mPickedBackgroundColor = gConfig.getBackgroundColor();

    mpShowWelcomeDialogCheckBox->setChecked(gConfig.getShowWelcomeDialog());
    mpShowPopupHelpCheckBox->setChecked(gConfig.getShowPopupHelp());
    mpAntiAliasingCheckBox->setChecked(gConfig.getAntiAliasing());
    mpInvertWheelCheckBox->setChecked(gConfig.getInvertWheel());
    mpSnappingCheckBox->setChecked(gConfig.getSnapping());
    mpEnableProgressBarCheckBox->setChecked(gConfig.getEnableProgressBar());
    mpProgressBarSpinBox->setValue(gConfig.getProgressBarStep());
    mpProgressBarSpinBox->setEnabled(gConfig.getEnableProgressBar());
    mpUseMulticoreCheckBox->setChecked(gConfig.getUseMulticore());
    mpThreadsSpinBox->setValue(gConfig.getNumberOfThreads());
    mpThreadsSpinBox->setEnabled(gConfig.getUseMulticore());

    QDialog::show();
}


void OptionsDialog::addValueUnit()
{
    addCustomUnitDialog("Value");
}

void OptionsDialog::addPressureUnit()
{
    addCustomUnitDialog("Pressure");
}

void OptionsDialog::addFlowUnit()
{
    addCustomUnitDialog("Flow");
}

void OptionsDialog::addForceUnit()
{
    addCustomUnitDialog("Force");
}

void OptionsDialog::addPositionUnit()
{
    addCustomUnitDialog("Position");
}

void OptionsDialog::addVelocityUnit()
{
    addCustomUnitDialog("Velocity");
}

void OptionsDialog::addTorqueUnit()
{
    addCustomUnitDialog("Torque");
}

void OptionsDialog::addAngleUnit()
{
    addCustomUnitDialog("Angle");
}

void OptionsDialog::addAngularVelocityUnit()
{
    addCustomUnitDialog("Angular Velocity");
}


//! Slot that opens "Add Custom Unit" dialog
void OptionsDialog::addCustomUnitDialog(QString physicalQuantity)
{
    mPhysicalQuantityToModify = physicalQuantity;
    mpAddUnitDialog = new QDialog(this);
    mpAddUnitDialog->setWindowTitle("Add Custom " + physicalQuantity + " Unit");

    mpNameLabel = new QLabel("Unit Name: ", this);
    mpUnitNameBox = new QLineEdit(this);
    mpScaleLabel = new QLabel("Scaling from SI unit: ", this);
    mpScaleBox = new QLineEdit(this);
    mpScaleBox->setValidator(new QDoubleValidator(this));
    mpDoneInUnitDialogButton = new QPushButton("Done", this);
    mpCancelInUnitDialogButton = new QPushButton("Cancel", this);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpDoneInUnitDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpCancelInUnitDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(this);
    pDialogLayout->addWidget(mpNameLabel,0,0);
    pDialogLayout->addWidget(mpUnitNameBox,0,1);
    pDialogLayout->addWidget(mpScaleLabel,1,0);
    pDialogLayout->addWidget(mpScaleBox,1,1);
    pDialogLayout->addWidget(pButtonBox,2,0,1,2);
    mpAddUnitDialog->setLayout(pDialogLayout);
    mpAddUnitDialog->show();

    connect(mpDoneInUnitDialogButton,SIGNAL(clicked()),this,SLOT(addCustomUnit()));
    connect(mpCancelInUnitDialogButton,SIGNAL(clicked()),mpAddUnitDialog,SLOT(close()));
}




void OptionsDialog::addCustomUnit()
{
    gConfig.addCustomUnit(mPhysicalQuantityToModify, mpUnitNameBox->text(), mpScaleBox->text().toDouble());
    this->updateCustomUnits();
    mpAddUnitDialog->close();
}



void OptionsDialog::updateCustomUnits()
{
    mpValueUnitComboBox->clear();
    QMap<QString, double> customValueUnits = gConfig.getCustomUnits("Value");
    QMap<QString, double>::iterator it;
    for(it = customValueUnits.begin(); it != customValueUnits.end(); ++it)
    {
        mpValueUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpValueUnitComboBox->count(); ++i)
    {
        if(mpValueUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Value"))
        {
            mpValueUnitComboBox->setCurrentIndex(i);
        }
    }

    mpPressureUnitComboBox->clear();
    QMap<QString, double> customPressureUnits = gConfig.getCustomUnits("Pressure");
    for(it = customPressureUnits.begin(); it != customPressureUnits.end(); ++it)
    {
        mpPressureUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpPressureUnitComboBox->count(); ++i)
    {
        if(mpPressureUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Pressure"))
        {
            mpPressureUnitComboBox->setCurrentIndex(i);
        }
    }

    mpFlowUnitComboBox->clear();
    QMap<QString, double> customFlowUnits = gConfig.getCustomUnits("Flow");
    for(it = customFlowUnits.begin(); it != customFlowUnits.end(); ++it)
    {
        mpFlowUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpFlowUnitComboBox->count(); ++i)
    {
        if(mpFlowUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Flow"))
        {
            mpFlowUnitComboBox->setCurrentIndex(i);
        }
    }

    mpForceUnitComboBox->clear();
    QMap<QString, double> customForceUnits = gConfig.getCustomUnits("Force");
    for(it = customForceUnits.begin(); it != customForceUnits.end(); ++it)
    {
        mpForceUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpForceUnitComboBox->count(); ++i)
    {
        if(mpForceUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Force"))
        {
            mpForceUnitComboBox->setCurrentIndex(i);
        }
    }

    mpPositionUnitComboBox->clear();
    QMap<QString, double> customPositionUnits = gConfig.getCustomUnits("Position");
    for(it = customPositionUnits.begin(); it != customPositionUnits.end(); ++it)
    {
        mpPositionUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpPositionUnitComboBox->count(); ++i)
    {
        if(mpPositionUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Position"))
        {
            mpPositionUnitComboBox->setCurrentIndex(i);
        }
    }

    mpVelocityUnitComboBox->clear();
    QMap<QString, double> customVelocityUnits = gConfig.getCustomUnits("Velocity");
    for(it = customVelocityUnits.begin(); it != customVelocityUnits.end(); ++it)
    {
        mpVelocityUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpVelocityUnitComboBox->count(); ++i)
    {
        if(mpVelocityUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Velocity"))
        {
            mpVelocityUnitComboBox->setCurrentIndex(i);
        }
    }

    mpTorqueUnitComboBox->clear();
    QMap<QString, double> customTorqueUnits = gConfig.getCustomUnits("Torque");
    for(it = customTorqueUnits.begin(); it != customTorqueUnits.end(); ++it)
    {
        mpTorqueUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpTorqueUnitComboBox->count(); ++i)
    {
        if(mpTorqueUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Torque"))
        {
            mpTorqueUnitComboBox->setCurrentIndex(i);
        }
    }

    mpAngleUnitComboBox->clear();
    QMap<QString, double> customAngleUnits = gConfig.getCustomUnits("Angle");
    for(it = customAngleUnits.begin(); it != customAngleUnits.end(); ++it)
    {
        mpAngleUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpAngleUnitComboBox->count(); ++i)
    {
        if(mpAngleUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Angle"))
        {
            mpAngleUnitComboBox->setCurrentIndex(i);
        }
    }

    mpAngularVelocityUnitComboBox->clear();
    QMap<QString, double> customAngularVelocityUnits = gConfig.getCustomUnits("Angular Velocity");
    for(it = customAngularVelocityUnits.begin(); it != customAngularVelocityUnits.end(); ++it)
    {
        mpAngularVelocityUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpAngularVelocityUnitComboBox->count(); ++i)
    {
        if(mpAngularVelocityUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Angular Velocity"))
        {
            mpAngularVelocityUnitComboBox->setCurrentIndex(i);
        }
    }
}
