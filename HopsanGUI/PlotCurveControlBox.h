/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   PlotCurveControlBox.h
//! @author Flumes
//! @date   2014
//!
//! @brief Contains a class for plot curve control box
//!
//$Id: ModelHandler.cpp 5551 2013-06-20 08:54:16Z petno25 $

#ifndef PLOTCURVECONTROLBOX_H
#define PLOTCURVECONTROLBOX_H

#include <QObject>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include <QLineEdit>
#include <QLabel>

// Forward declarations
class PlotArea;
class PlotCurve;
class VectorVariable;

class CustomXDataControl : public QWidget
{
    Q_OBJECT
public:
    CustomXDataControl(QWidget *pParent);
    void updateInfo(const VectorVariable *pData);

signals:
    void newXData(QString fullName);
    void resetXData();

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

private:
    QLabel *mpNameLabel;
    QLabel *mpXLabel;
    QToolButton *mpResetXDataButton;
};

class PlotCurveControlBox : public QWidget
{
    Q_OBJECT
public:
    PlotCurveControlBox(PlotCurve *pPlotCurve, PlotArea *pParentArea);
    PlotCurve *getCurve();

signals:
    void removeCurve(PlotCurve* pCurve);

public slots:
    void updateInfo();
    void updateColor(const QColor color);
    void markActive(bool active);

private slots:
    void activateCurve(bool active);
    void setXData(QString fullName);
    void resetXData();
    void setGeneration(const int gen);
    void removeTheCurve();

private:
    void refreshTitle();
    PlotCurve *mpPlotCurve;
    PlotArea *mpPlotArea;
    QLabel *mpTitle, *mpGenerationLabel, *mpSourceLable;
    QToolButton *mpColorBlob;
    QSpinBox *mpGenerationSpinBox;
    CustomXDataControl *mpCustomXDataDrop;

};

#endif // PLOTCURVECONTROLBOX_H
