//$Id$

#ifndef GUICONNECTOR_H
#define GUICONNECTOR_H


#include <vector>
#include <QGraphicsWidget>
#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMoveEvent>
#include "GUIPort.h"
#include "GUIConnector.h"


class GUIConnectorLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, QGraphicsItem *parent = 0);
    ~GUIConnectorLine();

    enum geometryType {VERTICAL, HORIZONTAL, DIAGONAL};
    QPointF startPos;
    QPointF endPos;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);
    void addEndArrow();
    void addStartArrow();
    void setActive();
    void setPassive();
    void setHovered();
    void setGeometry(geometryType geometry);
    void setLine(qreal x1, qreal y1, qreal x2, qreal y2);
    void setPen(const QPen &pen);
    int getLineNumber();
    geometryType getGeometry();

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
    bool mParentConnectorEndPortConnected;
    bool mHasStartArrow;
    bool mHasEndArrow;
    QPen mPrimaryPen;
    QPen mActivePen;
    QPen mHoverPen;
    int mLineNumber;
    geometryType mGeometry;
    QGraphicsLineItem *mArrowLine1;
    QGraphicsLineItem *mArrowLine2;
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
    std::vector<GUIConnectorLine*> mLines;
    void addFreeLine();
    void addFixedLine(int length, int heigth, GUIConnectorLine::geometryType geometry);
    void updateConnector(QPointF startPos, QPointF endPos);
    void removeLine(QPointF cursorPos);
    void setStartPort(GUIPort *port);
    void setEndPort(GUIPort *port);
    void setPen(QPen pen);
    int getNumberOfLines();
    int getLineNumber();
    GUIPort *getStartPort();
    GUIPort *getEndPort();
    GUIConnectorLine *getLine(int line);
    GUIConnectorLine *getThirdLastLine();
    GUIConnectorLine *getSecondLastLine();
    GUIConnectorLine *getLastLine();

    enum { Type = UserType + 2 };
    int type() const;

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
    bool mIsActive;
    bool mEndPortConnected;
    bool mFirstFixedLineAdded;
    QPen mPassivePen;
    QPen mActivePen;
    QPen mHoverPen;
    GUIPort *mpStartPort;
    GUIPort *mpEndPort;
    GraphicsView *mpParentView;
    GUIConnectorLine *mpTempLine;

};

#endif // GUICONNECTOR_H
