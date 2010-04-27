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

#include <string>
#include <map>
#include <QListWidget>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>
#include "AppearanceData.h"


class LibraryContentItem;

#include <QListWidgetItem>
#include <QStringList>


//Här borde en funktion finnas som växlar mellan iso-ikon och user-ikon
class LibraryContentItem : public QListWidgetItem
{
public:
    LibraryContentItem(AppearanceData *pAppearanceData, QListWidget *parent = 0);
    LibraryContentItem(const QListWidgetItem &other);
    AppearanceData *getAppearanceData();

//public slots:
    void selectIcon(bool useIso=false);

private:
    AppearanceData *mpAppearanceData;

};

//Forward declaration
class LibraryWidget;

class LibraryContent : public QListWidget
{
    Q_OBJECT

public:
    LibraryContent(LibraryContent *pParentLibraryContent, LibraryWidget *pParentLibraryWidget);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    QPoint dragStartPosition;
    LibraryWidget *mpParentLibraryWidget;
};


class MainWindow;

class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    //Member functions
    LibraryWidget(MainWindow *parent = 0);
    void addEmptyLibrary(QString libraryName, QString parentLibraryName=QString());
    void addLibrary(QString libDir, QString parentLib=QString());
    void addLibraryContentItem(QString libraryName, QString parentLibraryName, LibraryContentItem *newComponent);
    AppearanceData *getAppearanceData(QString componentType);

public slots:
    void addLibrary();
    void useIsoGraphics(bool useISO);

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void hideAllLib();

private:
    //Member variables
    MainWindow *mpParentMainWindow;
    QMap<QString, AppearanceData*> mAppearanceDataMap;
    QTreeWidget *mpTree;
    QMap<QString, LibraryContent*> mLibraryContentMapPtrs;
    QVBoxLayout *mpGrid;

};

#endif // LIBRARYWIDGET_H
