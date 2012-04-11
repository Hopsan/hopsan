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
    QWebView *mpWeb;
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

signals:
    void hovered();

public slots:

private slots:
    void debugSlot();
    void updateHoverEffects();
    void openRecentModel();
    void openExampleModel();
    void showNews(QNetworkReply *pReply);
    void checkVersion(bool loadedSuccessfully);
    void updateLoadingWebProgressBar();
    void urlClicked(const QUrl &link);
    void openDownloadPage();
};

#endif // WELCOMEWIDGET_H
