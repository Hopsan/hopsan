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
//! @file   WelcomeWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012
//!
//! @brief Contains the welcome widget class
//!
//$Id: UndoWidget.h 3547 2011-10-25 11:48:47Z petno25 $

//Qt includes
#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include <QStringList>
#include <QWebFrame>
#include <QtXml>

//Hopsan includes
#include "common.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/OptionsDialog.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "version_gui.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/PlotWidget.h" //!< @todo why is this needed in here
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/WelcomeWidget.h"


WelcomeWidget::WelcomeWidget(QWidget *parent) :
    QWidget(parent)
{
    mFrameH = 180;
    mFrameW = 140;
    mSpacing = 20;

    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_NoMousePropagation, false);

    mpHeading = new QLabel(this);
    QPixmap image;
    QDate today = QDate::currentDate();
    if(today.month() == 12 && today.day() > 20 && today.day() < 31)
        image.load(QString(GRAPHICSPATH) + "welcome_xmas.png");
    else if((today.month() == 12 && today.day() == 31) || (today.month() == 1 && today.day() == 1))
        image.load(QString(GRAPHICSPATH) + "welcome_newyear.png");
    else
        image.load(QString(GRAPHICSPATH) + "welcome.png");
    mpHeading->setPixmap(image);
//    mpHeading->setText("Welcome to Hopsan!");
//    QFont tempFont = mpHeading->font();
//    tempFont.setPointSize(25);
//    tempFont.setBold(true);
//    mpHeading->setFont(tempFont);
    //mpHeading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpHeading->setAlignment(Qt::AlignCenter);

    mpNewIcon = new QLabel(this);
    mpNewIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "new.png"));
    mpNewIcon->setMouseTracking(true);
    mpNewText = new QLabel("New Model", this);
    mpNewText->setAlignment(Qt::AlignCenter);
    mpNewText->setMouseTracking(true);
    mpNewLayout = new QVBoxLayout(this);
    mpNewLayout->addWidget(mpNewIcon,1,Qt::AlignCenter);
    mpNewLayout->addWidget(mpNewText,0,Qt::AlignBottom);

    mpNewFrame = new QFrame(this);
    mpNewFrame->setFrameShape(QFrame::StyledPanel);
    mpNewFrame->setMouseTracking(true);
    mpNewFrame->setFixedSize(mFrameW,mFrameH);
    mpNewFrame->setLayout(mpNewLayout);

    mpLoadIcon = new QLabel(this);
    mpLoadIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "open.png"));
    mpLoadIcon->setMouseTracking(true);
    mpLoadText = new QLabel("Load Model", this);
    mpLoadText->setAlignment(Qt::AlignCenter);
    mpLoadText->setMouseTracking(true);
    mpLoadLayout = new QVBoxLayout(this);
    mpLoadLayout->addWidget(mpLoadIcon,1,Qt::AlignCenter);
    mpLoadLayout->addWidget(mpLoadText,0,Qt::AlignBottom);

    mpLoadFrame = new QFrame(this);
    mpLoadFrame->setFrameShape(QFrame::StyledPanel);
    mpLoadFrame->setMouseTracking(true);
    mpLoadFrame->setFixedSize(mFrameW,mFrameH);
    mpLoadFrame->setLayout(mpLoadLayout);

    mpLastSessionIcon = new QLabel(this);
    mpLastSessionIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "lastsession.png"));
    mpLastSessionIcon->setMouseTracking(true);
    mpLastSessionIcon->setEnabled(!gConfig.getLastSessionModels().empty());
    mpLastSessionText = new QLabel("Last Session", this);
    mpLastSessionText->setAlignment(Qt::AlignCenter);
    mpLastSessionText->setMouseTracking(true);
    mpLastSessionText->setEnabled(!gConfig.getLastSessionModels().empty());
    mpLastSessionLayout = new QVBoxLayout(this);
    mpLastSessionLayout->addWidget(mpLastSessionIcon,1,Qt::AlignCenter);
    mpLastSessionLayout->addWidget(mpLastSessionText,0,Qt::AlignBottom);

    mpLastSessionFrame = new QFrame(this);
    mpLastSessionFrame->setFrameShape(QFrame::StyledPanel);
    mpLastSessionFrame->setMouseTracking(true);
    mpLastSessionFrame->setFixedSize(mFrameW,mFrameH);
    mpLastSessionFrame->setLayout(mpLastSessionLayout);

    mpRecentList = new QListWidget(this);
    mpRecentList->setFixedSize(mFrameW*2+mSpacing-20,mFrameH-50);
    mpRecentList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpRecentList->setMouseTracking(true);
    connect(mpRecentList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(openRecentModel()));
    mpRecentText = new QLabel("Recent Models", this);
    mpRecentText->setAlignment(Qt::AlignCenter);
    mpRecentText->setMouseTracking(true);
    mpRecentLayout = new QVBoxLayout(this);
    mpRecentLayout->addWidget(mpRecentList,1, Qt::AlignCenter);
    mpRecentLayout->addWidget(mpRecentText,0,Qt::AlignBottom);
    mpRecentFrame = new QFrame(this);
    mpRecentFrame->setFrameShape(QFrame::StyledPanel);
    mpRecentFrame->setMouseTracking(true);
    mpRecentFrame->setFixedSize(mFrameW*2+mSpacing,mFrameH);
    mpRecentFrame->setLayout(mpRecentLayout);

    updateRecentList();


    mpExampleList = new QListWidget(this);
    mpExampleList->setFixedSize(mFrameW*2+mSpacing-20,mFrameH-50);
    mpExampleList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpExampleList->setMouseTracking(true);
    connect(mpExampleList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(openExampleModel()));
    mpExampleText = new QLabel("Example Models", this);
    mpExampleText->setAlignment(Qt::AlignCenter);
    mpExampleText->setMouseTracking(true);
    mpExampleLayout = new QVBoxLayout(this);
    mpExampleLayout->addWidget(mpExampleList,1, Qt::AlignCenter);
    mpExampleLayout->addWidget(mpExampleText,0,Qt::AlignBottom);
    mpExampleFrame = new QFrame(this);
    mpExampleFrame->setFrameShape(QFrame::StyledPanel);
    mpExampleFrame->setMouseTracking(true);
    mpExampleFrame->setFixedSize(mFrameW*2+mSpacing,mFrameH);
    mpExampleFrame->setLayout(mpExampleLayout);
    QDir exampleModelsDir(gDesktopHandler.getMainPath()+"Models/Example Models/");
    QStringList filters;
    filters << "*.hmf";
    exampleModelsDir.setNameFilters(filters);
    QStringList exampleModels = exampleModelsDir.entryList();
    for(int i=0; i<exampleModels.size(); ++i)
    {
        if(!exampleModels.at(i).isEmpty())
        {
            mExampleModelList.append(exampleModels.at(i));
            QString displayName = exampleModels.at(i);
            displayName = displayName.section('/', -1);
            displayName.chop(4);
            QString toolTipName = displayName;
            while(mpExampleList->fontMetrics().width(displayName) > mFrameW*2+mSpacing-35)
            {
                displayName.chop(4);
                displayName.append("...");
            }
            mpExampleList->addItem(displayName);
            mpExampleList->item(mpExampleList->count()-1)->setToolTip(toolTipName);
        }
    }

    mpOptionsIcon = new QLabel(this);
    mpOptionsIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "options.png"));
    mpOptionsIcon->setMouseTracking(true);
    mpOptionsText = new QLabel("Options", this);
    mpOptionsText->setAlignment(Qt::AlignCenter);
    mpOptionsText->setMouseTracking(true);
    mpOptionsLayout = new QVBoxLayout(this);
    mpOptionsLayout->addWidget(mpOptionsIcon,1,Qt::AlignCenter);
    mpOptionsLayout->addWidget(mpOptionsText,0,Qt::AlignBottom);

    mpOptionsFrame = new QFrame(this);
    mpOptionsFrame->setFrameShape(QFrame::StyledPanel);
    mpOptionsFrame->setMouseTracking(true);
    mpOptionsFrame->setFixedSize(mFrameW,mFrameH);
    mpOptionsFrame->setLayout(mpOptionsLayout);

    mpLoadingWebProgressBar = new QProgressBar(this);
    mpLoadingWebProgressBar->setRange(0, 0);
    mpLoadingWebProgressBar->setFixedWidth(mFrameW*1.95);
    mpLoadingWebLabel = new QLabel("Loading news...", this);
    mpLoadingWebProgressBarTimer = new QTimer(this);
    connect(mpLoadingWebProgressBarTimer, SIGNAL(timeout()), this, SLOT(updateLoadingWebProgressBar()));
    mpLoadingWebProgressBarTimer->setInterval(1);
    mpLoadingWebProgressBarTimer->start();

    mpLoadingWebLayout = new QVBoxLayout(this);
    mpLoadingWebLayout->addWidget(mpLoadingWebLabel);
    mpLoadingWebLayout->addWidget(mpLoadingWebProgressBar);
    mpLoadingWebLayout->setAlignment(mpLoadingWebLabel, Qt::AlignCenter);
    mpLoadingWebLayout->setAlignment(mpLoadingWebProgressBar, Qt::AlignCenter);
    mpLoadingWebWidget = new QWidget(this);
    //mpLoadingWebWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpLoadingWebWidget->setFixedHeight(168);
    //mpLoadingWebWidget->setFixedWidth(mFrameW*1.9);
    mpLoadingWebWidget->setLayout(mpLoadingWebLayout);


    mpWeb = new QWebView(this);     //! @todo This could be a QNetworkAccessManager as well, since it is never shown
    mpWeb->hide();
    connect(mpWeb, SIGNAL(loadFinished(bool)), this, SLOT(checkVersion(bool)));
    mpWeb->load(QUrl(QString(VERSIONLINK)));

    mpFeed = new QNetworkAccessManager(this);
    connect(mpFeed, SIGNAL(finished(QNetworkReply*)), this, SLOT(showNews(QNetworkReply*)));
    mpFeed->get(QNetworkRequest(QUrl(NEWSLINK)));

    mpNewsScrollWidget = new QWidget(this);
    //mpNewsScrollWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpNewsScrollLayout = new QVBoxLayout(mpNewsScrollWidget);
    mpNewsScrollArea = new QScrollArea(this);
    //mpNewsScrollArea->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpNewsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpNewsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpNewsScrollArea->setWidget(mpNewsScrollWidget);
    mpNewsScrollArea->setWidgetResizable(true);

    mpNewsText = new QLabel("Hopsan News", this);
    mpNewsText->setAlignment(Qt::AlignCenter);
    mpNewsText->setMouseTracking(true);
    //mpNewsText->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpNewsLayout = new QVBoxLayout(this);
    mpNewsLayout->addWidget(mpNewsText, 0, Qt::AlignTop);
    mpNewsLayout->addWidget(mpLoadingWebWidget);
    //mpNewsLayout->setAlignment(mpLoadingWebWidget, Qt::AlignCenter);
    //mpNewsLayout->addWidget(mpWeb);
    mpNewsLayout->addWidget(mpNewsScrollArea);
    mpNewsFrame = new QFrame(this);
    //mpNewsFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpNewsFrame->setFrameShape(QFrame::StyledPanel);
    mpNewsFrame->setMouseTracking(true);
    mpNewsFrame->setFixedSize(mFrameW*2,mFrameH*2+mSpacing);
    mpNewsFrame->setLayout(mpNewsLayout);

    mpAutoUpdateAction = new QAction("Launch Auto Updater", this);
    mpGoToDownloadPageAction = new QAction("Open Download Page In Browser", this);
    mpNewVersionMenu = new QMenu(this);
    mpNewVersionMenu->addAction(mpAutoUpdateAction);
    mpNewVersionMenu->addAction(mpGoToDownloadPageAction);
    connect(mpAutoUpdateAction, SIGNAL(triggered()), gpMainWindow, SLOT(launchAutoUpdate()));
    connect(mpGoToDownloadPageAction, SIGNAL(triggered()), this, SLOT(openDownloadPage()));

    mpNewVersionButton = new QPushButton("New Version Available!");
    QPalette tempPalette = mpNewVersionButton->palette();
    tempPalette.setColor(QPalette::ButtonText, QColor("darkred"));
    mpNewVersionButton->setPalette(tempPalette);
    QFont tempFont = mpNewVersionButton->font();
    tempFont.setPixelSize(14);
    tempFont.setBold(true);
    mpNewVersionButton->setFont(tempFont);
    mpNewVersionButton->hide();
    mpNewVersionButton->setMenu(mpNewVersionMenu);

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpHeading,                          0, 0, 1, 3);
    mpLayout->addWidget(mpNewFrame,                         1, 0, 1, 1);
    mpLayout->addWidget(mpLoadFrame,                        1, 1, 1, 1);
    mpLayout->addWidget(mpLastSessionFrame,                 1, 2, 1, 1);
    mpLayout->addWidget(mpOptionsFrame,                     1, 3, 1, 1);
    mpLayout->addWidget(mpRecentFrame,                      2, 0, 1, 2);
    mpLayout->addWidget(mpExampleFrame,                     2, 2, 1, 2);
    mpLayout->addWidget(mpNewsFrame,                        1, 4, 2, 1);
    mpLayout->addWidget(mpNewVersionButton,                 0, 4, 1, 1);
    mpLayout->addWidget(new QWidget(this),                  3, 0, 1, 4);
    mpLayout->addWidget(new QWidget(this),                  0, 5, 2, 1);
    mpLayout->setSpacing(mSpacing);
    mpLayout->setRowStretch(3,1);
    mpLayout->setColumnStretch(5, 1);
    mpLayout->setSizeConstraint(QLayout::SetFixedSize);
    //mpLayout->addWidget(pTestLabel);

    this->setLayout(mpLayout);

    connect(this, SIGNAL(hovered()), gpMainWindow->mpLibrary, SLOT(clearHoverEffects()));
    connect(this, SIGNAL(hovered()), gpMainWindow->mpPlotWidget, SLOT(clearHoverEffects()));
    connect(gpMainWindow->mpCentralTabs, SIGNAL(currentChanged(int)), this, SLOT(autoHide()));
}


