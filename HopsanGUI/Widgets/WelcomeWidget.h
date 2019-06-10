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
//! @file   WelcomeWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012
//!
//! @brief Contains the welcome widget class
//!
//$Id$

#ifndef WELCOMEWIDGET_H
#define WELCOMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QSvgWidget>

class WelcomeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WelcomeWidget(QWidget *parent = 0);
    void updateRecentList();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);

private:
    QFrame *mpNewFrame;
    QSvgWidget *mpNewIcon;
    QLabel *mpNewText;

    QFrame *mpLoadFrame;
    QSvgWidget *mpLoadIcon;
    QLabel *mpLoadText;

    QFrame *mpLastSessionFrame;
    QSvgWidget *mpLastSessionIcon;
    QLabel *mpLastSessionText;

    QFrame *mpRecentFrame;
    QListWidget *mpRecentList;
    QLabel *mpRecentText;

    QFrame *mpExampleFrame;
    QListWidget *mpExampleList;
    QLabel *mpExampleText;

    QFrame *mpOptionsFrame;
    QSvgWidget *mpOptionsIcon;
    QLabel *mpOptionsText;

    QFrame *mpNewsFrame;
    QLabel *mpNewsText;
    QProgressBar *mpLoadingWebProgressBar;
    QLabel *mpLoadingWebLabel;
    QTimer *mpLoadingWebProgressBarTimer;
    QWidget *mpLoadingWebWidget;
    QVBoxLayout *mpNewsScrollLayout;
    QScrollArea *mpNewsScrollArea;
    QWidget *mpNewsScrollWidget;
    QNetworkAccessManager *mpNewsFeedNAM;
    QNetworkAccessManager *mpVersioncheckNAM;

    QStringList mRecentModelList;
    QStringList mExampleModelList;
    QMap <QListWidgetItem*, QString> exampleModelsMap;

    int mFrameH;
    int mFrameW;
    int mSpacing;

    // New version and Auto update
    QPushButton *mpNewVersionButton;
    QNetworkReply *mpAUDownloadStatus;
    QProgressDialog *mpAUDownloadDialog;
    QAction *mpAutoUpdateAction=nullptr;
    QUrl mAutoUpdateFileLink;

signals:
    void hovered();

public slots:
    void autoHide();

private slots:
    void updateHoverEffects();
    void openRecentModel();
    void openExampleModel();
    void showNews(QNetworkReply *pReply);
    void checkVersion(QNetworkReply *pReply);
    void updateLoadingWebProgressBar();
    void urlClicked(const QUrl &link);
    void openDownloadPage();
    void launchAutoUpdate();
    void updateDownloadProgressBar(qint64 bytesReceived, qint64 bytesTotal);
    void commenceAutoUpdate(QNetworkReply* reply);
};

#endif // WELCOMEWIDGET_H
