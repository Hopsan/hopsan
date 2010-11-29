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
