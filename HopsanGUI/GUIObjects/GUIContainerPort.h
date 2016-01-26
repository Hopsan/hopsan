/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
    ContainerPort(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Selected, GraphicsTypeEnumT gfxType = UserGraphics);
    void deleteInHopsanCore();
    QString getTypeName() const;
    void refreshDisplayName(QString overrideName="");

    // Type info
    enum { Type = ContainerPortType };
    int type() const;
    virtual QString getHmfTagName() const;
    bool isSystemPort() const;

    void openPropertiesDialog();

protected:
    void createPorts();
    void saveCoreDataToDomElement(QDomElement &rDomElement);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    Port *mpPort;
};

#endif
