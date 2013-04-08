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
//! @file   GUIContainerPort.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the ContainerPort class
//!
//$Id$

#ifndef GUICONTAINERPORT_H
#define GUICONTAINERPORT_H

#include "GUIModelObject.h"

class ContainerPort : public ModelObject
{
    Q_OBJECT
public:
    ContainerPort(QPointF position, qreal rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Selected, GraphicsTypeEnumT gfxType = UserGraphics);
    void deleteInHopsanCore();
    QString getTypeName() const;
    void refreshDisplayName(QString overrideName="");

    enum { Type = ContainerPortType };
    int type() const;

protected:
    void createPorts();
    void saveCoreDataToDomElement(QDomElement &rDomElement);
    void openPropertiesDialog();
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    bool mIsSystemPort;
    Port *mpPort;
};

#endif
