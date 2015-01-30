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

#include <QMessageBox>
#include <QDebug>
#include <QColorDialog>
#include <QDesktopServices>
#include <QMainWindow>
#include <QLineEdit>
#include <QFileDialog>

#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIObjects/GUIContainerObject.h"
#include "OptionsDialog.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/ModelWidget.h"
#include "ModelHandler.h"

class CentralTabWidget;

class UnitScaleSelector : public QWidget
{
    Q_OBJECT
public:
    UnitScaleSelector(const QString &rQuantity, QWidget *pParent) : QWidget(pParent)
    {
        mQuantity = rQuantity;
        mpDefaultUnitComboBox = new QComboBox(this);
        QToolButton *pAddCustomUnitButton = new QToolButton(this);
        pAddCustomUnitButton->setIcon(QIcon("://graphics/uiicons/Hopsan-Add.png"));
        pAddCustomUnitButton->setFixedWidth(48);
        pAddCustomUnitButton->setToolTip("Add Custom "+mQuantity+" Unit");
        QToolButton *pRemoveCustomUnitButton = new QToolButton(this);
        pRemoveCustomUnitButton->setIcon(QIcon("://graphics/uiicons/Hopsan-Discard.png"));
        pRemoveCustomUnitButton->setFixedWidth(48);
        pRemoveCustomUnitButton->setToolTip("Remove Custom "+mQuantity+" Unit");
        QToolButton *pShowCustomUnitsButton = new QToolButton(this);
        pShowCustomUnitsButton->setIcon(QIcon("://graphics/uiicons/Hopsan-Zoom.png"));
        pShowCustomUnitsButton->setFixedWidth(48);
        pShowCustomUnitsButton->setToolTip("Show unit scales for "+mQuantity);

        QString original = gpConfig->getSIUnit(rQuantity);

        QHBoxLayout *pLayout = new QHBoxLayout(this);
        pLayout->setContentsMargins(0,0,0,0);
        pLayout->addWidget(new QLabel(mQuantity, this));
        pLayout->addWidget(new QLabel(QString("[%1]").arg(original), this), 0, Qt::AlignRight);
        pLayout->addWidget(new QLabel("Default:", this), 0, Qt::AlignRight);
        pLayout->addWidget(mpDefaultUnitComboBox);
        pLayout->addWidget(pAddCustomUnitButton);
        pLayout->addWidget(pRemoveCustomUnitButton);
        pLayout->addWidget(pShowCustomUnitsButton);

        connect(pAddCustomUnitButton, SIGNAL(clicked()), this, SLOT(execAddCustomUnitDialog()));
        connect(pRemoveCustomUnitButton, SIGNAL(clicked()), this, SLOT(removeSelectedUnitScale()));
        connect(pShowCustomUnitsButton, SIGNAL(clicked()), this, SLOT(showScales()));
    }

    void updateUnitList()
    {
        //! @todo The scale value should also be shown in the comboboxes
        mpDefaultUnitComboBox->clear();
        QString defaultUnit = gpConfig->getDefaultUnit(mQuantity);
        if (defaultUnit.isEmpty())
        {
            defaultUnit = gpConfig->getSIUnit(mQuantity);
        }
        QMap<QString, double> unit_scales = gpConfig->getUnitScales(mQuantity);
        QMap<QString, double>::iterator it;
        for(it = unit_scales.begin(); it != unit_scales.end(); ++it)
        {
            mpDefaultUnitComboBox->addItem(it.key());
        }
        for(int i=0; i<mpDefaultUnitComboBox->count(); ++i)
        {
            if(mpDefaultUnitComboBox->itemText(i) == defaultUnit )
            {
                mpDefaultUnitComboBox->setCurrentIndex(i);
                break;
            }
        }
    }

    QString getDefaultUnit() const
    {
        return mpDefaultUnitComboBox->currentText();
    }

