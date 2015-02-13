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
//! @file   PlotHandler.h
//! @author Peter Nordin <robert.braun@liu.se>
//! @date   2013
//!
//! @brief Contains a handler for plotting
//!
//$Id$

#ifndef PLOTHANDLER_H
#define PLOTHANDLER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QColor>
#include <QSharedPointer>

// Forward Declarations
class ContainerObject;
class PlotWindow;
class VectorVariable;
class HopsanVariable;

class PlotHandler : public QObject
{
    Q_OBJECT
public:
    explicit PlotHandler(QObject *pParent=0);
    ~PlotHandler();

    void createNewPlotWindowIfItNotAlreadyExists(QString name="");
    PlotWindow *createNewUniquePlotWindow(const QString &rName);
    PlotWindow *createNewPlotWindowOrGetCurrentOne(QString name="");
    PlotWindow *createNewOrReplacePlotwindow(const QString &rName="");
    PlotWindow *getPlotWindow(const QString &rName);

    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow,   HopsanVariable data,  int axis, bool autoRefresh=true, QColor curveColor=QColor(), int type=1, int thickness=2);
    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow,   HopsanVariable xdata, HopsanVariable ydata, int axis, bool autoRefresh=true, QColor curveColor=QColor(), int type=1, int thickness=2);
    PlotWindow *setPlotWindowXData(PlotWindow *pPlotWindow, HopsanVariable xdata, bool force=false);

    PlotWindow *plotDataToWindow(QString windowName,        HopsanVariable data,  int axis, QColor curveColor=QColor(), int type=1, int thickness=2);
    PlotWindow *plotDataToWindow(QString windowName,        HopsanVariable xdata, HopsanVariable ydata, int axis, QColor curveColor=QColor(), int type=1, int thickness=2);


    void closeWindow(const QString &rWindowName);
    void closeAllOpenWindows();

signals:
    
protected slots:
    void forgetPlotWindow(PlotWindow *pWindow);

private:
    QMap<QString, PlotWindow*> mOpenPlotWindows;
    
};

#endif // PLOTHANDLER_H
