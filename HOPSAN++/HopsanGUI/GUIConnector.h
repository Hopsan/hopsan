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
class GUIObject;
class GUIPort;
class GUISystem;
class GUIContainerObject;

class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
    friend class ConnectorLine;
public:
    GUIConnector(GUIContainerObject *pParentContainer);
    GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> points, GUIContainerObject *pParentContainer, QStringList geometries = QStringList());
    ~GUIConnector();

    void setParentContainer(GUIContainerObject *pParentContainer);
    GUIContainerObject *getParentContainer();

    enum { Type = UserType + 1 };           //Va tusan gÃ¶r den hÃ¤r?! -Det du!

    void addPoint(QPointF point);
    void removePoint(bool deleteIfEmpty = false);
    void setStartPort(GUIPort *port);
    void setEndPort(GUIPort *port);
    void finishCreation();
    void setPens(QPen activePen, QPen primaryPen, QPen hoverPen);
    int getNumberOfLines();
    connectorGeometry getGeometry(int lineNumber);
    GUIPort *getStartPort();
    GUIPort *getEndPort();
    QPointF getStartPoint();
    QPointF getEndPoint();
    QString getStartPortName();
    QString getEndPortName();
    QString getStartComponentName();
    QString getEndComponentName();
    ConnectorLine *getLine(int line);
    ConnectorLine *getLastLine();
    bool isFirstOrLastDiagonal();
    bool isFirstAndLastDiagonal();
    void determineAppearance();

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
    void commonConstructorCode();
    void refreshConnectedSystemportsGraphics();
    void disconnectPortSigSlots(GUIPort* pPort);
    void connectPortSigSlots(GUIPort* pPort);
    void setupGeometries(const QStringList &rGeometries);
    void addLine(ConnectorLine *pLine);

    bool mIsActive;
    bool mIsConnected;
    bool mMakingDiagonal;
    bool mIsDashed;

    GUIContainerObject *mpParentContainerObject;
    ConnectorAppearance *mpGUIConnectorAppearance;
    GUIPort *mpStartPort;
    GUIPort *mpEndPort;

    QVector<ConnectorLine*> mpLines;
    QVector<connectorGeometry> mGeometries;
    QVector<QPointF> mPoints;
};


class ConnectorLine : public QObject, public QGraphicsLineItem
{
    friend class GUIConnector;
    Q_OBJECT
public:
    ConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, ConnectorAppearance *pConnApp, int lineNumber, GUIConnector *parent = 0);
    ~ConnectorLine();

    GUIConnector *mpParentGUIConnector;
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
    bool mIsActive;
    bool mParentConnectorEndPortConnected;
    bool mHasStartArrow;
    bool mHasEndArrow;
    int mLineNumber;
    ConnectorAppearance *mpConnectorAppearance;
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
