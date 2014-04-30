#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QTextEdit>

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MessageWidget(QWidget *parent = 0);

signals:

public slots:
    void addText(const QString &text, const QColor color);
    void clear();

private:
    QTextEdit *mpTextEdit;

};

#endif // MESSAGEWIDGET_H