    QString getQuantity() const
    {
        return mQuantity;
    }


private slots:
    void execAddCustomUnitDialog()
    {
        QDialog *pDialog = new QDialog(this);
        pDialog->setWindowTitle("Add Custom "+mQuantity+" Unit");

        QLineEdit *pUnitNameBox = new QLineEdit(this);
        QLineEdit *pScaleBox = new QLineEdit(this);
        pScaleBox->setValidator(new QDoubleValidator(this));

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, pDialog);
        connect(pButtonBox, SIGNAL(accepted()), pDialog, SLOT(accept()));
        connect(pButtonBox, SIGNAL(rejected()), pDialog, SLOT(reject()));

        QGridLayout *pLayout = new QGridLayout(pDialog);
        pLayout->addWidget(new QLabel("Unit Name: ", this),0,0);
        pLayout->addWidget(pUnitNameBox,0,1);
        pLayout->addWidget(new QLabel("Scaling from SI unit: ", this),1,0);
        pLayout->addWidget(pScaleBox,1,1);
        pLayout->addWidget(pButtonBox,2,0,1,2);

        int rc = pDialog->exec();
        if (rc == QDialog::Accepted)
        {
            gpConfig->addCustomUnit(mQuantity, pUnitNameBox->text(), pScaleBox->text().toDouble());
            updateUnitList();
        }

        pDialog->deleteLater();
    }

    void removeSelectedUnitScale()
    {
        QString unit = mpDefaultUnitComboBox->currentText();
        gpConfig->removeUnitScale(mQuantity, unit);
        updateUnitList();
    }

    void showScales()
    {
        QDialog *pDialog = new QDialog(this);
        pDialog->setWindowTitle("Custom "+mQuantity+" Unit Scales");
        QGridLayout *pLayout = new QGridLayout(pDialog);

        QString si = gpConfig->getSIUnit(mQuantity);
        QList<UnitScale> scales;
        gpConfig->getUnitScales(mQuantity, scales);

        int r=0;
        foreach(const UnitScale& scale, scales)
        {
            pLayout->addWidget(new QLabel("1 "+si, pDialog), r, 0);
            pLayout->addWidget(new QLabel(" = ", pDialog), r, 1);
            pLayout->addWidget(new QLabel(scale.mScale, pDialog), r, 2);
            pLayout->addWidget(new QLabel(scale.mUnit, pDialog), r, 3);
            ++r;
        }

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, pDialog);
        connect(pButtonBox, SIGNAL(rejected()), pDialog, SLOT(reject()));
        pLayout->addWidget(pButtonBox,r,3);

        pDialog->exec();
        pDialog->deleteLater();
    }


