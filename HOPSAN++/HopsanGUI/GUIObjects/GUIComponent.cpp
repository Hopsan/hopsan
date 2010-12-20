//$Id$

#include "GUIComponent.h"

#include <QVector>
#include <QtGui> //!< @todo maybe only need qtfile dialog

#include "../common.h"
//! @todo clean up these includes
#include "../Widgets/ProjectTabWidget.h"
#include "../MainWindow.h"
#include "../Dialogs/ComponentPropertiesDialog.h"
#include "../GUIPort.h"
#include "../GUIConnector.h"
#include "../Utilities/GUIUtilities.h"
#include "../UndoStack.h"
#include "../Widgets/MessageWidget.h"
#include "../GraphicsView.h"
#include "../Widgets/LibraryWidget.h"
#include "../loadObjects.h"

#include "GUIGroup.h"
#include "GUISystem.h"
#include "../Widgets/SystemParametersWidget.h"

GUIComponent::GUIComponent(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, GUIContainerObject *pParentContainer, selectionStatus startSelected, graphicsType gfxType)
    : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    //Create the object in core, and get its default core name
    mGUIModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->createComponent(mGUIModelObjectAppearance.getTypeName(), mGUIModelObjectAppearance.getName()));

    //Sets the ports
    createPorts();

    refreshDisplayName(); //Make sure name window is correct size for center positioning

    //std::cout << "GUIcomponent: " << this->mGUIModelObjectAppearance.getTypeName().toStdString() << std::endl;

    //Set the hmf save tag name
    mHmfTagName = HMF_COMPONENTTAG;
}

GUIComponent::~GUIComponent()
{
    //Remove in core
    //! @todo maybe change to delte instead of remove with dodelete yes
    mpParentContainerObject->getCoreSystemAccessPtr()->removeSubComponent(this->getName(), true);
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
////            mGUIModelObjectAppearance.setName(mpParentSystem->getCoreSystemAccessPtr()->renameSubComponent(this->getName(), newName));
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

    openPropertiesDialog();

}


//! Returns a string with the component type.
QString GUIComponent::getTypeName()
{
    return mGUIModelObjectAppearance.getTypeName();
}

QString GUIComponent::getTypeCQS()
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getSubComponentTypeCQS(this->getName());
}


QString GUIComponent::getHelpPicture()
{
    return mGUIModelObjectAppearance.getHelpPicture();
}


QString GUIComponent::getHelpText()
{
    return mGUIModelObjectAppearance.getHelpText();
}


//! @brief Get a vector with the names of the available parameters
QVector<QString> GUIComponent::getParameterNames()
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterNames(this->getName());
}

QString GUIComponent::getParameterUnit(QString name)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterUnit(this->getName(), name);
}

QString GUIComponent::getParameterDescription(QString name)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterDescription(this->getName(), name);
}

double GUIComponent::getParameterValue(QString name)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterValue(this->getName(), name);
}

QString GUIComponent::getParameterValueTxt(QString name)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterValueTxt(this->getName(), name);
}

//! @brief Set a parameter value to be mapped to a System parameter
bool GUIComponent::setParameterValue(QString name, QString sysParName)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->setParameter(this->getName(), name, sysParName);
}

//! @brief Set a start value to be mapped to a System parameter
bool GUIComponent::setStartValue(QString portName, QString variable, QString sysParName)
{
    QVector<QString> vVariable;
    QVector<QString> vSysParName;
    vVariable.append(variable);
    vSysParName.append(sysParName);
    return this->getPort(portName)->setStartValueDataByNames(vVariable, vSysParName);
}


//! @brief Set a start value to be mapped to a System parameter
QString GUIComponent::getStartValueTxt(QString portName, QString variable)
{
    QVector<QString> vVariable, vSysParName, vUnit;
    this->getPort(portName)->getStartValueDataNamesValuesAndUnits(vVariable, vSysParName, vUnit);
    int idx = vVariable.indexOf(variable);
    if(idx < 0)
        return "";
    else
        return vSysParName[idx];
}


//void GUIComponent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
//{
//    QMenu menu;
//    this->buildBaseContextMenu(menu, event->screenPos());

////    QAction *groupAction;
////    if (!this->scene()->selectedItems().empty())
////    {
////        groupAction = menu.addAction(tr("Group components"));
////    }

////    QAction *parameterAction = menu.addAction(tr("Change parameters"));
////    //menu.insertSeparator(parameterAction);

////    QAction *showNameAction = menu.addAction(tr("Show name"));
////    showNameAction->setCheckable(true);
////    showNameAction->setChecked(mpNameText->isVisible());

////    QAction *selectedAction = menu.exec(event->screenPos());

////    if (selectedAction == parameterAction)
////    {
////        openComponentPropertiesDialog();
////    }
////    else if (selectedAction == showNameAction)
////    {
////        if(mpNameText->isVisible())
////        {
////            this->hideName();
////        }
////        else
////        {
////            this->showName();
////        }
////    }
////    else if (selectedAction == groupAction)
////    {
//////        //groupComponents(mpParentQGraphicsScene->selectedItems());
//////        GUIModelObjectAppearance appdata;
//////        appdata.setIconPathUser("subsystemtmp.svg");
//////        appdata.setBasePath("../../HopsanGUI/"); //!< @todo This is EXTREAMLY BAD
//////        //! @todo add like all other guimodelobjects add as proper object

