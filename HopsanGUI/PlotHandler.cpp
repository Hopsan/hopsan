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
#include "PlotCurve.h"
#include "Configuration.h"

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
        if(gpConfig->getBoolSetting(cfg::plotwindowsontop))
            pPlotWindow = new PlotWindow(name, gpMainWindowWidget);
        else
            pPlotWindow = new PlotWindow(name, 0);
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

    PlotWindow *pPlotWindow;
    if(gpConfig->getBoolSetting(cfg::plotwindowsontop))
        pPlotWindow = new PlotWindow(rName, gpMainWindowWidget);
    else
        pPlotWindow = new PlotWindow(rName, 0);
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
    // Destruction of plot window data will happen by itself so we don't need to do that here
    mOpenPlotWindows.remove(mOpenPlotWindows.key(pWindow));
}


PlotWindow *PlotHandler::getPlotWindow(const QString &rName)
{
    if(mOpenPlotWindows.contains(rName))
    {
        return mOpenPlotWindows.find(rName).value();
    }
    return 0;
}

PlotWindow *PlotHandler::plotDataToWindow(QString windowName, SharedVectorVariableT data, int axis, PlotCurveStyle style)
{
    PlotWindow *pWindow = createNewPlotWindowOrGetCurrentOne(windowName);
    plotDataToWindow(pWindow, data, axis, true, style);
    return pWindow;
}

PlotWindow *PlotHandler::plotDataToWindow(QString windowName, SharedVectorVariableT xdata, SharedVectorVariableT ydata, int axis, PlotCurveStyle style)
{
    if (xdata && ydata)
    {
        PlotWindow *pWindow = createNewPlotWindowOrGetCurrentOne(windowName);
        plotDataToWindow(pWindow, xdata, ydata, axis, true, style);
        return pWindow;
    }
    return 0;
}

PlotWindow *PlotHandler::setPlotWindowXData(PlotWindow *pPlotWindow, SharedVectorVariableT xdata, bool force)
{
    if(!pPlotWindow)
    {
        pPlotWindow = createNewPlotWindowOrGetCurrentOne();
    }
    pPlotWindow->setXData(xdata,force);
    return pPlotWindow;
}

PlotWindow *PlotHandler::plotDataToWindow(PlotWindow *pPlotWindow, SharedVectorVariableT data, int axis, bool autoRefresh, PlotCurveStyle style)
{
    if(!pPlotWindow)
    {
        pPlotWindow = createNewPlotWindowOrGetCurrentOne();
    }
    PlotCurve *pCurve = pPlotWindow->addPlotCurve(data, QwtPlot::Axis(axis), style);
    // We want to preserve the internal auto update setting from the curve if it is off, so we can actually only turn it off here
    if (pCurve)
    {
        pCurve->setAutoUpdate(autoRefresh && pCurve->isAutoUpdating());
    }
    return pPlotWindow;
}

PlotWindow *PlotHandler::plotDataToWindow(PlotWindow *pPlotWindow, SharedVectorVariableT xdata, SharedVectorVariableT ydata, int axis, bool autoRefresh, PlotCurveStyle style)
{
    if(!pPlotWindow)
    {
        pPlotWindow = createNewPlotWindowOrGetCurrentOne();
    }
    PlotCurve *pCurve = pPlotWindow->addPlotCurve(xdata, ydata, QwtPlot::Axis(axis), style);
    // We want to preserve the internal auto update setting from the curve if it is off, so we can actually only turn it off here
    if (pCurve)
    {
        pCurve->setAutoUpdate(autoRefresh && pCurve->isAutoUpdating());
    }
    return pPlotWindow;
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