QString WelcomeWidget::getUpdateLink()
{
    return mpUpdateLink;
}

void WelcomeWidget::updateRecentList()
{
    mpRecentList->clear();
    mRecentModelList.clear();
    QStringList recentModels = gConfig.getRecentModels();
    for(int i=0; i<recentModels.size(); ++i)
    {
        if(!recentModels.at(i).isEmpty())
        {
            mRecentModelList.append(recentModels.at(i));
            QString displayName = recentModels.at(i);
            displayName = displayName.section('/', -1);
            displayName.chop(4);
            QString toolTipName = displayName;
            while(mpRecentList->fontMetrics().width(displayName) > mFrameW*2+mSpacing-35)
            {
                displayName.chop(4);
                displayName.append("...");
            }
            mpRecentList->addItem(displayName);
            mpRecentList->item(mpRecentList->count()-1)->setToolTip(toolTipName);
        }
    }
}


void WelcomeWidget::mouseMoveEvent(QMouseEvent *event)
{
    updateHoverEffects();

    emit hovered();

    QWidget::mouseMoveEvent(event);
}


void WelcomeWidget::debugSlot()
{
    qDebug() << "Debug slot!";
}


void WelcomeWidget::updateHoverEffects()
{
    if(mpNewFrame->underMouse())
    {
        mpNewFrame->setFrameShape(QFrame::Panel);
        mpNewIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "newactive.png"));
    }
    else
    {
        mpNewFrame->setFrameShape(QFrame::StyledPanel);
        mpNewIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "new.png"));
    }

    if(mpLoadFrame->underMouse())
    {
        mpLoadFrame->setFrameShape(QFrame::Panel);
        mpLoadIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "openactive.png"));
    }
    else
    {
        mpLoadFrame->setFrameShape(QFrame::StyledPanel);
        mpLoadIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "open.png"));
    }

    if(mpLastSessionFrame->underMouse() && mpLastSessionIcon->isEnabled())
    {
        mpLastSessionFrame->setFrameShape(QFrame::Panel);
        mpLastSessionIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "lastsessionactive.png"));
    }
    else
    {
        mpLastSessionFrame->setFrameShape(QFrame::StyledPanel);
        mpLastSessionIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "lastsession.png"));
    }

    if(mpRecentFrame->underMouse() || mpRecentList->underMouse())
    {
        mpRecentFrame->setFrameShape(QFrame::Panel);
    }
    else
    {
        mpRecentFrame->setFrameShape(QFrame::StyledPanel);
    }

    if(mpExampleFrame->underMouse() || mpRecentList->underMouse())
    {
        mpExampleFrame->setFrameShape(QFrame::Panel);
    }
    else
    {
        mpExampleFrame->setFrameShape(QFrame::StyledPanel);
    }

    if(mpOptionsFrame->underMouse())
    {
        mpOptionsFrame->setFrameShape(QFrame::Panel);
        mpOptionsIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "optionsactive.png"));
    }
    else
    {
        mpOptionsFrame->setFrameShape(QFrame::StyledPanel);
        mpOptionsIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "options.png"));
    }

    if(mpNewsFrame->underMouse())
    {
        mpNewsFrame->setFrameShape(QFrame::Panel);
    }
    else
    {
        mpNewsFrame->setFrameShape(QFrame::StyledPanel);
    }
}


