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
//! @file   LibraryWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-10-23
//!
//! @brief Contains classes for Library Widgets
//!
//$Id$

#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

//Qt includes
#include <QObject>
#include <QWidget>
#include <QSize>
#include <QTreeWidget>
#include <QListWidget>
#include <QToolButton>
#include <QLabel>

//Hopsan includes
#include "GUIObjects/GUIModelObjectAppearance.h"

//Forward declarations
class LibraryHandler;


//! @brief Library widget class
class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    // Public Member functions
    LibraryWidget(QWidget *parent=0);
    QSize sizeHint() const;
    void setGfxType(GraphicsTypeEnumT gfxType);

    GraphicsTypeEnumT mGfxType;

public slots:
    void update();

private slots:
    void handleItemClick(QTreeWidgetItem* item, int);
    void handleItemClick(QListWidgetItem* item);
    void handleItemEntered(QListWidgetItem* item);

protected:
    void mouseMoveEvent(QMouseEvent *event);


private:

    void getAllSubTreeItems(QTreeWidgetItem *pParentItem, QList<QTreeWidgetItem *> &rSubItems);

    //GUI Stuff
    QTreeWidget *mpTree;
    QTreeWidget *mpDualTree;
    QListWidget *mpList;
    QTreeWidgetItem *mpLoadLibraryItem;
    QTreeWidgetItem *mpAddModelicaComponentItem;
    QTreeWidgetItem *mpAddCppComponentItem;
    QTreeWidgetItem *mpLoadLibraryItemDual;
    QTreeWidgetItem *mpAddModelicaComponentItemDual;
    QTreeWidgetItem *mpAddCppComponentItemDual;
    QLabel *mpComponentNameLabel;
    QToolButton *mpTreeViewButton;
    QToolButton *mpDualViewButton;
    QToolButton *mpHelpButton;

    //Maps between GUI objects and library contents
    QMap<QTreeWidgetItem *, QString> mItemToTypeNameMap;        //Map between component items and typenames
    QMap<QListWidgetItem *, QString> mListItemToTypeNameMap;    //Map between component items in dual view list and typenames
    QMap<QTreeWidgetItem *, QStringList> mItemToLibFilesMap;    //Map between component items and libraries it might origin from
    QMap<QTreeWidgetItem *, QStringList> mFolderToContentsMap;  //Map between folders and typenames of sub-components, for updating list in dual view
};

#endif // LIBRARYWIDGET_H
