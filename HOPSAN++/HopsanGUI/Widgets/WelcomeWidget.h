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
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QWebView>
#include <QPushButton>
#include <QCheckBox>
#include <QScrollArea>

class WelcomeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WelcomeWidget(QWidget *parent = 0);
    QString getUpdateLink();
    void updateRecentList();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);

private:
    QLabel *mpHeading;

    QFrame *mpNewFrame;
    QLabel *mpNewIcon;
    QLabel *mpNewText;
    QVBoxLayout *mpNewLayout;

    QFrame *mpLoadFrame;
    QLabel *mpLoadIcon;
    QLabel *mpLoadText;
    QVBoxLayout *mpLoadLayout;

    QFrame *mpLastSessionFrame;
    QLabel *mpLastSessionIcon;
    QLabel *mpLastSessionText;
    QVBoxLayout *mpLastSessionLayout;

    QFrame *mpRecentFrame;
    QListWidget *mpRecentList;
    QLabel *mpRecentText;
    QVBoxLayout *mpRecentLayout;

    QFrame *mpExampleFrame;
    QListWidget *mpExampleList;
    QLabel *mpExampleText;
    QVBoxLayout *mpExampleLayout;

    QFrame *mpOptionsFrame;
    QLabel *mpOptionsIcon;
    QLabel *mpOptionsText;
    QVBoxLayout *mpOptionsLayout;

    QFrame *mpNewsFrame;
    QVBoxLayout *mpNewsLayout;
    QLabel *mpNewsText;
    QProgressBar *mpLoadingWebProgressBar;
    QLabel *mpLoadingWebLabel;
    QTimer *mpLoadingWebProgressBarTimer;
    QVBoxLayout *mpLoadingWebLayout;
    QWidget *mpLoadingWebWidget;
    QNetworkAccessManager *mpVersioncheckNAM;
    QVBoxLayout *mpNewsScrollLayout;
    QScrollArea *mpNewsScrollArea;
    QWidget *mpNewsScrollWidget;
    QNetworkAccessManager *mpFeed;

    QStringList mRecentModelList;
    QStringList mExampleModelList;

    QVBoxLayout *mpRightLayout;
    QGridLayout *mpLayout;

    QPushButton *mpNewVersionButton;
    QMenu *mpNewVersionMenu;
    QAction *mpAutoUpdateAction;
    QAction *mpGoToDownloadPageAction;

    QString mpUpdateLink;

    QMap <QListWidgetItem *, QString> exampleModelsMap;

    int mFrameH;
    int mFrameW;
    int mSpacing;

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
};

#endif // WELCOMEWIDGET_H
