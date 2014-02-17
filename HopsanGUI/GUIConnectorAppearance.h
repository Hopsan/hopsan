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
//! @file   GUIConnectorAppearance.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIConnectorAppearance class
//!
//$Id$

#ifndef CONNECTORAPPEARANCE_H
#define CONNECTORAPPEARANCE_H

#include <QString>
#include <QPen>
#include "common.h"

class ConnectorAppearance
{
public:
    ConnectorAppearance(QString porttype, GraphicsTypeEnumT gfxType);

    void setStyle(ConnectorStyleEnumT style);
    ConnectorStyleEnumT getStyle() const;
    void setIsoStyle(GraphicsTypeEnumT gfxType);
    void setTypeAndIsoStyle(QString porttype, GraphicsTypeEnumT gfxType);

    QPen getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, const QString &rSituation) const;
    QPen getPen(const QString &rSituation) const;

    void setCustomColor(const QColor &rColor);
    QColor getCustomColor() const;

private:
    ConnectorStyleEnumT mConnectorStyle;
    GraphicsTypeEnumT mGfxType;
    QColor mCustomColor;
};

#endif // GUICONNECTORAPPEARANCE_H
