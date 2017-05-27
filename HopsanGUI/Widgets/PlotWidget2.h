/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   PlotWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains the PlotWidget and related classes
//!
//$Id$

#ifndef PLOTWIDGET2_H
#define PLOTWIDGET2_H

#include "LogDataHandler2.h"

// Forward declaration
class GenerationSelector;
class VariableTree;

class PlotWidget2 : public QWidget
{
    Q_OBJECT
public:
    PlotWidget2(QWidget *pParent=0);
    void setLogDataHandler(QPointer<LogDataHandler2> pLogDataHandler);
    LogDataHandler2 *getLogDataHandler();
    void setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow);

public slots:
    void updateList();
    void clearList();

    void openNewPlotWindow();

protected:
    virtual void showEvent(QShowEvent *event);

    GenerationSelector *mpGenerationSelector;
    VariableTree *mpVariableTree;
    bool mHasPendingUpdate;
    QMap<QString, QPair<QStringList, QStringList>> mExpandedItems;
};


#endif // PLOTWIDGET2_H
