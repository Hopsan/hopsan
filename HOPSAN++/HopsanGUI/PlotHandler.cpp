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
//! @file   PlotHandler.cpp
//! @author Peter Nordin <robert.braun@liu.se>
//! @date   2013
//!
//! @brief Contains a handler for plotting
//!
//$Id$

#include "PlotHandler.h"

#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "PlotWindow.h"
#include "LogDataHandler.h"
#include "MainWindow.h"
#include "PlotCurve.h"

PlotHandler::PlotHandler(QObject *pParent) : QObject(pParent){}

PlotHandler::~PlotHandler()
{
    closeAllOpenWindows();
}

//! @brief Creates a new PlotWindow or nothing if window with desired name already exist
//! @param [in] name PlotWindow name
void PlotHandler::createNewPlotWindowIfItNotAlreadyExists(QString name)
{
    createNewPlotWindowOrGetCurrentOne(name);
}

//! @brief Returns ptr to new PlotWindow or existing one if window with desired name already exists
//! @param [in] name PlotWindow name
PlotWindow *PlotHandler::createNewPlotWindowOrGetCurrentOne(QString name)
{
    if(name.isEmpty())
    {
        // Make sure we find a unique name
        int ctr = 0;
        do
        {
            name = QString("PlotWindow%1").arg(ctr++);
        }while(mOpenPlotWindows.contains(name));
    }

    PlotWindow* pPlotWindow = getPlotWindow(name);
    if (pPlotWindow==0)
    {
        pPlotWindow = new PlotWindow(name, gpMainWindow);
        pPlotWindow->show();
        mOpenPlotWindows.insert(name, pPlotWindow);
        connect(pPlotWindow, SIGNAL(windowClosed(PlotWindow*)), this, SLOT(forgetPlotWindow(PlotWindow*)));
    }

    return pPlotWindow;
}

//! @brief Returns ptr to a new PlotWindow, the supplied name will be used as display name but the actual window name is hidden
//! @param [in] name PlotWindow name
PlotWindow *PlotHandler::createNewUniquePlotWindow(const QString &rName)
{
    // Here we assume that no one will actually name a plotwindow like this
    const QString actualName = "HopsanVeryUniquePlotWindow";

    // Make sure we find a unique name
    int ctr = 0;
    QString keyName = actualName;
    while(mOpenPlotWindows.contains(keyName))
    {
        keyName = actualName+QString("%1").arg(ctr++);
    }

    PlotWindow *pPlotWindow = new PlotWindow(rName, gpMainWindow);
    pPlotWindow->show();
    mOpenPlotWindows.insert(keyName, pPlotWindow);
    connect(pPlotWindow, SIGNAL(windowClosed(PlotWindow*)), this, SLOT(forgetPlotWindow(PlotWindow*)));

    return pPlotWindow;
}

PlotWindow *PlotHandler::createNewOrReplacePlotwindow(const QString &rName)
{
    PlotWindow *pWindow = getPlotWindow(rName);
    if (pWindow)
    {
        // We clear all contents, this will then look like a new plot window, but remain in the same position and size
        pWindow->closeAllTabs();
    }
    return createNewPlotWindowOrGetCurrentOne(rName);
}

void PlotHandler::forgetPlotWindow(PlotWindow *pWindow)
{
    // Destruction of plot window data will happen by itself so we dont need to do that here
    mOpenPlotWindows.remove(mOpenPlotWindows.key(pWindow));
}

//! @todo write equivalent function
//!< @todo FIXA /Peter
PlotWindow *PlotHandler::createPlotWindow(QVector<double> xVector, QVector<double> yVector, int /*axis*/, QString /*componentName*/, QString /*portName*/, QString /*dataName*/, QString /*dataUnit*/, QString name)
{
    if((xVector.isEmpty()) || (yVector.isEmpty()))
        return 0;

    if(name.isEmpty())
    {
        name = "PlotWindow"+mOpenPlotWindows.size();
    }

    PlotWindow *plotWindow = new PlotWindow(name, gpMainWindow);
    plotWindow->show();



    mOpenPlotWindows.insert(name, plotWindow);

    //plotWindow->addPlotCurve(0, componentName, portName, dataName, dataUnit, axis);
    //!< @todo FIXA /Peter

    return plotWindow;
}

PlotWindow *PlotHandler::getPlotWindow(const QString &rName)
{
    if(mOpenPlotWindows.contains(rName))
    {
        return mOpenPlotWindows.find(rName).value();
    }
    return 0;
}

QString PlotHandler::plotDataToWindow(QString windowName, SharedLogVariableDataPtrT pData, int axis, QColor curveColor)
{
    PlotWindow *pWindow = createNewPlotWindowOrGetCurrentOne(windowName);
    plotDataToWindow(pWindow, pData, axis, curveColor);
    return pWindow->getName();
}

QString PlotHandler::plotDataToWindow(QString windowName, SharedLogVariableDataPtrT pDataX, SharedLogVariableDataPtrT pDataY, int axis, QColor curveColor)
{
    if (pDataX && pDataY)
    {
        PlotWindow *pWindow = createNewPlotWindowOrGetCurrentOne(windowName);
        plotDataToWindow(pWindow, pDataX, pDataY, axis, curveColor);
        return pWindow->getName();
    }
    return "";
}

void PlotHandler::closeWindow(const QString &rWindowName)
{
    PlotWindow *pPW = getPlotWindow(rWindowName);
    if (pPW)
    {
        pPW->close();
    }
}

void PlotHandler::closeAllOpenWindows()
{
    while (!mOpenPlotWindows.empty())
    {
        PlotWindow* pPW = mOpenPlotWindows.begin().value();
        pPW->close();
    }
}

PlotWindow *PlotHandler::plotDataToWindow(PlotWindow *pPlotWindow, SharedLogVariableDataPtrT pData, int axis, QColor curveColor)
{
    if(!pPlotWindow)
    {
        pPlotWindow = createNewPlotWindowOrGetCurrentOne();
    }
    pPlotWindow->addPlotCurve(pData, axis, curveColor);

    return pPlotWindow;
}

PlotWindow *PlotHandler::plotDataToWindow(PlotWindow *pPlotWindow, SharedLogVariableDataPtrT pDataX, SharedLogVariableDataPtrT pDataY, int axis, QColor curveColor)
{
    if(!pPlotWindow)
    {
        pPlotWindow = createNewPlotWindowOrGetCurrentOne();
    }
    pPlotWindow->addPlotCurve(pDataX, pDataY, axis, curveColor);
    return pPlotWindow;
}
