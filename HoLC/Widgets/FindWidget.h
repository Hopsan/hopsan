#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>
#include <QLineEdit>

class FindWidget : public QWidget
{
    Q_OBJECT
public:
    FindWidget(QWidget *parent = 0);

signals:
    void findPrevious(QString);
    void findNext(QString);

public slots:
    virtual void setVisible(bool visible);

private slots:
    void findPrevious();
    void findNext();

private:
    QLineEdit *mpFindLineEdit;
};

#endif // FINDWIDGET_H
