/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

#include <QPointer>

//Forward declarations
class Connector;
class Port;
class ContainerObject;
class PlotWindow;

class Component : public ModelObject
{
    Q_OBJECT

public:
    Component(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);
    void deleteInHopsanCore();

    bool hasPowerPorts();

    bool setParameterValue(QString name, QString value, bool force=0);
    bool setStartValue(QString portName, QString variable, QString sysParName);

    QString getTypeName() const;
    QString getTypeCQS();

    // Type info
    enum { Type = ComponentType };
    int type() const;
    virtual QString getHmfTagName() const;

private slots:
    virtual void setVisible(bool visible);

protected:
    void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);
    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void createPorts();
};

class ScopeComponent : public Component
{
    Q_OBJECT
public:
    ScopeComponent(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);

    enum { Type = ScopeComponentType };
    int type() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void rotate(double angle, UndoStatusEnumT undoSettings = Undo);
    void flipVertical(UndoStatusEnumT undoSettings = Undo);
    void flipHorizontal(UndoStatusEnumT undoSettings = Undo);

    QPointer<PlotWindow> mpPlotWindow;
};

#endif // GUICOMPONENT_H
