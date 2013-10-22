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

#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIObjects/GUIContainerObject.h"
#include "MainWindow.h"
#include "OptionsDialog.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/ModelWidget.h"
#include "ModelHandler.h"

class CentralTabWidget;


//! @class OptionsDialog
//! @brief A class for displaying a dialog window where user can change global program settings
//!
//! Settings are either stored in global config object or discarded, depending on user input.
//!

//! Constructor for the options dialog
//! @param parent Pointer to the main window
OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    // Set the name and size of the main window
    this->setObjectName("OptionsDialog");
    this->resize(640,480);
    this->setWindowTitle("Options");
    this->setPalette(gConfig.getPalette());

    // Interface Options
    QLabel *mpBackgroundColorLabel = new QLabel(tr("Work Area Background Color:"));
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


    mpAlwaysLoadLastSessionCheckBox = new QCheckBox(tr("Always Load Last Session On Startup"));
    mpAlwaysLoadLastSessionCheckBox->setCheckable(true);

    mpShowPopupHelpCheckBox = new QCheckBox(tr("Show Popup Help Messages"));
    mpShowPopupHelpCheckBox->setCheckable(true);


    mpAntiAliasingCheckBox = new QCheckBox(tr("Use Anti-Aliasing"));
    mpAntiAliasingCheckBox->setCheckable(true);

    mpInvertWheelCheckBox = new QCheckBox(tr("Invert Mouse Wheel"));
    mpInvertWheelCheckBox->setCheckable(true);

    mpSnappingCheckBox = new QCheckBox(tr("Auto Snap Components"));
    mpSnappingCheckBox->setCheckable(true);

    mpInterfaceWidget = new QWidget(this);
    mpInterfaceLayout = new QGridLayout;
    mpInterfaceLayout->addWidget(mpNativeStyleSheetCheckBox,    0, 0);
    mpInterfaceLayout->addWidget(mpAlwaysLoadLastSessionCheckBox,   1, 0);
    mpInterfaceLayout->addWidget(mpShowPopupHelpCheckBox,       2, 0);
    mpInterfaceLayout->addWidget(mpInvertWheelCheckBox,         3, 0);
    mpInterfaceLayout->addWidget(mpAntiAliasingCheckBox,        4, 0);
    mpInterfaceLayout->addWidget(mpSnappingCheckBox,            5, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorLabel,        6, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorButton,       6, 1);
    mpInterfaceLayout->addWidget(new QWidget(),                 7, 0, 1, 2);
    mpInterfaceLayout->setRowStretch(7,1);
    mpInterfaceWidget->setLayout(mpInterfaceLayout);

        //Simulation Options
    mpEnableProgressBarCheckBox = new QCheckBox(tr("Enable Simulation Progress Bar"));
    mpEnableProgressBarCheckBox->setCheckable(true);

    mpProgressBarLabel = new QLabel(tr("Progress Bar Time Step [ms]"));
    mpProgressBarLabel->setEnabled(gConfig.getEnableProgressBar());
    mpProgressBarSpinBox = new QSpinBox();
    mpProgressBarSpinBox->setMinimum(1);
    mpProgressBarSpinBox->setMaximum(5000);
    mpProgressBarSpinBox->setSingleStep(10);

    mpUseMulticoreCheckBox = new QCheckBox(tr("Use Multi-Threaded Simulation"));
    mpUseMulticoreCheckBox->setCheckable(true);

    mpThreadsLabel = new QLabel(tr("Number of Simulation Threads \n(0 = Auto Detect)"));
    mpThreadsSpinBox = new QSpinBox();
    mpThreadsSpinBox->setMinimum(0);
    mpThreadsSpinBox->setMaximum(1000000);
    mpThreadsSpinBox->setSingleStep(1);

    //mpThreadsWarningLabel = new QLabel(tr("Caution! Choosing more threads than the number of processor cores may be unstable on some systems."));
    //mpThreadsWarningLabel->setWordWrap(true);
    //QPalette palette = mpThreadsWarningLabel->palette();
    //palette.setColor(mpThreadsWarningLabel->backgroundRole(), Qt::darkRed);
    //palette.setColor(mpThreadsWarningLabel->foregroundRole(), Qt::darkRed);
    //mpThreadsWarningLabel->setPalette(palette);

    mpSimulationWidget = new QWidget(this);
    mpSimulationLayout = new QGridLayout;
    mpSimulationLayout->addWidget(mpEnableProgressBarCheckBox, 0, 0);
    mpSimulationLayout->addWidget(mpProgressBarLabel, 1, 0);
    mpSimulationLayout->addWidget(mpProgressBarSpinBox, 1, 1);
    mpSimulationLayout->addWidget(mpUseMulticoreCheckBox, 2, 0, 1, 2);
    mpSimulationLayout->addWidget(mpThreadsLabel, 3, 0);
    mpSimulationLayout->addWidget(mpThreadsSpinBox, 3, 1);
    mpSimulationLayout->addWidget(new QWidget(), 4, 0, 1, 2);
    mpSimulationLayout->setRowStretch(4, 1);
    //mpSimulationLayout->addWidget(mpThreadsWarningLabel, 4, 0, 1, 2);
    mpSimulationWidget->setLayout(mpSimulationLayout);

    QLabel *pGenerationLimitLabel = new QLabel(tr("Limit number of plot generations to"));
    mpGenerationLimitSpinBox = new QSpinBox();
    mpGenerationLimitSpinBox->setMinimum(1);
    mpGenerationLimitSpinBox->setMaximum(5000000);
    mpGenerationLimitSpinBox->setSingleStep(1);

    mpAutoLimitGenerationsCheckBox = new QCheckBox("Autoremove last generation when limit is reached");
    mpCacheLogDataCeckBox = new QCheckBox("Cache log data on hard drive");
    mpShowHiddenNodeDataVarCheckBox = new QCheckBox("Show (and collect) hidden NodeData variables");

    //! @todo these should not be harcoded, should be build automatically depending on the contents from gConfigure loaded from have subtags in XML
    QLabel *pPressureUnitLabel = new QLabel(tr("Default Pressure Unit"));
    mpPressureUnitComboBox = new QComboBox();
    QLabel *pFlowUnitLabel = new QLabel(tr("Default Flow Unit"));
    mpFlowUnitComboBox = new QComboBox();
    QLabel *pForceUnitLabel = new QLabel(tr("Default Force Unit"));
    mpForceUnitComboBox = new QComboBox();
    QLabel *pPositionUnitLabel = new QLabel(tr("Default Position Unit"));
    mpPositionUnitComboBox = new QComboBox();
    QLabel *pVelocityUnitLabel = new QLabel(tr("Default Velocity Unit"));
    mpVelocityUnitComboBox = new QComboBox();
    QLabel *pTorqueUnitLabel = new QLabel(tr("Default Torque Unit"));
    mpTorqueUnitComboBox = new QComboBox();
    QLabel *pAngleUnitLabel = new QLabel(tr("Default Angle Unit"));
    mpAngleUnitComboBox = new QComboBox();
    QLabel *pAngularVelocityUnitLabel = new QLabel(tr("Default Angular Velocity Unit"));
    mpAngularVelocityUnitComboBox = new QComboBox();
    QLabel *pTimeUnitLabel = new QLabel(tr("Default Time Unit"));
    mpTimeUnitComboBox = new QComboBox();

    QPushButton *pAddPressureUnitButton = new QPushButton("Add Custom Pressure Unit", this);
    QPushButton *pAddFlowUnitButton = new QPushButton("Add Custom Flow Unit", this);
    QPushButton *pAddForceUnitButton = new QPushButton("Add Custom Force Unit", this);
    QPushButton *pAddPositionUnitButton = new QPushButton("Add Custom Position Unit", this);
    QPushButton *pAddVelocityUnitButton = new QPushButton("Add Custom Velocity Unit", this);
    QPushButton *pAddTorqueUnitButton = new QPushButton("Add Custom Torque Unit", this);
    QPushButton *pAddAngleUnitButton = new QPushButton("Add Custom Angle Unit", this);
    QPushButton *pAddAngularVelocityUnitButton = new QPushButton("Add Custom Angular Velocity Unit", this);
    QPushButton *pTimeUnitButton = new QPushButton("Add Custom Time Unit", this);

    int r=0;
    mpPlottingWidget = new QWidget(this);
    mpPlottingLayout = new QGridLayout;
    mpPlottingLayout->addWidget(mpCacheLogDataCeckBox,             r, 0, 1, 3);
    ++r;
    mpPlottingLayout->addWidget(mpAutoLimitGenerationsCheckBox,    r, 0, 1, 3);
    ++r;
    mpPlottingLayout->addWidget(pGenerationLimitLabel,             r, 0, 1, 3);
    mpPlottingLayout->addWidget(mpGenerationLimitSpinBox,          r, 2, 1, 1);
    ++r;
    mpPlottingLayout->addWidget(mpShowHiddenNodeDataVarCheckBox,   r, 0, 1, 3);
    ++r;
    mpPlottingLayout->addWidget(pPressureUnitLabel,                r, 0);
    mpPlottingLayout->addWidget(mpPressureUnitComboBox,            r, 1);
    mpPlottingLayout->addWidget(pAddPressureUnitButton,            r, 2);
    ++r;
    mpPlottingLayout->addWidget(pFlowUnitLabel,                    r, 0);
    mpPlottingLayout->addWidget(mpFlowUnitComboBox,                r, 1);
    mpPlottingLayout->addWidget(pAddFlowUnitButton,                r, 2);
    ++r;
    mpPlottingLayout->addWidget(pForceUnitLabel,                   r, 0);
    mpPlottingLayout->addWidget(mpForceUnitComboBox,               r, 1);
    mpPlottingLayout->addWidget(pAddForceUnitButton,               r, 2);
    ++r;
    mpPlottingLayout->addWidget(pPositionUnitLabel,                r, 0);
    mpPlottingLayout->addWidget(mpPositionUnitComboBox,            r, 1);
    mpPlottingLayout->addWidget(pAddPositionUnitButton,            r, 2);
    ++r;
    mpPlottingLayout->addWidget(pVelocityUnitLabel,                r, 0);
    mpPlottingLayout->addWidget(mpVelocityUnitComboBox,            r, 1);
    mpPlottingLayout->addWidget(pAddVelocityUnitButton,            r, 2);
    ++r;
    mpPlottingLayout->addWidget(pTorqueUnitLabel,                  r, 0);
    mpPlottingLayout->addWidget(mpTorqueUnitComboBox,              r, 1);
    mpPlottingLayout->addWidget(pAddTorqueUnitButton,              r, 2);
    ++r;
    mpPlottingLayout->addWidget(pAngleUnitLabel,                   r, 0);
    mpPlottingLayout->addWidget(mpAngleUnitComboBox,               r, 1);
    mpPlottingLayout->addWidget(pAddAngleUnitButton,               r, 2);
    ++r;
    mpPlottingLayout->addWidget(pAngularVelocityUnitLabel,         r, 0);
    mpPlottingLayout->addWidget(mpAngularVelocityUnitComboBox,     r, 1);
    mpPlottingLayout->addWidget(pAddAngularVelocityUnitButton,     r, 2);
    ++r;
    mpPlottingLayout->addWidget(pTimeUnitLabel,                    r, 0);
    mpPlottingLayout->addWidget(mpTimeUnitComboBox,                r, 1);
    mpPlottingLayout->addWidget(pTimeUnitButton,                   r, 2);
    ++r;
    mpPlottingLayout->addWidget(new QWidget(),                     r, 0, 1, 3);
    mpPlottingLayout->setRowStretch(10, 1);
    mpPlottingWidget->setLayout(mpPlottingLayout);


    QPushButton *mpResetButton = new QPushButton(tr("&Reset Defaults"), this);
    mpResetButton->setAutoDefault(false);
    QPushButton *mpOpenXmlButton = new QPushButton(tr("&Open Settings File"), this);
    mpOpenXmlButton->setAutoDefault(false);
    QPushButton *mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    QPushButton *mpOkButton = new QPushButton(tr("&Done"), this);
    mpOkButton->setDefault(true);

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpResetButton, QDialogButtonBox::ResetRole);
    pButtonBox->addButton(mpOpenXmlButton, QDialogButtonBox::ResetRole);
    pButtonBox->addButton(mpCancelButton, QDialogButtonBox::RejectRole);
    pButtonBox->addButton(mpOkButton, QDialogButtonBox::AcceptRole);

    connect(mpEnableProgressBarCheckBox,    SIGNAL(toggled(bool)),  mpProgressBarLabel,     SLOT(setEnabled(bool)));
    connect(mpEnableProgressBarCheckBox,    SIGNAL(toggled(bool)),  mpProgressBarSpinBox,   SLOT(setEnabled(bool)));
    connect(mpBackgroundColorButton,        SIGNAL(clicked()),      this,                   SLOT(colorDialog()));
    connect(mpResetButton,                  SIGNAL(clicked()),      this,                   SLOT(reset()));
    connect(mpOpenXmlButton,                SIGNAL(clicked()),      this,                   SLOT(openXml()));
    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpOkButton,                     SIGNAL(clicked()),      this,                   SLOT(updateValues()));

    connect(pAddPressureUnitButton,        SIGNAL(clicked()),      this,                   SLOT(addPressureUnit()));
    connect(pAddFlowUnitButton,            SIGNAL(clicked()),      this,                   SLOT(addFlowUnit()));
    connect(pAddForceUnitButton,           SIGNAL(clicked()),      this,                   SLOT(addForceUnit()));
    connect(pAddPositionUnitButton,        SIGNAL(clicked()),      this,                   SLOT(addPositionUnit()));
    connect(pAddVelocityUnitButton,        SIGNAL(clicked()),      this,                   SLOT(addVelocityUnit()));
    connect(pAddTorqueUnitButton,          SIGNAL(clicked()),      this,                   SLOT(addTorqueUnit()));
    connect(pAddAngleUnitButton,           SIGNAL(clicked()),      this,                   SLOT(addAngleUnit()));
    connect(pAddAngularVelocityUnitButton, SIGNAL(clicked()),      this,                   SLOT(addAngularVelocityUnit()));
    connect(pTimeUnitButton,               SIGNAL(clicked()),      this,                   SLOT(addTimeUnit()));

    connect(mpUseMulticoreCheckBox,         SIGNAL(toggled(bool)),  mpThreadsLabel,         SLOT(setEnabled(bool)));
    connect(mpUseMulticoreCheckBox,         SIGNAL(toggled(bool)),  mpThreadsSpinBox,       SLOT(setEnabled(bool)));

    QTabWidget *pTabWidget = new QTabWidget(this);
    pTabWidget->addTab(mpInterfaceWidget, "Interface");
    pTabWidget->addTab(mpSimulationWidget, "Simulation");
    pTabWidget->addTab(mpPlottingWidget, "Plotting");

    QGridLayout *pLayout = new QGridLayout;
    //pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(pTabWidget, 0, 0);
