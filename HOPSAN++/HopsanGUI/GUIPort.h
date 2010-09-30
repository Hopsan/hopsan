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

#include "common.h"
#include "GUIPortAppearance.h"

//Forward declarations
class GUIObject;
class GUISystem;
class CoreSystemAccess;
class GraphicsView;

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    GUIPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIObject *pParent = 0, CoreSystemAccess *pGUIRootSystem=0);
    GUISystem *mpParentSystem;
    GUIObject *mpParentGuiObject;

    void updatePosition();
    GUISystem *getParentSystem();
    GUIObject *getGuiObject();
    void magnify(bool blowup);
    portDirection getPortDirection();
    void setPortDirection(portDirection direction);
    void hide();

    QString getName();
    void setDisplayName(const QString name);
    QString getGUIComponentName();

    QPointF rectPos;
    int getPortNumber();

    bool getLastNodeData(QString dataName, double& rData);

    QString getPortType();
    QString getNodeType();

    bool isConnected;

public slots:
    void hideIfNotConnected(bool hidePortsActionTriggered);
    void setVisible(bool value);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);


    //protected slots:
public: //! @todo This was made public temporarly to test plot in Python
    void plot(QString dataName, QString dataUnit=QString());

signals:
    void portClicked(GUIPort *item);
    void portMoved(GUIPort *item);

private:
    QColor myLineColor;
    qreal myLineWidth;

    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    CoreSystemAccess *mpGUIRootSystem;
    QGraphicsTextItem *mpPortLabel;
    qreal mMag;
    bool mIsMag;

    GUIPortAppearance *mpPortAppearance;
    qreal mXpos;
    qreal mYpos;
    QString name;

};

#endif // GUIPORT_H
