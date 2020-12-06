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
class SystemObject;
class PlotWindow;

class Component : public ModelObject
{
    Q_OBJECT

public:
    Component(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, SystemObject *pParentSystem, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);
    void deleteInHopsanCore() override;

    bool hasPowerPorts();

    bool setParameterValue(QString name, QString value, bool force=0) override;
    bool setStartValue(QString portName, QString variable, QString sysParName) override;

    void loadParameterValuesFromFile(QString parameterFile = {}) override;

    QString getTypeName() const override;
    QString getTypeCQS() const override;

    // Type info
    virtual int type() const override;
    virtual QString getHmfTagName() const override;

private slots:
    virtual void setVisible(bool visible);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel) override;
    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void createPorts();
};

class ScopeComponent final : public Component
{
    Q_OBJECT
public:
    ScopeComponent(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, SystemObject *pParentSystem, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);

    virtual int type() const override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void rotate(double angle, UndoStatusEnumT undoSettings = Undo) override;
    void flipVertical(UndoStatusEnumT undoSettings = Undo) override;
    void flipHorizontal(UndoStatusEnumT undoSettings = Undo) override;

    QPointer<PlotWindow> mpPlotWindow;
};

#endif // GUICOMPONENT_H