//    pLayout->addWidget(mpInterfaceGroupBox);
//    pLayout->addWidget(mpSimulationGroupBox);
//    pLayout->addWidget(mpPlottingGroupBox);
    pLayout->addWidget(pButtonBox, 1, 0);
    setLayout(pLayout);
}


//! @brief Resets all program settings to default values. Asks user first.
void OptionsDialog::reset()
{
    QMessageBox resetWarningBox(QMessageBox::Warning, tr("Warning"),tr("This will reset ALL settings to default values. Do you want to continue?"), 0, 0);
    resetWarningBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
    resetWarningBox.addButton(tr("&No"), QMessageBox::RejectRole);
    resetWarningBox.setWindowIcon(gpMainWindow->windowIcon());
    bool doIt = (resetWarningBox.exec() == QMessageBox::AcceptRole);

    if(doIt)
    {
        gConfig.loadDefaultsFromXml();
        gConfig.saveToXml();
        show();
    }
}


//! @brief Opens settings XML file outside Hopsan with default application
void OptionsDialog::openXml()
{
    qDebug() << "Opening: " << gDesktopHandler.getDataPath() + QString("hopsanconfig.xml");
    QDesktopServices::openUrl(QUrl("file:///"+gDesktopHandler.getDataPath() + QString("hopsanconfig.xml")));
}


