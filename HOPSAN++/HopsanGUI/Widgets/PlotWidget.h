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
//! @brief Contains the PlotWidget and otehr plot related classes
//!
//$Id$

#ifndef PlotWidget_H
#define PlotWidget_H

#include <QGridLayout>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QString>

#include "LogDataHandler.h"

class MainWindow;
class ContainerObject;
class PlotWindow;
class PlotTreeWidget;
class PlotVariableTree;


class PlotVariableTreeItem : public QTreeWidgetItem
{
public:
    PlotVariableTreeItem(LogVariableData *pData, QTreeWidgetItem *parent);
    LogVariableData *getDataPtr();
    QString getFullName() const;
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();
    QString getAliasName();

private:
    LogVariableData *mpData;
};


class PlotVariableTree : public QTreeWidget
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotTreeWidget;
public:
    PlotVariableTree(MainWindow *parent = 0);
    PlotWindow *createPlotWindow(LogVariableData *pData, QColor desiredColor=QColor(), QString name="");
    PlotWindow *createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit, QString name="");
    PlotWindow *getPlotWindow(QString name);
    void reportClosedPlotWindow(PlotWindow *window);

    ContainerObject *mpCurrentContainer;

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    QPointF dragStartPosition;

public slots:
    void updateList();
    PlotWindow *createPlotWindow(QTreeWidgetItem *item);

private:
    QList<VariableDescription> mAvailableVariables;
    QMap<QString, PlotWindow*> mOpenPlotWindows;
};


class PlotTreeWidget : public QWidget
{
    Q_OBJECT
public:
    PlotTreeWidget(MainWindow *parent = 0);
    PlotVariableTree *mpPlotVariableTree;

public slots:
    void loadFromXml();
    void clearHoverEffects();

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

private:
    QPushButton *mpLoadButton;
    QGridLayout *mpLayout;
};

#endif // PlotWidget_H
