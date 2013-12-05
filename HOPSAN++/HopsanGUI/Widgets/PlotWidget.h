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

#ifndef PlotWidget_H
#define PlotWidget_H

#include <QPushButton>
#include <QTreeWidget>
#include "LogDataHandler.h"

class VariableTree : public QTreeWidget
{
    Q_OBJECT
public:
    VariableTree(QWidget *pParent=0);

    void setLogDataHandler(QPointer<LogDataHandler> pLogDataHandler);
    LogDataHandler *getLogDataHandler();
    void setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow);

    void addFullVariable(SharedLogVariableDataPtrT pData);
    void addAliasVariable(SharedLogVariableDataPtrT pData);
    void addImportedVariable(SharedLogVariableDataPtrT pData);
    void refreshImportedVariables();

    void updateList();
    void clear();

protected slots:
    PlotWindow *plotToPreferedPlotWindow(QTreeWidgetItem *item);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    void resetImportedItemParent();
    void resetAliasItemParent();

    void getExpandedFullVariables(QStringList &rList);
    void getExpandedImportFiles(QStringList &rList);

    void expandImportFileItems(const QStringList &rList);
    void expandFullVariableItems(const QStringList &rList);

    QPointF mDragStartPosition;
    QMap<QString, QTreeWidgetItem*> mFullVariableItemMap;
    QMap<QString, QTreeWidgetItem*> mAliasVariableItemMap;
    QMap<QString, QTreeWidgetItem*> mImportedFileItemMap;

    QTreeWidgetItem* mpImportedItemParent;
    QTreeWidgetItem* mpAliasItemParent;

    //QList<VariableCommonDescription> mAvailableVariables;
    QPointer<LogDataHandler> mpLogDataHandler;
    QPointer<PlotWindow> mpPreferedPlotWindow;
};

class PlotWidget : public QWidget
{
    Q_OBJECT
public:
    PlotWidget(QWidget *pParent=0);
    void setLogDataHandler(QPointer<LogDataHandler> pLogDataHandler);
    LogDataHandler *getLogDataHandler();
    void setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow);

public slots:
    void updateList();
    void clearList();

    void openNewPlotWindow();
    void loadFromXml();

protected:
    virtual void showEvent(QShowEvent *event);

    VariableTree *mpVariableTree;
    QPushButton *mpNewWindowButton;
    QPushButton *mpLoadButton;
    bool mHasPendingUpdate;
};

#endif // PlotWidget_H