//////        //qDebug() << "Group event pos: " << event->pos() << " in scene: " << this->mapToScene(event->pos());
//////        GUIGroup *pGroup = new GUIGroup(this->mapToScene(event->pos()).toPoint(), 0.0, &appdata, this->mpParentContainerObject);
//////        this->mpParentContainerObject->getContainedScenePtr()->addItem(pGroup);

////        //! @todo this does not work at all will fix tomorrow if I can figure it out
////        connect(this, SIGNAL(groupSelected(QPointF)), this->mpParentContainerObject, SLOT(groupSelected(QPointF)));
////        emit groupSelected(this->mapToScene(event->pos()));
////        //disconnect(this, SIGNAL(groupSelected(QPointF)), this->mpParentContainerObject, SLOT(groupSelected(QPointF)));
////    }
//    QGraphicsItem::contextMenuEvent(event);

//}


//! @brief Slot that opens the parameter dialog for the component
void GUIComponent::openPropertiesDialog()
{
    ComponentPropertiesDialog *pDialog = new ComponentPropertiesDialog(this);
    pDialog->exec();
    delete pDialog;
}


//! @brief Help function to create ports in the component when it is created
void GUIComponent::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (not really necessary in guicomponents)
    QString cqsType = mpParentContainerObject->getCoreSystemAccessPtr()->getSubComponentTypeCQS(getName());
    PortAppearanceMapT::iterator i;
    for (i = mGUIModelObjectAppearance.getPortAppearanceMap().begin(); i != mGUIModelObjectAppearance.getPortAppearanceMap().end(); ++i)
    {
        QString nodeType = mpParentContainerObject->getCoreSystemAccessPtr()->getNodeType(this->getName(), i.key());
        QString portType = mpParentContainerObject->getCoreSystemAccessPtr()->getPortType(this->getName(), i.key());
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


////! @brief Save GuiObject to a text stream
//void GUIComponent::saveToTextStream(QTextStream &rStream, QString prepend)
//{
//    GUIModelObject::saveToTextStream(rStream, prepend);

//    QVector<QString> parameterNames = mpParentContainerObject->getCoreSystemAccessPtr()->getParameterNames(this->getName());
//    QVector<QString>::iterator pit;
//    for(pit = parameterNames.begin(); pit != parameterNames.end(); ++pit)
//    {
//        //! @todo It is a bit strange that we can not control the parameter keyword, but then agian spliting this into a separate function with its own prepend variable would also be wierd
//        rStream << "PARAMETER " << addQuotes(getName()) << " " << addQuotes(*pit) << " " <<
//                mpParentContainerObject->getCoreSystemAccessPtr()->getParameterValue(this->getName(), (*pit)) << "\n";
//    }
//}

void GUIComponent::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    GUIModelObject::saveCoreDataToDomElement(rDomElement);

    //Save parameters (also core related)
    QDomElement xmlParameters = appendDomElement(rDomElement, HMF_PARAMETERS);
    //! @todo need more efficient fetching of both par names and values in one call to avoid re-searching every time
    QVector<QString> parameterNames = mpParentContainerObject->getCoreSystemAccessPtr()->getParameterNames(this->getName());
    QVector<QString>::iterator pit;
    for(pit = parameterNames.begin(); pit != parameterNames.end(); ++pit)
    {
        QDomElement xmlParam = appendDomElement(xmlParameters, HMF_PARAMETERTAG);
//        appendDomTextNode(xmlParam, HMF_NAMETAG, *pit);
//        appendDomValueNode(xmlParam, HMF_VALUETAG, mpParentSystem->getCoreSystemAccessPtr()->getParameterValue(this->getName(), (*pit)));
        xmlParam.setAttribute(HMF_NAMETAG, *pit);
        xmlParam.setAttribute(HMF_VALUETAG, mpParentContainerObject->getCoreSystemAccessPtr()->getParameterValueTxt(this->getName(), (*pit)));
        /*if(this->isParameterMappedToSystemParameter(*pit))
        {
            xmlParam.setAttribute(HMF_SYSTEMPARAMETERTAG, this->getSystemParameterKey(*pit));
        }*/
    }

    //Save start values
    QDomElement xmlStartValues = appendDomElement(rDomElement, HMF_STARTVALUES);
    QVector<QString> startValueNames;
    QVector<QString> startValueValuesTxt;
    QVector<QString> dummy;
    QList<GUIPort*>::iterator portIt;
    for(portIt = mPortListPtrs.begin(); portIt != mPortListPtrs.end(); ++portIt)
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->getStartValueDataNamesValuesAndUnits(this->getName(), (*portIt)->getName(), startValueNames, startValueValuesTxt, dummy);
        if((!startValueNames.empty()))
        {
//            QDomElement xmlPort = appendDomElement(rDomElement, HMF_PORTTAG);
//            appendDomTextNode(xmlPort, HMF_NAMETAG, (*portIt)->getName());
            for(int i = 0; i < startValueNames.size(); ++i)
            {
                //QDomElement xmlStartValue = appendDomElement(xmlPort, "startvalue");
                QDomElement xmlStartValue = appendDomElement(xmlStartValues, "startvalue");
//                appendDomTextNode(xmlStartValue, "quantity", startValueNames[i]);
//                appendDomValueNode(xmlStartValue, "value", startValueValues[i]);
                xmlStartValue.setAttribute("portname", (*portIt)->getName());
                xmlStartValue.setAttribute("variable", startValueNames[i]);
                xmlStartValue.setAttribute("value", startValueValuesTxt[i]);
            }
        }
    }
}
