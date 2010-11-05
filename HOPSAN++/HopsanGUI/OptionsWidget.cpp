//!
//! @file   OptionsWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the options dialog
//!
//$Id: OptionsWidget.cpp 1196 2010-04-01 09:55:04Z robbr48 $


#include <QtGui>
#include <QDebug>

#include "OptionsWidget.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GraphicsView.h"
#include "PlotWidget.h"

class ProjectTabWidget;


//! Constructor for the options dialog
//! @param parent Pointer to the main window
OptionsWidget::OptionsWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;

        //Set the name and size of the main window
    this->setObjectName("OptionsWidget");
    this->resize(640,480);
    this->setWindowTitle("Options");

        //Interface Options
    mpBackgroundColorLabel = new QLabel(tr("Work Area Background Color:"));
    mpBackgroundColorButton = new QToolButton();
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(mpParentMainWindow->mBackgroundColor.red());
    greenString.setNum(mpParentMainWindow->mBackgroundColor.green());
    blueString.setNum(mpParentMainWindow->mBackgroundColor.blue());
    mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mpBackgroundColorButton->setAutoRaise(true);

    mpAntiAliasingCheckBox = new QCheckBox(tr("Use Anti-Aliasing"));
    mpAntiAliasingCheckBox->setCheckable(true);
    mpAntiAliasingCheckBox->setChecked(mpParentMainWindow->mAntiAliasing);

    mpInvertWheelCheckBox = new QCheckBox(tr("Invert Mouse Wheel"));
    mpInvertWheelCheckBox->setCheckable(true);
    mpInvertWheelCheckBox->setChecked(mpParentMainWindow->mInvertWheel);

    mpSnappingCheckBox = new QCheckBox(tr("Auto Snap Components"));
    mpSnappingCheckBox->setCheckable(true);
    mpSnappingCheckBox->setChecked(mpParentMainWindow->mSnapping);

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
    mpEnableProgressBarCheckBox->setChecked(mpParentMainWindow->mEnableProgressBar);

    mpProgressBarLabel = new QLabel(tr("Progress Bar Time Step [ms]"));
    mpProgressBarLabel->setEnabled(mpParentMainWindow->mEnableProgressBar);
    mpProgressBarSpinBox = new QSpinBox();
    mpProgressBarSpinBox->setMinimum(1);
    mpProgressBarSpinBox->setMaximum(5000);
    mpProgressBarSpinBox->setSingleStep(10);
    mpProgressBarSpinBox->setValue(mpParentMainWindow->mProgressBarStep);
    mpProgressBarSpinBox->setEnabled(mpParentMainWindow->mEnableProgressBar);

    mpUseMulticoreCheckBox = new QCheckBox(tr("Use Multi-Threaded Simulation"));
    mpUseMulticoreCheckBox->setCheckable(true);
    mpUseMulticoreCheckBox->setChecked(mpParentMainWindow->mUseMulticore);

    mpSimulationGroupBox = new QGroupBox(tr("Simulation"));
    mpSimulationLayout = new QGridLayout;
    mpSimulationLayout->addWidget(mpEnableProgressBarCheckBox, 0, 0);
    mpSimulationLayout->addWidget(mpProgressBarLabel, 1, 0);
    mpSimulationLayout->addWidget(mpProgressBarSpinBox, 1, 1);
    mpSimulationLayout->addWidget(mpUseMulticoreCheckBox, 2, 0, 1, 2);
    mpSimulationGroupBox->setLayout(mpSimulationLayout);

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

    mpAddPressureUnitButton = new QPushButton("Add Custom Pressure Unit");
    mpAddFlowUnitButton = new QPushButton("Add Custom Flow Unit");
    mpAddForceUnitButton = new QPushButton("Add Custom Force Unit");
    mpAddPositionUnitButton = new QPushButton("Add Custom Position Unit");
    mpAddVelocityUnitButton = new QPushButton("Add Custom Velocity Unit");

    mpPlottingGroupBox = new QGroupBox(tr("Plotting"));
    mpPlottingLayout = new QGridLayout;
    mpPlottingLayout->addWidget(mpPressureUnitLabel, 0, 0);
    mpPlottingLayout->addWidget(mpPressureUnitComboBox, 0, 1);
    mpPlottingLayout->addWidget(mpAddPressureUnitButton, 0, 2);
    mpPlottingLayout->addWidget(mpFlowUnitLabel, 1, 0);
    mpPlottingLayout->addWidget(mpFlowUnitComboBox, 1, 1);
    mpPlottingLayout->addWidget(mpAddFlowUnitButton, 1, 2);
    mpPlottingLayout->addWidget(mpForceUnitLabel, 2, 0);
    mpPlottingLayout->addWidget(mpForceUnitComboBox, 2, 1);
    mpPlottingLayout->addWidget(mpAddForceUnitButton, 2, 2);
    mpPlottingLayout->addWidget(mpPositionUnitLabel, 3, 0);
    mpPlottingLayout->addWidget(mpPositionUnitComboBox, 3, 1);
    mpPlottingLayout->addWidget(mpAddPositionUnitButton, 3, 2);
    mpPlottingLayout->addWidget(mpVelocityUnitLabel, 4, 0);
    mpPlottingLayout->addWidget(mpVelocityUnitComboBox, 4, 1);
    mpPlottingLayout->addWidget(mpAddVelocityUnitButton, 4, 2);
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

    connect(mpParentMainWindow->optionsAction,SIGNAL(triggered()),this,SLOT(show()));
    connect(mpEnableProgressBarCheckBox,SIGNAL(toggled(bool)), mpProgressBarLabel, SLOT(setEnabled(bool)));
    connect(mpEnableProgressBarCheckBox,SIGNAL(toggled(bool)), mpProgressBarSpinBox, SLOT(setEnabled(bool)));
    connect(mpBackgroundColorButton, SIGNAL(pressed()), this, SLOT(colorDialog()));
    connect(mpCancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(mpOkButton, SIGNAL(pressed()), this, SLOT(updateValues()));

    connect(mpAddPressureUnitButton, SIGNAL(pressed()), this, SLOT(addPressureUnit()));
    connect(mpAddFlowUnitButton, SIGNAL(pressed()), this, SLOT(addFlowUnit()));
    connect(mpAddForceUnitButton, SIGNAL(pressed()), this, SLOT(addForceUnit()));
    connect(mpAddPositionUnitButton, SIGNAL(pressed()), this, SLOT(addPositionUnit()));
    connect(mpAddVelocityUnitButton, SIGNAL(pressed()), this, SLOT(addVelocityUnit()));

    QGridLayout *pLayout = new QGridLayout;
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(mpInterfaceGroupBox);
    pLayout->addWidget(mpSimulationGroupBox);
    pLayout->addWidget(mpPlottingGroupBox);
    pLayout->addWidget(mpButtonBox, 4, 0);
    setLayout(pLayout);
}