void WelcomeWidget::mousePressEvent(QMouseEvent *event)
{
    if(mpNewFrame->underMouse())
    {
        gpMainWindow->mpModelHandler->addNewModel();
    }
    else if(mpLoadFrame->underMouse())
    {
        gpMainWindow->mpModelHandler->loadModel();
    }
    else if(mpLastSessionFrame->underMouse())
    {
        for(int i=0; i<gConfig.getLastSessionModels().size(); ++i)
        {
            gpMainWindow->mpModelHandler->loadModel(gConfig.getLastSessionModels().at(i));
        }
    }
    else if(mpOptionsFrame->underMouse())
    {
        gpMainWindow->getOptionsDialog()->show();
    }

    QWidget::mousePressEvent(event);
}



//! @brief Opens selected recent model from the list and closes the welcome dialog.
void WelcomeWidget::openRecentModel()
{
    gpMainWindow->mpModelHandler->loadModel(mRecentModelList.at(mpRecentList->currentIndex().row()));
}


//! @brief Slot that loads an example model, based on the name of the calling action
void WelcomeWidget::openExampleModel()
{
    gpMainWindow->mpModelHandler->loadModel(gDesktopHandler.getMainPath()+"Models/Example Models/"+mExampleModelList.at(mpExampleList->currentIndex().row()));
}



