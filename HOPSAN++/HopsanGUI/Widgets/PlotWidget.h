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


class PlotVariableTreeItem : public QTreeWidgetItem
{
public:
    PlotVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *parent);
    SharedLogVariableDataPtrT getDataPtr();
    QString getFullName() const;
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();
    QString getAliasName();

private:
    SharedLogVariableDataPtrT mpData;
};


class PlotVariableTree : public QTreeWidget
{
    Q_OBJECT
    friend class PlotTreeWidget;
public:
    PlotVariableTree(MainWindow *parent = 0);
    ContainerObject *mpCurrentContainer;

public slots:
    void updateList();

protected slots:
    PlotWindow *createPlotWindow(QTreeWidgetItem *item);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    QPointF dragStartPosition;

private:
    QList<VariableDescription> mAvailableVariables;
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
