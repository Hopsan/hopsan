#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QDragMoveEvent>

class GraphicsView : public QGraphicsView
{
public:
    GraphicsView(QWidget *parent = 0);

    void dragMoveEvent(QDragMoveEvent *event);
};

#endif // GRAPHICSVIEW_H
