#include "ContainerPropertiesDialog.h"
#include "GUIObjects/GUIContainerObject.h"

//#include <QtGui>
#include <QDebug>

#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GUIObjects/GUISystem.h" //! @todo should we really need this we are going through main window to set stuff in our selfes (see below)
#include "LibraryWidget.h"
//#include "Configuration.h"

//! @brief Constructor for the parameter dialog for containers
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ContainerPropertiesDialog::ContainerPropertiesDialog(GUIContainerObject *pContainerObject, QWidget *pParentWidget)
    : QDialog(pParentWidget)
{
    //First build the properties box
    mpNameEdit = new QLineEdit(pContainerObject->getName());

    //Set the name and size of the main window
    this->setObjectName("PreferenceDialog");
    this->resize(640,480);
    this->setWindowTitle("Model Preferences");

    //Define items in the dialog box
    mpIsoCheckBox = new QCheckBox(tr("Use ISO 1219 Graphics"));
    mpIsoCheckBox->setCheckable(true);
    mpIsoCheckBox->setChecked(gpMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);
    //! @todo should we really need this we are going through main window to set stuff in our selfes

    mpDisableUndoCheckBox = new QCheckBox(tr("Disable Undo Function"));
    mpDisableUndoCheckBox->setCheckable(true);
    mpDisableUndoCheckBox->setChecked(gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);

    mpNumberOfSamplesLabel = new QLabel(tr("Number of Log Samples"));
    mpNumberOfSamplesLabel->setEnabled(true);
    mpNumberOfSamplesBox = new QLineEdit(this);
    mpNumberOfSamplesBox->setValidator(new QIntValidator(0, 2000000000, this));
    QString samplesText;
    samplesText.setNum(gpMainWindow->mpProjectTabs->getCurrentSystem()->getNumberOfLogSamples());
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

    mpCQSLable = new QLabel("CQS: ");
    mpCQSLineEdit = new QLineEdit(pContainerObject->getTypeCQS());

    //Create connections
    connect(mpCancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(mpOkButton, SIGNAL(pressed()), this, SLOT(updateValues()));
    connect(gpMainWindow->preferencesAction,SIGNAL(triggered()),this,SLOT(show()));
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
    mpLayout->addWidget(mpCQSLable, 5, 0);
    mpLayout->addWidget(mpCQSLineEdit, 5, 1);
    mpLayout->addWidget(mpButtonBox, 6, 1, 2, 2, Qt::AlignHCenter);
    setLayout(mpLayout);

}

//! @brief Reimplementation of QDialog::show(), used to update values in the box to current settings every time it is shown
void ContainerPropertiesDialog::show()
{
    mpIsoCheckBox->setChecked(gpMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);
    mpDisableUndoCheckBox->setChecked(gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);
    mpUserIconPath->setText(gpMainWindow->mpProjectTabs->getCurrentSystem()->getUserIconPath());
    mpIsoIconPath->setText(gpMainWindow->mpProjectTabs->getCurrentSystem()->getIsoIconPath());
    QDialog::show();
}


//! @brief Updates model settings according to the selected values
void ContainerPropertiesDialog::updateValues()
{
    if(mpIsoCheckBox->isChecked())
    {
        if(gpMainWindow->mpProjectTabs->count() > 0)
        {
            gpMainWindow->mpProjectTabs->getCurrentSystem()->setGfxType(ISOGRAPHICS);
        }
        gpMainWindow->mpLibrary->setGfxType(ISOGRAPHICS);
    }
    else
    {
        if(gpMainWindow->mpProjectTabs->count() > 0)
        {
            gpMainWindow->mpProjectTabs->getCurrentSystem()->setGfxType(USERGRAPHICS);
        }
        gpMainWindow->mpLibrary->setGfxType(USERGRAPHICS);
    }

    if( (mpDisableUndoCheckBox->isChecked()) != (gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled) )
    {
        gpMainWindow->mpProjectTabs->getCurrentSystem()->disableUndo();
        mpDisableUndoCheckBox->setChecked(gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);
    }

    gpMainWindow->mpProjectTabs->getCurrentSystem()->setUserIconPath(mpUserIconPath->text());
    gpMainWindow->mpProjectTabs->getCurrentSystem()->setIsoIconPath(mpIsoIconPath->text());
    gpMainWindow->mpProjectTabs->getCurrentSystem()->setNumberOfLogSamples(mpNumberOfSamplesBox->text().toInt());
    this->accept();
}


//! @brief Slot that opens a file dialog where user can select a user icon for the system
void ContainerPropertiesDialog::browseUser()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose user icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    mpUserIconPath->setText(modelFileName);
}


//! @brief Slot that opens a file dialog where user can select an iso icon for the system
void ContainerPropertiesDialog::browseIso()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose ISO icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    mpIsoIconPath->setText(modelFileName);
}
