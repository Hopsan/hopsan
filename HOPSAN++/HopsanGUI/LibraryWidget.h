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
    LibraryContentItem(const QIcon &icon, const QString &text, QListWidget *parent = 0);
    LibraryContentItem(AppearanceData *pAppearanceData, QListWidget *parent = 0);
    LibraryContentItem(const QListWidgetItem &other);

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

    //QMimeData *mimeData(const QList<QListWidgetItem*> items) const;

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
    void addComponent(QString libraryName, QString parentLibraryName, LibraryContentItem *newComponent, QStringList parameterData);
    QStringList getAppearanceData(QString);

    //Member variables
    MainWindow *mpParentMainWindow;
    QTreeWidget *mpTree;
    QMap<QString, QListWidget *> mLibraryMapPtrs;
    QVBoxLayout *mpGrid;

public slots:
    void addLibrary();

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void hideAllLib();

private:
    std::map<QString, QStringList> mAppearanceDataMap;

};

#endif // LIBRARYWIDGET_H
