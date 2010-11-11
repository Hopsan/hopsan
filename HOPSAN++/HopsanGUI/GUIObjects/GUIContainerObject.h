#ifndef GUICONTAINEROBJECT_H
#define GUICONTAINEROBJECT_H

#include "GUIModelObject.h"
#include "GraphicsScene.h"
#include "CoreSystemAccess.h"

class GUIContainerObject : public GUIModelObject
{
    Q_OBJECT
public:
    enum CONTAINERSTATUS {CLOSED, OPEN, ROOT};
    GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, GUISystem *system=0, QGraphicsItem *parent = 0);
    void makeRootSystem();
    virtual void updateExternalPortPositions();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y);

    virtual CoreSystemAccess* getCoreSystemAccessPtr();

    GraphicsScene* getContainedScenePtr();
    //void addConnector(GUIConnector *pConnector);
    //Public for now, will fix later
    GraphicsScene *mpScene;

protected:
    CONTAINERSTATUS getContainerStatus();
    CONTAINERSTATUS mContainerStatus;

    //CoreSystemAccess *mpCoreSystemAccess;

};

#endif // GUICONTAINEROBJECT_H
