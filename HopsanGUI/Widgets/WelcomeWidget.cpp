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
//! @file   WelcomeWidget.cpp
//!
//! @brief Contains the welcome widget class
//!
//$Id$

// Qt includes
#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QXmlStreamReader>
#include <QNetworkReply>
#include <QProgressBar>
#include <QMenu>
#include <QApplication>
#include <QDesktopServices>
#include <QProcess>
#include <QDate>

// Hopsan includes
#include "Widgets/WelcomeWidget.h"
#include "global.h"
#include "common.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/OptionsDialog.h"
#include "ModelHandler.h"
#include "Widgets/ProjectTabWidget.h"
#include "MessageHandler.h"
#include "version_gui.h"

// Hopsan Core includes
#include "CoreUtilities/HmfLoader.h"

namespace {

QString withTrailingSlash(QString text)
{
    if (!text.endsWith('/')) {
        text.append('/');
    }
    return text;
}

QString withoutLeadingSlash(QString text)
{
    while (text.startsWith('/')) {
        text.remove(0,1);
    }
    return text;
}

void resolveRelativeURL(const QUrl& baseURL, QUrl& rUrl)
{
    if (rUrl.isValid() && rUrl.isRelative()) {
        rUrl = baseURL.resolved(rUrl);
    }
}

struct HopsanRelease
{
    QString version;
    QUrl base_url;
    QUrl url_installer_with_compiler;
    QUrl url_installer_wo_compiler;

    void resolveInstallerURLs()
    {
        resolveRelativeURL(base_url, url_installer_with_compiler);
        resolveRelativeURL(base_url, url_installer_wo_compiler);
    }
};

QVector<HopsanRelease> parseHopsanReleases(QXmlStreamReader &reader, const QString &minimumVersion)
{
    QVector<HopsanRelease> releases;
    while (reader.readNextStartElement() && reader.name() == "release")
    {
        bool addThis = true;
        HopsanRelease release;
        while (reader.readNextStartElement())
        {
            if (reader.name() == "version")
            {
                release.version =  reader.readElementText();
                addThis = hopsan::isVersionAGreaterThanB( release.version.toStdString().c_str(), minimumVersion.toStdString().c_str());
            }
            else if (reader.name() == "url")
            {
                release.base_url = withTrailingSlash(reader.readElementText());
            }
#ifdef _WIN32
#ifdef HOPSANCOMPILED64BIT
            else if (reader.name() == "win64_installer_with_compiler")
            {
                release.url_installer_with_compiler = withoutLeadingSlash(reader.readElementText());
            }
            else if (reader.name() == "win64_installer_wo_compiler")
            {
                release.url_installer_wo_compiler = withoutLeadingSlash(reader.readElementText());
            }
#else
            else if (reader.name() == "win32_installer_with_compiler")
            {
                release.url_installer_with_compiler = withoutLeadingSlash(reader.readElementText());
            }
            else if (reader.name() == "win32_installer_wo_compiler")
            {
                release.url_installer_wo_compiler = withoutLeadingSlash(reader.readElementText());
            }
#endif
#endif
            else
            {
                // Discard element text (so that readNextStartElement proceeds to the next sibling)
                reader.readElementText();
            }
        }
        release.resolveInstallerURLs();

        if (addThis)
        {
            releases.append(release);
        }
    }
    return releases;
}

HopsanRelease getNewestRelease(QVector<HopsanRelease> &official, QVector<HopsanRelease> &development)
{
    // Assume sorted, newest first
    if (official.isEmpty() && !development.isEmpty())
    {
        return development.front();
    }
    else if (!official.isEmpty() && development.isEmpty())
    {
        return official.front();
    }
    else
    {
        if (hopsan::isVersionAGreaterThanB(official.front().version.toStdString().c_str(),
                                           development.front().version.toStdString().c_str()))
        {
            return official.front();
        }
        else
        {
            return development.front();
        }
    }
}

} // End anonymous namespace

