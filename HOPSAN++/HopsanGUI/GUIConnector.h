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

class GUIConnectorLine;
class GraphicsView;
class GUIPort;

class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIConnector(QPointF startpos, QPen primaryPen, QPen activePen, QPen hoverPen, GraphicsView *parentView, QGraphicsItem *parent = 0);
    GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> mPoints, QPen primaryPen, QPen activePen, QPen hoverPen, GraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIConnector();

    enum geometryType {VERTICAL, HORIZONTAL, DIAGONAL};
    enum { Type = UserType + 1 };           //Va tusan gör den här?!

    void addPoint(QPointF point);
    void removePoint(bool deleteIfEmpty = false);
    void setStartPort(GUIPort *port);
    void setEndPort(GUIPort *port);
    void setPens(QPen activePen, QPen primaryPen, QPen hoverPen);
    int getNumberOfLines();
    GUIConnector::geometryType getGeometry(int lineNumber);
    QVector<QPointF> getPointsVector();
    GUIPort *getStartPort();
    GUIPort *getEndPort();
    GUIConnectorLine *getLine(int line);
    GUIConnectorLine *getLastLine();
    GUIConnectorLine *getSecondLastLine();
    GUIConnectorLine *getThirdLastLine();
    bool isConnected();
    bool isMakingDiagonal();
    bool isActive();
    void saveToTextStream(QTextStream &rStream);
    GraphicsView *mpParentView;

public slots:
    void drawConnector();
    void updateStartPoint(QPointF point);
    void updateEndPoint(QPointF point);
    void moveAllPoints(qreal offsetX, qreal offsetY);
    void updateLine(int);
    void makeDiagonal(bool diagonal);
    void doSelect(bool lineSelected, int lineNumber);
    void selectIfBothComponentsSelected();
    void setActive();
    void setPassive();
    void setHovered();
    void setUnHovered();
    void deleteMe();
    void deleteMeWithNoUndo();

signals:
    void endPortConnected();

private:
    bool mIsActive;
    bool mEndPortConnected;
    bool mMakingDiagonal;
    QPen mPrimaryPen;
    QPen mActivePen;
    QPen mHoverPen;
    GUIPort *mpStartPort;
    GUIPort *mpEndPort;
    GUIConnectorLine *mpTempLine;
    std::vector<GUIConnectorLine*> mpLines;
    QVector<QPointF> mPoints;
    std::vector<geometryType> mGeometries;

};


class GUIConnectorLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, GUIConnector *parent = 0);
    ~GUIConnectorLine();

    GUIConnector *mpParentGUIConnector;
    QPointF startPos;
    QPointF endPos;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);
    void addEndArrow();
    void addStartArrow();
    void setActive();
    void setPassive();
    void setHovered();
    void setGeometry(GUIConnector::geometryType geometry);
    void setLine(QPointF pos1, QPointF pos2);
    void setPen(const QPen &pen);
    void setPens(QPen activePen, QPen primaryPen, QPen hoverPen);
    int getLineNumber();

public slots:
    void setConnected();

signals:
    void lineClicked();
    void lineMoved(int);
    void lineHoverEnter();
    void lineHoverLeave();
    void lineSelected(bool isSelected, int lineNumber);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
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
    GUIConnector::geometryType mGeometry;
    QGraphicsLineItem *mArrowLine1;
    QGraphicsLineItem *mArrowLine2;
    qreal mArrowSize;
    qreal mArrowAngle;
    QPointF mOldPos;
};

#endif // GUICONNECTOR_H
