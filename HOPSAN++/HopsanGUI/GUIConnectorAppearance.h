//!
//! @file   GUIConnectorAppearance.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIConnectorAppearance class
//!
//$Id$

#ifndef GUICONNECTORAPPEARANCE_H
#define GUICONNECTORAPPEARANCE_H

#include <QString>
#include <QPen>
#include "common.h"

class GUIConnectorAppearance
{
public:
    //GUIConnectorAppearance();
    GUIConnectorAppearance(QString porttype, graphicsType gfxType);
    void setStyle(connectorStyle style);
    connectorStyle getStyle();
    void setIsoStyle(graphicsType gfxType);
    void setTypeAndIsoStyle(QString porttype, graphicsType gfxType);
    QPen getPen(connectorStyle style, graphicsType gfxType, QString situation);
    QPen getPen(QString situation);

private:
    connectorStyle mConnectorStyle;
    graphicsType mGfxType;

};

#endif // GUICONNECTORAPPEARANCE_H
