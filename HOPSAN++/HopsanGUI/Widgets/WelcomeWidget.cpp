#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include <QStringList>
#include <QWebFrame>

#include "Configuration.h"
#include "MainWindow.h"
#include "common.h"
#include "version_gui.h"
#include "Dialogs/OptionsDialog.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/WelcomeWidget.h"

WelcomeWidget::WelcomeWidget(QWidget *parent) :
    QWidget(parent)
{
    int frameH = 230;
    int frameW = 160;
    int spacing = 20;

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
    mpHeading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    mpNewFrame->setFixedSize(frameW,frameH);
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
    mpLoadFrame->setFixedSize(frameW,frameH);
    mpLoadFrame->setLayout(mpLoadLayout);

    mpLastSessionIcon = new QLabel(this);
    mpLastSessionIcon->setPixmap(QPixmap(QString(GRAPHICSPATH) + "lastsession.png"));
    mpLastSessionIcon->setMouseTracking(true);
    mpLastSessionText = new QLabel("Last Session", this);
    mpLastSessionText->setAlignment(Qt::AlignCenter);
    mpLastSessionText->setMouseTracking(true);
    mpLastSessionLayout = new QVBoxLayout(this);
    mpLastSessionLayout->addWidget(mpLastSessionIcon,1,Qt::AlignCenter);
    mpLastSessionLayout->addWidget(mpLastSessionText,0,Qt::AlignBottom);

    mpLastSessionFrame = new QFrame(this);
    mpLastSessionFrame->setFrameShape(QFrame::StyledPanel);
    mpLastSessionFrame->setMouseTracking(true);
    mpLastSessionFrame->setFixedSize(frameW,frameH);
    mpLastSessionFrame->setLayout(mpLastSessionLayout);

    mpRecentList = new QListWidget(this);
    mpRecentList->setFixedSize(frameW-20,frameH-50);
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
    mpRecentFrame->setFixedSize(frameW,frameH);
    mpRecentFrame->setLayout(mpRecentLayout);
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
            while(mpRecentList->fontMetrics().width(displayName) > frameW-35)
            {
                displayName.chop(4);
                displayName.append("...");
            }
            mpRecentList->addItem(displayName);
            mpRecentList->item(mpRecentList->count()-1)->setToolTip(toolTipName);
        }
    }


    mpExampleList = new QListWidget(this);
    mpExampleList->setFixedSize(frameW-20,frameH-50);
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
    mpExampleFrame->setFixedSize(frameW,frameH);
    mpExampleFrame->setLayout(mpExampleLayout);
    QDir exampleModelsDir(QString(MODELS_DEV_PATH+"Example Models/"));
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
            while(mpExampleList->fontMetrics().width(displayName) > frameW-35)
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
    mpOptionsFrame->setFixedSize(frameW,frameH);
    mpOptionsFrame->setLayout(mpOptionsLayout);

    mpLoadingWebProgressBar = new QProgressBar(this);
    mpLoadingWebProgressBar->setRange(0, 0);
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
    mpLoadingWebWidget->setFixedHeight(168);
    mpLoadingWebWidget->setFixedWidth(400);
    mpLoadingWebWidget->setLayout(mpLoadingWebLayout);


    mpWeb = new QWebView(this);
    //mpNewsLabel->hide();
    mpWeb->hide();
    connect(mpWeb, SIGNAL(loadFinished(bool)), this, SLOT(showNews(bool)));
    mpWeb->load(QUrl(QString(NEWSLINK)));
   // mpWeb->setMaximumHeight(140);
    //mpWeb->setMaximumWidth(400);
    mpWeb->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(mpWeb, SIGNAL(linkClicked(const QUrl &)), this, SLOT(urlClicked(const QUrl &)));
    mpWeb->setMouseTracking(true);
    mpNewsText = new QLabel("Hopsan News", this);
    mpNewsText->setAlignment(Qt::AlignCenter);
    mpNewsText->setMouseTracking(true);
    mpNewsLayout = new QVBoxLayout(this);
    mpNewsLayout->addWidget(mpNewsText, 0, Qt::AlignTop);
    mpNewsLayout->addWidget(mpLoadingWebWidget);
    mpNewsLayout->addWidget(mpWeb);
    mpNewsFrame = new QFrame(this);
    mpNewsFrame->setFrameShape(QFrame::StyledPanel);
    mpNewsFrame->setMouseTracking(true);
    mpNewsFrame->setFixedSize(frameW*2,frameH*2+spacing);
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
    mpLayout->addWidget(mpRecentFrame,                      2, 0, 1, 1);
    mpLayout->addWidget(mpExampleFrame,                     2, 1, 1, 1);
    mpLayout->addWidget(mpOptionsFrame,                     2, 2, 1, 1);
    mpLayout->addWidget(mpNewsFrame,                        1, 3, 2, 1);
    mpLayout->addWidget(mpNewVersionButton,                 0, 3, 1, 1);
    mpLayout->setSpacing(spacing);
    mpLayout->setSizeConstraint(QLayout::SetFixedSize);
    //mpLayout->addWidget(pTestLabel);

    this->setLayout(mpLayout);
}


