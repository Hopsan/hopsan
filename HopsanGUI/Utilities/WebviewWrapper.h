#ifndef WEBVIEWWRAPPER_H
#define WEBVIEWWRAPPER_H

#include <QWidget>
#include <QUrl>

class WebViewWrapperPrivates;

class WebViewWrapper : public QWidget
{
    Q_OBJECT
public:
    explicit WebViewWrapper(const bool useToolbar, QWidget *parent = nullptr);
    ~WebViewWrapper();
    void loadHtmlFile(const QUrl &url);
    void showText(const QString &text);

private:
    WebViewWrapperPrivates* mpPrivates;

signals:

public slots:
};

#endif // WEBVIEWWRAPPER_H