//! @brief Slot that shows the news box if the page was successfully loaded.
//! @param loadedSuccessfully True if a page was loaded (this does NOT mean that the loaded page is the correct one!)
void WelcomeWidget::showNews(QNetworkReply *pReply)
{
    QVariant redirection = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirection.toUrl().isEmpty())
    {
        mpFeed->get(QNetworkRequest(redirection.toUrl()));
        return;
    }

    if(pReply->error() == QNetworkReply::NoError)
    {

        QByteArray feedData=pReply->readAll();
        QString feedString(feedData);
        //qDebug() << "Feed: " << feedString;

        QDomDocument feedDocument;
        feedDocument.setContent(feedString);

        QDomElement feedRoot = feedDocument.documentElement();

        QDomElement entryElement = feedRoot.firstChildElement("entry");
        while(!entryElement.isNull())
        {
            QString title = entryElement.firstChildElement("title").text();
            QString link = entryElement.firstChildElement("link").attribute("href");
            QString date = entryElement.firstChildElement("updated").text().left(10);
            QString html = entryElement.firstChildElement("content").text().left(120).append("...");

            QLabel *pTitleLabel = new QLabel("<a href=\""+link+"\">"+title+"</a>", this);
            pTitleLabel->setOpenExternalLinks(true);
            QFont boldFont = pTitleLabel->font();
            boldFont.setBold(true);
            pTitleLabel->setFont(boldFont);

            QLabel *pDateLabel = new QLabel(date, this);
            QFont italicFont = pDateLabel->font();
            italicFont.setItalic(true);
            pDateLabel->setFont(italicFont);

            QLabel *pContentLabel = new QLabel(html, this);
            pContentLabel->setWordWrap(true);

            mpNewsScrollLayout->addWidget(pTitleLabel);
            mpNewsScrollLayout->addWidget(pDateLabel);
            mpNewsScrollLayout->addWidget(pContentLabel);
            //mpNewsScrollLayout->setSizeConstraint(QLayout::SetNoConstraint);

            entryElement = entryElement.nextSiblingElement("entry");
        }

        mpLoadingWebWidget->setVisible(false);
    }

    mpLoadingWebProgressBarTimer->stop();

}


