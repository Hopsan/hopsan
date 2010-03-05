//$Id$

#ifndef GUICOMPONENTSELECTIONBOX_H
#define GUICOMPONENTSELECTIONBOX_H

//Should be as few includes as possible in h-files
#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMoveEvent>
#include <vector>

class GUIComponent;

class GUIComponentSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    GUIComponentSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIComponent *parent = 0);
    ~GUIComponentSelectionBox();
    void setActive();
    void setPassive();
    void setHovered();

    GUIComponent *mpParentGUIComponent;

private:
    std::vector<QGraphicsLineItem*> mLines;
    QPen mActivePen;
    QPen mHoverPen;
};

#endif // GUICONNECTOR_H