//! Slot that updates and saves the settings based on the choices made in the dialog box
void OptionsDialog::updateValues()
{
    gConfig.setAlwaysLoadLastSession(mpAlwaysLoadLastSessionCheckBox->isChecked());
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
    for(int i=0; i<gpModelHandler->count(); ++i)
    {
        gpModelHandler->getModel(i)->getGraphicsView()->setRenderHint(QPainter::Antialiasing, gConfig.getAntiAliasing());
    }
    gConfig.setBackgroundColor(mPickedBackgroundColor);
    for(int i=0; i<gpModelHandler->count(); ++i)
    {
        gpModelHandler->getModel(i)->getGraphicsView()->updateViewPort();
    }
    gConfig.setEnableProgressBar(mpEnableProgressBarCheckBox->isChecked());
    gConfig.setProgressBarStep(mpProgressBarSpinBox->value());
    gConfig.setUseMultiCore(mpUseMulticoreCheckBox->isChecked());
    gConfig.setNumberOfThreads(mpThreadsSpinBox->value());
    gConfig.setAutoLimitLogDataGenerations(mpAutoLimitGenerationsCheckBox->isChecked());
    gConfig.setShowHiddenNodeDataVariables(mpShowHiddenNodeDataVarCheckBox->isChecked());
    gConfig.setGenerationLimit(mpGenerationLimitSpinBox->value());
    gConfig.setCacheLogData(mpCacheLogDataCeckBox->isChecked());
    for(int i=0; i<gpModelHandler->count(); ++i)       //Loop through all containers and reduce their plot data
    {
        gpModelHandler->getViewContainerObject(i)->getLogDataHandler()->limitPlotGenerations();
    }
    gConfig.setDefaultUnit("Pressure", mpPressureUnitComboBox->currentText());
    gConfig.setDefaultUnit("Flow", mpFlowUnitComboBox->currentText());
    gConfig.setDefaultUnit("Force", mpForceUnitComboBox->currentText());
    gConfig.setDefaultUnit("Position", mpPositionUnitComboBox->currentText());
    gConfig.setDefaultUnit("Velocity", mpVelocityUnitComboBox->currentText());
    gConfig.setDefaultUnit("Torque", mpTorqueUnitComboBox->currentText());
    gConfig.setDefaultUnit("Angle", mpAngleUnitComboBox->currentText());
    gConfig.setDefaultUnit("Angular Velocity", mpAngularVelocityUnitComboBox->currentText());
    gConfig.setDefaultUnit("Time", mpTimeUnitComboBox->currentText());
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

    mpNativeStyleSheetCheckBox->setChecked(gConfig.getUseNativeStyleSheet());
    mpAlwaysLoadLastSessionCheckBox->setChecked(gConfig.getAlwaysLoadLastSession());
    mpShowPopupHelpCheckBox->setChecked(gConfig.getShowPopupHelp());
    mpAntiAliasingCheckBox->setChecked(gConfig.getAntiAliasing());
    mpInvertWheelCheckBox->setChecked(gConfig.getInvertWheel());
    mpSnappingCheckBox->setChecked(gConfig.getSnapping());
    mpEnableProgressBarCheckBox->setChecked(gConfig.getEnableProgressBar());
    mpProgressBarSpinBox->setValue(gConfig.getProgressBarStep());
    mpProgressBarSpinBox->setEnabled(gConfig.getEnableProgressBar());
    mpThreadsSpinBox->setEnabled(gConfig.getUseMulticore());
    mpUseMulticoreCheckBox->setChecked(gConfig.getUseMulticore());
    mpThreadsSpinBox->setValue(gConfig.getNumberOfThreads());
    mpThreadsLabel->setEnabled(gConfig.getUseMulticore());
    mpGenerationLimitSpinBox->setValue(gConfig.getGenerationLimit());
    mpAutoLimitGenerationsCheckBox->setChecked(gConfig.getAutoLimitLogDataGenerations());
    mpShowHiddenNodeDataVarCheckBox->setChecked(gConfig.getShowHiddenNodeDataVariables());
    mpCacheLogDataCeckBox->setChecked(gConfig.getCacheLogData());
    updateCustomUnits();

    QDialog::show();
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
    addCustomUnitDialog("AngularVelocity");
}

