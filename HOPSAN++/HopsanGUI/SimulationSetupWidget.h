//!
//! @file   simulationsetupwidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-08
//!
//! @brief Contains a class for setting simulation times
//!
//$Id$

#ifndef SIMULATIONSETUPWIDGET_H
#define SIMULATIONSETUPWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QString>
#include <QtGui>


class QHBoxLayout;
class QLineEdit;
class QLabel;
class QPushButton;
class MainWindow;


class SimulationSetupWidget : public QGroupBox
{
    Q_OBJECT

public:
    SimulationSetupWidget(const QString &title, MainWindow *parent = 0);

    void setStartTimeLabel(double startTime);
    void setTimeStepLabel(double timeStep);
    void setFinishTimeLabel(double finishTime);

    double getStartTimeLabel();
    double getTimeStepLabel();
    double getFinishTimeLabel();

    MainWindow *mpParentMainWindow;
    QGroupBox *mpGroupBox;
    QHBoxLayout *mpSimulationLayout;
    QLineEdit *mpStartTimeLabel;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;
    QLineEdit *mpTimeStepLabel;
    QLineEdit *mpFinishTimeLabel;
    QPushButton *mpSimulateButton;

private:
    void setValue(QLineEdit *lineEdit, double value);
    double getValue(QLineEdit *lineEdit);
    void fixFinishTime();
    void fixTimeStep();

public slots:
    void fixLabelValues();

};

#endif // SIMULATIONSETUPWIDGET_H
