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
#include <QHBoxLayout>
#include <QDialogButtonBox>

#include "../GUIObjects/GUIContainerObject.h"
#include "../GUIObjects/GUISystem.h"
#include "../MainWindow.h"
#include "../Widgets/LibraryWidget.h"

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

        //This is the main Vertical layout of the dialog
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

    //Define items in the dialog box
        //Name edit
    QHBoxLayout *pNameLayout = new QHBoxLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    mpNameEdit = new QLineEdit(pContainerObject->getName(), this);
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);
    pMainLayout->addLayout(pNameLayout);

        //Icon paths
    QLabel *pIconLabel = new QLabel("Icon Paths:", this);
    pMainLayout->addWidget(pIconLabel);
    QHBoxLayout *pUserIconLayout = new QHBoxLayout();
    QHBoxLayout *pIsoIconLayout = new QHBoxLayout();
    QLabel *pUserIconLabel = new QLabel("User:", this);
    QLabel *pIsoIconLabel = new QLabel( "ISO:", this);
    mpUserIconPath = new QLineEdit(mpContainerObject->getUserIconPath(), this);
    mpIsoIconPath = new QLineEdit(mpContainerObject->getIsoIconPath(), this);
    QPushButton *pIsoIconBrowseButton = new QPushButton(tr("..."), this);
    QPushButton *pUserIconBrowseButton = new QPushButton(tr("..."), this);
    pIsoIconBrowseButton->setFixedSize(25, 22);
    pUserIconBrowseButton->setFixedSize(25, 22);
    pIsoIconBrowseButton->setAutoDefault(false);
    pUserIconBrowseButton->setAutoDefault(false);

    pUserIconLayout->addWidget(pUserIconLabel);
    pUserIconLayout->addWidget(mpUserIconPath);
    pUserIconLayout->addWidget(pUserIconBrowseButton, 0, Qt::AlignRight);
    pMainLayout->addLayout(pUserIconLayout);

    pIsoIconLayout->addWidget(pIsoIconLabel);
    pIsoIconLayout->addWidget(mpIsoIconPath);
    pIsoIconLayout->addWidget(pIsoIconBrowseButton, 0, Qt::AlignRight);
    pMainLayout->addLayout(pIsoIconLayout);

        //Graphics checkboxes
    mpIsoCheckBox = new QCheckBox(tr("Use ISO 1219 Graphics"), this);
    mpIsoCheckBox->setCheckable(true);
    mpIsoCheckBox->setChecked(mpContainerObject->mGfxType);

        //Undo checkbox
    mpDisableUndoCheckBox = new QCheckBox(tr("Disable Undo Function"), this);
    mpDisableUndoCheckBox->setCheckable(true);
    mpDisableUndoCheckBox->setChecked(mpContainerObject->mUndoDisabled);

    pMainLayout->addWidget(mpIsoCheckBox);
    pMainLayout->addWidget(mpDisableUndoCheckBox);

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
        pNSamplesLayout->addWidget(pNSamplesLabel);
        pNSamplesLayout->addWidget(mpNSamplesEdit);
        pMainLayout->addLayout(pNSamplesLayout);

            //CQS Type
        QHBoxLayout *pCQSLayout = new QHBoxLayout();
        QLabel *pCQSLabel = new QLabel("CQS: ", this);
        mpCQSLineEdit = new QLineEdit(pContainerObject->getTypeCQS(), this);
        pCQSLayout->addWidget(pCQSLabel);
        pCQSLayout->addWidget(mpCQSLineEdit);
        pMainLayout->addLayout(pCQSLayout);
    }

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
    connect(pCancelButton,         SIGNAL(pressed()), this, SLOT(close()));
    connect(pDoneButton,           SIGNAL(pressed()), this, SLOT(setValues()));
    connect(pIsoIconBrowseButton,  SIGNAL(clicked()), this, SLOT(browseIso()));
    connect(pUserIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseUser()));

    setLayout(pMainLayout); //Is this really necessary as pMainLayout has this as parent
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