private:
    QString mQuantity;
    QComboBox *mpDefaultUnitComboBox;
};
#include "OptionsDialog.moc"


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
    this->setPalette(gpConfig->getPalette());

    // Interface Options
    QLabel *mpBackgroundColorLabel = new QLabel(tr("Work Area Background Color:"));
    mpBackgroundColorButton = new QToolButton();
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(gpConfig->getBackgroundColor().red());
    greenString.setNum(gpConfig->getBackgroundColor().green());
    blueString.setNum(gpConfig->getBackgroundColor().blue());
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

    mpShowPopupHelpCheckBox = new QCheckBox(tr("Show Popup Help Messages"));
    mpShowPopupHelpCheckBox->setCheckable(true);


    mpAntiAliasingCheckBox = new QCheckBox(tr("Use Anti-Aliasing"));
    mpAntiAliasingCheckBox->setCheckable(true);

    mpInvertWheelCheckBox = new QCheckBox(tr("Invert Mouse Wheel"));
    mpInvertWheelCheckBox->setCheckable(true);

    mpSnappingCheckBox = new QCheckBox(tr("Auto Snap Components"));
    mpSnappingCheckBox->setCheckable(true);

    mpAutoSetPwdToMwdCheckBox = new QCheckBox(tr("Automtaically set HCOM working directory to model diretory"));
    mpAutoSetPwdToMwdCheckBox->setCheckable(true);

    mpInterfaceWidget = new QWidget(this);
    QGridLayout *pInterfaceLayout = new QGridLayout;
    pInterfaceLayout->addWidget(mpNativeStyleSheetCheckBox,    0, 0);
    pInterfaceLayout->addWidget(mpShowPopupHelpCheckBox,       2, 0);
    pInterfaceLayout->addWidget(mpInvertWheelCheckBox,         3, 0);
    pInterfaceLayout->addWidget(mpAntiAliasingCheckBox,        4, 0);
    pInterfaceLayout->addWidget(mpSnappingCheckBox,            5, 0);
    pInterfaceLayout->addWidget(mpAutoSetPwdToMwdCheckBox,     6, 0);
    pInterfaceLayout->addWidget(mpBackgroundColorLabel,        7, 0);
    pInterfaceLayout->addWidget(mpBackgroundColorButton,       7, 1);
    pInterfaceLayout->addWidget(new QWidget(),                 8, 0, 1, 2);
    pInterfaceLayout->setRowStretch(8,1);
    mpInterfaceWidget->setLayout(pInterfaceLayout);

        //Simulation Options
    mpEnableProgressBarCheckBox = new QCheckBox(tr("Enable Simulation Progress Bar"));
    mpEnableProgressBarCheckBox->setCheckable(true);

    QLabel *pProgressBarLabel = new QLabel(tr("Progress Bar Time Step [ms]"));
    pProgressBarLabel->setEnabled(gpConfig->getEnableProgressBar());
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
    QGridLayout *pSimulationLayout = new QGridLayout;
    pSimulationLayout->addWidget(mpEnableProgressBarCheckBox, 0, 0);
    pSimulationLayout->addWidget(pProgressBarLabel, 1, 0);
    pSimulationLayout->addWidget(mpProgressBarSpinBox, 1, 1);
    pSimulationLayout->addWidget(mpUseMulticoreCheckBox, 2, 0, 1, 2);
    pSimulationLayout->addWidget(mpThreadsLabel, 3, 0);
    pSimulationLayout->addWidget(mpThreadsSpinBox, 3, 1);
    pSimulationLayout->addWidget(new QWidget(), 4, 0, 1, 2);
    pSimulationLayout->setRowStretch(4, 1);
    //mpSimulationLayout->addWidget(mpThreadsWarningLabel, 4, 0, 1, 2);
    mpSimulationWidget->setLayout(pSimulationLayout);

    QLabel *pGenerationLimitLabel = new QLabel(tr("Limit number of plot generations to"));
    mpGenerationLimitSpinBox = new QSpinBox();
    mpGenerationLimitSpinBox->setMinimum(1);
    mpGenerationLimitSpinBox->setMaximum(5000000);
    mpGenerationLimitSpinBox->setSingleStep(1);

    mpAutoLimitGenerationsCheckBox = new QCheckBox("Autoremove last generation when limit is reached");
    mpCacheLogDataCeckBox = new QCheckBox("Cache log data on hard drive");
    mpShowHiddenNodeDataVarCheckBox = new QCheckBox("Show (and collect) hidden node data variables");
    mpPlotWindowsOnTop = new QCheckBox("Show plot windows on top of main window");

    mpUnitScaleWidget = new QWidget(this);
    mpUnitScaleWidget->setPalette(this->palette());
    QScrollArea *pUnitScaleScrollArea = new QScrollArea(this);
    pUnitScaleScrollArea->setWidgetResizable(true);
    pUnitScaleScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pUnitScaleScrollArea->setWidget(mpUnitScaleWidget);
    QVBoxLayout *pUnitScaleLayout = new QVBoxLayout(mpUnitScaleWidget);
    QStringList unitQuantities = gpConfig->getUnitQuantities();
    foreach (QString quantity, unitQuantities)
    {
        UnitScaleSelector *pUSS = new UnitScaleSelector(quantity, this);
        pUSS->updateUnitList();
        pUnitScaleLayout->addWidget(pUSS);
    }

    int r=0;
    mpPlottingWidget = new QWidget(this);
    QGridLayout *pPlottingLayout = new QGridLayout(mpPlottingWidget);
    pPlottingLayout->addWidget(mpCacheLogDataCeckBox,             r, 0, 1, 3);
    ++r;
    pPlottingLayout->addWidget(mpAutoLimitGenerationsCheckBox,    r, 0, 1, 3);
    ++r;
    pPlottingLayout->addWidget(pGenerationLimitLabel,             r, 0, 1, 3);
    pPlottingLayout->addWidget(mpGenerationLimitSpinBox,          r, 2, 1, 1);
    ++r;
    pPlottingLayout->addWidget(mpShowHiddenNodeDataVarCheckBox,   r, 0, 1, 3);
    ++r;
    pPlottingLayout->addWidget(mpPlotWindowsOnTop,                r, 0, 1, 3);
    ++r;
    pPlottingLayout->addWidget(new QWidget(),                     r, 0, 1, 3);
    pPlottingLayout->setRowStretch(10, 1);


