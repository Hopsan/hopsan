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
    void setType(const QString porttype);
    void setIsoStyle(graphicsType gfxType);
    void setTypeAndIsoStyle(QString porttype, graphicsType gfxType);
    QPen getPen(QString situation, QString type, graphicsType gfxType);
    QPen getPen(QString situation);
    void adjustToZoom(qreal zoomFactor);

private:
    QPen mPrimaryPenPowerUser;
    QPen mActivePenPowerUser;
    QPen mHoverPenPowerUser;
    QPen mPrimaryPenSignalUser;
    QPen mActivePenSignalUser;
    QPen mHoverPenSignalUser;
    QPen mPrimaryPenPowerIso;
    QPen mActivePenPowerIso;
    QPen mHoverPenPowerIso;
    QPen mPrimaryPenSignalIso;
    QPen mActivePenSignalIso;
    QPen mHoverPenSignalIso;
    QPen mNonFinishedPen;

    QString mConnectorType;
    graphicsType mGfxType;

};

#endif // GUICONNECTORAPPEARANCE_H
