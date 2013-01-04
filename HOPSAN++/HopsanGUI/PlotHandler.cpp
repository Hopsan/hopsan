#include "PlotHandler.h"

#include "GUIObjects/GUIContainerObject.h"
#include "PlotWindow.h"
#include "LogDataHandler.h"
#include "MainWindow.h"

PlotHandler::PlotHandler(ContainerObject *pContainerObject) :
    QObject(pContainerObject)
{
    mpContainerObject = pContainerObject;
}

//! @brief Creates a new PlotWindow or nothing if window with desired name already exist
//! @param [in] name PlotWindow name
void PlotHandler::createPlotWindow(QString name)
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

void PlotHandler::forgetPlotWindow(PlotWindow *pWindow)
{
    // Destruction of plot window data will happen by itself so we dont need to do that here
    mOpenPlotWindows.remove(mOpenPlotWindows.key(pWindow));
}

//! @todo write equivalent function
PlotWindow *PlotHandler::createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit, QString name)
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

PlotWindow *PlotHandler::getPlotWindow(const QString name)
{
    if(mOpenPlotWindows.contains(name))
    {
        return mOpenPlotWindows.find(name).value();
    }
    return 0;
}

QString PlotHandler::plotDataToWindow(QString windowName, LogVariableData *pData, int axis, QColor curveColor)
{
    PlotWindow *pWindow = createNewPlotWindowOrGetCurrentOne(windowName);
    plotDataToWindow(pWindow, pData, axis, curveColor);
    return pWindow->getName();
}

PlotWindow *PlotHandler::plotDataToWindow(PlotWindow *pPlotWindow, LogVariableData *pData, int axis, QColor curveColor)
{
    if(!pPlotWindow)
    {
        pPlotWindow = createNewPlotWindowOrGetCurrentOne();
    }
    pPlotWindow->addPlotCurve(pData, axis, QString(), curveColor);

    return pPlotWindow;
}