#ifdef _WIN32
    QLabel *pCompiler32Label = new QLabel("32-bit GCC Compiler Path:");
    mpCompiler32LineEdit = new QLineEdit(this);
    QToolButton *pCompiler32Button = new QToolButton(this);
    pCompiler32Button->setIcon(QIcon(":graphics/uiicons/Hopsan-Open.png"));

    mpCompiler32WarningLabel = new QLabel(this);
    mpCompiler32WarningLabel->setText("<font color='red'>Warning! GCC compiler not found in specified location!</font>");

    QLabel *pCompiler64Label = new QLabel("64-bit GCC Compiler Path:");
    mpCompiler64LineEdit = new QLineEdit(this);
    QToolButton *pCompiler64Button = new QToolButton(this);
    pCompiler64Button->setIcon(QIcon(":graphics/uiicons/Hopsan-Open.png"));

    mpCompiler64WarningLabel = new QLabel(this);
    mpCompiler64WarningLabel->setText("<font color='red'>Warning! GCC compiler not found in specified location!</font>");


    QWidget *pCompilersWidget = new QWidget(this);
    QGridLayout *pCompilersLayout = new QGridLayout(pCompilersWidget);

    int row=-1;
    pCompilersLayout->addWidget(pCompiler32Label,                      ++row,0,1,1);
    pCompilersLayout->addWidget(mpCompiler32LineEdit,                    row,1,1,1);
    pCompilersLayout->addWidget(pCompiler32Button,                       row,2,1,1);
    pCompilersLayout->addWidget(mpCompiler32WarningLabel,              ++row,0,1,3);
    pCompilersLayout->addWidget(pCompiler64Label,                      ++row,0,1,1);
    pCompilersLayout->addWidget(mpCompiler64LineEdit,                    row,1,1,1);
    pCompilersLayout->addWidget(pCompiler64Button,                       row,2,1,1);
    pCompilersLayout->addWidget(mpCompiler64WarningLabel,              ++row,0,1,3);
    pCompilersLayout->addWidget(new QWidget(this),                     ++row,0,1,3);
    pCompilersLayout->setRowStretch(row,1);

    setCompilerPath(gpConfig->getGcc32Dir(), false);
    setCompilerPath(gpConfig->getGcc64Dir(), true);

    connect(pCompiler32Button, SIGNAL(clicked()), this, SLOT(setCompiler32Path()));
    connect(mpCompiler32LineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompiler32Path(QString)));
    connect(pCompiler64Button, SIGNAL(clicked()), this, SLOT(setCompiler64Path()));
    connect(mpCompiler64LineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompiler64Path(QString)));