void OptionsDialog::addTimeUnit()
{
    addCustomUnitDialog("Time");
}


//! @brief Slot that opens "Add Custom Unit" dialog
void OptionsDialog::addCustomUnitDialog(QString physicalQuantity)
{
    mPhysicalQuantityToModify = physicalQuantity;
    mpAddUnitDialog = new QDialog(this); //!< @todo is this ever deleted
    mpAddUnitDialog->setWindowTitle("Add Custom " + physicalQuantity + " Unit");

    QLabel *pNameLabel = new QLabel("Unit Name: ", this);
    mpUnitNameBox = new QLineEdit(this);
    QLabel *mpScaleLabel = new QLabel("Scaling from SI unit: ", this);
    mpScaleBox = new QLineEdit(this);
    mpScaleBox->setValidator(new QDoubleValidator(this));
    QPushButton *pDoneInUnitDialogButton = new QPushButton("Done", this);
    QPushButton *pCancelInUnitDialogButton = new QPushButton("Cancel", this);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pDoneInUnitDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelInUnitDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(this);
    pDialogLayout->addWidget(pNameLabel,0,0);
    pDialogLayout->addWidget(mpUnitNameBox,0,1);
    pDialogLayout->addWidget(mpScaleLabel,1,0);
    pDialogLayout->addWidget(mpScaleBox,1,1);
    pDialogLayout->addWidget(pButtonBox,2,0,1,2);
    mpAddUnitDialog->setLayout(pDialogLayout);
    mpAddUnitDialog->show();

    connect(pDoneInUnitDialogButton,SIGNAL(clicked()),this,SLOT(addCustomUnit()));
    connect(pCancelInUnitDialogButton,SIGNAL(clicked()),mpAddUnitDialog,SLOT(close()));
}




