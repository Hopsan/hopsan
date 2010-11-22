//$Id$

#ifndef GUICONNECTOR_H
#define GUICONNECTOR_H

#include <QGraphicsWidget>
#include <QObject>
#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMoveEvent>
#include <QTextStream>
#include <QtXml>

#include "common.h"
#include "GUIConnectorAppearance.h"

class GUIConnectorLine;
class GraphicsView;
class GUIObject;
class GUIPort;
class GUISystem;
class GUIContainerObject;

class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
    friend class GUIConnectorLine;
public:
    GUIConnector(GUIPort *startPort, GUIContainerObject *parentSystem, QGraphicsItem *parent = 0);
    GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> mPoints, GUIContainerObject *parentSystem, QStringList geometries = QStringList(), QGraphicsItem *parent = 0);
    ~GUIConnector();

    enum { Type = UserType + 1 };           //Va tusan gör den här?! -Det du!

    void addPoint(QPointF point);
    void removePoint(bool deleteIfEmpty = false);
    void setStartPort(GUIPort *port);
    void setEndPort(GUIPort *port);
    void setPens(QPen activePen, QPen primaryPen, QPen hoverPen);
    int getNumberOfLines();
    connectorGeometry getGeometry(int lineNumber);
    QVector<QPointF> getPointsVector();
    GUIPort *getStartPort();
    GUIPort *getEndPort();
    QString getStartPortName();
    QString getEndPortName();
    QString getStartComponentName();
    QString getEndComponentName();
    GUIConnectorLine *getLine(int line);
    GUIConnectorLine *getLastLine();
    GUIConnectorLine *getSecondLastLine();
    GUIConnectorLine *getThirdLastLine();
    bool isFirstOrLastDiagonal();
    bool isFirstAndLastDiagonal();
    void determineAppearance();

    bool isConnected();
    bool isMakingDiagonal();
    bool isActive();

    void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    void saveToDomElement(QDomElement &rDomElement);

    GUIContainerObject *mpParentContainerObject;
    QVector<QPointF> mPoints;

public slots:
    void setIsoStyle(graphicsType gfxType);
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
    void deselect();
    void select();

signals:
    void endPortConnected();

private:
    bool mIsActive;
    bool mEndPortConnected;
    bool mMakingDiagonal;

    GUIConnectorAppearance *mpGUIConnectorAppearance;
    GUIPort *mpStartPort;
    GUIPort *mpEndPort;
    GUIConnectorLine *mpTempLine;
    QVector<GUIConnectorLine*> mpLines;

    QVector<connectorGeometry> mGeometries;

};


class GUIConnectorLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, GUIConnectorAppearance *pConnApp, int lineNumber, GUIConnector *parent = 0);
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
    void setGeometry(connectorGeometry geometry);
    void setLine(QPointF pos1, QPointF pos2);
    int getLineNumber();
    QPointF mOldPos;

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
    void setPen(const QPen &pen);

    bool mIsActive;
    bool mParentConnectorEndPortConnected;
    bool mHasStartArrow;
    bool mHasEndArrow;
    int mLineNumber;
    GUIConnectorAppearance *mpConnectorAppearance;
    connectorGeometry mGeometry;
    QGraphicsLineItem *mArrowLine1;
    QGraphicsLineItem *mArrowLine2;
    qreal mArrowSize;
    qreal mArrowAngle;
};

#endif // GUICONNECTOR_H
