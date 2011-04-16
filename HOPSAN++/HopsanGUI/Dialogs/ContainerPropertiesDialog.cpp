//!
//! @file   ContainerPropertiesDialog.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-24
//!
//! @brief Contains a class for manimulation of Container properties
//!
//$Id$

#include "ContainerPropertiesDialog.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QDialogButtonBox>

#include "../GUIObjects/GUIContainerObject.h"
#include "../GUIObjects/GUISystem.h"
#include "../MainWindow.h"
#include "../Widgets/LibraryWidget.h"
#include "../Configuration.h"
#include "../Widgets/ProjectTabWidget.h"

//! @brief Constructor for the container properties dialog
//! @param[in] pContainerObject Pointer to the container
//! @param[in] pParentWidget Pointer to the parent widget
ContainerPropertiesDialog::ContainerPropertiesDialog(GUIContainerObject *pContainerObject, QWidget *pParentWidget)
    : QDialog(pParentWidget)
{
    mpContainerObject = pContainerObject;

        //Set the name and size of the main window
    this->setObjectName("SystemPropertiesDialog");
    this->resize(380,200);
    this->setWindowTitle("System Properties");
    this->setPalette(gConfig.getPalette());

    //Define items in the dialog box
        //Name edit
    mpNameLayout = new QHBoxLayout();
    mpNameLabel = new QLabel("Name: ", this);
    mpNameEdit = new QLineEdit(mpContainerObject->getName(), this);

       //Icon paths
    mpUserIconLabel = new QLabel("User Icon Path:", this);
    mpIsoIconLabel = new QLabel( "ISO Icon Path:", this);
    mpUserIconPath = new QLineEdit(mpContainerObject->getIconPath(USERGRAPHICS), this);
    mpIsoIconPath = new QLineEdit(mpContainerObject->getIconPath(ISOGRAPHICS), this);
    mpUserIconLabel->setMinimumWidth(100);
    mpUserIconPath->setFixedWidth(200);
    mpIsoIconBrowseButton = new QPushButton(tr("..."), this);
    mpUserIconBrowseButton = new QPushButton(tr("..."), this);
//    mpIsoIconBrowseButton->setFixedSize(25, 22);
//    mpUserIconBrowseButton->setFixedSize(25, 22);
    mpIsoIconBrowseButton->setAutoDefault(false);
    mpUserIconBrowseButton->setAutoDefault(false);

        //Appearance Group Box
    mpAppearanceGroupBox = new QGroupBox("Appearance", this);
    mpAppearanceLayout = new QGridLayout(this);
    mpAppearanceLayout->addWidget(mpNameLabel, 0, 0);
    mpAppearanceLayout->addWidget(mpNameEdit, 0, 1, 1, 2);
    mpAppearanceLayout->addWidget(mpUserIconLabel, 1, 0);
    mpAppearanceLayout->addWidget(mpUserIconPath, 1, 1);
    mpAppearanceLayout->addWidget(mpUserIconBrowseButton, 1, 2);
    mpAppearanceLayout->addWidget(mpIsoIconLabel, 2, 0);
    mpAppearanceLayout->addWidget(mpIsoIconPath, 2, 1);
    mpAppearanceLayout->addWidget(mpIsoIconBrowseButton, 2, 2);
    mpAppearanceGroupBox->setLayout(mpAppearanceLayout);

        //Graphics checkboxes
    mpIsoCheckBox = new QCheckBox(tr("Use ISO 1219 Graphics"), this);
    mpIsoCheckBox->setCheckable(true);
    mpIsoCheckBox->setChecked(mpContainerObject->mGfxType);

        //Undo checkbox
    mpDisableUndoCheckBox = new QCheckBox(tr("Disable Undo Function"), this);
    mpDisableUndoCheckBox->setCheckable(true);
    mpDisableUndoCheckBox->setChecked(mpContainerObject->mUndoDisabled);

    //! @todo Make the script path do something
    mpPyScriptLabel = new QLabel("Script File:", this);
    mpPyScriptPath = new QLineEdit(this);
    mpPyScriptLabel->setFixedWidth(100);
    mpPyScriptPath->setFixedWidth(200);
    mpPyScriptBrowseButton = new QPushButton(tr("..."), this);

        //Settings Group Box
    mpSettingsGroupBox = new QGroupBox("Settings", this);
    mpSettingsLayout = new QGridLayout(this);
    mpSettingsLayout->addWidget(mpPyScriptLabel, 0, 0);
    mpSettingsLayout->addWidget(mpPyScriptPath, 0, 1);
    mpSettingsLayout->addWidget(mpPyScriptBrowseButton, 0, 2);
    mpSettingsLayout->addWidget(mpIsoCheckBox, 1, 0, 1, 2);
    mpSettingsLayout->addWidget(mpDisableUndoCheckBox, 2, 0, 1, 2);
    mpSettingsGroupBox->setLayout(mpSettingsLayout);

        //Set GuiSystem specific stuff
    if (mpContainerObject->type() == GUISYSTEM)
    {
            //Log sampels
        mpNSamplesLayout = new QHBoxLayout();
        mpNSamplesLabel = new QLabel(tr("Log Samples:"), this);
        mpNSamplesLabel->setEnabled(true);
        QString NSamplesText;
        NSamplesText.setNum(mpContainerObject->getNumberOfLogSamples()); //!< @todo what if group
        mpNSamplesEdit = new QLineEdit(this);
        mpNSamplesEdit->setValidator(new QIntValidator(0, 2000000000, this));
        mpNSamplesEdit->setText(NSamplesText);
        mpSettingsLayout->addWidget(mpNSamplesLabel, 3, 0);
        mpSettingsLayout->addWidget(mpNSamplesEdit, 3, 1);

            //CQS Type
        mpCQSLayout = new QHBoxLayout();
        mpCQSLabel = new QLabel("CQS-type: ", this);
        mpCQSTypeLabel = new QLabel(mpContainerObject->getTypeCQS(), this);
        mpSettingsLayout->addWidget(mpCQSLabel, 4, 0);
        mpSettingsLayout->addWidget(mpCQSTypeLabel, 4, 1);

        mpPyScriptPath->setText(mpContainerObject->getScriptFile());
    }


        //System Parameters Group Box
    if(mpContainerObject != gpMainWindow->mpProjectTabs->getCurrentContainer() && !mpContainerObject->getCoreSystemAccessPtr()->getSystemParametersMap().isEmpty())
    {
        mpSystemParametersGroupBox = new QGroupBox("System Parameters", this);
        mpSystemParametersLayout = new QGridLayout(this);

        QMap<std::string, double>::iterator it;
        QMap<std::string, double> tempMap;
        tempMap = mpContainerObject->getCoreSystemAccessPtr()->getSystemParametersMap();
        int row = 0;
        for(it=tempMap.begin(); it!=tempMap.end(); ++it)
        {
            QLabel *pParameterLabel = new QLabel(QString(it.key().c_str()), this);
            pParameterLabel->setMinimumWidth(100);
            QString numStr;
            numStr.setNum(it.value());
            QLineEdit *pParameterLineEdit = new QLineEdit(numStr, this);
            pParameterLineEdit->setValidator(new QDoubleValidator(this));
            mpSystemParametersLayout->addWidget(pParameterLabel, row, 0);
            mpSystemParametersLayout->addWidget(pParameterLineEdit, row, 1);
            mSystemParameterLabels.append(pParameterLabel);
            mSystemParameterLineEdits.append(pParameterLineEdit);
            ++row;
        }
        mpSystemParametersGroupBox->setLayout(mpSystemParametersLayout);
    }

        //This is the main Vertical layout of the dialog
    mpMainLayout = new QVBoxLayout(this);
    mpMainLayout->addWidget(mpAppearanceGroupBox);
    mpMainLayout->addWidget(mpSettingsGroupBox);
    if(mpContainerObject != gpMainWindow->mpProjectTabs->getCurrentContainer() && !mpContainerObject->getCoreSystemAccessPtr()->getSystemParametersMap().isEmpty())
    {
        mpMainLayout->addWidget(mpSystemParametersGroupBox);
    }

        //Done and Cancel Buttons
    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpDoneButton = new QPushButton(tr("&Done"), this);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpDoneButton, QDialogButtonBox::ActionRole);
    mpCancelButton->setAutoDefault(false);
    mpDoneButton->setAutoDefault(true);
    mpMainLayout->addWidget(mpButtonBox, 0, Qt::AlignHCenter);

    //Create connections
    connect(mpCancelButton,         SIGNAL(clicked()), this, SLOT(close()));
    connect(mpDoneButton,           SIGNAL(clicked()), this, SLOT(setValues()));
    connect(mpIsoIconBrowseButton,  SIGNAL(clicked()), this, SLOT(browseIso()));
    connect(mpUserIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseUser()));
    connect(mpPyScriptBrowseButton, SIGNAL(clicked()), this, SLOT(browseScript()));
}


