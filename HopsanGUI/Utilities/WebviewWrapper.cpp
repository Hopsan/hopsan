#include "WebviewWrapper.h"

#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>
#ifdef USEWEBKIT
#include <QWebView>
#else
#include <QLabel>
#endif

#include "common.h"

class WebViewWrapperPrivates {
public:
    QVBoxLayout* mpLayout = nullptr;
#ifdef USEWEBKIT
    QWebView* mpWebView = nullptr;
#else
    QLabel* mpText = nullptr;
#endif

};

WebViewWrapper::WebViewWrapper(const bool useToolbar, QWidget *parent) : QWidget(parent), mpPrivates(new WebViewWrapperPrivates())
{
    mpPrivates->mpLayout = new QVBoxLayout(this);
#ifdef USEWEBKIT
    mpPrivates->mpWebView = new QWebView(this);
    if (useToolbar)
    {
        QAction *pBackAction = mpPrivates->mpWebView->pageAction(QWebPage::Back);
        pBackAction->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-StepLeft.png")));
        QAction *pForwardAction = mpPrivates->mpWebView->pageAction(QWebPage::Forward);
        pForwardAction->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-StepRight.png")));

        QToolBar *pToolBar = new QToolBar(this);
        pToolBar->addAction(pBackAction);
        pToolBar->addAction(pForwardAction);

        mpPrivates->mpLayout->addWidget(pToolBar);
    }
    mpPrivates->mpLayout->addWidget(mpPrivates->mpWebView);
    mpPrivates->mpLayout->setStretch(1,1);
#else
    Q_UNUSED(useToolbar)
    mpPrivates->mpText = new QLabel(this);
    mpPrivates->mpText->setOpenExternalLinks(true);
    mpPrivates->mpLayout->addWidget(mpPrivates->mpText);
#endif
}

WebViewWrapper::~WebViewWrapper()
{
    delete mpPrivates;
    // Note! member data should be deleted automatically by qt when parent dialog is destoryed
}


void WebViewWrapper::loadHtmlFile(const QUrl &url)
{
#ifdef USEWEBKIT
    mpPrivates->mpWebView->load(url);
#else
    mpPrivates->mpText->setText(QString("<a href=\"%1\">%2</a>").arg(url.toString()).arg(url.toString()));
#endif

}

void WebViewWrapper::showText(const QString &text)
{
#ifdef USEWEBKIT
    mpPrivates->mpWebView->setHtml(QString("<p>%1</p>").arg(text));
#else
    mpPrivates->mpText->setText(text);
#endif
}