//! Slot that updates and saves the settings based on the choices made in the dialog box
void OptionsWidget::updateValues()
{
    mpParentMainWindow->mInvertWheel = mpInvertWheelCheckBox->isChecked();
    mpParentMainWindow->mAntiAliasing = mpAntiAliasingCheckBox->isChecked();
    mpParentMainWindow->mSnapping = mpSnappingCheckBox->isChecked();
    for(size_t i=0; i<mpParentMainWindow->mpProjectTabs->count(); ++i)
    {
        mpParentMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->setRenderHint(QPainter::Antialiasing, mpParentMainWindow->mAntiAliasing);
    }
    mpParentMainWindow->mBackgroundColor = mPickedBackgroundColor;
    for(size_t i=0; i<mpParentMainWindow->mpProjectTabs->count(); ++i)
    {
        mpParentMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->updateViewPort();
    }
    mpParentMainWindow->mEnableProgressBar = mpEnableProgressBarCheckBox->isChecked();
    mpParentMainWindow->mProgressBarStep = mpProgressBarSpinBox->value();
    mpParentMainWindow->mUseMulticore = mpUseMulticoreCheckBox->isChecked();
    mpParentMainWindow->mDefaultUnits.remove("Pressure");
    mpParentMainWindow->mDefaultUnits.insert("Pressure", mpPressureUnitComboBox->currentText());
    mpParentMainWindow->mDefaultUnits.remove("Flow");
    mpParentMainWindow->mDefaultUnits.insert("Flow", mpFlowUnitComboBox->currentText());
    mpParentMainWindow->mDefaultUnits.remove("Force");
    mpParentMainWindow->mDefaultUnits.insert("Force", mpForceUnitComboBox->currentText());
    mpParentMainWindow->mDefaultUnits.remove("Position");
    mpParentMainWindow->mDefaultUnits.insert("Position", mpPositionUnitComboBox->currentText());
    mpParentMainWindow->mDefaultUnits.remove("Velocity");
    mpParentMainWindow->mDefaultUnits.insert("Velocity", mpVelocityUnitComboBox->currentText());
    mpParentMainWindow->saveSettings();
    this->accept();
}


//! Slot that opens a color dialog where user can select a background color
void OptionsWidget::colorDialog()
{
    mPickedBackgroundColor = QColorDialog::getColor(mpParentMainWindow->mBackgroundColor, this);
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
        mPickedBackgroundColor = mpParentMainWindow->mBackgroundColor;
    }
}


//! Reimplementation of show() slot. This is used to make sure that the background color button resets its color if the cancel button was pressed last time options were opened.
void OptionsWidget::show()
{
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(mpParentMainWindow->mBackgroundColor.red());
    greenString.setNum(mpParentMainWindow->mBackgroundColor.green());
    blueString.setNum(mpParentMainWindow->mBackgroundColor.blue());
    mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mPickedBackgroundColor = mpParentMainWindow->mBackgroundColor;

    QDialog::show();
}



