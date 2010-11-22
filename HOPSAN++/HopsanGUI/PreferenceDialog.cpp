//!
//! @file   PreferenceDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the model preferences dialog
//!
//$Id$

//! @todo Rename this class and file to PreferencesDialog

#include <QtGui>
#include <QDebug>

#include "PreferenceDialog.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GUIObjects/GUISystem.h"
#include "LibraryWidget.h"
#include "Configuration.h"


//! @brief Constructor for Model Preferences dialog
//! @param parent Pointer to the main window
PreferenceDialog::PreferenceDialog(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;

        //Set the name and size of the main window
    this->setObjectName("PreferenceDialog");
    this->resize(640,480);
    this->setWindowTitle("Model Preferences");

        //Define items in the dialog box
    mpIsoCheckBox = new QCheckBox(tr("Use ISO 1219 Graphics"));
    mpIsoCheckBox->setCheckable(true);
    mpIsoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);

    mpDisableUndoCheckBox = new QCheckBox(tr("Disable Undo Function"));
    mpDisableUndoCheckBox->setCheckable(true);
    mpDisableUndoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);

    mpNumberOfSamplesLabel = new QLabel(tr("Number of Log Samples"));
    mpNumberOfSamplesLabel->setEnabled(gConfig.getEnableProgressBar());
    mpNumberOfSamplesBox = new QLineEdit(this);
    mpNumberOfSamplesBox->setValidator(new QIntValidator(0, 2000000000, this));
    QString samplesText;
    samplesText.setNum(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->getNumberOfLogSamples());
    mpNumberOfSamplesBox->setText(samplesText);

    mpCancelButton = new QPushButton(tr("&Cancel"));
    mpCancelButton->setAutoDefault(false);
    mpOkButton = new QPushButton(tr("&Done"));
    mpOkButton->setAutoDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);

    mpUserIconLabel = new QLabel("Icon Path:");
    mpIsoIconLabel = new QLabel("ISO Icon Path:");
    mpUserIconPath = new QLineEdit();
    mpIsoIconPath = new QLineEdit();

    mpIsoIconBrowseButton = new QPushButton(tr("..."));
    mpUserIconBrowseButton = new QPushButton(tr("..."));
    mpIsoIconBrowseButton->setFixedSize(25, 22);
    mpUserIconBrowseButton->setFixedSize(25, 22);

        //Create connections
    connect(mpCancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(mpOkButton, SIGNAL(pressed()), this, SLOT(updateValues()));
    connect(mpParentMainWindow->preferencesAction,SIGNAL(triggered()),this,SLOT(show()));
    connect(mpIsoIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseIso()));
    connect(mpUserIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseUser()));

        //Define the layout of the box
    mpLayout = new QGridLayout();
    mpLayout->setSizeConstraint(QLayout::SetFixedSize);
    mpLayout->addWidget(mpUserIconPath, 0, 1);
    mpLayout->addWidget(mpIsoIconPath, 1, 1);
    mpLayout->addWidget(mpUserIconBrowseButton, 0, 1, 1, 1, Qt::AlignRight);
    mpLayout->addWidget(mpIsoIconBrowseButton, 1, 1, 1, 1, Qt::AlignRight);
    mpLayout->addWidget(mpUserIconLabel, 0, 0);
    mpLayout->addWidget(mpIsoIconLabel, 1, 0);
    mpLayout->addWidget(mpIsoCheckBox, 2, 0);
    mpLayout->addWidget(mpDisableUndoCheckBox, 3, 0);
    mpLayout->addWidget(mpNumberOfSamplesLabel, 4, 0);
    mpLayout->addWidget(mpNumberOfSamplesBox, 4, 1);
    mpLayout->addWidget(mpButtonBox, 5, 1, 2, 2, Qt::AlignHCenter);
    setLayout(mpLayout);
}


//! @brief Reimplementation of QDialog::show(), used to update values in the box to current settings every time it is shown
void PreferenceDialog::show()
{
    mpIsoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);
    mpDisableUndoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);
    mpUserIconPath->setText(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->getUserIconPath());
    mpIsoIconPath->setText(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->getIsoIconPath());
    QDialog::show();
}


//! @brief Updates model settings according to the selected values
void PreferenceDialog::updateValues()
{
    if(mpIsoCheckBox->isChecked())
    {
        if(mpParentMainWindow->mpProjectTabs->count() > 0)
        {
            mpParentMainWindow->mpProjectTabs->getCurrentSystem()->setGfxType(ISOGRAPHICS);
        }
        mpParentMainWindow->mpLibrary->setGfxType(ISOGRAPHICS);
    }
    else
    {
        if(mpParentMainWindow->mpProjectTabs->count() > 0)
        {
            mpParentMainWindow->mpProjectTabs->getCurrentSystem()->setGfxType(USERGRAPHICS);
        }
        mpParentMainWindow->mpLibrary->setGfxType(USERGRAPHICS);
    }

    if( (mpDisableUndoCheckBox->isChecked()) != (mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled) )
    {
        mpParentMainWindow->mpProjectTabs->getCurrentSystem()->disableUndo();
        mpDisableUndoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);
    }

    mpParentMainWindow->mpProjectTabs->getCurrentSystem()->setUserIconPath(mpUserIconPath->text());
    mpParentMainWindow->mpProjectTabs->getCurrentSystem()->setIsoIconPath(mpIsoIconPath->text());
    mpParentMainWindow->mpProjectTabs->getCurrentSystem()->setNumberOfLogSamples(mpNumberOfSamplesBox->text().toInt());
    this->accept();
}


//! @brief Slot that opens a file dialog where user can select a user icon for the system
void PreferenceDialog::browseUser()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose user icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    mpUserIconPath->setText(modelFileName);
}


//! @brief Slot that opens a file dialog where user can select an iso icon for the system
void PreferenceDialog::browseIso()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose ISO icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    mpIsoIconPath->setText(modelFileName);
}
