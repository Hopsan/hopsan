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
class LogVariableData;
typedef QSharedPointer<LogVariableData> SharedLogVariableDataPtrT;

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

    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow, SharedLogVariableDataPtrT pData, int axis, QColor curveColor=QColor());
    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow, SharedLogVariableDataPtrT pDataX, SharedLogVariableDataPtrT pDataY, int axis, QColor curveColor=QColor());
    QString plotDataToWindow(QString windowName, SharedLogVariableDataPtrT pData, int axis, QColor curveColor=QColor());
    QString plotDataToWindow(QString windowName, SharedLogVariableDataPtrT pDataX, SharedLogVariableDataPtrT pDataY, int axis, QColor curveColor=QColor());

    void closeWindow(const QString &rWindowName);
    void closeAllOpenWindows();

    PlotWindow *createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit, QString name="");
    
signals:
    
protected slots:
    void forgetPlotWindow(PlotWindow *pWindow);

private:
    QMap<QString, PlotWindow*> mOpenPlotWindows;
    
};

#endif // PLOTHANDLER_H
