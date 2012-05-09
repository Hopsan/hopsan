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
//! @file   GUIPortAppearance.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPortAppearance class
//!
//$Id$

#ifndef GUIPORTAPPEARANCE_H
#define GUIPORTAPPEARANCE_H

#include <QString>
#include <QHash>

class PortAppearance
{
public:
    PortAppearance();
    void selectPortIcon(QString cqstype, QString porttype, QString nodetype);

    qreal x,y,rot;
    QString mMainIconPath;
    QString mCQSOverlayPath;
    QString mMultiPortOverlayPath;
    bool mEnabled;
    bool mAutoPlaced;
    QString mDescription;
};

typedef QHash<QString, PortAppearance> PortAppearanceMapT;

#endif // GUIPORTAPPEARANCE_H
