//$Id$

#ifndef GUICONNECTOR_H
#define GUICONNECTOR_H


#include <vector>
#include <QGraphicsWidget>
#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMoveEvent>
#include "GUIPort.h"
#include "GUIConnectorLine.h"


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
    void addEndArrow();
    void addStartArrow();
    void setPen(const QPen &pen);

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
    QGraphicsLineItem *mArrowLine1;
    QGraphicsLineItem *mArrowLine2;
    bool mHasStartArrow;
    bool mHasEndArrow;
    qreal mArrowSize;
    qreal mArrowAngle;

};


class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, QPen passivePen, QPen activePen, QPen hoverPen, GraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIConnector();
    QPointF startPos;
    QPointF endPos;
    void setStartPort(GUIPort *port);
    void setEndPort(GUIPort *port);
    GUIPort *getStartPort();
    GUIPort *getEndPort();
    void drawLine(QPointF startPos, QPointF endPos);
    void addLine();
    void addFixedLine(int length, int heigth, GUIConnectorLine::geometryType geometry);
    void removeLine(QPointF cursorPos);
    void setPen(QPen pen);
    int getNumberOfLines();
    int getLineNumber();
    GUIConnectorLine *getLine(int line);
    GUIConnectorLine *getOldLine();
    GUIConnectorLine *getLastLine();
    GUIConnectorLine *getThisLine();
    std::vector<GUIConnectorLine*> mLines;

public slots:
    void updatePos();
    void setActive();
    void setPassive();
    void setHovered();
    void setUnHovered();
    void deleteMe();
    //void deleteMeIfMeIsActive();
    void updateLine(int);
    void doSelect(bool lineSelected);

signals:
    void endPortConnected();

private:
    GUIPort *mpStartPort;
    GUIPort *mpEndPort;
    GraphicsView *mpParentView;
    GUIConnectorLine *mpTempLine;
    QPen mPassivePen;
    QPen mActivePen;
    QPen mHoverPen;
    bool mIsActive;
    bool mEndPortConnected;
    bool mFirstFixedLineAdded;
};

#endif // GUICONNECTOR_H
