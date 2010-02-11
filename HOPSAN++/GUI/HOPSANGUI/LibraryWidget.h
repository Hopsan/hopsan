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

#include <QWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QMimeData>
#include <QStringList>
#include "listwidget.h"

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

    QTreeWidget *tree;

    QMap<QString,QListWidget *> libraryMap;

    QVBoxLayout *grid;

//    void addLibrary(QString libraryName);
    void addLibrary(QString libraryName, QString parentLibraryName=QString());
    void addComponent(QString libraryName, QString componentName, QIcon icon, QStringList list);
    void addComponent(QString libraryName, ListWidgetItem *newComponent);

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void hideAllLib();

};

#endif // LIBRARYWIDGET_H
