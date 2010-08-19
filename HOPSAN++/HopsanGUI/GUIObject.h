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

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsSvgItem>

#include "common.h"

#include "AppearanceData.h"
#include <assert.h>

class ProjectTabWidget;
class GraphicsScene;
class GraphicsView;
class GUIConnector;
class GUIObjectDisplayName;
class HopsanEssentials;
class Component;
class GUIObjectSelectionBox;
class GUIPort;
class GUISystem;

enum GUIObjectEnumT {GUIOBJECT=QGraphicsItem::UserType+1, GUISYSTEM, GUICOMPONENT, GUISYSTEMPORT, GUIGROUP, GUIGROUPPORT};

class GUIObject : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIObject(QPoint position, qreal rotation, AppearanceData appearanceData, selectionStatus startSelected = DESELECTED, graphicsType graphics = USERGRAPHICS, GUISystem *system = 0, QGraphicsItem *parent = 0);
    ~GUIObject();

    void rememberConnector(GUIConnector *item);
    void forgetConnector(GUIConnector *item);

    QList<GUIConnector*> getGUIConnectorPtrs();

    virtual QString getName();
    void refreshDisplayName();
    //virtual void setName(QString name, renameRestrictions renameSettings=UNRESTRICTED);
    void setDisplayName(QString name);
    virtual QString getTypeName();
    virtual QString getTypeCQS() {assert(false);} //Only available in GUISystemComponent adn GuiComponent for now
    virtual void setTypeCQS(QString typestring) {assert(false);} //Only available in GUISystemComponent

    AppearanceData* getAppearanceData();
    void refreshAppearance();

    int getNameTextPos();
    void setNameTextPos(int textPos);

    void showPorts(bool visible);
    GUIPort *getPort(QString name);
    QList<GUIPort*> getPortListPtrs();

    virtual QVector<QString> getParameterNames();
    virtual QString getParameterUnit(QString name) {assert(false);} //Only availible in GUIComponent for now
    virtual QString getParameterDescription(QString name) {assert(false);} //Only availible in GUIComponent for now
    virtual double getParameterValue(QString name);
    virtual void setParameterValue(QString name, double value);

    GUISystem *mpParentSystem;

    virtual void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    virtual void loadFromHMF(QString modelFileName=QString()) {assert(false);} //Only available in GUISubsystem for now

    enum { Type = GUIOBJECT };
    int type() const;
    GUIObjectDisplayName *mpNameText;
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
     void rotate(undoStatus undoSettings = UNDO);
     void rotateTo(qreal angle);
     void moveUp();
     void moveDown();
     void moveLeft();
     void moveRight();
     void flipVertical(undoStatus undoSettings = UNDO);
     void flipHorizontal(undoStatus undoSettings = UNDO);
     void hideName();
     void showName();
     void setIcon(graphicsType);
     void deselect();


protected:
    QList<GUIConnector*> mpGUIConnectorPtrs;

    void groupComponents(QList<QGraphicsItem*> compList);
    QGraphicsSvgItem *mpIcon;

    GUIObjectSelectionBox *mpSelectionBox;
    double mTextOffset;
    QGraphicsLineItem *mpTempLine;

    int mNameTextPos;
    graphicsType mIconType;
    bool mIconRotation;
    bool mIsFlipped;
    AppearanceData mAppearanceData;
    QPointF mOldPos;

    QList<GUIPort*> mPortListPtrs;

    virtual void createPorts() {assert(false);} //Only availible in GUIComponent for now

protected slots:
    void fixTextPosition(QPointF pos);
    void adjustTextPositionToZoom();

private:

};

class GUIObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT
private:
    GUIObject* mpParentGUIObject;

public:
    GUIObjectDisplayName(GUIObject *pParent);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    //void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

public slots:
    void deselect();

signals:
    void textMoved(QPointF pos);
};



class GUIObjectSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent = 0);
    void setActive();
    void setPassive();
    void setHovered();

    GUIObject *mpParentGUIObject;

private:
    std::vector<QGraphicsLineItem*> mLines;
    QPen mActivePen;
    QPen mHoverPen;
};

class GUIContainerObject : public GUIObject
{
    Q_OBJECT
public:
    enum CONTAINERSTATUS {CLOSED, OPEN, ROOT};
    GUIContainerObject(QPoint position, qreal rotation, AppearanceData appearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, GUISystem *system=0, QGraphicsItem *parent = 0);
    void makeRootSystem();

protected:
    CONTAINERSTATUS getContainerStatus();
    CONTAINERSTATUS mContainerStatus;

};


class GUIComponent : public GUIObject
{
    Q_OBJECT
public:
    GUIComponent(AppearanceData appearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
    ~GUIComponent();

    QVector<QString> getParameterNames();
    QString getParameterUnit(QString name);
    QString getParameterDescription(QString name);
    double getParameterValue(QString name);
    void setParameterValue(QString name, double value);

    void saveToTextStream(QTextStream &rStream, QString prepend=QString());

    void setName(QString name, renameRestrictions renameSettings=UNRESTRICTED);
    QString getTypeName();
    QString getTypeCQS();

    enum { Type = GUICOMPONENT };
    int type() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openParameterDialog();

    void createPorts();

    QString mComponentTypeName;

public slots:

private:

};


//class GUISubsystem : public GUIContainerObject
//{
//    Q_OBJECT
//public:
//    GUISubsystem(AppearanceData appearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
//
//    void deleteInHopsanCore();
//
//    QString getTypeName();
//    void setName(QString newName, renameRestrictions renameSettings=UNRESTRICTED);
//    void setTypeCQS(QString typestring);
//    QString getTypeCQS();
//    void loadFromFile(QString modelFileName=QString());
//
//    void saveToTextStream(QTextStream &rStream, QString prepend);
//
//    QVector<QString> getParameterNames();
//
//    enum { Type = UserType + 4 };
//    int type() const;
//
//protected:
//    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
//    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
//    void openParameterDialog();
//    void createPorts();
//
//private:
//    QString mModelFilePath;
//    //QString mGraphicsFilePath;
//    bool   mIsEmbedded;
//    QString mLoadType;
//};


class GUISystemPort : public GUIObject
{
    Q_OBJECT
public:
    GUISystemPort(AppearanceData appearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected = SELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
    ~GUISystemPort();
    QString getTypeName();
    void setName(QString newName, renameRestrictions renameSettings);

    enum { Type = GUISYSTEMPORT };
    int type() const;

protected:
    void createPorts();

private:
    GUIPort *mpGuiPort;
};


class GUIGroup : public GUIContainerObject
{
    Q_OBJECT
public:
    GUIGroup(QList<QGraphicsItem*> compList, AppearanceData appearanceData, GUISystem *system, QGraphicsItem *parent = 0);
    ~GUIGroup();
//    QString getName();
//    void setName(QString name, bool doOnlyLocalRename=false);

    enum { Type = GUIGROUP };
    int type() const;

    QString getTypeName();

protected:
    GraphicsScene *mpParentScene;
    GraphicsScene *mpGroupScene;

    QList<GUIComponent*> mGUICompList;
    QList<GUIConnector*> mGUIConnList;
    QList<GUIConnector*> mGUITransitConnList;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void showParent();

private:
//    GUIPort *mpGuiPort;

//    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
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
    GUIGroupPort(AppearanceData appearanceData, QPoint position, GUISystem *system, QGraphicsItem *parent = 0);
    QString getTypeName();
    void setName(QString newName);

    void setOuterGuiPort(GUIPort *pPort);

    enum { Type = GUIGROUPPORT };
    int type() const;

private:
    GUIPort *mpGuiPort;
    GUIPort *mpOuterGuiPort;
};


#endif // GUIOBJECT_H
