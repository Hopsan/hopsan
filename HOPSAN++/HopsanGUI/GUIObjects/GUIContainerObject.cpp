#include "GUIContainerObject.h"


GUIContainerObject::GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUISystem *system, QGraphicsItem *parent)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    //Something
}

void GUIContainerObject::makeRootSystem()
{
    mContainerStatus = ROOT;
}

void GUIContainerObject::updateExternalPortPositions()
{
    //Nothing for now
}

GUIContainerObject::CONTAINERSTATUS GUIContainerObject::getContainerStatus()
{
    return mContainerStatus;
}

//! @brief Use this function to calculate the placement of the ports on a subsystem icon.
//! @param[in] w width of the subsystem icon
//! @param[in] h heigth of the subsystem icon
//! @param[in] angle the angle in radians of the line between center and the actual port
//! @param[out] x the new calculated horizontal placement for the port
//! @param[out] y the new calculated vertical placement for the port
//! @todo rename this one and maybe change it a bit as it is now included in this class, it should be common for subsystems and groups
void GUIContainerObject::calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y)
{
    //! @todo make common PI declaration, maybe also PIhalf or include math.h and use M_PI
    if(angle>3.1415*3.0/2.0)
    {
        x=-std::max(std::min(h/tan(angle), w), -w);
        y=std::max(std::min(w*tan(angle), h), -h);
    }
    else if(angle>3.1415)
    {
        x=-std::max(std::min(h/tan(angle), w), -w);
        y=-std::max(std::min(w*tan(angle), h), -h);
    }
    else if(angle>3.1415/2.0)
    {
        x=std::max(std::min(h/tan(angle), w), -w);
        y=-std::max(std::min(w*tan(angle), h), -h);
    }
    else
    {
        x=std::max(std::min(h/tan(angle), w), -w);
        y=std::max(std::min(w*tan(angle), h), -h);
    }
}


CoreSystemAccess* GUIContainerObject::getCoreSystemAccessPtr()
{
    //Should be overloaded
    return 0;
}

//! @brief Retunrs a pointer to the contained scene
GraphicsScene* GUIContainerObject::getContainedScenePtr()
{
    return this->mpScene;
}

////! @brief Adds connector the the scene in this container
////! @todo maybe should be called addConnectorToScene to be more clere, dont know
//void GUIContainerObject::addConnector(GUIConnector *pConnector)
//{
//    this->mpScene->addItem(pConnector);

//}
