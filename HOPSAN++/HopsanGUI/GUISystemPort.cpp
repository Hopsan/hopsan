#include "GUISystemPort.h"
#include "GUISystem.h"
#include "GUIPort.h"
#include "loadObjects.h"

GUISystemPort::GUISystemPort(AppearanceData* pAppearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    //Sets the ports
    createPorts();
    refreshDisplayName();
}

GUISystemPort::~GUISystemPort()
{
    //! @todo delete systemport in core
}

//! @brief Help function to create ports in the component when it is created
void GUISystemPort::createPorts()
{
    //! @todo Only one port in system ports could simplify this
    PortAppearanceMapT::iterator i;
    for (i = mAppearanceData.getPortAppearanceMap().begin(); i != mAppearanceData.getPortAppearanceMap().end(); ++i)
    {
        qreal x = i.value().x;
        qreal y = i.value().y;

        i.value().selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)

        mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->addSystemPort(i.key()));

        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        mpGuiPort = new GUIPort(mAppearanceData.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this, mpParentSystem->mpCoreSystemAccess);
        mPortListPtrs.append(mpGuiPort);
    }
}

//void GUISystemPort::saveToDomElement(QDomElement &rDomElement)
//{
//    //! @todo The tag name should be set in constructor so that we cn reuse the saveToDomElement in all subclasses instead of having to rewrite it every where, the save core stuff and save gui stuff can be specific
//    QDomElement xmlObject = appendDomElement(rDomElement, HMF_SYSTEMPORTTAG);

//    saveCoreDataToDomElement(xmlObject);
//    saveGuiDataToDomElement(xmlObject);
//}

//void GUISystemPort::saveCoreDataToDomElement(QDomElement &rDomElement)
//{
//    appendDomTextNode(rDomElement, HMF_TYPETAG, getTypeName());
//    appendDomTextNode(rDomElement, HMF_NAMETAG, getName());
//}


//! Returns a string with the GUIObject type.
QString GUISystemPort::getTypeName()
{
    return "SystemPort";
}

//! Set the name of a system port
void GUISystemPort::setName(QString newName, renameRestrictions renameSettings)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        if (renameSettings == CORERENAMEONLY)
        {
            //Set name in core component, Also set the current name to the resulting one (might have been changed)
            mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->renameSystemPort(oldName, newName));
            refreshDisplayName();
            mpGuiPort->setDisplayName(mAppearanceData.getName()); //change the actual gui port name
        }
        else
        {
            //Rename
            mpParentSystem->renameGUIObject(oldName, newName);
        }
    }
}


int GUISystemPort::type() const
{
    return Type;
}
