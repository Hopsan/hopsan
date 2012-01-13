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

#include "common.h"
//#include "HopsanCore.h"
#include "CoreAccess.h"

#include <QListWidget>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QStringList>
#include <QVector>
#include <QToolButton>
#include <QDir>

class ModelObjectAppearance;
class LibraryListWidgetItem;

class LibraryComponent;
class MainWindow;
class LibraryContentsTree;
class LibraryListWidget;

class LibraryWidget : public QWidget
{
    Q_OBJECT

    friend class LibraryListWidget;

public:
    //Member functions
    LibraryWidget(MainWindow *parent = 0);
    void update();
    void loadTreeView(LibraryContentsTree *tree, QTreeWidgetItem *parentItem = 0);
    void loadDualView(LibraryContentsTree *tree, QTreeWidgetItem *parentItem = 0);
    void loadLibrary(QString libDir, const InternalExternalEnumT int_ext = INTERNAL);
    void loadAndRememberExternalLibrary(const QString libDir);
    void unloadExternalLibrary(const QString libName);
    void updateExternalLibraries();
    void loadHiddenSecretDir(QString dir);

    ModelObjectAppearance *getAppearanceData(QString componentType);
    QSize sizeHint() const;

    graphicsType mGfxType;

    QLabel *mpComponentNameField;

    QStringList mLoadedComponents;

public slots:
    void generateComponent();
    void addExternalLibrary(QString libDir = QString());
    void importFmu();
    void setGfxType(graphicsType gfxType);
    void setListView();
    void setDualView();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void initializeDrag(QListWidgetItem* item);
    void initializeDrag(QTreeWidgetItem* item, int dummy);

private:
    void loadLibraryFolder(QString libDir, const QString libRootDir, const bool doRecurse, LibraryContentsTree *pParentTree=0);
    void updateLibraryFolder(LibraryContentsTree *pTree);
    void unLoadLibrarySubTree(LibraryContentsTree *pTree);

    YesNoToAllEnumT mUpConvertAllCAF;

    LibraryContentsTree *mpContentsTree;
    LibraryContentsTree *mpSecretHiddenContentsTree;

    QTreeWidget *mpTree;

    LibraryListWidget *mpList;
    QToolButton *mpTreeViewButton;
    QToolButton *mpDualViewButton;
    QToolButton *mpGenerateComponentButton;
    QToolButton *mpLoadExternalButton;
    QToolButton *mpLoadFmuButton;
    QGridLayout *mpGrid;
    int mViewMode;
    CoreLibraryAccess *mpCoreAccess;
    QMap<QListWidgetItem *, LibraryComponent *> mListItemToContentsMap;
    QMap<QTreeWidgetItem *, LibraryComponent *> mTreeItemToContentsMap;
    QMap<QTreeWidgetItem *, LibraryContentsTree *> mTreeItemToContentsTreeMap;

    QDir mUpdateXmlBackupDir;
};


class LibraryListWidget : public QListWidget
{
    Q_OBJECT
public:
    LibraryListWidget(LibraryWidget *parent);
protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
private:
    LibraryWidget *mpLibraryWidget;
};


class LibraryContentsTree
{
public:
    LibraryContentsTree(QString name = QString(), LibraryContentsTree* pParent=0);
    bool isEmpty();
    LibraryContentsTree *addChild(QString name);
    bool removeChild(QString name);
    LibraryContentsTree *findChildByName(QString name);
    LibraryContentsTree *findChildByPath(QString path);
    LibraryComponent *addComponent(ModelObjectAppearance *pAppearanceData);
    LibraryComponent *findComponent(QString typeName);

    QString mName;
    QString mLibDir;
    QVector<QString> mLoadedLibraryDLLs;
    QVector<LibraryContentsTree *> mChildNodesPtrs;
    QVector<LibraryComponent *> mComponentPtrs;
    LibraryContentsTree* mpParent;

};


class LibraryComponent
{
public:
    LibraryComponent(ModelObjectAppearance *pAppearanceData);
    QIcon getIcon(graphicsType gfxType);
    QString getName();
    QString getTypeName();
    ModelObjectAppearance *getAppearanceData();

private:
    ModelObjectAppearance *mpAppearanceData;
    QIcon mUserIcon;
    QIcon mIsoIcon;
};

#endif // LIBRARYWIDGET_H
