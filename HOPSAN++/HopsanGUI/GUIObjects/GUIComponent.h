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
//! @file   GUIComponent.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI class representing Components
//!
//$Id$

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include "GUIModelObject.h"
#include "common.h"
#include "Utilities/XMLUtilities.h"
#include <assert.h>

//Forward declarations
class Connector;
class Port;
class ContainerObject;

class Component : public ModelObject
{
    Q_OBJECT

public:
    Component(QPointF position, qreal rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);
    void deleteInHopsanCore();

    bool hasPowerPorts();

    bool setParameterValue(QString name, QString value, bool force=0);
    //QString getStartValueTxt(QString portName, QString variable);
    bool setStartValue(QString portName, QString variable, QString sysParName);

    QString getTypeName() const;
    QString getTypeCQS();

    enum { Type = ComponentType };
    int type() const;

private slots:
    virtual void setVisible(bool visible);

protected:
    void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);
    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void openPropertiesDialog();
    void createPorts();
};

class ScopeComponent : public Component
{
    Q_OBJECT
public:
    ScopeComponent(QPointF position, qreal rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);

    enum { Type = ScopeComponentType };
    int type() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void rotate(qreal angle, UndoStatusEnumT undoSettings = Undo);
    void flipVertical(UndoStatusEnumT undoSettings = Undo);
    void flipHorizontal(UndoStatusEnumT undoSettings = Undo);

};

#endif // GUICOMPONENT_H
