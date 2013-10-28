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
//! @file   WelcomeWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012
//!
//! @brief Contains the welcome widget class
//!
//$Id: UndoWidget.h 3547 2011-10-25 11:48:47Z petno25 $

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
    QLabel *mpNewIcon;
    QLabel *mpNewText;

    QFrame *mpLoadFrame;
    QLabel *mpLoadIcon;
    QLabel *mpLoadText;

    QFrame *mpLastSessionFrame;
    QLabel *mpLastSessionIcon;
    QLabel *mpLastSessionText;

    QFrame *mpRecentFrame;
    QListWidget *mpRecentList;
    QLabel *mpRecentText;

    QFrame *mpExampleFrame;
    QListWidget *mpExampleList;
    QLabel *mpExampleText;

    QFrame *mpOptionsFrame;
    QLabel *mpOptionsIcon;
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
    QString mAUFileLink;

signals:
    void hovered();

public slots:
    void autoHide();

private slots:
    void debugSlot();
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
