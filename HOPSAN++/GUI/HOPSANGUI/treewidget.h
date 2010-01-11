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

class TreeWidget : public QTreeWidget
{
public:
    TreeWidget(QWidget *parent = 0);
    ~TreeWidget();

    QTreeWidgetItem *item;
    QByteArray *data;
    QMimeData *mimeData;
    QDrag *drag;

    void startDrag(Qt::DropActions);
};

#endif // TREEWIDGET_H