QString WelcomeWidget::getUpdateLink()
{
    return mpUpdateLink;
}


void WelcomeWidget::mouseMoveEvent(QMouseEvent *event)
{
    updateHoverEffects();

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

    if(mpLastSessionFrame->underMouse())
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
        gpMainWindow->mpProjectTabs->addNewProjectTab();
    }
    else if(mpLoadFrame->underMouse())
    {
        gpMainWindow->mpProjectTabs->loadModel();
    }
    else if(mpLastSessionFrame->underMouse())
    {
        for(int i=0; i<gConfig.getLastSessionModels().size(); ++i)
        {
            gpMainWindow->mpProjectTabs->loadModel(gConfig.getLastSessionModels().at(i));
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
    gpMainWindow->mpProjectTabs->loadModel(mRecentModelList.at(mpRecentList->currentIndex().row()));
}


//! @brief Slot that loads an example model, based on the name of the calling action
void WelcomeWidget::openExampleModel()
{
    gpMainWindow->mpProjectTabs->loadModel(QString(MODELS_DEV_PATH)+"Example Models/"+mExampleModelList.at(mpExampleList->currentIndex().row()));
}



//! @brief Slot that shows the news box if the page was successfully loaded.
//! @param loadedSuccessfully True if a page was loaded (this does NOT mean that the loaded page is the correct one!)
void WelcomeWidget::showNews(bool loadedSuccessfully)
{
    //Verify that the loaded page is the correct one, otherwise do not show it
    if(mpWeb->page()->currentFrame()->metaData().contains("type", "hopsanngnews"))
    {
        mpLoadingWebWidget->setVisible(!loadedSuccessfully);
        //mpNewsLabel->setVisible(loadedSuccessfully);
        mpWeb->setVisible(loadedSuccessfully);

        QString webVersionString = mpWeb->page()->currentFrame()->metaData().find("hopsanversionfull").value();
        webVersionString.remove('.');
        double webVersion = webVersionString.toDouble();
        QString thisVersionString = QString(HOPSANGUIVERSION);
        thisVersionString.remove('.');
        double thisVersion = thisVersionString.toDouble();
        webVersionString = mpWeb->page()->currentFrame()->metaData().find("hopsanversionfull").value();
#ifndef DEVELOPMENET
        mpNewVersionButton->setText("Version " + webVersionString + " is now available!");
        mpNewVersionButton->setVisible(webVersion>thisVersion);
#endif
        mpUpdateLink = mpWeb->page()->currentFrame()->metaData().find("hopsanupdatelink").value();

        if(loadedSuccessfully)
        {
            mpLoadingWebProgressBarTimer->stop();
        }
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