#endif

    QWidget *pRemoteHopsanSettingsWidget = new QWidget();
    QGridLayout *pRemoteHopsanSettingsLayout = new QGridLayout(pRemoteHopsanSettingsWidget);
    QLabel *pRemoteHopsanAddressLabel = new QLabel("RemoteHopsanAddress: ");
    QLabel *pRemoteHopsanDispatchAddressLabel = new QLabel("RemoteHopsanDispatchAddress: ");
    QLabel *pUseRemoteHopsanDispatchLabel = new QLabel("Use remote Hopsan dispatch server: ");
    mpUseRemoteHopsanDispatch = new QCheckBox();
    mpRemoteHopsanAddress = new QLineEdit();
    mpRemoteHopsanDispatchAddress = new QLineEdit();
    pRemoteHopsanSettingsLayout->addWidget(pRemoteHopsanAddressLabel, 0, 0);
    pRemoteHopsanSettingsLayout->addWidget(mpRemoteHopsanAddress, 0, 1);
    pRemoteHopsanSettingsLayout->addWidget(pRemoteHopsanDispatchAddressLabel, 1, 0);
    pRemoteHopsanSettingsLayout->addWidget(mpRemoteHopsanDispatchAddress, 1, 1);
    pRemoteHopsanSettingsLayout->addWidget(pUseRemoteHopsanDispatchLabel, 2, 0);
    pRemoteHopsanSettingsLayout->addWidget(mpUseRemoteHopsanDispatch, 2, 1);

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

    connect(mpEnableProgressBarCheckBox,    SIGNAL(toggled(bool)),  pProgressBarLabel,     SLOT(setEnabled(bool)));
    connect(mpEnableProgressBarCheckBox,    SIGNAL(toggled(bool)),  mpProgressBarSpinBox,   SLOT(setEnabled(bool)));
    connect(mpBackgroundColorButton,        SIGNAL(clicked()),      this,                   SLOT(colorDialog()));
    connect(mpResetButton,                  SIGNAL(clicked()),      this,                   SLOT(resetConfigDefaults()));
    connect(mpOpenXmlButton,                SIGNAL(clicked()),      this,                   SLOT(openConfigFile()));
    connect(mpCancelButton,                 SIGNAL(clicked()),      this,                   SLOT(reject()));
    connect(mpOkButton,                     SIGNAL(clicked()),      this,                   SLOT(setValues()));

    connect(mpUseMulticoreCheckBox,         SIGNAL(toggled(bool)),  mpThreadsLabel,         SLOT(setEnabled(bool)));
    connect(mpUseMulticoreCheckBox,         SIGNAL(toggled(bool)),  mpThreadsSpinBox,       SLOT(setEnabled(bool)));

    QTabWidget *pTabWidget = new QTabWidget(this);
    pTabWidget->addTab(mpInterfaceWidget, "Interface");
    pTabWidget->addTab(mpSimulationWidget, "Simulation");
    pTabWidget->addTab(pUnitScaleScrollArea, "Unit Scaling");
    pTabWidget->addTab(mpPlottingWidget, "Plotting");
#ifdef _WIN32
    pTabWidget->addTab(pCompilersWidget, "Compilers");
#endif
    pTabWidget->addTab(pRemoteHopsanSettingsWidget, "Remote Hopsan");

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
void OptionsDialog::resetConfigDefaults()
{
    QMessageBox resetWarningBox(QMessageBox::Warning, tr("Warning"),tr("This will reset ALL settings to default values. Do you want to continue?"), 0, 0);
    resetWarningBox.addButton(QMessageBox::Yes);
    resetWarningBox.addButton(QMessageBox::No);
    resetWarningBox.setWindowIcon(gpMainWindowWidget->windowIcon());

    int rc = resetWarningBox.exec();
    if(rc == QMessageBox::Yes)
    {
        gpConfig->loadDefaultsFromXml();
        gpConfig->saveToXml();
        show();
    }
}


//! @brief Opens settings XML file outside Hopsan with default application
void OptionsDialog::openConfigFile()
{
    qDebug() << "Opening: " << gpDesktopHandler->getConfigPath() + QString("hopsanconfig.xml");
    QDesktopServices::openUrl(QUrl("file:///"+gpDesktopHandler->getConfigPath() + QString("hopsanconfig.xml")));
}


