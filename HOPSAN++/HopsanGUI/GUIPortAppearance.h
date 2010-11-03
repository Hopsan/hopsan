#ifndef GUIPORTAPPEARANCE_H
#define GUIPORTAPPEARANCE_H

#include <QString>
#include <QHash>

//#include "common.h"

class GUIPortAppearance
{
public:
    void selectPortIcon(QString cqstype, QString porttype, QString nodetype);

    qreal x;
    qreal y;
    qreal rot;
    QString mIconPath;
    QString mIconOverlayPath;
    //portDirection direction; //!< @todo Does this have to be common, (include common.h would not be necessary)
};

typedef QHash<QString, GUIPortAppearance> PortAppearanceMapT;

#endif // GUIPORTAPPEARANCE_H