void OptionsWidget::addPressureUnit()
{
    addAlternativeUnitDialog("Pressure");
}

void OptionsWidget::addFlowUnit()
{
    addAlternativeUnitDialog("Flow");
}

void OptionsWidget::addForceUnit()
{
    addAlternativeUnitDialog("Force");
}

void OptionsWidget::addPositionUnit()
{
    addAlternativeUnitDialog("Position");
}

void OptionsWidget::addVelocityUnit()
{
    addAlternativeUnitDialog("Velocity");
}



//! Slot that opens "Add Custom Unit" dialog
void OptionsWidget::addAlternativeUnitDialog(QString physicalQuantity)
{
    mPhysicalQuantityToModify = physicalQuantity;
    mpAddUnitDialog = new QDialog(this);
    mpAddUnitDialog->setWindowTitle("Add Custom " + physicalQuantity + "Unit");

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

    connect(mpDoneInUnitDialogButton,SIGNAL(clicked()),this,SLOT(addAlternativeUnit()));
    connect(mpCancelInUnitDialogButton,SIGNAL(clicked()),mpAddUnitDialog,SLOT(close()));
}




void OptionsWidget::addAlternativeUnit()
{
    mpParentMainWindow->mCustomUnits.find(mPhysicalQuantityToModify).value().insert(mpUnitNameBox->text(), mpScaleBox->text().toDouble());
    this->updateCustomUnits();
    mpAddUnitDialog->close();
}



void OptionsWidget::updateCustomUnits()
{
    QMap<QString, double>::iterator it;

    mpPressureUnitComboBox->clear();
    for(it = mpParentMainWindow->mCustomUnits.find("Pressure").value().begin();
        it != mpParentMainWindow->mCustomUnits.find("Pressure").value().end(); ++it)
    {
        mpPressureUnitComboBox->addItem(it.key());
    }
    for(size_t i = 0; i<mpPressureUnitComboBox->count(); ++i)
    {
        if(mpPressureUnitComboBox->itemText(i) == mpParentMainWindow->mDefaultUnits.find("Pressure").value())
        {
            mpPressureUnitComboBox->setCurrentIndex(i);
        }
    }

    mpFlowUnitComboBox->clear();
    for(it = mpParentMainWindow->mCustomUnits.find("Flow").value().begin();
        it != mpParentMainWindow->mCustomUnits.find("Flow").value().end(); ++it)
    {
        mpFlowUnitComboBox->addItem(it.key());
    }
    for(size_t i = 0; i<mpFlowUnitComboBox->count(); ++i)
    {
        if(mpFlowUnitComboBox->itemText(i) == mpParentMainWindow->mDefaultUnits.find("Flow").value())
        {
            mpFlowUnitComboBox->setCurrentIndex(i);
        }
    }

    mpForceUnitComboBox->clear();
    for(it = mpParentMainWindow->mCustomUnits.find("Force").value().begin();
        it != mpParentMainWindow->mCustomUnits.find("Force").value().end(); ++it)
    {
        mpForceUnitComboBox->addItem(it.key());
    }
    for(size_t i = 0; i<mpForceUnitComboBox->count(); ++i)
    {
        if(mpForceUnitComboBox->itemText(i) == mpParentMainWindow->mDefaultUnits.find("Force").value())
        {
            mpForceUnitComboBox->setCurrentIndex(i);
        }
    }

    mpPositionUnitComboBox->clear();
    for(it = mpParentMainWindow->mCustomUnits.find("Position").value().begin();
        it != mpParentMainWindow->mCustomUnits.find("Position").value().end(); ++it)
    {
        mpPositionUnitComboBox->addItem(it.key());
    }
    for(size_t i = 0; i<mpPositionUnitComboBox->count(); ++i)
    {
        if(mpPositionUnitComboBox->itemText(i) == mpParentMainWindow->mDefaultUnits.find("Position").value())
        {
            mpPositionUnitComboBox->setCurrentIndex(i);
        }
    }

    mpVelocityUnitComboBox->clear();
    for(it = mpParentMainWindow->mCustomUnits.find("Velocity").value().begin();
        it != mpParentMainWindow->mCustomUnits.find("Velocity").value().end(); ++it)
    {
        mpVelocityUnitComboBox->addItem(it.key());
    }
    for(size_t i = 0; i<mpVelocityUnitComboBox->count(); ++i)
    {
        if(mpVelocityUnitComboBox->itemText(i) == mpParentMainWindow->mDefaultUnits.find("Velocity").value())
        {
            mpVelocityUnitComboBox->setCurrentIndex(i);
        }
    }
}