//! Slot that updates and saves the settings based on the choices made in the dialog box
void OptionsDialog::setValues()
{
    gpConfig->setShowPopupHelp(mpShowPopupHelpCheckBox->isChecked());
    gpConfig->setUseNativeStyleSheet(mpNativeStyleSheetCheckBox->isChecked());

    if(gpConfig->getUseNativeStyleSheet())
    {
        gpMainWindowWidget->setStyleSheet((" "));
        QMainWindow dummy;
        gpMainWindowWidget->setPalette(dummy.palette());
        this->setPalette(dummy.palette());
    }
    else
    {
        gpMainWindowWidget->setStyleSheet(gpConfig->getStyleSheet());
        gpMainWindowWidget->setPalette(gpConfig->getPalette());
        this->setPalette(gpConfig->getPalette());
    }
    emit paletteChanged();
    gpConfig->setInvertWheel(mpInvertWheelCheckBox->isChecked());
    gpConfig->setAntiAliasing(mpAntiAliasingCheckBox->isChecked());
    gpConfig->setSnapping(mpSnappingCheckBox->isChecked());
    gpConfig->setAutoSetPwdToMwd(mpAutoSetPwdToMwdCheckBox->isChecked());
    for(int i=0; i<gpModelHandler->count(); ++i)
    {
        gpModelHandler->getModel(i)->getGraphicsView()->setRenderHint(QPainter::Antialiasing, gpConfig->getAntiAliasing());
    }
    gpConfig->setBackgroundColor(mPickedBackgroundColor);
    for(int i=0; i<gpModelHandler->count(); ++i)
    {
        gpModelHandler->getModel(i)->getGraphicsView()->updateViewPort();
    }
    gpConfig->setEnableProgressBar(mpEnableProgressBarCheckBox->isChecked());
    gpConfig->setProgressBarStep(mpProgressBarSpinBox->value());
    gpConfig->setUseMultiCore(mpUseMulticoreCheckBox->isChecked());
    gpConfig->setNumberOfThreads(mpThreadsSpinBox->value());
    gpConfig->setAutoLimitLogDataGenerations(mpAutoLimitGenerationsCheckBox->isChecked());
    gpConfig->setShowHiddenNodeDataVariables(mpShowHiddenNodeDataVarCheckBox->isChecked());
    gpConfig->setPlotWindowsOnTop(mpPlotWindowsOnTop->isChecked());
    gpConfig->setGenerationLimit(mpGenerationLimitSpinBox->value());
    gpConfig->setCacheLogData(mpCacheLogDataCeckBox->isChecked());
    for(int i=0; i<gpModelHandler->count(); ++i)       //Loop through all containers and reduce their plot data
    {
        gpModelHandler->getViewContainerObject(i)->getLogDataHandler()->limitPlotGenerations();
    }

    // Set default units
    QObjectList unitselectors =  mpUnitScaleWidget->children();
    for (int i=0; i<unitselectors.size(); ++i)
    {
        UnitScaleSelector *pSelector = qobject_cast<UnitScaleSelector*>(unitselectors[i]);
        if (pSelector)
        {
            gpConfig->setDefaultUnit(pSelector->getQuantity(), pSelector->getDefaultUnit());
        }
    }

    gpConfig->setGcc32Dir(mpCompiler32LineEdit->text());
    gpConfig->setGcc64Dir(mpCompiler64LineEdit->text());

    gpConfig->setRemoteHopsanAddress(mpRemoteHopsanAddress->text());
    gpConfig->setRemoteHopsanDispatchAddress(mpRemoteHopsanDispatchAddress->text());
    gpConfig->setUseRemoteHopsanDispatch(mpUseRemoteHopsanDispatch->isChecked());

    gpConfig->saveToXml();

    this->accept();
}


//! Slot that opens a color dialog where user can select a background color
void OptionsDialog::colorDialog()
{
    mPickedBackgroundColor = QColorDialog::getColor(gpConfig->getBackgroundColor(), this);
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
        mPickedBackgroundColor = gpConfig->getBackgroundColor();
    }
}