WelcomeWidget::WelcomeWidget(QWidget *parent) :
    QWidget(parent)
{
    mFrameH = 180;
    mFrameW = 140;
    mSpacing = 20;

    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_NoMousePropagation, false);

    QSvgWidget *pHeading = new QSvgWidget(this);
    QDate today = QDate::currentDate();
    if(today.month() == 12 && today.day() > 20 && today.day() < 31)
        pHeading->load(QString(GRAPHICSPATH) + "welcome_xmas.svg");
    else if((today.month() == 12 && today.day() == 31) || (today.month() == 1 && today.day() == 1))
        pHeading->load(QString(GRAPHICSPATH) + "welcome_newyear.svg");
    else
        pHeading->load(QString(GRAPHICSPATH) + "welcome.svg");

    mpNewIcon = new QSvgWidget(QString(GRAPHICSPATH) + "new.svg", this);
    mpNewIcon->setFixedSize(110,110);
    mpNewIcon->setMouseTracking(true);
    mpNewText = new QLabel("New Model", this);
    mpNewText->setAlignment(Qt::AlignCenter);
    mpNewText->setMouseTracking(true);
    QVBoxLayout *pNewLayout = new QVBoxLayout(this);
    pNewLayout->addWidget(mpNewIcon,1,Qt::AlignCenter);
    pNewLayout->addWidget(mpNewText,0,Qt::AlignBottom);

    mpNewFrame = new QFrame(this);
    mpNewFrame->setFrameShape(QFrame::StyledPanel);
    mpNewFrame->setMouseTracking(true);
    mpNewFrame->setFixedSize(mFrameW,mFrameH);
    mpNewFrame->setLayout(pNewLayout);

    mpLoadIcon = new QSvgWidget(QString(GRAPHICSPATH) + "open.svg", this);
    mpLoadIcon->setFixedSize(110,110);
    mpLoadIcon->setMouseTracking(true);
    mpLoadText = new QLabel("Load Model", this);
    mpLoadText->setAlignment(Qt::AlignCenter);
    mpLoadText->setMouseTracking(true);
    QVBoxLayout *pLoadLayout = new QVBoxLayout(this);
    pLoadLayout->addWidget(mpLoadIcon,1,Qt::AlignCenter);
    pLoadLayout->addWidget(mpLoadText,0,Qt::AlignBottom);

    mpLoadFrame = new QFrame(this);
    mpLoadFrame->setFrameShape(QFrame::StyledPanel);
    mpLoadFrame->setMouseTracking(true);
    mpLoadFrame->setFixedSize(mFrameW,mFrameH);
    mpLoadFrame->setLayout(pLoadLayout);

    mpLastSessionIcon = new QSvgWidget(QString(GRAPHICSPATH) + "lastsession.svg", this);
    mpLastSessionIcon->setFixedSize(110,110);
    mpLastSessionIcon->setMouseTracking(true);
    mpLastSessionIcon->setEnabled(!gpConfig->getLastSessionModels().empty());
    mpLastSessionText = new QLabel("Last Session", this);
    mpLastSessionText->setAlignment(Qt::AlignCenter);
    mpLastSessionText->setMouseTracking(true);
    mpLastSessionText->setEnabled(!gpConfig->getLastSessionModels().empty());
    QVBoxLayout *pLastSessionLayout = new QVBoxLayout(this);
    pLastSessionLayout->addWidget(mpLastSessionIcon,1,Qt::AlignCenter);
    pLastSessionLayout->addWidget(mpLastSessionText,0,Qt::AlignBottom);

    mpLastSessionFrame = new QFrame(this);
    mpLastSessionFrame->setFrameShape(QFrame::StyledPanel);
    mpLastSessionFrame->setMouseTracking(true);
    mpLastSessionFrame->setFixedSize(mFrameW,mFrameH);
    mpLastSessionFrame->setLayout(pLastSessionLayout);

    mpRecentList = new QListWidget(this);
    //mpRecentList->setMinimumSize(mFrameW*2+mSpacing-20,mFrameH-50);
    mpRecentList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpRecentList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpRecentList->setMouseTracking(true);
    //mpRecentList->setResizeMode(QListView::Adjust);
    connect(mpRecentList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(openRecentModel()));
    mpRecentText = new QLabel("Recent Models", this);
    mpRecentText->setAlignment(Qt::AlignCenter);
    mpRecentText->setMouseTracking(true);
    QVBoxLayout *pRecentLayout = new QVBoxLayout(this);
    pRecentLayout->addWidget(mpRecentList,1);
    pRecentLayout->addWidget(mpRecentText,0,Qt::AlignBottom);
    //mpRecentLayout->setStretch(0,1);
    mpRecentFrame = new QFrame(this);
    mpRecentFrame->setFrameShape(QFrame::StyledPanel);
    mpRecentFrame->setMouseTracking(true);
    //mpRecentFrame->setMinimumSize(mFrameW*2+mSpacing,mFrameH);
    mpRecentFrame->setMinimumWidth(mFrameW+mSpacing);
    mpRecentFrame->setLayout(pRecentLayout);
    mpRecentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    updateRecentList();


    mpExampleList = new QListWidget(this);
    //mpExampleList->setMinimumSize(mFrameW*2+mSpacing-20,mFrameH-50);
    mpExampleList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpExampleList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpExampleList->setMouseTracking(true);
    connect(mpExampleList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(openExampleModel()));
    mpExampleText = new QLabel("Example Models", this);
    mpExampleText->setAlignment(Qt::AlignCenter);
    mpExampleText->setMouseTracking(true);
    QVBoxLayout *pExampleLayout = new QVBoxLayout(this);
    pExampleLayout->addWidget(mpExampleList,1);
    pExampleLayout->addWidget(mpExampleText,0,Qt::AlignBottom);
    pExampleLayout->setStretch(0,1);
    mpExampleFrame = new QFrame(this);
    mpExampleFrame->setFrameShape(QFrame::StyledPanel);
    mpExampleFrame->setMouseTracking(true);
    //mpExampleFrame->setMinimumSize(mFrameW*2+mSpacing,mFrameH);
    mpExampleFrame->setMinimumWidth(mFrameW+mSpacing);
    mpExampleFrame->setLayout(pExampleLayout);
    mpExampleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QDir exampleModelsDir(gpDesktopHandler->getMainPath()+"Models/Example Models/");
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

    mpOptionsIcon = new QSvgWidget(QString(GRAPHICSPATH) + "options.svg", this);
    mpOptionsIcon->setFixedSize(110,110);
    mpOptionsIcon->setMouseTracking(true);
    mpOptionsText = new QLabel("Options", this);
    mpOptionsText->setAlignment(Qt::AlignCenter);
    mpOptionsText->setMouseTracking(true);
    QVBoxLayout *pOptionsLayout = new QVBoxLayout(this);
    pOptionsLayout->addWidget(mpOptionsIcon,1,Qt::AlignCenter);
    pOptionsLayout->addWidget(mpOptionsText,0,Qt::AlignBottom);

    mpOptionsFrame = new QFrame(this);
    mpOptionsFrame->setFrameShape(QFrame::StyledPanel);
    mpOptionsFrame->setMouseTracking(true);
    mpOptionsFrame->setFixedSize(mFrameW,mFrameH);
    mpOptionsFrame->setLayout(pOptionsLayout);

    mpLoadingWebProgressBar = new QProgressBar(this);
    mpLoadingWebProgressBar->setRange(0, 0);
    mpLoadingWebProgressBar->setFixedWidth(mFrameW*1.95);
    mpLoadingWebLabel = new QLabel("Loading news...", this);
    mpLoadingWebProgressBarTimer = new QTimer(this);
    connect(mpLoadingWebProgressBarTimer, SIGNAL(timeout()), this, SLOT(updateLoadingWebProgressBar()));
    mpLoadingWebProgressBarTimer->setInterval(1);
    mpLoadingWebProgressBarTimer->start();

    QVBoxLayout *pLoadingWebLayout = new QVBoxLayout(this);
    pLoadingWebLayout->addWidget(mpLoadingWebLabel);
    pLoadingWebLayout->addWidget(mpLoadingWebProgressBar);
    pLoadingWebLayout->setAlignment(mpLoadingWebLabel, Qt::AlignCenter);
    pLoadingWebLayout->setAlignment(mpLoadingWebProgressBar, Qt::AlignCenter);
    mpLoadingWebWidget = new QWidget(this);
    //mpLoadingWebWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    //mpLoadingWebWidget->setFixedHeight(168);
    //mpLoadingWebWidget->setFixedWidth(mFrameW*1.9);
    mpLoadingWebWidget->setLayout(pLoadingWebLayout);


    mpVersioncheckNAM = new QNetworkAccessManager(this);
    connect(mpVersioncheckNAM, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkVersion(QNetworkReply*)));
    mpVersioncheckNAM->get(QNetworkRequest(QUrl(hopsanweblinks::releases)));

    mpNewsFeedNAM = new QNetworkAccessManager(this);
    connect(mpNewsFeedNAM, SIGNAL(finished(QNetworkReply*)), this, SLOT(showNews(QNetworkReply*)));
    mpNewsFeedNAM->get(QNetworkRequest(QUrl(hopsanweblinks::news)));

    mpNewsScrollWidget = new QWidget(this);
    //mpNewsScrollWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpNewsScrollLayout = new QVBoxLayout(mpNewsScrollWidget);
    mpNewsScrollArea = new QScrollArea(this);
    mpNewsScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpNewsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpNewsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpNewsScrollArea->setWidget(mpNewsScrollWidget);
    mpNewsScrollArea->setWidgetResizable(true);

    mpNewsText = new QLabel("Hopsan News", this);
    mpNewsText->setAlignment(Qt::AlignCenter);
    mpNewsText->setMouseTracking(true);
    //mpNewsText->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QVBoxLayout *pNewsLayout = new QVBoxLayout(this);
    pNewsLayout->addWidget(mpNewsText, 0, Qt::AlignTop);
    pNewsLayout->addWidget(mpLoadingWebWidget);
    //mpNewsLayout->setAlignment(mpLoadingWebWidget, Qt::AlignCenter);
    //mpNewsLayout->addWidget(mpWeb);
    pNewsLayout->addWidget(mpNewsScrollArea);
    mpNewsFrame = new QFrame(this);
    //mpNewsFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpNewsFrame->setFrameShape(QFrame::StyledPanel);
    mpNewsFrame->setMouseTracking(true);
    //mpNewsFrame->setMinimumSize(mFrameW*2,mFrameH*2+mSpacing);
    //mpNewsFrame->setMinimumWidth(mFrameW*2);
    mpNewsFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpNewsFrame->setLayout(pNewsLayout);

    QMenu *pNewVersionMenu = new QMenu(this);
