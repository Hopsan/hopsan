/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   ContainerPortPropertiesDialog.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a class for manipulation of ContainerPort properties
//!
//$Id$

#include "ContainerPortPropertiesDialog.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QPushButton>

#include "global.h"
#include "GUIObjects/GUIContainerPort.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Configuration.h"

//! @brief Constructor for the container properties dialog
//! @param[in] pContainerObject Pointer to the container
//! @param[in] pParentWidget Pointer to the parent widget
SystemPortPropertiesDialog::SystemPortPropertiesDialog(SystemPortObject *pSystemPort, QWidget *pParentWidget)
    : QDialog(pParentWidget)
{
    mpSystemPort = pSystemPort;

        //Set the name and size of the main window
    this->resize(10,10);        //Make very small initially, so it can expand when contents are added
    this->setWindowTitle("Container Port Properties");
    this->setPalette(gpConfig->getPalette());

        //This is the main Vertical layout of the dialog
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);

    //Define items in the dialog box
        //Name edit
    QHBoxLayout *pNameLayout = new QHBoxLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    mpNameEdit = new QLineEdit(mpSystemPort->getName(), this);
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);
    pMainLayout->addLayout(pNameLayout);

        //Done and Cancel Buttons
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pDoneButton = new QPushButton(tr("&Done"), this);
    pDoneButton->setDefault(true);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::ActionRole);
    pMainLayout->addWidget(pButtonBox, 0, Qt::AlignHCenter);

    //Create connections
    connect(pCancelButton,         SIGNAL(clicked()), this, SLOT(close()));
    connect(pDoneButton,           SIGNAL(clicked()), this, SLOT(setValues()));

    setLayout(pMainLayout);

    this->setMinimumWidth(300);
}


//! @brief Updates settings according to the selected values
void SystemPortPropertiesDialog::setValues()
{
    mpSystemPort->getParentSystemObject()->renameModelObject(mpSystemPort->getName(), mpNameEdit->text());
    this->accept();
}
