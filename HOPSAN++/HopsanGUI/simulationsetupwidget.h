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


class SimulationSetupWidget : public QGroupBox
{
    Q_OBJECT

public:
    SimulationSetupWidget(const QString &title, QWidget *parent = 0);

    QGroupBox *mpGroupBox;
    QHBoxLayout *mpSimulationLayout;
    QLineEdit *mpStartTimeLabel;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;
    QLineEdit *mpTimeStepLabel;
    QLineEdit *mpFinishTimeLabel;
    QPushButton *mpSimulateButton;

public slots:
    void fixFinishTime();
    void fixTimeStep();

};

#endif // SIMULATIONSETUPWIDGET_H