#ifdef _WIN32
    mpAutoUpdateAction = new QAction("Launch Auto Updater", this);
    pNewVersionMenu->addAction(mpAutoUpdateAction);
    connect(mpAutoUpdateAction, SIGNAL(triggered()), this, SLOT(launchAutoUpdate()));
#endif
    QAction *pGoToDownloadPageAction = new QAction("Open Download Page In Browser", this);
    pNewVersionMenu->addAction(pGoToDownloadPageAction);
    connect(pGoToDownloadPageAction, SIGNAL(triggered()), this, SLOT(openDownloadPage()));

    mpNewVersionButton = new QPushButton("New Version Available!");
    QPalette tempPalette = mpNewVersionButton->palette();
    tempPalette.setColor(QPalette::ButtonText, QColor("darkred"));
    mpNewVersionButton->setPalette(tempPalette);
    QFont tempFont = mpNewVersionButton->font();
    tempFont.setPixelSize(14);
    tempFont.setBold(true);
    mpNewVersionButton->setFont(tempFont);
    mpNewVersionButton->hide();
    mpNewVersionButton->setMenu(pNewVersionMenu);

    QGridLayout *mpLayout = new QGridLayout(this);
    mpLayout->addWidget(pHeading,                           0, 0, 1, 3);
    mpLayout->addWidget(mpNewFrame,                         1, 0, 1, 1);
    mpLayout->addWidget(mpLoadFrame,                        1, 1, 1, 1);
    mpLayout->addWidget(mpLastSessionFrame,                 1, 2, 1, 1);
    mpLayout->addWidget(mpOptionsFrame,                     1, 3, 1, 1);
    mpLayout->addWidget(mpRecentFrame,                      2, 0, 1, 2);
    mpLayout->addWidget(mpExampleFrame,                     2, 2, 1, 2);
    mpLayout->addWidget(mpNewsFrame,                        1, 4, 2, 1);
    mpLayout->addWidget(mpNewVersionButton,                 0, 4, 1, 1);
    mpLayout->setSpacing(mSpacing);
    mpLayout->setRowStretch(2,1);
    mpLayout->setColumnStretch(4, 1);
    //mpLayout->setSizeConstraint(QLayout::SetFixedSize);
    //mpLayout->addWidget(pTestLabel);

    this->setLayout(mpLayout);

    connect(gpCentralTabWidget, SIGNAL(currentChanged(int)), this, SLOT(autoHide()));
}


