/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HelpPopUpWidget.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2014-01-14
//!
//! @brief Contains the help popup widget
//!
//$Id: PlotWindow.h 6422 2014-01-13 16:10:52Z petno25 $

#include <QGroupBox>
#include <QHBoxLayout>
#include <QTimer>

#include "HelpPopUpWidget.h"
#include "global.h"
#include "Configuration.h"

HelpPopUpWidget::HelpPopUpWidget(QWidget *pParent)
    : QWidget(pParent)
{
    QLabel *pIcon = new QLabel();
    pIcon->setMouseTracking(true);
    pIcon->setPixmap(QPixmap(QString(ICONPATH) + "Hopsan-Info.png"));
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
