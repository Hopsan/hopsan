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

//! @brief Constructor for the container properties dialog
//! @param[in] pContainerObject Pointer to the container
//! @param[in] pParentWidget Pointer to the parent widget
ContainerPropertiesDialog::ContainerPropertiesDialog(GUIContainerObject *pContainerObject, QWidget *pParentWidget)
    : QDialog(pParentWidget)
{
    mpContainerObject = pContainerObject;

        //Set the name and size of the main window
    this->setObjectName("SystemPropertiesDialog");
    this->resize(640,480);
    this->setWindowTitle("System Properties");
    this->setPalette(gConfig.getPalette());

    //Define items in the dialog box
        //Name edit
    QHBoxLayout *pNameLayout = new QHBoxLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    mpNameEdit = new QLineEdit(pContainerObject->getName(), this);

        //Icon paths
    QHBoxLayout *pUserIconLayout = new QHBoxLayout();
    QHBoxLayout *pIsoIconLayout = new QHBoxLayout();
    QLabel *pUserIconLabel = new QLabel("User Icon Path:", this);
    QLabel *pIsoIconLabel = new QLabel( "ISO Icon Path:", this);
    mpUserIconPath = new QLineEdit(mpContainerObject->getUserIconPath(), this);
    mpIsoIconPath = new QLineEdit(mpContainerObject->getIsoIconPath(), this);
    mpUserIconPath->setMinimumWidth(200);
    QPushButton *pIsoIconBrowseButton = new QPushButton(tr("..."), this);
    QPushButton *pUserIconBrowseButton = new QPushButton(tr("..."), this);
    pIsoIconBrowseButton->setFixedSize(25, 22);
    pUserIconBrowseButton->setFixedSize(25, 22);
    pIsoIconBrowseButton->setAutoDefault(false);
    pUserIconBrowseButton->setAutoDefault(false);

        //Appearance Group Box
    QGroupBox *pAppearanceGroupBox = new QGroupBox("Appearance", this);
    QGridLayout *pAppearanceLayout = new QGridLayout(this);
    pAppearanceLayout->addWidget(pNameLabel, 0, 0);
    pAppearanceLayout->addWidget(mpNameEdit, 0, 1, 1, 2);
    pAppearanceLayout->addWidget(pUserIconLabel, 1, 0);
    pAppearanceLayout->addWidget(mpUserIconPath, 1, 1);
    pAppearanceLayout->addWidget(pUserIconBrowseButton, 1, 2);
    pAppearanceLayout->addWidget(pIsoIconLabel, 2, 0);
    pAppearanceLayout->addWidget(mpIsoIconPath, 2, 1);
    pAppearanceLayout->addWidget(pIsoIconBrowseButton, 2, 2);
    pAppearanceGroupBox->setLayout(pAppearanceLayout);

        //Graphics checkboxes
    mpIsoCheckBox = new QCheckBox(tr("Use ISO 1219 Graphics"), this);
    mpIsoCheckBox->setCheckable(true);
    mpIsoCheckBox->setChecked(mpContainerObject->mGfxType);

        //Undo checkbox
    mpDisableUndoCheckBox = new QCheckBox(tr("Disable Undo Function"), this);
    mpDisableUndoCheckBox->setCheckable(true);
    mpDisableUndoCheckBox->setChecked(mpContainerObject->mUndoDisabled);

        //Settings Group Box
    QGroupBox *pSettingsGroupBox = new QGroupBox("Settings", this);
    QGridLayout *pSettingsLayout = new QGridLayout(this);
    pSettingsLayout->addWidget(mpIsoCheckBox, 0, 0, 1, 2);
    pSettingsLayout->addWidget(mpDisableUndoCheckBox, 1, 0, 1, 2);
    pSettingsGroupBox->setLayout(pSettingsLayout);

        //Set GuiSystem specific stuff
    if (mpContainerObject->type() == GUISYSTEM)
    {
            //Log sampels
        QHBoxLayout *pNSamplesLayout = new QHBoxLayout();
        QLabel *pNSamplesLabel = new QLabel(tr("Number of Log Samples:"), this);
        pNSamplesLabel->setEnabled(true);
        QString NSamplesText;
        NSamplesText.setNum(mpContainerObject->getNumberOfLogSamples()); //!< @todo what if group
        mpNSamplesEdit = new QLineEdit(this);
        mpNSamplesEdit->setValidator(new QIntValidator(0, 2000000000, this));
        mpNSamplesEdit->setText(NSamplesText);
        pSettingsLayout->addWidget(pNSamplesLabel, 2, 0);
        pSettingsLayout->addWidget(mpNSamplesEdit, 2, 1);

            //CQS Type
        QHBoxLayout *pCQSLayout = new QHBoxLayout();
        QLabel *pCQSLabel = new QLabel("CQS-type: ", this);
        mpCQSLineEdit = new QLineEdit(pContainerObject->getTypeCQS(), this);
        pSettingsLayout->addWidget(pCQSLabel, 3, 0);
        pSettingsLayout->addWidget(mpCQSLineEdit, 3, 1);
    }


        //This is the main Vertical layout of the dialog
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->setSizeConstraint(QLayout::SetFixedSize);
    pMainLayout->addWidget(pAppearanceGroupBox);
    pMainLayout->addWidget(pSettingsGroupBox);

        //Done and Cancel Buttons
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pDoneButton = new QPushButton(tr("&Done"), this);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::ActionRole);
    pCancelButton->setAutoDefault(false);
    pDoneButton->setAutoDefault(true);
    pMainLayout->addWidget(pButtonBox, 0, Qt::AlignHCenter);

    //Create connections
    connect(pCancelButton,         SIGNAL(clicked()), this, SLOT(close()));
    connect(pDoneButton,           SIGNAL(clicked()), this, SLOT(setValues()));
    connect(pIsoIconBrowseButton,  SIGNAL(clicked()), this, SLOT(browseIso()));
    connect(pUserIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseUser()));
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
    if ( (mpContainerObject->getIsoIconPath() != mpIsoIconPath->text()) || (mpContainerObject->getUserIconPath() != mpUserIconPath->text()) )
    {
        //mpContainerObject->setUserIconPath(mpUserIconPath->text());
        //mpContainerObject->setIsoIconPath(mpIsoIconPath->text());
        //! @todo in the future we should be able to set iso and user icons with different paths separately, right now only user icon is available only try iso if user empty
        QString newpath;
        if (mpUserIconPath->text().isEmpty())
        {
            newpath = mpIsoIconPath->text();
        }
        else
        {
            newpath = mpUserIconPath->text();
        }

        mpContainerObject->setIsoIconPath("");
        mpContainerObject->setUserIconPath(newpath);

        mpContainerObject->refreshAppearance();
    }

    //Set GuiSystem specific stuff
    if (mpContainerObject->type() == GUISYSTEM)
    {
        mpContainerObject->setNumberOfLogSamples(mpNSamplesEdit->text().toInt());
        mpContainerObject->setTypeCQS(this->mpCQSLineEdit->text());
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
