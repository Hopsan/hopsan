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
//! @file   WelcomeDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the Welcome dialog
//!
//$Id: WelcomeDialog.cpp 2426 2010-12-30 19:58:15Z petno25 $

#include "WelcomeDialog.h"
#include "../MainWindow.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../Configuration.h"
#include "../GraphicsView.h"
#include <QPixmap>
#include <QColor>
#include <QWebView>

#include "../common.h"
#include "../version.h"


//! @class WelcomeDialog
//! @brief A class for displaying the "Welcome to Hopsan" dialog
//!
//! Shows a cool picture, some logotypes, current version and some license information
//!

//! Constructor for the about dialog
//! @param parent Pointer to the main window
WelcomeDialog::WelcomeDialog(MainWindow *parent)
    : QDialog(parent)
{
        //Set the name and size of the main window
    this->setWindowIcon(QIcon(QString(QString(ICONPATH) + "hopsan.png")));
    this->setObjectName("WelcomeDialog");
    this->resize(480,640);
    this->setWindowTitle("Welcome to Hopsan");
    this->setPalette(QPalette(QColor("gray"), QColor("whitesmoke")));
    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_NoMousePropagation, false);

    mpHeading = new QLabel();
    QPixmap image;
    image.load(QString(GRAPHICSPATH) + "welcome.png");
    mpHeading->setPixmap(image);
    mpHeading->setAlignment(Qt::AlignCenter);

    mpNew = new QPushButton(this);
    mpNew->setFlat(true);
    mpNewIcon = new QIcon(QPixmap(QString(GRAPHICSPATH) + "new.png"));
    mpNewActiveIcon = new QIcon(QPixmap(QString(GRAPHICSPATH) + "newactive.png"));
    mpNew->setIcon(*mpNewActiveIcon);
    mpNew->setIconSize(QSize(120, 120));
    mpNew->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpNew->setMouseTracking(true);
    mpNew->setStyleSheet(" QPushButton:flat { border: none; background: none; } ");
    mpNew->setDefault(true);

    mpOpen = new QPushButton(this);
    mpOpenIcon = new QIcon(QPixmap(QString(GRAPHICSPATH) + "open.png"));
    mpOpenActiveIcon = new QIcon(QPixmap(QString(GRAPHICSPATH) + "openactive.png"));
    mpOpen->setIcon(*mpOpenIcon);
    mpOpen->setIconSize(QSize(120, 120));
    mpOpen->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpOpen->setMouseTracking(true);
    mpOpen->setFlat(true);
    mpOpen->setStyleSheet(" QPushButton:flat { border: none; background: none; } ");

    mpLastSession = new QPushButton(this);
    mpLastSessionIcon = new QIcon(QPixmap(QString(GRAPHICSPATH) + "lastsession.png"));
    mpLastSessionActiveIcon = new QIcon(QPixmap(QString(GRAPHICSPATH) + "lastsessionactive.png"));
    mpLastSession->setIcon(*mpLastSessionIcon);
    mpLastSession->setIconSize(QSize(120, 120));
    mpLastSession->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpLastSession->setMouseTracking(true);
    mpLastSession->setEnabled(!gConfig.getLastSessionModels().empty());
    mpLastSession->setFlat(true);
    mpLastSession->setStyleSheet(" QPushButton:flat { border: none; background: none; } ");

    QHBoxLayout *pButtonLayout = new QHBoxLayout();
    pButtonLayout->addWidget(mpNew);
    pButtonLayout->addWidget(mpOpen);
    pButtonLayout->addWidget(mpLastSession);
    pButtonLayout->setSpacing(0);
    pButtonLayout->setContentsMargins(0, 0, 0, 0);

    mpActionText = new QLabel();
    mpActionText->setText("Create New Model");
    QFont tempFont = mpActionText->font();
    tempFont.setPixelSize(20);
    mpActionText->setFont(tempFont);
    mpActionText->setAlignment(Qt::AlignCenter);

    mpRecentList = new QListWidget(this);
    mpRecentList->setVisible(!gConfig.getRecentModels().empty());

    for(int i=0; i<gConfig.getRecentModels().size(); ++i)
    {
        if(!gConfig.getRecentModels().at(i).isEmpty())
        {
            mModelList.append(gConfig.getRecentModels().at(i));
            QString displayName = gConfig.getRecentModels().at(i);
            mpRecentList->addItem(displayName.section('/', -1));
        }
    }
    mpRecentList->setFixedHeight(4+16*mpRecentList->count());
    connect(mpRecentList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(openRecentModel()));

    mpDontShowMe = new QCheckBox("Always load last session");
    mpDontShowMe->setChecked(!gConfig.getShowWelcomeDialog());

    QGridLayout *pLayout = new QGridLayout;
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(mpHeading,       0, 0);
    pLayout->addLayout(pButtonLayout,   1, 0);
    pLayout->addWidget(mpActionText,    2, 0);
    pLayout->addWidget(mpRecentList,     3, 0);
    pLayout->addWidget(mpDontShowMe,    4, 0);
    setLayout(pLayout);

    QPalette tempPalette;
    tempPalette = this->palette();
    tempPalette.setColor(QPalette::Window, QColor(235, 245, 242));
    this->setPalette(tempPalette);

    connect(mpNew, SIGNAL(clicked()), this, SLOT(createNewModel()));
    connect(mpOpen, SIGNAL(clicked()), this, SLOT(loadExistingModel()));
    connect(mpLastSession, SIGNAL(clicked()), this, SLOT(loadLastSession()));
}


void WelcomeDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(mpNew->underMouse())
    {
        mpNew->setFocus();
    }
    else if(mpOpen->underMouse())
    {
        mpOpen->setFocus();
    }
    else if(mpLastSession->underMouse() && mpLastSession->isEnabled())
    {
        mpLastSession->setFocus();
    }

    this->updateGraphics();

    QDialog::mouseMoveEvent(event);
}


bool WelcomeDialog::focusNextPrevChild(bool next)
{
    QDialog::focusNextPrevChild(next);

    qDebug() << "Key pressed!";
    this->updateGraphics();

    return true;    //Silly, but will supress warning message
}



void WelcomeDialog::updateGraphics()
{
    if(mpNew->hasFocus())
    {
        mpActionText->setText("Create New Model");
        mpNew->setIcon(*mpNewActiveIcon);
        mpOpen->setIcon(*mpOpenIcon);
        mpLastSession->setIcon(*mpLastSessionIcon);
    }
    else if(mpOpen->hasFocus())
    {
        mpActionText->setText("Open Existing Model");
        mpNew->setIcon(*mpNewIcon);
        mpOpen->setIcon(*mpOpenActiveIcon);
        mpLastSession->setIcon(*mpLastSessionIcon);
    }
    else if(mpLastSession->hasFocus())
    {
        mpActionText->setText("Open Last Session");
        mpNew->setIcon(*mpNewIcon);
        mpOpen->setIcon(*mpOpenIcon);
        mpLastSession->setIcon(*mpLastSessionActiveIcon);
    }
    else
    {
        mpNew->setIcon(*mpNewIcon);
        mpOpen->setIcon(*mpOpenIcon);
        mpLastSession->setIcon(*mpLastSessionIcon);
    }
}


void WelcomeDialog::createNewModel()
{
    gpMainWindow->mpProjectTabs->addNewProjectTab();
    gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
    gConfig.setShowWelcomeDialog(!mpDontShowMe->isChecked());
    this->close();
}


void WelcomeDialog::loadExistingModel()
{
    gpMainWindow->mpProjectTabs->loadModel();
    if(gpMainWindow->mpProjectTabs->count() > 0)
    {
        gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
    }
    gConfig.setShowWelcomeDialog(!mpDontShowMe->isChecked());
    this->close();
}


void WelcomeDialog::loadLastSession()
{

    for(int i=0; i<gConfig.getLastSessionModels().size(); ++i)
    {
        qDebug() << "Opening last session model: " << gConfig.getLastSessionModels().at(i);
        gpMainWindow->mpProjectTabs->loadModel(gConfig.getLastSessionModels().at(i));
    }
    gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
    gConfig.setShowWelcomeDialog(!mpDontShowMe->isChecked());
    this->close();
}


void WelcomeDialog::openRecentModel()
{
    gpMainWindow->mpProjectTabs->loadModel(mModelList.at(mpRecentList->currentIndex().row()));
    gConfig.setShowWelcomeDialog(!mpDontShowMe->isChecked());
    this->close();
}
