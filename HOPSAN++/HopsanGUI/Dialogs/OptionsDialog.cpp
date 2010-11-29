//!
//! @file   OptionsDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the options dialog
//!
//$Id: OptionsDialog.cpp 1196 2010-04-01 09:55:04Z robbr48 $

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

        //Interface Options
    mpBackgroundColorLabel = new QLabel(tr("Work Area Background Color:"));
    mpBackgroundColorButton = new QToolButton();
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(gConfig.getBackgroundColor().red());
    greenString.setNum(gConfig.getBackgroundColor().green());
    blueString.setNum(gConfig.getBackgroundColor().blue());
    mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mpBackgroundColorButton->setAutoRaise(true);

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
    mpInterfaceLayout->addWidget(mpInvertWheelCheckBox, 0, 0);
    mpInterfaceLayout->addWidget(mpAntiAliasingCheckBox, 1, 0);
    mpInterfaceLayout->addWidget(mpSnappingCheckBox, 2, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorLabel, 3, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorButton, 3, 1);
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

    mpThreadsLabel = new QLabel(tr("Number of Simulation Threads (0 = Auto)"));
    mpThreadsLabel->setEnabled(gConfig.getUseMulticore());
    mpThreadsSpinBox = new QSpinBox();
    mpThreadsSpinBox->setMinimum(0);
    mpThreadsSpinBox->setMaximum(1000000);
    mpThreadsSpinBox->setSingleStep(1);
    mpThreadsSpinBox->setValue(gConfig.getNumberOfThreads());
    mpThreadsSpinBox->setEnabled(gConfig.getUseMulticore());

    mpThreadsWarningLabel = new QLabel(tr("Caution! Choosing more threads than the number of processor cores may be unstable on some systems."));
    mpThreadsWarningLabel->setWordWrap(true);
    QPalette palette = mpThreadsWarningLabel->palette();
    palette.setColor(mpThreadsWarningLabel->backgroundRole(), Qt::darkRed);
    palette.setColor(mpThreadsWarningLabel->foregroundRole(), Qt::darkRed);
    mpThreadsWarningLabel->setPalette(palette);


    mpSimulationGroupBox = new QGroupBox(tr("Simulation"));
    mpSimulationLayout = new QGridLayout;
    mpSimulationLayout->addWidget(mpEnableProgressBarCheckBox, 0, 0);
    mpSimulationLayout->addWidget(mpProgressBarLabel, 1, 0);
    mpSimulationLayout->addWidget(mpProgressBarSpinBox, 1, 1);
    mpSimulationLayout->addWidget(mpUseMulticoreCheckBox, 2, 0, 1, 2);
    mpSimulationLayout->addWidget(mpThreadsLabel, 3, 0);
    mpSimulationLayout->addWidget(mpThreadsSpinBox, 3, 1);
    mpSimulationLayout->addWidget(mpThreadsWarningLabel, 4, 0, 1, 2);
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

    this->updateCustomUnits();

    mpAddValueUnitButton = new QPushButton("Add Custom Value Unit");
    mpAddPressureUnitButton = new QPushButton("Add Custom Pressure Unit");
    mpAddFlowUnitButton = new QPushButton("Add Custom Flow Unit");
    mpAddForceUnitButton = new QPushButton("Add Custom Force Unit");
    mpAddPositionUnitButton = new QPushButton("Add Custom Position Unit");
    mpAddVelocityUnitButton = new QPushButton("Add Custom Velocity Unit");

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
    mpPlottingGroupBox->setLayout(mpPlottingLayout);

    //QLabel *mpPressureUnitLabel;
    //QComboBox *mpPressureUnitComboBox;
    //QGroupBox *mpPlottingGroupBox;
    //QGridLayout *mpPlottingLayout;

    mpCancelButton = new QPushButton(tr("&Cancel"));
    mpCancelButton->setAutoDefault(false);
    mpOkButton = new QPushButton(tr("&Done"));
    mpOkButton->setAutoDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);

    connect(gpMainWindow->optionsAction,SIGNAL(triggered()),this,SLOT(show()));
    connect(mpEnableProgressBarCheckBox,SIGNAL(toggled(bool)), mpProgressBarLabel, SLOT(setEnabled(bool)));
    connect(mpEnableProgressBarCheckBox,SIGNAL(toggled(bool)), mpProgressBarSpinBox, SLOT(setEnabled(bool)));
    connect(mpBackgroundColorButton, SIGNAL(pressed()), this, SLOT(colorDialog()));
    connect(mpCancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(mpOkButton, SIGNAL(pressed()), this, SLOT(updateValues()));

    connect(mpAddValueUnitButton, SIGNAL(pressed()), this, SLOT(addValueUnit()));
    connect(mpAddPressureUnitButton, SIGNAL(pressed()), this, SLOT(addPressureUnit()));
    connect(mpAddFlowUnitButton, SIGNAL(pressed()), this, SLOT(addFlowUnit()));
    connect(mpAddForceUnitButton, SIGNAL(pressed()), this, SLOT(addForceUnit()));
    connect(mpAddPositionUnitButton, SIGNAL(pressed()), this, SLOT(addPositionUnit()));
    connect(mpAddVelocityUnitButton, SIGNAL(pressed()), this, SLOT(addVelocityUnit()));

    connect(mpUseMulticoreCheckBox, SIGNAL(toggled(bool)), mpThreadsLabel, SLOT(setEnabled(bool)));
    connect(mpUseMulticoreCheckBox, SIGNAL(toggled(bool)), mpThreadsSpinBox, SLOT(setEnabled(bool)));

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
    gConfig.setInvertWheel(mpInvertWheelCheckBox->isChecked());
    gConfig.setAntiAliasing(mpAntiAliasingCheckBox->isChecked());
    gConfig.setSnapping(mpSnappingCheckBox->isChecked());
    for(int i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
    {
        gpMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->setRenderHint(QPainter::Antialiasing, gConfig.getAntiAliasing());
    }
    gConfig.setBackgroundColor(mPickedBackgroundColor);
    for(int i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
    {
        gpMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->updateViewPort();
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
        mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
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
    mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mPickedBackgroundColor = gConfig.getBackgroundColor();

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
}
