/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   GUIConnector.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIConnector class
//!
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

class ConnectorLine;
class GraphicsView;
class WorkspaceObject;
class Port;
class SystemContainer;
class ContainerObject;

class Connector : public QGraphicsWidget
{
    Q_OBJECT
    friend class ConnectorLine;
public:
    Connector(ContainerObject *pParentContainer);
    ~Connector();

    void setParentContainer(ContainerObject *pParentContainer);
    ContainerObject *getParentContainer();

    enum { Type = UserType + 1 };           //!< @todo is this really necessary, we dont check Type on connectors (only one version exist)

    void addPoint(QPointF point);
    void removePoint(bool deleteIfEmpty = false);
    void finishCreation();
    void setPointsAndGeometries(const QVector<QPointF> &rPoints, const QStringList &rGeometries);

    void setStartPort(Port *port);
    void setEndPort(Port *port);

    Port *getStartPort();
    Port *getEndPort();
    QString getStartPortName();
    QString getEndPortName();
    QString getStartComponentName();
    QString getEndComponentName();

    QPointF getStartPoint();
    QPointF getEndPoint();
    ConnectorLine *getLine(int line);
    ConnectorLine *getLastLine();
    int getNumberOfLines();
    bool isFirstOrLastDiagonal();
    bool isFirstAndLastDiagonal();
    connectorGeometry getGeometry(const int lineNumber);

    void setPens(QPen activePen, QPen primaryPen, QPen hoverPen);

    void refreshConnectorAppearance();

    bool isConnected();
    bool isMakingDiagonal();
    bool isActive();

    void saveToDomElement(QDomElement &rDomElement);

public slots:
    void setIsoStyle(graphicsType gfxType);
    void drawConnector(bool alignOperation=false);
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
    void deleteMe(undoStatus undo=UNDO);
    void deleteMeWithNoUndo();
    void deselect();
    void select();
    void setDashed(bool value);

private slots:
    void setVisible(bool visible);

signals:
    void connectionFinished();

private:
    void determineAppearance();
    void refreshConnectedSystemportsGraphics();
    void disconnectPortSigSlots(Port* pPort);
    void connectPortSigSlots(Port* pPort);
    void addLine(ConnectorLine *pLine);
    void removeAllLines();

    bool mIsActive;
    bool mIsConnected;
    bool mMakingDiagonal;
    bool mIsDashed;

    ContainerObject *mpParentContainerObject;
    ConnectorAppearance *mpConnectorAppearance;
    Port *mpStartPort;
    Port *mpEndPort;

    QVector<ConnectorLine*> mpLines;
    QVector<connectorGeometry> mGeometries;
    QVector<QPointF> mPoints;
};


class ConnectorLine : public QObject, public QGraphicsLineItem
{
    friend class Connector;
    Q_OBJECT
public:
    ConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, ConnectorAppearance *pConnApp, int lineNumber, Connector *parent = 0);
    ~ConnectorLine();

    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);
    void addEndArrow();
    void addStartArrow();
    void setActive();
    void setPassive();
    void setHovered();
    void setGeometry(connectorGeometry geometry);
    void setLine(QPointF pos1, QPointF pos2);
    int getLineNumber();
    void setPen(const QPen &pen);

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
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    Connector *mpParentConnector;
    ConnectorAppearance *mpConnectorAppearance;

    bool mIsActive;
    bool mParentConnectorEndPortConnected;
    bool mHasStartArrow;
    bool mHasEndArrow;
    int mLineNumber;

    connectorGeometry mGeometry;
    QGraphicsLineItem *mArrowLine1;
    QGraphicsLineItem *mArrowLine2;
    qreal mArrowSize;
    qreal mArrowAngle;

    QPointF mStartPos;
    QPointF mEndPos;
    QPointF mOldPos;
};

#endif // GUICONNECTOR_H
