//!
//! @file   ContainerPropertiesDialog.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a class for manimulation of ContainerPort properties
//!
//$Id$

#include "ContainerPortPropertiesDialog.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>

#include "../GUIObjects/GUIContainerPort.h"
#include "../GUIObjects/GUIContainerObject.h"
#include "../Configuration.h"

//! @brief Constructor for the container properties dialog
//! @param[in] pContainerObject Pointer to the container
//! @param[in] pParentWidget Pointer to the parent widget
ContainerPortPropertiesDialog::ContainerPortPropertiesDialog(GUIContainerPort *pContainerPort, QWidget *pParentWidget)
    : QDialog(pParentWidget)
{
    mpContainerPort = pContainerPort;

        //Set the name and size of the main window
    this->resize(10,10);        //Make very small initially, so it can expand when contents are added
    this->setWindowTitle("Container Port Properties");
    this->setPalette(gConfig.getPalette());

        //This is the main Vertical layout of the dialog
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);

    //Define items in the dialog box
        //Name edit
    QHBoxLayout *pNameLayout = new QHBoxLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    mpNameEdit = new QLineEdit(mpContainerPort->getName(), this);
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);
    pMainLayout->addLayout(pNameLayout);

        //Done and Cancel Buttons
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pDoneButton = new QPushButton(tr("&Done"), this);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::ActionRole);
    pDoneButton->setDefault(true);
    pMainLayout->addWidget(pButtonBox, 0, Qt::AlignHCenter);

    //Create connections
    connect(pCancelButton,         SIGNAL(clicked()), this, SLOT(close()));
    connect(pDoneButton,           SIGNAL(clicked()), this, SLOT(setValues()));

    setLayout(pMainLayout);

    this->setMinimumWidth(300);
}


//! @brief Updates settings according to the selected values
void ContainerPortPropertiesDialog::setValues()
{
    mpContainerPort->getParentContainerObject()->renameGUIModelObject(mpContainerPort->getName(), mpNameEdit->text());
    this->done(0);
}
