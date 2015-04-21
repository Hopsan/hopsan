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
};


#endif // PLOTWIDGET2_H
