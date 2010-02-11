#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QApplication>
#include "listwidgetitem.h"

class ListWidget : public QListWidget
{
public:
    ListWidget(QWidget *parent = 0);
    ~ListWidget();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    QPoint dragStartPosition;
};

#endif // LISTWIDGET_H
