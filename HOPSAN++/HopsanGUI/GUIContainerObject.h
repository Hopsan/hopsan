#ifndef GUICONTAINEROBJECT_H
#define GUICONTAINEROBJECT_H

#include "GUIModelObject.h"

//class GUIConnector;
//class GUIModelObjectDisplayName;
//class GUIPort;
//class GUISystem;

class GUIContainerObject : public GUIModelObject
{
    Q_OBJECT
public:
    enum CONTAINERSTATUS {CLOSED, OPEN, ROOT};
    GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, GUISystem *system=0, QGraphicsItem *parent = 0);
    void makeRootSystem();
    virtual void updateExternalPortPositions();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y);

protected:
    CONTAINERSTATUS getContainerStatus();
    CONTAINERSTATUS mContainerStatus;

};

#endif // GUICONTAINEROBJECT_H