//! Reimplementation of show() slot. This is used to make sure that the background color button resets its color if the cancel button was pressed last time options were opened.
void OptionsDialog::show()
{
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(gpConfig->getBackgroundColor().red());
    greenString.setNum(gpConfig->getBackgroundColor().green());
    blueString.setNum(gpConfig->getBackgroundColor().blue());
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
    mPickedBackgroundColor = gpConfig->getBackgroundColor();

    mpNativeStyleSheetCheckBox->setChecked(gpConfig->getUseNativeStyleSheet());
    mpShowPopupHelpCheckBox->setChecked(gpConfig->getShowPopupHelp());
    mpAntiAliasingCheckBox->setChecked(gpConfig->getAntiAliasing());
    mpInvertWheelCheckBox->setChecked(gpConfig->getInvertWheel());
    mpSnappingCheckBox->setChecked(gpConfig->getSnapping());
    mpAutoSetPwdToMwdCheckBox->setChecked(gpConfig->getAutoSetPwdToMwd());
    mpEnableProgressBarCheckBox->setChecked(gpConfig->getEnableProgressBar());
    mpProgressBarSpinBox->setValue(gpConfig->getProgressBarStep());
    mpProgressBarSpinBox->setEnabled(gpConfig->getEnableProgressBar());
    mpThreadsSpinBox->setEnabled(gpConfig->getUseMulticore());
    mpUseMulticoreCheckBox->setChecked(gpConfig->getUseMulticore());
    mpThreadsSpinBox->setValue(gpConfig->getNumberOfThreads());
    mpThreadsLabel->setEnabled(gpConfig->getUseMulticore());
    mpGenerationLimitSpinBox->setValue(gpConfig->getGenerationLimit());
    mpAutoLimitGenerationsCheckBox->setChecked(gpConfig->getAutoLimitLogDataGenerations());
    mpShowHiddenNodeDataVarCheckBox->setChecked(gpConfig->getShowHiddenNodeDataVariables());
    mpPlotWindowsOnTop->setChecked(gpConfig->getPlotWindowsOnTop());
    mpCacheLogDataCeckBox->setChecked(gpConfig->getCacheLogData());

    mpRemoteHopsanAddress->setText(gpConfig->getRemoteHopsanAddress());
    mpRemoteHopsanDispatchAddress->setText(gpConfig->getRemoteHopsanDispatchAddress());
    mpUseRemoteHopsanDispatch->setChecked(gpConfig->getUseRemoteHopsanDispatch());

    // Update units scale lists
    QObjectList unitselectors =  mpUnitScaleWidget->children();
    for (int i=0; i<unitselectors.size(); ++i)
    {
        UnitScaleSelector *pSelector = qobject_cast<UnitScaleSelector*>(unitselectors[i]);
        if (pSelector)
        {
            pSelector->updateUnitList();
        }
    }

    QDialog::show();
}




void OptionsDialog::setCompiler32Path()
{
    QString path = QFileDialog::getExistingDirectory(this, "Set Compiler Path:", gpConfig->getGcc32Dir());

    if(path.isEmpty()) return;

    setCompiler32Path(path);
}


void OptionsDialog::setCompiler32Path(QString path)
{
    setCompilerPath(path, false);
}


void OptionsDialog::setCompiler64Path()
{
    QString path = QFileDialog::getExistingDirectory(this, "Set Compiler Path:", gpConfig->getGcc64Dir());

    if(path.isEmpty()) return;

    setCompiler64Path(path);
}


void OptionsDialog::setCompiler64Path(QString path)
{
    setCompilerPath(path, true);
}


void OptionsDialog::setCompilerPath(QString path, bool x64)
{
    bool exists;
#ifdef linux
    if(path.endsWith("/gcc"))
    {
        path.chop(4);
    }
    exists = QFile::exists(path+"/gcc");
#else
    if(path.endsWith("/g++.exe") || path.endsWith("\\g++.exe"))
    {
        path.chop(8);
    }
    exists = QFile::exists(path+"/gcc.exe");
#endif
    //! @todo We should also check that it is the correct version of gcc!


    if(x64)
    {
        if(!mpCompiler64LineEdit->hasFocus())
        {
            disconnect(mpCompiler64LineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompiler64Path(QString)));
            mpCompiler64LineEdit->setText(path);
            connect(mpCompiler64LineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompiler64Path(QString)));
        }

        mpCompiler64WarningLabel->setVisible(!exists);
    }
    else
    {
        if(!mpCompiler32LineEdit->hasFocus())
        {
            disconnect(mpCompiler32LineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompiler32Path(QString)));
            mpCompiler32LineEdit->setText(path);
            connect(mpCompiler32LineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompiler32Path(QString)));
        }

        mpCompiler32WarningLabel->setVisible(!exists);
    }
}
