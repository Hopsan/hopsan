#include "ContainerPropertiesDialog.h"
#include "GUIObjects/GUIContainerObject.h"

//#include <QtGui>
#include <QDebug>

#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GUIObjects/GUISystem.h" //! @todo should we really need this we are going through main window to set stuff in our selfes (see below)
#include "LibraryWidget.h"

//! @brief Constructor for the parameter dialog for containers
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
//! @todo we need to delete once we are done, or add parent this on all new
ContainerPropertiesDialog::ContainerPropertiesDialog(GUIContainerObject *pContainerObject, QWidget *pParentWidget)
    : QDialog(pParentWidget)
{
    mpContainerObject = pContainerObject;

    //First build the properties box
    mpNameLabel = new QLabel("Name: ");
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
    mpDoneButton = new QPushButton(tr("&Done"));
    mpDoneButton->setAutoDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpDoneButton, QDialogButtonBox::ActionRole);

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
    connect(mpCancelButton, SIGNAL(pressed()), this, SLOT(close()));
    connect(mpDoneButton, SIGNAL(pressed()), this, SLOT(setValues()));
    //connect(gpMainWindow->preferencesAction,SIGNAL(triggered()),this,SLOT(show()));
    connect(mpIsoIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseIso()));
    connect(mpUserIconBrowseButton, SIGNAL(clicked()), this, SLOT(browseUser()));

    //Define the layout of the box
    mpLayout = new QGridLayout();
    mpLayout->setSizeConstraint(QLayout::SetFixedSize);
    mpLayout->addWidget(mpNameLabel, 0, 0);
    mpLayout->addWidget(mpNameEdit, 0, 1);

    mpLayout->addWidget(mpUserIconLabel, 1, 0);
    mpLayout->addWidget(mpIsoIconLabel, 2, 0);
    mpLayout->addWidget(mpUserIconPath, 1, 1);
    mpLayout->addWidget(mpIsoIconPath, 2, 1);
    mpLayout->addWidget(mpUserIconBrowseButton, 1, 1, 1, 1, Qt::AlignRight);
    mpLayout->addWidget(mpIsoIconBrowseButton, 2, 1, 1, 1, Qt::AlignRight);

    mpLayout->addWidget(mpIsoCheckBox, 3, 0);
    mpLayout->addWidget(mpDisableUndoCheckBox, 4, 0);

    //Set GuiSystem specific stuff
    if (mpContainerObject->type() == GUISYSTEM)
    {
        mpLayout->addWidget(mpNumberOfSamplesLabel, 5, 0);
        mpLayout->addWidget(mpNumberOfSamplesBox, 5, 1);

        mpLayout->addWidget(mpCQSLable, 6, 0);
        mpLayout->addWidget(mpCQSLineEdit, 6, 1);

        mpLayout->addWidget(mpButtonBox, 7, 1, 2, 2, Qt::AlignHCenter);
    }
    else
    {
        mpLayout->addWidget(mpButtonBox, 5, 1, 2, 2, Qt::AlignHCenter);
    }

    setLayout(mpLayout);
}

//! @brief Reimplementation of QDialog::show(), used to update values in the box to current settings every time it is shown
//! @todo is this really necessary as uptodate values are set in constructor
void ContainerPropertiesDialog::show()
{
//    mpIsoCheckBox->setChecked(gpMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);
//    mpDisableUndoCheckBox->setChecked(gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);
//    mpUserIconPath->setText(gpMainWindow->mpProjectTabs->getCurrentSystem()->getUserIconPath());
//    mpIsoIconPath->setText(gpMainWindow->mpProjectTabs->getCurrentSystem()->getIsoIconPath());
    QDialog::show();
}


//! @brief Updates model settings according to the selected values
void ContainerPropertiesDialog::setValues()
{
    //! @todo set name, need to figure out how to do it for containers in genereal

    if(mpIsoCheckBox->isChecked())
    {
//        if(gpMainWindow->mpProjectTabs->count() > 0)
//        {
//            gpMainWindow->mpProjectTabs->getCurrentSystem()->setGfxType(ISOGRAPHICS);
//        }
        this->mpContainerObject->setGfxType(ISOGRAPHICS);
        gpMainWindow->mpLibrary->setGfxType(ISOGRAPHICS);
    }
    else
    {
//        if(gpMainWindow->mpProjectTabs->count() > 0)
//        {
//            gpMainWindow->mpProjectTabs->getCurrentSystem()->setGfxType(USERGRAPHICS);
//        }
        this->mpContainerObject->setGfxType(USERGRAPHICS);
        gpMainWindow->mpLibrary->setGfxType(USERGRAPHICS);
    }

    if( (mpDisableUndoCheckBox->isChecked()) != (mpContainerObject->mUndoDisabled) )
    {
        mpContainerObject->disableUndo();
        mpDisableUndoCheckBox->setChecked(false);
    }

    mpContainerObject->setUserIconPath(mpUserIconPath->text());
    mpContainerObject->setIsoIconPath(mpIsoIconPath->text());
    //Set GuiSystem specific stuff
    if (mpContainerObject->type() == GUISYSTEM)
    {
        mpContainerObject->setNumberOfLogSamples(mpNumberOfSamplesBox->text().toInt());
        mpContainerObject->setTypeCQS(this->mpCQSLineEdit->text());
    }
//    this->accept();
    this->done(0);
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
