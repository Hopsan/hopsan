//$Id$

#ifndef GUICONNECTORLINE_H
#define GUICONNECTORLINE_H

#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMoveEvent>

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
    int getLineNumber();
    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);
    QPointF startPos;
    QPointF endPos;
    void setLine(qreal x1, qreal y1, qreal x2, qreal y2);

public slots:
    void setConnected();

signals:
    void lineClicked();
    void lineMoved(int);
    void lineHoverEnter();
    void lineHoverLeave();
    void lineSelected(bool isSelected);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
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
    bool mParentConnectorEndPortConnected;

};

#endif // GUICONNECTOR_H
