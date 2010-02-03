#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QIODevice>
#include <QtCore/QMimeData>
#include <QtGui/QDrag>
#include <QtCore/QString>
#include <QMouseEvent>
#include <QPoint>
#include <QApplication>
#include <QObject>

#include "treewidgetitem.h"

class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    TreeWidget(QWidget *parent = 0);
    ~TreeWidget();

    QPoint dragStartPosition;

protected:
    //virtual void startDrag(Qt::DropActions);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
    void test(QTreeWidgetItem *item,int num);
};

#endif // TREEWIDGET_H
