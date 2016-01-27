/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   PlotCurveControlBox.h
//! @author Flumes
//! @date   2014
//!
//! @brief Contains a class for plot curve control box
//!
//$Id$

#ifndef PLOTCURVECONTROLBOX_H
#define PLOTCURVECONTROLBOX_H

#include <QObject>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>

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
    void hideCurve(PlotCurve* pCurve);
    void showCurve(PlotCurve* pCurve);

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
    QCheckBox *mpAutoUpdateCheckBox;
    QCheckBox *mpInvertCurveCheckBox;
    QToolButton *mpColorBlob;
    QSpinBox *mpGenerationSpinBox;
    CustomXDataControl *mpCustomXDataDrop;

};

#endif // PLOTCURVECONTROLBOX_H
