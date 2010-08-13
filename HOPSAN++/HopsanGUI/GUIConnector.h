/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$

#ifndef GUICONNECTOR_H
#define GUICONNECTOR_H

#include <QGraphicsWidget>
#include <QObject>
#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMoveEvent>

#include "common.h"

#include "AppearanceData.h"

class GUIConnectorLine;
class GraphicsView;
class GUIPort;

class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIConnector(QPointF startpos, GraphicsView *parentView, QGraphicsItem *parent = 0);
    GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> mPoints, GraphicsView *parentView, QGraphicsItem *parent = 0);
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
    GUIConnectorLine *getLine(int line);
    GUIConnectorLine *getLastLine();
    GUIConnectorLine *getSecondLastLine();
    GUIConnectorLine *getThirdLastLine();
    void determineAppearance();

    bool isConnected();
    bool isMakingDiagonal();
    bool isActive();
    void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    GraphicsView *mpParentGraphicsView;

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
    void adjustToZoom();

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
    QVector<QPointF> mPoints;
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
    QPointF mOldPos;
};

#endif // GUICONNECTOR_H
