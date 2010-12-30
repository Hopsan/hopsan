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

class GUIPortAppearance
{
public:
    void selectPortIcon(QString cqstype, QString porttype, QString nodetype);

    qreal x;
    qreal y;
    qreal rot;
    QString mIconPath;
    QString mIconOverlayPath;
};

typedef QHash<QString, GUIPortAppearance> PortAppearanceMapT;

#endif // GUIPORTAPPEARANCE_H
