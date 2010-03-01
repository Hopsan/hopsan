//$Id$

#ifndef GUICONNECTORLINE_H
#define GUICONNECTORLINE_H

//Should be as few includes as possible in h-files
#include <QPen>
//#include <QCursor>
//#include <QBrush>
//#include <QGraphicsScene>
#include <QGraphicsLineItem>
//#include <vector>
//#include <QGraphicsRectItem>
//#include <QCursor>
//#include <QBrush>
//#include <QGraphicsLineItem>
//#include <QGraphicsScene>
//#include "GUIConnector.h"
//#include "GUIPort.h"
//#include <vector>
//#include "GUIConnectorLine.h"
//#include "GUIConnector.h"
//#include <QGraphicsLineItem>
//#include <QGraphicsItem>
#include <QGraphicsSceneMoveEvent>

class GUIConnector;     //Forward declaration

class GUIConnectorLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, QGraphicsItem *parent = 0);
    ~GUIConnectorLine();
    void setActive();
    void setPassive();
    void setHovered();
    enum geometryType {VERTICAL, HORIZONTAL, DIAGONAL};
    geometryType getGeometry();
    void setGeometry(geometryType geometry);

signals:
    void lineClicked();
    void lineMoved(int);
    void lineHoverEnter();
    void lineHoverLeave();
    void lineSelected();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void moveEvent(QGraphicsSceneMoveEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    bool mIsActive;
    QPen mPrimaryPen;
    QPen mActivePen;
    QPen mHoverPen;
    int mLineNumber;
    geometryType mGeometry;


};

#endif // GUICONNECTOR_H
