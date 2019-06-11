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
    QLabel* mpNotice = nullptr;
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
        pBackAction->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-StepLeft.svg")));
        QAction *pForwardAction = mpPrivates->mpWebView->pageAction(QWebPage::Forward);
        pForwardAction->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-StepRight.svg")));

        QToolBar *pToolBar = new QToolBar(this);
        pToolBar->addAction(pBackAction);
        pToolBar->addAction(pForwardAction);

        mpPrivates->mpLayout->addWidget(pToolBar);
    }
    mpPrivates->mpLayout->addWidget(mpPrivates->mpWebView);
    mpPrivates->mpLayout->setStretch(1,1);
#else
    Q_UNUSED(useToolbar)
    mpPrivates->mpNotice = new QLabel(this);
    mpPrivates->mpText = new QLabel(this);
    mpPrivates->mpText->setOpenExternalLinks(true);
    mpPrivates->mpLayout->addWidget(mpPrivates->mpNotice);
    mpPrivates->mpLayout->addWidget(mpPrivates->mpText);
    mpPrivates->mpLayout->addStretch(1);
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
    mpPrivates->mpNotice->setText("Sorry, no WebKit or WebEngine support in this release, open in external browser!");
    mpPrivates->mpNotice->show(); // If previously hidden by showText
    mpPrivates->mpText->setText(QString("<a href=\"%1\">%2</a>").arg(url.toString()).arg(url.toString()));
#endif

}

void WebViewWrapper::showText(const QString &text)
{
#ifdef USEWEBKIT
    mpPrivates->mpWebView->setHtml(QString("<p>%1</p>").arg(text));
#else
    mpPrivates->mpNotice->hide();
    mpPrivates->mpText->setText(text);
#endif
}
