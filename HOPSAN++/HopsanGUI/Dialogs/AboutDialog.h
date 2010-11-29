//$Id$

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>

class MainWindow;
class QTimer;

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(MainWindow *parent = 0);
    QTimer *timer;

protected:
    void keyPressEvent(QKeyEvent *event);

public slots:
    void update();
    void setDate();

private:
    QLabel *mpHopsanLogotype;
    int num;
    QString title;
    bool dateOk;
};

#endif // ABOUTDIALOG_H