void WelcomeWidget::checkVersion(bool)
{
    //Verify that the loaded page is the correct one, otherwise do not show it
    if(mpWeb->page()->currentFrame()->metaData().contains("type", "hopsanngnews"))
    {
        QString webVersionString = mpWeb->page()->currentFrame()->metaData().find("hopsanversionfull").value();
        webVersionString.remove('.');
        double webVersion = webVersionString.toDouble();
        QString thisVersionString = QString(HOPSANGUIVERSION);
        thisVersionString.remove('.');
        double thisVersion = thisVersionString.toDouble();
        webVersionString = mpWeb->page()->currentFrame()->metaData().find("hopsanversionfull").value();
#ifndef DEVELOPMENT
        mpNewVersionButton->setText("Version " + webVersionString + " is now available!");
        mpNewVersionButton->setVisible(webVersion>thisVersion);
#endif
        mpUpdateLink = mpWeb->page()->currentFrame()->metaData().find("hopsanupdatelink").value();
    }
}


void WelcomeWidget::updateLoadingWebProgressBar()
{
    mpLoadingWebProgressBar->setValue(mpLoadingWebProgressBar->value()+1);
}


//! @brief Loads specified URL in external web browser.
//! @param link Contains the URL to open
void WelcomeWidget::urlClicked(const QUrl &link)
{
    QDesktopServices::openUrl(link);
}


//! @brief Opens the download page in external browser.
void WelcomeWidget::openDownloadPage()
{
    QDesktopServices::openUrl(QUrl(QString(DOWNLOADLINK)));
}


void WelcomeWidget::autoHide()
{
    if(gpMainWindow->mpCentralTabs->currentWidget() != this)
    {
        this->setFixedSize(1,1);
    }
    else
    {
        this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        this->setMinimumSize(0,0);
    }
}