void OptionsDialog::addCustomUnit()
{
    gConfig.addCustomUnit(mPhysicalQuantityToModify, mpUnitNameBox->text(), mpScaleBox->text().toDouble());
    this->updateCustomUnits();
    mpAddUnitDialog->close();
}


//! @todo this stuff should not be hardcoded like this, should build automatically from all existing custom units
//! @todo The scale value should also be shown in the comboboxes
void OptionsDialog::updateCustomUnits()
{
    QMap<QString, double>::iterator it;
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
    QMap<QString, double> customAngularVelocityUnits = gConfig.getCustomUnits("AngularVelocity");
    for(it = customAngularVelocityUnits.begin(); it != customAngularVelocityUnits.end(); ++it)
    {
        mpAngularVelocityUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpAngularVelocityUnitComboBox->count(); ++i)
    {
        if(mpAngularVelocityUnitComboBox->itemText(i) == gConfig.getDefaultUnit("AngularVelocity"))
        {
            mpAngularVelocityUnitComboBox->setCurrentIndex(i);
        }
    }

    mpTimeUnitComboBox->clear();
    QMap<QString, double> customTimeUnits = gConfig.getCustomUnits("Time");
    for(it = customTimeUnits.begin(); it != customTimeUnits.end(); ++it)
    {
        mpTimeUnitComboBox->addItem(it.key());
    }
    for(int i = 0; i<mpTimeUnitComboBox->count(); ++i)
    {
        if(mpTimeUnitComboBox->itemText(i) == gConfig.getDefaultUnit("Time"))
        {
            mpTimeUnitComboBox->setCurrentIndex(i);
        }
    }
}
