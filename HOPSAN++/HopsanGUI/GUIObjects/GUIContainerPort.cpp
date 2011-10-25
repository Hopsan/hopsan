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
//! @file   GUIContainerPort.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the ContainerPort class
//!
//$Id$

#include "GUIContainerPort.h"
#include "GUIContainerObject.h"
#include "GUIPort.h"
#include "Dialogs/ContainerPortPropertiesDialog.h"
#include "MainWindow.h"

//! @todo rename GUISystemPort to ContainerPort, rename files also
GUIContainerPort::GUIContainerPort(QPointF position, qreal rotation, GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *pParentContainer, selectionStatus startSelected, graphicsType gfxType)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    mIsSystemPort = (pParentContainer->type() == GUISYSTEM); //determine if I am a system port
    this->mHmfTagName = HMF_SYSTEMPORTTAG;
    //Sets the ports
    createPorts();
    refreshDisplayName();
}

GUIContainerPort::~GUIContainerPort()
{
    qDebug() << "GuiSystemPort destructor: " << this->getName();
    if (mIsSystemPort)
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->deleteSystemPort(this->getName());
    }
    else
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->unReserveUniqueName(this->getName());
    }
}

//! @brief Help function to create ports in the SystemPort Object when it is created
void GUIContainerPort::createPorts()
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


    if (mIsSystemPort)
    {
        qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Adding systemport with name: " << desiredportname;
        mGUIModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->addSystemPort(desiredportname));
        qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,resulting in name from core: " << mGUIModelObjectAppearance.getName();
    }
    else
    {
        qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Adding groupport with desired name: " << desiredportname;
        mGUIModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->reserveUniqueName(desiredportname));
    }

    mpGuiPort = new GUIPort(mGUIModelObjectAppearance.getName(), x*boundingRect().width(), y*boundingRect().height(), &(i.value()), this);
    mPortListPtrs.append(mpGuiPort);

}


//! Returns a string with the GUIObject type.
QString GUIContainerPort::getTypeName()
{
//    if (mIsSystemPort)
//    {
//        return HOPSANGUISYSTEMPORTTYPENAME;
//    }
//    else
//    {
//        //! @todo we should make sure that the gui can register these guispecific names in core to avoid creating objects with these type names
//        return HOPSANGUIGROUPPORTTYPENAME;
//    }

    //! @todo we should make sure that the gui can register these guispecific names in core to avoid creating objects with these type names
    return HOPSANGUICONTAINERPORTTYPENAME;
}

//! @brief Sets the name of the modelobject ContainerPort and the contained GUIPort
//! Note, this function will NOT change the core name of the component
void GUIContainerPort::setDisplayName(QString name)
{
    mGUIModelObjectAppearance.setName(name);
    mPortListPtrs[0]->setDisplayName(name);
    refreshDisplayName();
}

//! @brief ContainerPorts shal only save their port name if they are systemports, if they are group ports no core data should be saved
void GUIContainerPort::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    if (mIsSystemPort)
    {
        rDomElement.setAttribute(HMF_NAMETAG, getName());
    }
}

//! @brief Opens the properties dialog
void GUIContainerPort::openPropertiesDialog()
{
    ContainerPortPropertiesDialog *pDialog = new ContainerPortPropertiesDialog(this, gpMainWindow);
    pDialog->exec();
    delete pDialog;
}


//! @brief Event when double clicking on container port icon.
void GUIContainerPort::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);
    openPropertiesDialog();
}


int GUIContainerPort::type() const
{
    return Type;
}
