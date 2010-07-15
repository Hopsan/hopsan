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

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
//#include <QGraphicsScene>
//#include "GUIObject.h"

#include "AppearanceData.h"
class GUIRootSystem;

//Forward declaration
class GUIObject;
//class GUIComponent;
//class GUIRootSystem;
class GraphicsView;

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    GUIPort(QString name, qreal xpos, qreal ypos, PortAppearance* pPortAppearance, GUIObject *pParent = 0, GUIRootSystem *pGUIRootSystem=0);
    ~GUIPort();
    void updatePosition();
    GraphicsView *getParentView();
    GUIObject *getGuiObject();
    void magnify(bool blowup);
    PortAppearance::portDirectionType getPortDirection();
    void setPortDirection(PortAppearance::portDirectionType direction);
    void hide();

    QString getName();
    void setDisplayName(const QString name);
    QString getGUIComponentName();

    QPointF rectPos;
    int getPortNumber();

    QString getPortType();
    QString getNodeType();

    bool isConnected;

public slots:
    void hideIfNotConnected(bool justDoIt);
    void setVisible(bool visible);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);


    //protected slots:
    void plot(QString dataName, QString dataUnit=QString());

signals:
    void portClicked(GUIPort *item);
    void portMoved(GUIPort *item);

private:
    QColor myLineColor;
    qreal myLineWidth;

    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    GraphicsView *mpParentGraphicsView;
    GUIObject *mpParentGuiObject;
    GUIRootSystem *mpGUIRootSystem;
    QGraphicsTextItem *mpPortLabel;
    qreal mMag;
    bool mIsMag;

    PortAppearance *mpPortAppearance;
    qreal mXpos;
    qreal mYpos;
    QString name;

};

#endif // GUIPORT_H