//! @brief Updates model settings according to the selected values
void ContainerPropertiesDialog::setValues()
{
    this->mpContainerObject->setName(this->mpNameEdit->text());

    if(mpIsoCheckBox->isChecked())
    {
        this->mpContainerObject->setGfxType(ISOGRAPHICS);
        gpMainWindow->mpLibrary->setGfxType(ISOGRAPHICS);
    }
    else
    {
        this->mpContainerObject->setGfxType(USERGRAPHICS);
        gpMainWindow->mpLibrary->setGfxType(USERGRAPHICS);
    }

    if( (mpDisableUndoCheckBox->isChecked()) != (mpContainerObject->mUndoDisabled) )
    {
        mpContainerObject->disableUndo();
        mpDisableUndoCheckBox->setChecked(false);
    }

    //Set the icon paths, only update and refresh appearance if a change has occured
    if ( mpContainerObject->getIconPath(ISOGRAPHICS) != mpIsoIconPath->text() )
    {
        mpContainerObject->setIconPath(mpIsoIconPath->text(), ISOGRAPHICS);
        mpContainerObject->refreshAppearance();
    }
    if ( mpContainerObject->getIconPath(USERGRAPHICS) != mpUserIconPath->text() )
    {
        mpContainerObject->setIconPath(mpUserIconPath->text(), USERGRAPHICS);
        mpContainerObject->refreshAppearance();
    }

    //Set GuiSystem specific stuff
    if (mpContainerObject->type() == GUISYSTEM)
    {
        mpContainerObject->setNumberOfLogSamples(mpNSamplesEdit->text().toInt());
        mpContainerObject->setScriptFile(mpPyScriptPath->text());
    }

    for(int i=0; i<mSystemParameterLabels.size(); ++i)
    {
        mpContainerObject->getCoreSystemAccessPtr()->setSystemParameter(mSystemParameterLabels.at(i)->text(), mSystemParameterLineEdits.at(i)->text().toDouble());
    }

    this->done(0);
}


//! @brief Slot that opens a file dialog where user can select a user icon for the system
void ContainerPropertiesDialog::browseUser()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose user icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    if (!modelFileName.isEmpty())
    {
        mpUserIconPath->setText(modelFileName);
    }
}


//! @brief Slot that opens a file dialog where user can select an iso icon for the system
void ContainerPropertiesDialog::browseIso()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose ISO icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    if (!modelFileName.isEmpty())
    {
        mpIsoIconPath->setText(modelFileName);
    }
}


//! @brief Slot that opens a file dialog where user can select a script file for the system
void ContainerPropertiesDialog::browseScript()
{
    QDir fileDialogOpenDir;
    QString scriptFileName = QFileDialog::getOpenFileName(this, tr("Choose ISO icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    if (!scriptFileName.isEmpty())
    {
        mpPyScriptPath->setText(scriptFileName);
    }
}
