#include "GUISystemPort.h"
#include "GUISystem.h"
#include "GUIPort.h"
#include "loadObjects.h"

GUISystemPort::GUISystemPort(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    this->mHmfTagName = HMF_SYSTEMPORTTAG;
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
    //A system port only contains one port, which should be first in the map, ignore any others (should not be any more)
    PortAppearanceMapT::iterator i = mGUIModelObjectAppearance.getPortAppearanceMap().begin();

    //Check if a systemport name is given in appearance data (for example if the systemport is loaded from file)
    //In that case override the default port name with this name
    QString desiredportname;
    if (mGUIModelObjectAppearance.getName().isEmpty())
    {
        desiredportname = i.key();
    }
    else
    {
        desiredportname = mGUIModelObjectAppearance.getName();
    }

    qreal x = i.value().x;
    qreal y = i.value().y;

    //! @todo should make this function select a systemport icon not undefined
    i.value().selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)

    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Adding systemport with name: " << desiredportname;
    mGUIModelObjectAppearance.setName(mpParentSystem->getCoreSystemAccessPtr()->addSystemPort(desiredportname));
    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,resulting in name from core: " << mGUIModelObjectAppearance.getName();

    //We supply ptr to rootsystem to indicate that this is a systemport
    //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
    mpGuiPort = new GUIPort(mGUIModelObjectAppearance.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this/*, mpParentSystem->getCoreSystemAccessPtr()*/);
    mPortListPtrs.append(mpGuiPort);

}


//! Returns a string with the GUIObject type.
//! @todo maybe not hardcoded string
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
            mGUIModelObjectAppearance.setName(mpParentSystem->getCoreSystemAccessPtr()->renameSystemPort(oldName, newName));
            refreshDisplayName();
            mpGuiPort->setDisplayName(mGUIModelObjectAppearance.getName()); //change the actual gui port name
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
