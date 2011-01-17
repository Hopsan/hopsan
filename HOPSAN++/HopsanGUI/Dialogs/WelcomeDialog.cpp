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

#include "../common.h"
#include "../version.h"


//! @class WelcomeDialog
//! @brief A class for displaying the "Welcome HOPSAN" dialog
//!
//! Shows a cool picture, some logotypes, current version and some license information
//!

//! Constructor for the about dialog
//! @param parent Pointer to the main window
WelcomeDialog::WelcomeDialog(MainWindow *parent)
    : QDialog(parent)
{

        //Set the name and size of the main window
    this->setObjectName("WelcomeDialog");
    this->resize(480,640);
    this->setWindowTitle("Welcome to HOPSAN");
    this->setPalette(QPalette(QColor("gray"), QColor("whitesmoke")));
    this->setMouseTracking(true);

    mpHeading = new QLabel();
    QPixmap image;
    image.load(QString(GRAPHICSPATH) + "welcome.png");
    mpHeading->setPixmap(image);
    mpHeading->setAlignment(Qt::AlignCenter);


    mpNew = new QPushButton(this);
    QIcon newIcon;
    newIcon.addPixmap(QPixmap(QString(GRAPHICSPATH) + "new.png"));
    mpNew->setIcon(newIcon);
    mpNew->setIconSize(QSize(120, 120));
    mpNew->setStyleSheet("QPushButton:hover: { color: yellow; }");
    mpNew->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpNew->setMouseTracking(true);

    mpOpen = new QPushButton(this);
    QIcon openIcon;
    openIcon.addPixmap(QPixmap(QString(GRAPHICSPATH) + "open.png"));
    mpOpen->setIcon(openIcon);
    mpOpen->setIconSize(QSize(120, 120));
    mpOpen->setStyleSheet("QPushButton:hover: { color: yellow; }");
    mpOpen->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpOpen->setMouseTracking(true);

    mpLastSession = new QPushButton(this);
    QIcon lastSessionIcon;
    lastSessionIcon.addPixmap(QPixmap(QString(GRAPHICSPATH) + "lastsession.png"));
    mpLastSession->setIcon(lastSessionIcon);
    mpLastSession->setIconSize(QSize(120, 120));
    mpLastSession->setStyleSheet("QPushButton: { background-color: blue; } QPushButton:hover { background-color: yellow; }");
    mpLastSession->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpLastSession->setMouseTracking(true);
    mpLastSession->setEnabled(!gConfig.getLastSessionModels().empty());

    qDebug() << "lastsessionmodels = " << gConfig.getLastSessionModels();

    QHBoxLayout *pButtonLayout = new QHBoxLayout();
    pButtonLayout->addWidget(mpNew);
    pButtonLayout->addWidget(mpOpen);
    pButtonLayout->addWidget(mpLastSession);

    mpActionText = new QLabel();
    mpActionText->setText("Create New Model");
    QFont tempFont = mpActionText->font();
    tempFont.setPixelSize(20);
    mpActionText->setFont(tempFont);
    mpActionText->setAlignment(Qt::AlignCenter);

    QGridLayout *pLayout = new QGridLayout;
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(mpHeading, 0, 0);
    pLayout->addLayout(pButtonLayout, 1, 0);
    pLayout->addWidget(mpActionText, 2, 0);
    setLayout(pLayout);

    connect(mpNew, SIGNAL(clicked()), this, SLOT(createNewModel()));
    connect(mpOpen, SIGNAL(clicked()), this, SLOT(loadExistingModel()));
    connect(mpLastSession, SIGNAL(clicked()), this, SLOT(loadLastSession()));
}


void WelcomeDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(mpNew->underMouse())
    {
        mpActionText->setText("Create New Model");
    }
    else if(mpOpen->underMouse())
    {
        mpActionText->setText("Open Existing Model");
    }
    else if(mpLastSession->underMouse())
    {
        mpActionText->setText("Open Last Session");
    }

    QDialog::mouseMoveEvent(event);
}



void WelcomeDialog::createNewModel()
{
    gpMainWindow->mpProjectTabs->addNewProjectTab();
    gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
    this->close();
}


void WelcomeDialog::loadExistingModel()
{
    gpMainWindow->mpProjectTabs->loadModel();
    gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
    this->close();
}


void WelcomeDialog::loadLastSession()
{
    for(int i=0; i<gConfig.getLastSessionModels().size(); ++i)
    {
        gpMainWindow->mpProjectTabs->loadModel(gConfig.getLastSessionModels().at(i));
    }
    gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
    this->close();
}
