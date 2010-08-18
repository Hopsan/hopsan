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

//$Id$


#include <QtGui>
#include <QDebug>

#include "PreferenceWidget.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GUISystem.h"

//class ProjectTabWidget;

PreferenceWidget::PreferenceWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("PreferenceWidget");
    this->resize(640,480);
    this->setWindowTitle("Model Preferences");

    isoCheckBox = new QCheckBox(tr("Use ISO 1219 graphics"));
    isoCheckBox->setCheckable(true);
    isoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);
    //isoCheckBox->setChecked(false);

    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setAutoDefault(false);
    okButton = new QPushButton(tr("&Done"));
    okButton->setAutoDefault(true);

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    QLabel *userIconLabel = new QLabel("Icon Path:");
    QLabel *isoIconLabel = new QLabel("ISO Icon Path:");
    userIconPath = new QLineEdit();
    isoIconPath = new QLineEdit();

    connect(cancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(okButton, SIGNAL(pressed()), this, SLOT(updateValues()));
    //connect(isoCheckBox, SIGNAL(pressed(bool)), mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView, SLOT(setIsoGraphics(bool)));


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    //mainLayout->addLayout(topLeftLayout, 0, 0);
    mainLayout->addWidget(userIconPath, 0, 1);
    mainLayout->addWidget(isoIconPath, 1, 1);
    mainLayout->addWidget(userIconLabel, 0, 0);
    mainLayout->addWidget(isoIconLabel, 1, 0);
    mainLayout->addWidget(isoCheckBox, 2, 0);
    mainLayout->addWidget(buttonBox, 3, 0, 2, 0, Qt::AlignHCenter);
    setLayout(mainLayout);
}


void PreferenceWidget::show()
{
    isoCheckBox->setChecked(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mGfxType);
    QDialog::show();
}


void PreferenceWidget::updateValues()
{
    if(isoCheckBox->isChecked())
    {
        mpParentMainWindow->mpProjectTabs->setIsoGraphics(ISOGRAPHICS);
    }
    else
    {
        mpParentMainWindow->mpProjectTabs->setIsoGraphics(USERGRAPHICS);
    }

    mpParentMainWindow->mpProjectTabs->getCurrentSystem()->setUserIconPath(userIconPath->text());
    mpParentMainWindow->mpProjectTabs->getCurrentSystem()->setIsoIconPath(isoIconPath->text());
    //this->isoBool = this->isoCheckBox->isChecked();
    //qDebug() << isoBool;
    this->accept();
}
