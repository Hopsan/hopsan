//$Id$

#include "GUIComponent.h"

#include <QVector>
#include <QtGui> //!< @todo maybe only need qtfile dialog

#include "common.h"
//! @todo clean up these includes
#include "../ProjectTabWidget.h"
#include "../MainWindow.h"
#include "../ParameterDialog.h"
#include "../GUIPort.h"
#include "../GUIConnector.h"
#include "../Utilities/GUIUtilities.h"
#include "../UndoStack.h"
#include "../MessageWidget.h"
#include "../GraphicsScene.h"
#include "../GraphicsView.h"
#include "../LibraryWidget.h"
#include "../loadObjects.h"

#include "GUIGroup.h"
#include "GUISystem.h"

GUIComponent::GUIComponent(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
    : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    //Create the object in core, and get its default core name
    mGUIModelObjectAppearance.setName(mpParentSystem->mpCoreSystemAccess->createComponent(mGUIModelObjectAppearance.getTypeName(), mGUIModelObjectAppearance.getName()));

    //Sets the ports
    createPorts();

    refreshDisplayName(); //Make sure name window is correct size for center positioning

    std::cout << "GUIcomponent: " << this->mGUIModelObjectAppearance.getTypeName().toStdString() << std::endl;

    //Set the hmf save tag name
    mHmfTagName = HMF_COMPONENTTAG;
}

GUIComponent::~GUIComponent()
{
    //Remove in core
    //! @todo maybe change to delte instead of remove with dodelete yes
    mpParentSystem->mpCoreSystemAccess->removeSubComponent(this->getName(), true);
}



////!
////! @brief This function sets the desired component name
////! @param [in] newName The new name
////! @param [in] renameSettings  Dont use this if you dont know what you are doing
////!
////! The desired new name will be sent to the the core component and may be modified. Rename will be called in the graphics view to make sure that the guicomponent map key value is up to date.
////! renameSettings is a somewhat ugly hack, we need to be able to force setName without calling rename in some very special situations, it defaults to false
////!
//void GUIComponent::setName(QString newName, renameRestrictions renameSettings)
//{
//    QString oldName = getName();
//    //If name same as before do nothing
//    if (newName != oldName)
//    {
////        //Check if we want to avoid trying to rename in the graphics view map
////        if (renameSettings == CORERENAMEONLY)
////        {
////            mGUIModelObjectAppearance.setName(mpParentSystem->mpCoreSystemAccess->renameSubComponent(this->getName(), newName));
////            refreshDisplayName();
////        }
////        else
////        {
////            //Rename
////            mpParentSystem->renameGUIObject(oldName, newName);
////        }
//    }
//}


//! Event when double clicking on component icon.
void GUIComponent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);
    std::cout << "GUIComponent.cpp: " << "mouseDoubleClickEvent " << std::endl;

    openParameterDialog();

}


//! Returns a string with the component type.
QString GUIComponent::getTypeName()
{
    return mGUIModelObjectAppearance.getTypeName();
}

QString GUIComponent::getTypeCQS()
{
    return mpParentSystem->mpCoreSystemAccess->getSubComponentTypeCQS(this->getName());
}

//! @brief Get a vector with the names of the available parameters
QVector<QString> GUIComponent::getParameterNames()
{
    return mpParentSystem->mpCoreSystemAccess->getParameterNames(this->getName());
}

QString GUIComponent::getParameterUnit(QString name)
{
    return mpParentSystem->mpCoreSystemAccess->getParameterUnit(this->getName(), name);
}

QString GUIComponent::getParameterDescription(QString name)
{
    return mpParentSystem->mpCoreSystemAccess->getParameterDescription(this->getName(), name);
}

double GUIComponent::getParameterValue(QString name)
{
    return mpParentSystem->mpCoreSystemAccess->getParameterValue(this->getName(), name);
}

//! @brief Set a parameter value, wrapps hopsan core
void GUIComponent::setParameterValue(QString name, double value)
{
    mpParentSystem->mpCoreSystemAccess->setParameter(this->getName(), name, value);
}

void GUIComponent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QAction *groupAction;
    if (!this->scene()->selectedItems().empty())
    {
        groupAction = menu.addAction(tr("Group components"));
    }

    QAction *parameterAction = menu.addAction(tr("Change parameters"));
    //menu.insertSeparator(parameterAction);

    QAction *showNameAction = menu.addAction(tr("Show name"));
    showNameAction->setCheckable(true);
    showNameAction->setChecked(mpNameText->isVisible());

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == parameterAction)
    {
        openParameterDialog();
    }
    else if (selectedAction == groupAction)
    {
        //groupComponents(mpParentGraphicsScene->selectedItems());
        GUIModelObjectAppearance appdata;
        appdata.setIconPathUser("subsystemtmp.svg");
        appdata.setBasePath("../../HopsanGUI/"); //!< @todo This is EXTREAMLY BAD
        GUIGroup *pGroup = new GUIGroup(this->scene()->selectedItems(), &appdata, mpParentSystem);
        this->scene()->addItem(pGroup);
    }
    else if (selectedAction == showNameAction)
    {
        if(mpNameText->isVisible())
        {
            this->hideName();
        }
        else
        {
            this->showName();
        }
    }
    QGraphicsItem::contextMenuEvent(event);
}


