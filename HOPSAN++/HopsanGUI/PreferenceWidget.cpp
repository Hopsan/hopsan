/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   PreferenceWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the model preferences dialog
//!
//$Id$


#include <QtGui>
#include <QDebug>

#include "PreferenceWidget.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GUISystem.h"
#include "LibraryWidget.h"


//! @brief Constructor for Model Preferences dialog
//! @param parent Pointer to the main window
PreferenceWidget::PreferenceWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;

        //Set the name and size of the main window
    this->setObjectName("PreferenceWidget");
    this->resize(640,480);
    this->setWindowTitle("Model Preferences");

        //Define items in the dialog box
    mpIsoCheckBox = new QCheckBox(tr("Use ISO 1219 Graphics"));
    mpIsoCheckBox->setCheckable(true);
    mpIsoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);

    mpDisableUndoCheckBox = new QCheckBox(tr("Disable Undo Function"));
    mpDisableUndoCheckBox->setCheckable(true);
    mpDisableUndoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);

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
    mpLayout->addWidget(mpButtonBox, 4, 1, 2, 2, Qt::AlignHCenter);
    setLayout(mpLayout);
}


//! @brief Reimplementation of QDialog::show(), used to update values in the box to current settings every time it is shown
void PreferenceWidget::show()
{
    mpIsoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);
    mpDisableUndoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoDisabled);
    mpUserIconPath->setText(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->getUserIconPath());
    mpIsoIconPath->setText(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->getIsoIconPath());
    QDialog::show();
}


//! @brief Updates model settings according to the selected values
void PreferenceWidget::updateValues()
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
    this->accept();
}


//! @brief Slot that opens a file dialog where user can select a user icon for the system
void PreferenceWidget::browseUser()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose user icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    mpUserIconPath->setText(modelFileName);
}


//! @brief Slot that opens a file dialog where user can select an iso icon for the system
void PreferenceWidget::browseIso()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose ISO icon"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH));
    mpIsoIconPath->setText(modelFileName);
}
