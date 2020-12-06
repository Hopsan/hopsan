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

class SystemPortObject : public ModelObject
{
    Q_OBJECT
public:
    SystemPortObject(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, SystemObject *pParentSystem,
                     SelectionStatusEnumT startSelected = Selected, GraphicsTypeEnumT gfxType = UserGraphics);
    void deleteInHopsanCore() override;
    QString getTypeName() const override;
    void refreshDisplayName(QString overrideName="") override;

    // Type info
    virtual int type() const override;
    virtual QString getHmfTagName() const override;
    bool isSystemPort() const;

    void openPropertiesDialog() override;

protected:
    void createPorts();
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Port *mpPort;
};

#endif
