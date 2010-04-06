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


//class QWidget;
//class QTreeWidget;
//class QTreeWidgetItem;
//class QVBoxLayout;
class LibraryContentItem;

#include <QListWidgetItem>
//#include <QStringList>


class QStringList;

class LibraryContentItem : public QListWidgetItem
{
public:
    LibraryContentItem(const QIcon &icon, const QString &text, QListWidget *parent = 0);
    LibraryContentItem(const QListWidgetItem &other);
//    ~LibraryContentItem();

//    void setAppearanceData(QStringList list);
//    QStringList getAppearanceData();

private:
//    QStringList mAppearanceData;
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
    LibraryWidget(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

    QTreeWidget *mpTree;

    QMap<QString,QListWidget *> mLibraryMapPtrs;

    QVBoxLayout *mpGrid;

    void addEmptyLibrary(QString libraryName, QString parentLibraryName=QString());
    void addLibrary(QString libDir, QString parentLib=QString());
    void addComponent(QString libraryName, QString parentLibraryName, LibraryContentItem *newComponent, QStringList parameterData);
    QStringList getAppearanceData(QString);

public slots:
    void addLibrary();

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void hideAllLib();

private:
    std::map<QString, QStringList> mAppearanceDataMap;

};

#endif // LIBRARYWIDGET_H
