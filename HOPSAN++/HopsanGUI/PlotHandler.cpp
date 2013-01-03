#include "PlotHandler.h"

#include "GUIObjects/GUIContainerObject.h"
#include "PlotWindow.h"
#include "LogDataHandler.h"
#include "MainWindow.h"


#include "Widgets/PlotWidget.h" //!< @todo remove this later /Peter


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
        name = "PlotWindow"+mOpenPlotWindows.size();
    }

    PlotWindow* pPlotWindow = getPlotWindow(name);
    if (pPlotWindow==0)
    {
        pPlotWindow = new PlotWindow(gpMainWindow->mpPlotWidget->mpPlotVariableTree, gpMainWindow);
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

    PlotWindow *plotWindow = new PlotWindow(gpMainWindow->mpPlotWidget->mpPlotVariableTree, gpMainWindow);
    plotWindow->show();

    if(name.isEmpty())
    {
        name = "PlotWindow"+mOpenPlotWindows.size();
    }

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

void PlotHandler::addPlotCurve(QString windowName, LogVariableData *pData, int axis, QColor curveColor)
{
    PlotWindow *pWindow = getPlotWindow(windowName);
    if (pWindow)
    {
        pWindow->addPlotCurve(pData, axis, QString(), curveColor);
    }
    //! @todo add some error/warning message else
}
