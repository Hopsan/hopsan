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

#include <QListWidget>
#include <QStringList>
#include <string>
#include <map>

class QWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;
class ListWidgetItem;

class LibraryContent : public QListWidget
{
    Q_OBJECT

public:
    LibraryContent(QWidget *parent = 0);


    //QMimeData *mimeData(const QList<QListWidgetItem*> items) const;

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    QPoint dragStartPosition;
};


class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    LibraryWidget(QWidget *parent = 0);

    QTreeWidget *mpTree;

    QMap<QString,QListWidget *> mLibraryMapPtrs;

    QVBoxLayout *mpGrid;

    void addEmptyLibrary(QString libraryName, QString parentLibraryName=QString());
    void addLibrary(QString libDir, QString parentLib=QString());
    void addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList parameterData);
    QStringList getParameterData(QString);

public slots:
    void addLibrary();

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void hideAllLib();

private:
    std::map<QString, QStringList> mParameterMap;

};

#endif // LIBRARYWIDGET_H
