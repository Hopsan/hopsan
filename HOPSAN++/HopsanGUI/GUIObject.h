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

#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include "AppearanceData.h"
#include <assert.h>

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsSvgItem>

class ProjectTabWidget;
class GraphicsScene;
class GraphicsView;
class GUIConnector;
class GUIObjectDisplayName;
class HopsanEssentials;
class Component;
class GUIObjectSelectionBox;
class GUIPort;

class GUIObject : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIObject(QPoint position, AppearanceData appearanceData, GraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    ~GUIObject();

    void addConnector(GUIConnector *item);

    virtual QString getName();
    void refreshDisplayName();
    virtual void setName(QString name, bool doOnlyCoreRename=false);
    virtual QString getTypeName();
    virtual QString getTypeCQS() {assert(false);}; //Only available in GUISystemComponent adn GuiComponent for now

    int getPortNumber(GUIPort *port);
    int getNameTextPos();
    void setNameTextPos(int textPos);

    void showPorts(bool visible);
    GUIPort *getPort(QString name);

    virtual QVector<QString> getParameterNames();
    virtual QString getParameterUnit(QString name) {assert(false);}; //Only availible in GUIComponent for now
    virtual QString getParameterDescription(QString name) {assert(false);}; //Only availible in GUIComponent for now
    virtual double getParameterValue(QString name);
    virtual void setParameterValue(QString name, double value);


    GraphicsScene *mpParentGraphicsScene;
    GraphicsView *mpParentGraphicsView;

    virtual void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    virtual void deleteInHopsanCore();

    enum { Type = UserType + 2 };
    int type() const;
    QList<GUIPort*> mPortListPtrs;
    //QMap<QString, GUIPort*> mGuiPortPtrMap;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);


signals:
    void componentMoved();
    void componentDeleted();
    void componentSelected();

public slots:
     void deleteMe();
     void rotate(bool doNotRegisterUndo = false);
     void moveUp();
     void moveDown();
     void moveLeft();
     void moveRight();
     void flipVertical(bool doNotRegisterUndo = false);
     void flipHorizontal(bool doNotRegisterUndo = false);
     void hideName();
     void showName();
     void setIcon(bool useIso);


protected:
    void groupComponents(QList<QGraphicsItem*> compList);
    QGraphicsSvgItem *mpIcon;
    GUIObjectDisplayName *mpNameText;
    GUIObjectSelectionBox *mpSelectionBox;
    double mTextOffset;
    QGraphicsLineItem *mpTempLine;
    //QString mName;

    int mNameTextPos;
    bool mIconRotation;
    bool mIsFlipped;
    AppearanceData mAppearanceData;
    QPointF mOldPos;

protected slots:
    void fixTextPosition(QPointF pos);
    void adjustTextPositionToZoom();

private:

};

class GUIObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT
private:
    GUIObject* mpParentGUIComponent;

public:
    GUIObjectDisplayName(GUIObject *pParent);
    ~GUIObjectDisplayName();

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

signals:
    void textMoved(QPointF pos);
};



class GUIObjectSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent = 0);
    ~GUIObjectSelectionBox();
    void setActive();
    void setPassive();
    void setHovered();

    GUIObject *mpParentGUIObject;

private:
    std::vector<QGraphicsLineItem*> mLines;
    QPen mActivePen;
    QPen mHoverPen;
};


class GUIComponent : public GUIObject
{
    Q_OBJECT
public:
    GUIComponent(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);

    QVector<QString> getParameterNames();
    QString getParameterUnit(QString name);
    QString getParameterDescription(QString name);
    double getParameterValue(QString name);
    void setParameterValue(QString name, double value);

    void saveToTextStream(QTextStream &rStream, QString prepend=QString());

    void setName(QString name, bool doOnlyCoreRename=false);
    QString getTypeName();
    QString getTypeCQS();
    void deleteInHopsanCore();

    enum { Type = UserType + 3 };
    int type() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openParameterDialog();

    QString mComponentTypeName;

public slots:

private:

};

class GUISubsystem : public GUIObject
{
    Q_OBJECT
public:
    GUISubsystem(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);

    void deleteInHopsanCore();

    QString getTypeName();
    void setName(QString newName, bool doOnlyCoreRename);
    void setTypeCQS(QString typestring);
    QString getTypeCQS();

    QVector<QString> getParameterNames();

    enum { Type = UserType + 4 };
    int type() const;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openParameterDialog();

private:
    QString mModelFilePath;
    QString mGraphicsFilePath;
    bool   mIsEmbedded;
};

class GUISystemPort : public GUIObject
{
    Q_OBJECT
public:
    GUISystemPort(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);
    QString getTypeName();
    void setName(QString newName, bool doOnlyCoreRename);
    void deleteInHopsanCore();

    enum { Type = UserType + 5 };
    int type() const;

private:
    GUIPort *mpGuiPort;
};

class GUIGroup : public GUIObject
{
    Q_OBJECT
public:
    GUIGroup(QList<QGraphicsItem*> compList, AppearanceData appearanceData, GraphicsScene *scene, QGraphicsItem *parent = 0);

//    QString getName();
//    void setName(QString name, bool doOnlyLocalRename=false);

    enum { Type = UserType + 6 };
    int type() const;

protected:
    GraphicsScene *mpParentScene;
    GraphicsScene *mpGroupScene;

    QList<GUIComponent*> mGUICompList;
    QList<GUIConnector*> mGUIConnList;
    QList<GUIConnector*> mGUITransitConnList;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void showParent();

//    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
//    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
//    void openParameterDialog();
//
//    QString mComponentTypeName;
//
//    GraphicsScene *mpGroupScene;
//
//public slots:
//     void deleteMe();
};


class GUIGroupPort : public GUIObject
{
    Q_OBJECT
public:
    GUIGroupPort(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);
    QString getTypeName();
    void setName(QString newName);

    void setOuterGuiPort(GUIPort *pPort);

    void pGroupPort();

    enum { Type = UserType + 7 };
    int type() const;

private:
    GUIPort *mpGuiPort;
    GUIPort *mpOuterGuiPort;
};


#endif // GUIOBJECT_H
