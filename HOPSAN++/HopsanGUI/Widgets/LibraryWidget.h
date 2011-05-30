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
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contains classes for Library Widgets
//!
//$Id$

#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include "../common.h"
#include "HopsanCore.h"
#include "CoreAccess.h"

#include <QListWidget>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QStringList>

class GUIModelObjectAppearance;
class LibraryContentItem;

class LibraryContentItem : public QListWidgetItem
{
public:
    LibraryContentItem(GUIModelObjectAppearance *pAppearanceData, QListWidget *parent = 0);
    LibraryContentItem(const QListWidgetItem &other);
    GUIModelObjectAppearance *getAppearanceData();
    QString getTypeName();

//public slots:
    void selectIcon(graphicsType gfxType=USERGRAPHICS);

private:
    GUIModelObjectAppearance *mpAppearanceData;

};

//Forward declaration
class LibraryWidget;

class LibraryContent : public QListWidget
{
    Q_OBJECT

public:
    LibraryContent(LibraryContent *pParentLibraryContent, QString mapKey, LibraryWidget *pParentLibraryWidget, QTreeWidgetItem *pParentTreeWidgetItem);
    bool mIsUserLib;
    QTreeWidgetItem *mpParentTreeWidgetItem;
    QString mMapKey;

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

public slots:
    virtual void highLightItem(QListWidgetItem *item);

private:
    QPoint dragStartPosition;
    LibraryWidget *mpParentLibraryWidget;
    QListWidgetItem *mpHoveredItem;
};


class MainWindow;

class LibraryWidget : public QWidget
{
    Q_OBJECT

    friend class LibraryContent;
    friend class LibraryTreeWidget;
    friend class MainWindow;

public:
    //Member functions
    LibraryWidget(MainWindow *parent = 0);
    void addEmptyLibrary(QString libraryName, QString parentLibraryName=QString(), QString libraryPath=QString(), QString iconPath=QString());
    void addLibrary(QString libDir, QString parentLib=QString());
    void addLibraryContentItem(QString libraryName, QString parentLibraryName, LibraryContentItem *newComponent);
    void addExternalLibrary(QString libDir);
    GUIModelObjectAppearance *getAppearanceData(QString componentType);
    GUIModelObjectAppearance *getAppearanceDataByDisplayName(QString displayName);
    QSize sizeHint() const;

    graphicsType mGfxType;

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

public slots:
    void addLibrary();
    void setGfxType(graphicsType gfxType);

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void hideAllLib();

private:
    //Member variables
    //MainWindow *mpParentMainWindow;

    QHash<QString, LibraryContent*> mLibraryContentPtrsMap;
    QMultiMap<QString, LibraryContentItem*> mLibraryContentItemPtrsMap;

    QHash<QString, QString> mName2TypeMap; //!< @todo This is a temporary hack

    QLabel *mpComponentNameField;

    QTreeWidget *mpTree;
    QVBoxLayout *mpGrid;

    CoreLibraryAccess *mpCoreAccess;
};


class LibraryTreeWidget : public QTreeWidget
{
public:
    LibraryTreeWidget(LibraryWidget *parent = 0);
    LibraryWidget *mpParentLibraryWidget;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *);
};

#endif // LIBRARYWIDGET_H
