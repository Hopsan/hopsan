//$Id:$

#ifndef PREFERENCEWIDGET_H
#define PREFERENCEWIDGET_H

#include <QMainWindow>

class PreferenceWidget : public QMainWindow
{
    Q_OBJECT

public:
    PreferenceWidget(QWidget *parent = 0);
    ~PreferenceWidget();

    QWidget *mpCentralwidget;
//    QGridLayout *mpCentralgrid;
//    QGridLayout *mpTabgrid;
};

#endif // PREFERENCEWIDGET_H