//! @brief Slot that opens the parameter dialog for the component
void GUIComponent::openParameterDialog()
{
    ParameterDialog *dialog = new ParameterDialog(this);
    dialog->exec();
}


//! @brief Help function to create ports in the component when it is created
void GUIComponent::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (not really necessary in guicomponents)
    QString cqsType = mpParentSystem->mpCoreSystemAccess->getSubComponentTypeCQS(getName());
    PortAppearanceMapT::iterator i;
    for (i = mGUIModelObjectAppearance.getPortAppearanceMap().begin(); i != mGUIModelObjectAppearance.getPortAppearanceMap().end(); ++i)
    {
        QString nodeType = mpParentSystem->mpCoreSystemAccess->getNodeType(this->getName(), i.key());
        QString portType = mpParentSystem->mpCoreSystemAccess->getPortType(this->getName(), i.key());
        i.value().selectPortIcon(cqsType, portType, nodeType);

        qreal x = i.value().x * mpIcon->sceneBoundingRect().width();
        qreal y = i.value().y * mpIcon->sceneBoundingRect().height();

        GUIPort *pNewPort = new GUIPort(i.key(), x, y, &(i.value()), this);
        mPortListPtrs.append(pNewPort);
    }
}



int GUIComponent::type() const
{
    return Type;
}


//! @brief Save GuiObject to a text stream
void GUIComponent::saveToTextStream(QTextStream &rStream, QString prepend)
{
//    QPointF pos = mapToScene(boundingRect().center());
//    rStream << "COMPONENT " << getTypeName() << " " << addQuotes(it.value()->getName()) << " "
//            << pos.x() << " " << pos.y() << " " << rotation() << " " << getNameTextPos() << "\n";
    GUIModelObject::saveToTextStream(rStream, prepend);

    QVector<QString> parameterNames = mpParentSystem->mpCoreSystemAccess->getParameterNames(this->getName());
    QVector<QString>::iterator pit;
    for(pit = parameterNames.begin(); pit != parameterNames.end(); ++pit)
    {
        //! @todo It is a bit strange that we can not control the parameter keyword, but then agian spliting this into a separate function with its own prepend variable would also be wierd
        rStream << "PARAMETER " << addQuotes(getName()) << " " << addQuotes(*pit) << " " <<
                mpParentSystem->mpCoreSystemAccess->getParameterValue(this->getName(), (*pit)) << "\n";
    }
}

void GUIComponent::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    GUIModelObject::saveCoreDataToDomElement(rDomElement);

    //Save parameters (also core related)
    //! @todo need more efficient fetching of both par names and values in one call to avoid re-searching every time
    QVector<QString> parameterNames = mpParentSystem->mpCoreSystemAccess->getParameterNames(this->getName());
    QVector<QString>::iterator pit;
    for(pit = parameterNames.begin(); pit != parameterNames.end(); ++pit)
    {
        QDomElement xmlParam = appendDomElement(rDomElement, HMF_PARAMETERTAG);
//        appendDomTextNode(xmlParam, HMF_NAMETAG, *pit);
//        appendDomValueNode(xmlParam, HMF_VALUETAG, mpParentSystem->mpCoreSystemAccess->getParameterValue(this->getName(), (*pit)));
        xmlParam.setAttribute(HMF_NAMETAG, *pit);
        xmlParam.setAttribute(HMF_VALUETAG, mpParentSystem->mpCoreSystemAccess->getParameterValue(this->getName(), (*pit)));
    }

    //Save start values
    QVector<QString> startValueNames;
    QVector<double> startValueValues;
    QVector<QString> dummy;
    QList<GUIPort*>::iterator portIt;
    for(portIt = mPortListPtrs.begin(); portIt != mPortListPtrs.end(); ++portIt)
    {
        mpParentSystem->mpCoreSystemAccess->getStartValueDataNamesValuesAndUnits(this->getName(), (*portIt)->getName(), startValueNames, startValueValues, dummy);
        if((!startValueNames.empty()))
        {
//            QDomElement xmlPort = appendDomElement(rDomElement, HMF_PORTTAG);
//            appendDomTextNode(xmlPort, HMF_NAMETAG, (*portIt)->getName());
            for(size_t i = 0; i < startValueNames.size(); ++i)
            {
                //QDomElement xmlStartValue = appendDomElement(xmlPort, "startvalue");
                QDomElement xmlStartValue = appendDomElement(rDomElement, "startvalue");
//                appendDomTextNode(xmlStartValue, "quantity", startValueNames[i]);
//                appendDomValueNode(xmlStartValue, "value", startValueValues[i]);
                xmlStartValue.setAttribute("portname", (*portIt)->getName());
                xmlStartValue.setAttribute("variable", startValueNames[i]);
                xmlStartValue.setAttribute("value", startValueValues[i]);
            }
        }
    }
}
