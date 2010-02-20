#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include <QGraphicsWidget>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>
#include <QGraphicsView>
#include "GUIConnector.h"
#include <vector>
#include <QGraphicsItem>


class GUIConnector;

class GUIComponent : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIComponent(const QString &fileName, QString componentName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIComponent();
    QGraphicsView *getParentView();
    void addConnector(GUIConnector *item);

protected:
    //virtual void moveEvent(QGraphicsItem::GraphicsItemChange);
    virtual void moveEvent(QGraphicsItem::GraphicsItemChange *change);

signals:
    void componentMoved();

private:
    QGraphicsView *mParentView;
    std::vector<GUIConnector*> mConnectors;        //Inteded to store connectors for each component
};

#endif // GUICOMPONENT_H