void WelcomeWidget::updateRecentList()
{
    mpRecentList->clear();
    mRecentModelList.clear();
    QStringList recentModels = gpConfig->getRecentModels();
    for(int i=0; i<recentModels.size(); ++i)
    {
        if(!recentModels.at(i).isEmpty())
        {
            mRecentModelList.append(recentModels.at(i));
            QString displayName = recentModels.at(i);
            displayName = displayName.section('/', -1);
            if (displayName.endsWith(".hmf") || displayName.endsWith(".xml") )
            {
                displayName.chop(4);
            }
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


void WelcomeWidget::updateHoverEffects()
{
    if(mpNewFrame->underMouse())
    {
        mpNewFrame->setFrameShape(QFrame::Panel);
        mpNewIcon->load(QString(GRAPHICSPATH) + "new-active.svg");
    }
    else
    {
        mpNewFrame->setFrameShape(QFrame::StyledPanel);
        mpNewIcon->load(QString(GRAPHICSPATH) + "new.svg");
    }

    if(mpLoadFrame->underMouse())
    {
        mpLoadFrame->setFrameShape(QFrame::Panel);
        mpLoadIcon->load(QString(GRAPHICSPATH) + "open-active.svg");
    }
    else
    {
        mpLoadFrame->setFrameShape(QFrame::StyledPanel);
        mpLoadIcon->load(QString(GRAPHICSPATH) + "open.svg");
    }

    if(mpLastSessionFrame->underMouse() && mpLastSessionIcon->isEnabled())
    {
        mpLastSessionFrame->setFrameShape(QFrame::Panel);
        mpLastSessionIcon->load(QString(GRAPHICSPATH) + "lastsession-active.svg");
    }
    else
    {
        mpLastSessionFrame->setFrameShape(QFrame::StyledPanel);
        mpLastSessionIcon->load(QString(GRAPHICSPATH) + "lastsession.svg");
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
        mpOptionsIcon->load(QString(GRAPHICSPATH) + "options-active.svg");
    }
    else
    {
        mpOptionsFrame->setFrameShape(QFrame::StyledPanel);
        mpOptionsIcon->load(QString(GRAPHICSPATH) + "options.svg");
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
        gpModelHandler->addNewModel();
    }
    else if(mpLoadFrame->underMouse())
    {
        gpModelHandler->loadModel();
    }
    else if(mpLastSessionFrame->underMouse())
    {
        for(int i=0; i<gpConfig->getLastSessionModels().size(); ++i)
        {
            gpModelHandler->loadModel(gpConfig->getLastSessionModels().at(i));
        }
    }
    else if(mpOptionsFrame->underMouse())
    {
        gpOptionsDialog->show();
    }

    QWidget::mousePressEvent(event);
}



//! @brief Opens selected recent model from the list and closes the welcome dialog.
void WelcomeWidget::openRecentModel()
{
    gpModelHandler->loadModel(mRecentModelList.at(mpRecentList->currentIndex().row()));
}


//! @brief Slot that loads an example model, based on the name of the calling action
void WelcomeWidget::openExampleModel()
{
    gpModelHandler->loadModel(gpDesktopHandler->getMainPath()+"Models/Example Models/"+mExampleModelList.at(mpExampleList->currentIndex().row()));
}



//! @brief Slot that shows the news box if the page was successfully loaded.
//! @param loadedSuccessfully True if a page was loaded (this does NOT mean that the loaded page is the correct one!)
void WelcomeWidget::showNews(QNetworkReply *pReply)
{
    QVariant redirection = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirection.toUrl().isEmpty())
    {
        mpNewsFeedNAM->get(QNetworkRequest(redirection.toUrl()));
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
            QString html = entryElement.firstChildElement("content").text();//.left(120).append("...");

            QLabel *pTitleLabel = new QLabel("<a href=\""+link+"\">"+title+"</a>", this);
            pTitleLabel->setOpenExternalLinks(true);
            QFont boldFont = pTitleLabel->font();
            boldFont.setBold(true);
            pTitleLabel->setFont(boldFont);
            pTitleLabel->setWordWrap(true);

            QLabel *pDateLabel = new QLabel(date, this);
            QFont italicFont = pDateLabel->font();
            italicFont.setItalic(true);
            pDateLabel->setFont(italicFont);
            pDateLabel->setWordWrap(true);

            QLabel *pContentLabel = new QLabel(html, this);
            pContentLabel->setWordWrap(true);

            mpNewsScrollLayout->addWidget(pTitleLabel);
            mpNewsScrollLayout->addWidget(pDateLabel);
            mpNewsScrollLayout->addWidget(pContentLabel);
            //mpNewsScrollLayout->setSizeConstraint(QLayout::SetNoConstraint);

            entryElement = entryElement.nextSiblingElement("entry");
        }
        mpNewsScrollLayout->addWidget(new QWidget(mpNewsScrollWidget), 1);

        mpLoadingWebWidget->setVisible(false);
    }

    mpLoadingWebProgressBarTimer->stop();

}


void WelcomeWidget::checkVersion(QNetworkReply *pReply)
{
    QVariant redirection = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirection.toUrl().isEmpty())
    {
        mpVersioncheckNAM->get(QNetworkRequest(redirection.toUrl()));
        return;
    }

    if (pReply->error() == QNetworkReply::NoError)
    {
        QByteArray all = pReply->readAll();
        QXmlStreamReader reader(all);

        QString format;
        QVector<HopsanRelease> official_releases, development_release;
        while (!reader.atEnd())
        {
            if (reader.readNextStartElement() && reader.name() == "hopsan_releases")
            {
                format = reader.attributes().value("format").toString();
            }

            if (format == "1")
            {
                if (reader.readNextStartElement() && reader.name() == "official")
                {
                    official_releases = parseHopsanReleases(reader, HOPSANRELEASEVERSION);
                }

                if (reader.readNextStartElement() && gpConfig->getBoolSetting(cfg::checkfordevelopmentupdates) && reader.name() == "development")
                {
                    development_release = parseHopsanReleases(reader, HOPSANRELEASEVERSION);
                }
            }
        }

        if (official_releases.size() + development_release.size() > 0)
        {
            // Assume sorted, first is highest
            HopsanRelease newest_release = getNewestRelease(official_releases, development_release);

            mpNewVersionButton->setText("Version " + newest_release.version + " is now available!");
#ifdef DEVELOPMENT
            mpNewVersionButton->setVisible(true);
#else
	    
            mpNewVersionButton->setVisible(hopsan::isVersionAGreaterThanB(newest_release.version.toStdString().c_str(), HOPSANRELEASEVERSION));
#endif
            if (gpDesktopHandler->getIncludedCompilerPath().isEmpty())
            {
                mAutoUpdateFileLink = newest_release.url_installer_wo_compiler;
            }
            else
            {
                mAutoUpdateFileLink = newest_release.url_installer_with_compiler;
            }

            // Disable auto update if no file given
            if (mpAutoUpdateAction && mAutoUpdateFileLink.isEmpty())
            {
                mpAutoUpdateAction->setDisabled(true);
            }
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
    QDesktopServices::openUrl(QUrl(hopsanweblinks::releases_archive));
}


void WelcomeWidget::autoHide()
{
    if(gpCentralTabWidget->currentWidget() != this)
    {
        this->setFixedSize(1,1);
    }
    else
    {
        this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        this->setMinimumSize(0,0);
    }
}

//! @brief Private slot that updates the progress bar during auto update downloads
//! @param bytesReceived Number of bytes downloaded
//! @param bytesTotal Total number of bytes to download
void WelcomeWidget::updateDownloadProgressBar(qint64 bytesReceived, qint64 bytesTotal)
{
    //! @todo this download / update stuff should be in a class by itself
    int progress = 100*bytesReceived/bytesTotal;
    mpAUDownloadDialog->setValue(progress);
}


//! @brief Private slot that saves the downloaded installer file, launches it and quit Hopsan
//! @param reply Contains information about the downloaded installation executable
void WelcomeWidget::commenceAutoUpdate(QNetworkReply* reply)
{
    QFileInfo auf_info(mAutoUpdateFileLink.toString());
    const QString file_name = gpDesktopHandler->getTempPath()+QString("/%1").arg(auf_info.fileName());
    QUrl update_url = reply->url();
    if (reply->error())
    {
        gpMessageHandler->addErrorMessage("Download of " + QString(update_url.toEncoded().constData()) + "failed: "+reply->errorString()+"\n");
        return;
    }
    else
    {
        QFile file(file_name);
        if (!file.open(QIODevice::WriteOnly)) {
            gpMessageHandler->addErrorMessage(QString("Could not open %1 for writing.").arg(file_name));
            return;
        }
        file.write(reply->readAll());
        file.close();
    }
    reply->deleteLater();

    QProcess *pProcess = new QProcess();
    QStringList arguments;
    QString dir = gpDesktopHandler->getExecPath();
    dir.chop(4);    //Remove "bin"
    // Note Do NOT add "dir" quotes to dir here, then QProcess or innosetup will somehow messup the dir argument and add C:\ twice.
    // QProcess::start will add " " automatically if needed (on windows)
    arguments << QString("/dir=%1").arg(dir);
    pProcess->startDetached(file_name, arguments);
    pProcess->deleteLater();
    QApplication::quit();
}

//! @brief This will attempt to download the latest installer and (if successful) launch it in silent mode and close Hopsan
//! @todo Disable this in Linux/Mac releases, or make it work for those platforms as well.
void WelcomeWidget::launchAutoUpdate()
{
    QNetworkAccessManager *pNetworkManager = new QNetworkAccessManager();
    connect(pNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(commenceAutoUpdate(QNetworkReply*)));

    mpAUDownloadDialog = new QProgressDialog("Downloading new version...", "Cancel",0, 100, this);
    mpAUDownloadDialog->setWindowTitle("Hopsan Auto Updater");
    mpAUDownloadDialog->setWindowModality(Qt::WindowModal);
    mpAUDownloadDialog->setMinimumWidth(300);
    mpAUDownloadDialog->setValue(0);

    auto request = QNetworkRequest(mAutoUpdateFileLink);
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    mpAUDownloadStatus = pNetworkManager->get(request);
    QVariant redirection = mpAUDownloadStatus->attribute(QNetworkRequest::RedirectionTargetAttribute);
    const auto redirectionUrl = redirection.toUrl();
    if(!redirectionUrl.isEmpty()) {
        mpAUDownloadStatus = pNetworkManager->get(QNetworkRequest(redirectionUrl));
    }
#else
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    mpAUDownloadStatus = pNetworkManager->get(request);
#endif
    connect(mpAUDownloadStatus, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDownloadProgressBar(qint64, qint64)));
    connect(mpAUDownloadDialog, SIGNAL(canceled()), mpAUDownloadStatus, SLOT(abort()));
    connect(mpAUDownloadDialog, SIGNAL(canceled()), mpAUDownloadDialog, SLOT(close()));
}
