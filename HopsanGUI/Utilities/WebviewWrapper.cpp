#include "WebviewWrapper.h"

#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>
#ifdef USEWEBKIT
#include <QWebView>
#else
#include <QWebEngineView>
#endif

#include "common.h"

class WebViewWrapperPrivates {
public:
    QVBoxLayout* mpLayout = nullptr;
#ifdef USEWEBKIT
    QWebView* mpWebView = nullptr;
#else
    QWebEngineView* mpWebView = nullptr;
#endif

};

WebViewWrapper::WebViewWrapper(const bool useToolbar, QWidget *parent) : QWidget(parent), mpPrivates(new WebViewWrapperPrivates())
{
    mpPrivates->mpLayout = new QVBoxLayout(this);
#ifdef USEWEBKIT
    mpPrivates->mpWebView = new QWebView(this);
#else
    mpPrivates->mpWebView = new QWebEngineView(this);
#endif
    if (useToolbar)
    {
#ifdef USEWEBKIT
        QAction *pBackAction = mpPrivates->mpWebView->pageAction(QWebPage::Back);
        QAction *pForwardAction = mpPrivates->mpWebView->pageAction(QWebPage::Forward);
#else
        QAction *pBackAction = mpPrivates->mpWebView->pageAction(QWebEnginePage::Back);
        QAction *pForwardAction = mpPrivates->mpWebView->pageAction(QWebEnginePage::Forward);
#endif
        pBackAction->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-StepLeft.svg")));
        pForwardAction->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-StepRight.svg")));

        QToolBar *pToolBar = new QToolBar(this);
        pToolBar->addAction(pBackAction);
        pToolBar->addAction(pForwardAction);
        mpPrivates->mpLayout->addWidget(pToolBar);
    }
    mpPrivates->mpLayout->addWidget(mpPrivates->mpWebView);
    mpPrivates->mpLayout->setStretch(1,1);
}

WebViewWrapper::~WebViewWrapper()
{
    delete mpPrivates;
    // Note! member data should be deleted automatically by qt when parent dialog is destoryed
}


void WebViewWrapper::loadHtmlFile(const QUrl &url)
{
    mpPrivates->mpWebView->load(url);
#ifdef _WIN32
    mpPrivates->mpWebView->setZoomFactor(1.3);
#endif
}

void WebViewWrapper::showText(const QString &text)
{
    mpPrivates->mpWebView->setHtml(QString("<p>%1</p>").arg(text));
}
