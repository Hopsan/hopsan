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
//! @file   HelpPopUpWidget.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2014-01-14
//!
//! @brief Contains the help popup widget
//!
//$Id$

//Qt includes
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTimer>
#include <QAction>
#include <QSvgWidget>

//Hopsan includes
#include "HelpPopUpWidget.h"
#include "global.h"
#include "Configuration.h"
#include "Dialogs/HelpDialog.h"
#include "ModelHandler.h"
#include "Dialogs/SensitivityAnalysisDialog.h"
#include "Widgets/LibraryWidget.h"
#include "GUIObjects/GUIContainerObject.h"


HelpPopUpWidget::HelpPopUpWidget(QWidget *pParent)
    : QWidget(pParent)
{

    QSvgWidget *pIcon = new QSvgWidget();
    pIcon->setMouseTracking(true);
    pIcon->load(QString(ICONPATH) + "svg/Hopsan-Info.svg");
    pIcon->setFixedSize(24,24);

    mpHelpText = new QLabel();
    mpHelpText->setMouseTracking(true);

    QGroupBox *pGroupBox = new QGroupBox(this);
    QHBoxLayout *pIconTextLayout = new QHBoxLayout(pGroupBox);
    pIconTextLayout->addWidget(pIcon);
    pIconTextLayout->addWidget(mpHelpText);
    pIconTextLayout->setContentsMargins(3,3,3,3);
    pGroupBox->setMouseTracking(true);

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(pGroupBox);

    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setMouseTracking(true);
    setBaseSize(50,30);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    setStyleSheet("QGroupBox { background-color : rgba(255,255,224,255); } QLabel { margin : 0px; } ");
    hide();
    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), this, SLOT(hide()));
}

//! @brief Shows the help popup message for 5 seconds with specified message.
//! Any message already being shown will be replaced. Messages can be hidden in advance by calling HelpPopUpWidget::hide().
//! @param message String with text so show in message
void HelpPopUpWidget::showHelpPopupMessage(const QString &rMessage)
{
    if(gpConfig->getShowPopupHelp())
    {
        mpHelpText->setText(rMessage);
        show();
        mpTimer->stop();
        mpTimer->start(5000);
    }
}



void HelpPopUpWidget::openContextHelp()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action != 0)
    {
        if(action->parent() == gpSensitivityAnalysisDialog)
        {
            gpHelpDialog->open("userSensitivityAnalysisPage.html");
        }
        else if(action->parent() == gpModelHandler->getCurrentViewContainerObject())
        {
            gpHelpDialog->open("userEnergyLossesPage.html");
        }
        else if(action->parent() == gpLibraryWidget)
        {
            gpHelpDialog->open("userModelingInHopsanPage.html");
        }
        else
        {
            gpHelpDialog->open();
        }
    }
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    if(button != 0)
    {
        qDebug() << "Name = " << button->objectName();
        if(button->parent() == gpLibraryWidget)
        {
            gpHelpDialog->open("userModelingInHopsanPage.html");
        }
        else if(button->objectName() == "optimizationHelpButton")
        {
            gpHelpDialog->open("userOptimizationPage.html");
        }
        else
        {
            gpHelpDialog->open();
        }
    }
    gpHelpDialog->centerOnScreen();
}



void HelpPopUpWidget::openContextHelp(QString file)
{
    gpHelpDialog->open(file);
    gpHelpDialog->centerOnScreen();
}
