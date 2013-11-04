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
//! @file   UndoStack.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains classes for the undo stack and the undo widget (which displays the stack)
//!
//$Id$

#include <QtGui>

#include "global.h"
#include "UndoStack.h"
#include "GUIConnector.h"
#include "loadFunctions.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/UndoWidget.h"


//! @class UndoStack
//! @brief The UndoStack class is used as an XML-based storage for undo and redo operations.
//!
//! The stack consists of "undo posts", where each post can contain several actions. One undo post equal pressing ctrl-z once. To add a new post, use newPost().
//! New actions are registered to the stack with their respective register functions. To undo or redo, use the undoOneStep() and redoOneStep() functions.
//! In order to maximize performance, it is important not to send more data than necessary to the register functions.
//!


//! @brief Constructor for the undo stack
//! @param parentSystem Pointer to the current system
UndoStack::UndoStack(ContainerObject *parentSystem) : QObject()
{
    mpParentContainerObject = parentSystem;
    mCurrentStackPosition = -1;
}


QDomElement UndoStack::toXml()
{
    return mUndoRoot;
}


void UndoStack::fromXml(QDomElement &undoElement)
{
    mUndoRoot.clear();
    mDomDocument.replaceChild(undoElement.cloneNode(), mDomDocument.firstChild());
    mUndoRoot = undoElement;
    mCurrentStackPosition = mUndoRoot.lastChildElement().attribute("number").toInt();
    gpUndoWidget->refreshList();
}


//! @brief Clears all contents in the undo stack
//! @param errorMsg (optional) Error message that will be displayed in message widget
void UndoStack::clear(QString errorMsg)
{
    mCurrentStackPosition = -1;
    mUndoRoot.clear();
    mUndoRoot = mDomDocument.createElement(HMF_UNDO);
    mDomDocument.appendChild(mUndoRoot);
    QDomElement firstPost = appendDomElement(mUndoRoot, "post");
    firstPost.setAttribute("number", mCurrentStackPosition);

    gpUndoWidget->refreshList();

    if(!errorMsg.isEmpty())
    {
        gpTerminalWidget->mpConsole->printErrorMessage(errorMsg);
    }
}


//! @brief Adds a new empty post to the undo stack
//! @param type String used to specify a post type. This tells the undo widget to display the entire post as one item. For example "paste" is used for paste operations, so that every added object, connector and widget is not shown in the undo widget list.
void UndoStack::newPost(QString type)
{
    ++mCurrentStackPosition;
    QDomElement maybeDeletePost = mUndoRoot.firstChildElement("post");
    while(!maybeDeletePost.isNull())
    {
        QDomElement tempPost = maybeDeletePost;
        maybeDeletePost = maybeDeletePost.nextSiblingElement("post");
        if(tempPost.attribute("number").toInt() > mCurrentStackPosition-1)
        {
            mUndoRoot.removeChild(tempPost);
        }
    }
    QDomElement newPost = appendDomElement(mUndoRoot, "post");
    newPost.setAttribute("number", mCurrentStackPosition);
    if(!type.isEmpty())
    {
        newPost.setAttribute("type", type);
    }

    gpUndoWidget->refreshList();
}


//! @brief Will undo the changes registered in the current stack position and decrease stack position one step
//! @see redoOneStep()
void UndoStack::undoOneStep()
{
    bool didSomething = false;
    QList<QDomElement> deletedConnectorList;
    QList<QDomElement> addedConnectorList;
    QList<QDomElement> addedObjectList;
    QList<QDomElement> addedcontainerportsList;
    QList<QDomElement> addedsubsystemsList;
    QStringList movedObjects;
    QList<int> addedWidgetList;
    int dx=0, dy=0;
    QDomElement stuffElement = getCurrentPost().firstChildElement("stuff");
    while(!stuffElement.isNull())
    {
        didSomething = true;
        if(stuffElement.attribute("what") == "deletedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            ModelObject* pObj = loadModelObject(componentElement, gpLibraryWidget, mpParentContainerObject, NoUndo);

            //Load parameter values
            QDomElement xmlParameters = componentElement.firstChildElement(HMF_PARAMETERS);
            QDomElement xmlParameter = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
            while (!xmlParameter.isNull())
            {
                loadParameterValue(xmlParameter, pObj, NoUndo);
                xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
            }
        }
        else if(stuffElement.attribute("what") == "deletedcontainerport")
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            loadContainerPortObject(systemPortElement, gpLibraryWidget, mpParentContainerObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == "deletedsubsystem")
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            loadModelObject(systemPortElement, gpLibraryWidget, mpParentContainerObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == "addedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            addedObjectList.append(componentElement);
        }
        else if(stuffElement.attribute("what") == "addedcontainerport")
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            addedcontainerportsList.append(systemPortElement);
        }
        else if(stuffElement.attribute("what") == "addedsubsystem")
        {
            QDomElement systemElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            addedsubsystemsList.append(systemElement);
        }
        else if(stuffElement.attribute("what") == "deletedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            deletedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "addedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            addedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "rename")
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            if(!mpParentContainerObject->hasModelObject(newName))
            {
                this->clear("Undo stack attempted to access non-existing conmponent. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->renameModelObject(newName, oldName, NoUndo);
        }
        else if(stuffElement.attribute("what") == "modifiedconnector")
        {
            double tempX, tempY;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, tempX, tempY);
            QPointF oldPos = QPointF(tempX, tempY);
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            parseDomValueNode2(newPosElement, tempX, tempY);
            QPointF newPos = QPointF(tempX, tempY);
            int lineNumber = stuffElement.attribute("linenumber").toDouble();
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");
            if(!mpParentContainerObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            Connector *item = mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort);
            QPointF dXY = newPos - oldPos;
            item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
            item->updateLine(lineNumber);
        }
        else if(stuffElement.attribute("what") == "movedobject")
        {
            double x, y, x_new, y_new;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(newPosElement, x_new, y_new);
            parseDomValueNode2(oldPosElement, x, y);
            QString name = stuffElement.attribute(HMF_NAMETAG);
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(name)->setPos(x, y);
            mpParentContainerObject->getModelObject(name)->updateOldPos();
            movedObjects.append(name);
            dx = x_new - x;
            dy = y_new - y;
        }
        else if(stuffElement.attribute("what") == "movedconnector")
        {
            double dx = stuffElement.attribute("dx").toDouble();
            double dy = stuffElement.attribute("dy").toDouble();

            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");

            if(!mpParentContainerObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort)->moveAllPoints(-dx, -dy);
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QString name = stuffElement.attribute("objectname");
            double angle = stuffElement.attribute("angle").toDouble();
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            //double targetAngle = mpParentContainerObject->getGUIModelObject(name)->rotation()-angle;
            //if(targetAngle >= 360) { targetAngle = 0; }
            //if(targetAngle < 0) { targetAngle = 270; }
            mpParentContainerObject->getModelObject(name)->rotate(-angle, NoUndo);

        }
        else if(stuffElement.attribute("what") == "verticalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(name)->flipVertical(NoUndo);
        }
        else if(stuffElement.attribute("what") == "horizontalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(name)->flipHorizontal(NoUndo);
        }
        else if(stuffElement.attribute("what") == "changedparameter")
        {
            QString objectName = stuffElement.attribute("objectname");
            QString parameterName = stuffElement.attribute("parametername");
            QString oldValue = stuffElement.attribute("oldvalue");
            if(!mpParentContainerObject->hasModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(objectName)->setParameterValue(parameterName, oldValue);
        }
//        else if(stuffElement.attribute("what") == "changedstartvalue")
//        {
//            QString objectName = stuffElement.attribute("objectname");
//            QString portName = stuffElement.attribute("portname");
//            QString parameterName = stuffElement.attribute("parametername");
//            QString oldValue = stuffElement.attribute("oldvalue");
//            if(!mpParentContainerObject->hasGUIModelObject(objectName))
//            {
//                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
//                return;
//            }
//            mpParentContainerObject->getGUIModelObject(objectName)->setStartValue(portName, parameterName, oldValue);
//        }
        else if(stuffElement.attribute("what") == "namevisibilitychange")
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            if(isVisible)
            {
                mpParentContainerObject->getModelObject(objectName)->hideName(NoUndo);
            }
            else
            {
                mpParentContainerObject->getModelObject(objectName)->showName(NoUndo);
            }
        }
        else if(stuffElement.attribute("what") == "addedtextboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            mpParentContainerObject->mWidgetMap.find(index).value()->deleteMe(NoUndo);
        }
        else if(stuffElement.attribute("what") == "deletedtextboxwidget")
        {
            QDomElement textBoxElement = stuffElement.firstChildElement(HMF_TEXTBOXWIDGETTAG);
            loadTextBoxWidget(textBoxElement, mpParentContainerObject, NoUndo);
            mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value()->setWidgetIndex(stuffElement.attribute("index").toInt());
            mpParentContainerObject->mWidgetMap.insert(stuffElement.attribute("index").toInt(), mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value());
            mpParentContainerObject->mWidgetMap.remove(mpParentContainerObject->mHighestWidgetIndex-1);
        }
        else if(stuffElement.attribute("what") == "resizedtextboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            double w_old = stuffElement.attribute("w_old").toDouble();
            double h_old = stuffElement.attribute("h_old").toDouble();
            double x_old, y_old;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, x_old, y_old);
            TextBoxWidget *tempWidget = qobject_cast<TextBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value());
            tempWidget->setSize(w_old, h_old);
            tempWidget->setPos(x_old, y_old);
        }
        else if(stuffElement.attribute("what") == "modifiedtextboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            TextBoxWidget *pWidget = qobject_cast<TextBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value());

            QFont font;
            font.fromString(stuffElement.attribute("font_old"));
            int lineWidth = stuffElement.attribute("linewidth_old").toInt();
            QString lineStyle = stuffElement.attribute("linestyle_old");
            bool boxVisibleBefore = stuffElement.attribute("box_visible_before").toInt();
            if(lineStyle == "solidline")
            {
                pWidget->setLineStyle(Qt::SolidLine);
            }
            else if(lineStyle == "dashline")
            {
                pWidget->setLineStyle(Qt::DashLine);
            }
            else if(lineStyle == "dotline")
            {
                pWidget->setLineStyle(Qt::DotLine);
            }
            else if(lineStyle == "dashdotline")
            {
                pWidget->setLineStyle(Qt::DashDotLine);
            }

            pWidget->setText(stuffElement.attribute("text_old"));
            pWidget->setFont(font);
            pWidget->setColor(QColor(stuffElement.attribute("color_old")));
            pWidget->setLineWidth(lineWidth);
            pWidget->setBoxVisible(boxVisibleBefore);


        }
        else if(stuffElement.attribute("what") == "movedwidget")
        {
            double x_old, y_old;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, x_old, y_old);
            size_t index = stuffElement.attribute("index").toInt();
            if(!mpParentContainerObject->mWidgetMap.contains(index))
            {
                qDebug() << "Failed to find index in map: " << index;
                this->clear("Undo stack attempted to access non-existing widget. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->mWidgetMap.find(index).value()->setPos(x_old, y_old);
        }
        stuffElement = stuffElement.nextSiblingElement("stuff");
    }

        //Re-add connectors after components, to make sure start and end component exist
    QList<QDomElement>::iterator it;
    for(it=deletedConnectorList.begin(); it!=deletedConnectorList.end(); ++it)
    {
        QString startComponent = (*it).attribute("startcomponent");
        QString endComponent = (*it).attribute("endcomponent");
        if(!mpParentContainerObject->hasModelObject(startComponent) || !mpParentContainerObject->hasModelObject(endComponent))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        loadConnector(*it, mpParentContainerObject, NoUndo);
    }

        //Remove connectors after modified connector action
    for(it=addedConnectorList.begin(); it!=addedConnectorList.end(); ++it)
    {
        QString startComponent = (*it).attribute("startcomponent");
        QString startPort = (*it).attribute("startport");
        QString endComponent = (*it).attribute("endcomponent");
        QString endPort = (*it).attribute("endport");
        if(!mpParentContainerObject->hasConnector(startComponent, startPort, endComponent, endPort))
        {
            this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
            return;
        }
        mpParentContainerObject->removeSubConnector(mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort), NoUndo);
    }

        //Remove objects after removing connectors, to make sure connectors don't lose their start and end components
    for(it = addedObjectList.begin(); it!=addedObjectList.end(); ++it)
    {
        QString name = (*it).attribute(HMF_NAMETAG);
        if(!mpParentContainerObject->hasModelObject(name))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        this->mpParentContainerObject->deleteModelObject(name, NoUndo);
    }

        //Remove system ports
    for(it = addedcontainerportsList.begin(); it!=addedcontainerportsList.end(); ++it)
    {
        QString name = (*it).attribute(HMF_NAMETAG);
        if(!mpParentContainerObject->hasModelObject(name))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        this->mpParentContainerObject->deleteModelObject(name, NoUndo);
    }

        //Remove subsystems
    for(it = addedsubsystemsList.begin(); it!=addedsubsystemsList.end(); ++it)
    {
        QString name = (*it).attribute(HMF_NAMETAG);
        SystemContainer *pItem = qobject_cast<SystemContainer *>(mpParentContainerObject->getModelObject(name));

        //Update information about Subsystem in dom thread, in case it has changed since registered
        QDomElement whatElement = (*it).parentNode().toElement();
        QDomElement parentElement = (*it).parentNode().parentNode().toElement();
        QDomElement stuffElement = appendDomElement(parentElement, "stuff");
        stuffElement.setAttribute("what", "addedsubsystem");
        pItem->saveToDomElement(stuffElement);
        parentElement.removeChild(whatElement);

        if(!mpParentContainerObject->hasModelObject(name))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        this->mpParentContainerObject->deleteModelObject(name, NoUndo);
    }

        //Move all connectors that are connected between two components that has moved (must be done after components have been moved)
    QList<Connector *>::iterator itc;
    for(itc=mpParentContainerObject->mSubConnectorList.begin(); itc!=mpParentContainerObject->mSubConnectorList.end(); ++itc)
    {
        //Error check should not be necessary for this action, because it was already done when moving the objects
        if(movedObjects.contains((*itc)->getStartComponentName()) && movedObjects.contains((*itc)->getEndComponentName()))
        {
            (*itc)->moveAllPoints(-dx, -dy);
            (*itc)->drawConnector();
        }
    }

    for(int i=0; i<addedWidgetList.size(); ++i)
    {
        mpParentContainerObject->mWidgetMap.find(addedWidgetList.at(i)).value()->deleteMe(NoUndo);
    }

        //Reduce stack position if something was done (otherwise stack is empty)
    if(didSomething)
    {
        mCurrentStackPosition--;
    }

    gpUndoWidget->refreshList();
}


//! @brief Will redo the previously undone changes if they exist, and increase stsack position one step
//! @see undoOneStep()
void UndoStack::redoOneStep()
{
    //gpMainWindow->mpHcomWidget->mpConsole->printDebugMessage(mDomDocument.toString(2));

    bool didSomething = false;
    ++mCurrentStackPosition;
    QList<QDomElement> addedConnectorList;
    QList<QDomElement> modifiedConnectorList;
    QStringList movedObjects;
    int dx=0, dy=0;
    QDomElement stuffElement = getCurrentPost().firstChildElement("stuff");
    while(!stuffElement.isNull())
    {
        didSomething = true;
        if(stuffElement.attribute("what") == "deletedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            QString name = componentElement.attribute(HMF_NAMETAG);
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            this->mpParentContainerObject->deleteModelObject(name, NoUndo);
        }
        else if(stuffElement.attribute("what") == "deletedcontainerport")
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            QString name = systemPortElement.attribute(HMF_NAMETAG);
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            this->mpParentContainerObject->deleteModelObject(name, NoUndo);
        }
        else if(stuffElement.attribute("what") == "deletedsubsystem")
        {
            QDomElement systemElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            QString name = systemElement.attribute(HMF_NAMETAG);
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            this->mpParentContainerObject->deleteModelObject(name, NoUndo);
        }
        else if(stuffElement.attribute("what") == "addedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            loadModelObject(componentElement, gpLibraryWidget, mpParentContainerObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == "addedcontainerport")
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            loadContainerPortObject(systemPortElement, gpLibraryWidget, mpParentContainerObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == "addedsubsystem")
        {
            QDomElement systemElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            loadModelObject(systemElement, gpLibraryWidget, mpParentContainerObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == "deletedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");
            if(!mpParentContainerObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->removeSubConnector(mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort), NoUndo);
        }
        else if(stuffElement.attribute("what") == "addedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            addedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "rename")
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            if(!mpParentContainerObject->hasModelObject(oldName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->renameModelObject(oldName, newName, NoUndo);
        }
        else if(stuffElement.attribute("what") == "modifiedconnector")
        {
            QDomElement tempElement = stuffElement;
            modifiedConnectorList.append(tempElement);
        }
        else if(stuffElement.attribute("what") == "movedobject")
        {
            double x, y, x_old, y_old;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(newPosElement, x, y);
            parseDomValueNode2(oldPosElement, x_old, y_old);
            QString name = stuffElement.attribute(HMF_NAMETAG);
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(name)->setPos(x, y);
            mpParentContainerObject->getModelObject(name)->updateOldPos();
            movedObjects.append(name);
            dx = x - x_old;
            dy = y - y_old;
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QString name = stuffElement.attribute("objectname");
            double angle = stuffElement.attribute("angle").toDouble();
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            //double targetAngle = mpParentContainerObject->getGUIModelObject(name)->rotation()+angle;
            //if(targetAngle >= 360) { targetAngle = 0; }
            //if(targetAngle < 0) { targetAngle = 270; }
            mpParentContainerObject->getModelObject(name)->rotate(angle, NoUndo);
        }
        else if(stuffElement.attribute("what") == "verticalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(name)->flipVertical(NoUndo);
        }
        else if(stuffElement.attribute("what") == "horizontalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(name)->flipHorizontal(NoUndo);
        }
        else if(stuffElement.attribute("what") == "changedparameter")
        {
            QString objectName = stuffElement.attribute("objectname");
            QString parameterName = stuffElement.attribute("parametername");
            QString newValue = stuffElement.attribute("newvalue");
            if(!mpParentContainerObject->hasModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getModelObject(objectName)->setParameterValue(parameterName, newValue);
        }
//        else if(stuffElement.attribute("what") == "changedstartvalue")
//        {
//            QString objectName = stuffElement.attribute("objectname");
//            QString portName = stuffElement.attribute("portname");
//            QString parameterName = stuffElement.attribute("parametername");
//            QString newValue = stuffElement.attribute("newvalue");
//            if(!mpParentContainerObject->hasGUIModelObject(objectName))
//            {
//                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
//                return;
//            }
//            mpParentContainerObject->getGUIModelObject(objectName)->setStartValue(portName, parameterName, newValue);
//        }
        else if(stuffElement.attribute("what") == "namevisibilitychange")
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            if(isVisible)
            {
                mpParentContainerObject->getModelObject(objectName)->showName(NoUndo);
            }
            else
            {
                mpParentContainerObject->getModelObject(objectName)->hideName(NoUndo);
            }
        }
        else if(stuffElement.attribute("what") == "addedtextboxwidget")
        {
            QDomElement textBoxElement = stuffElement.firstChildElement(HMF_TEXTBOXWIDGETTAG);
            loadTextBoxWidget(textBoxElement, mpParentContainerObject, NoUndo);
            mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value()->setWidgetIndex(stuffElement.attribute("index").toInt());
            mpParentContainerObject->mWidgetMap.insert(stuffElement.attribute("index").toInt(), mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value());
            mpParentContainerObject->mWidgetMap.remove(mpParentContainerObject->mHighestWidgetIndex-1);
        }
        else if(stuffElement.attribute("what") == "deletedtextboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            mpParentContainerObject->mWidgetMap.find(index).value()->deleteMe(NoUndo);
        }
        else if(stuffElement.attribute("what") == "resizedtextboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            double w_new = stuffElement.attribute("w_new").toDouble();
            double h_new = stuffElement.attribute("h_new").toDouble();
            double x_new, y_new;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            parseDomValueNode2(newPosElement, x_new, y_new);
            TextBoxWidget *tempWidget = qobject_cast<TextBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value());
            tempWidget->setSize(w_new, h_new);
            tempWidget->setPos(x_new, y_new);
        }
        else if(stuffElement.attribute("what") == "modifiedtextboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            TextBoxWidget *pWidget = qobject_cast<TextBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value());

            QFont font;
            font.fromString(stuffElement.attribute("font"));

            int lineWidth = stuffElement.attribute("linewidth").toInt();
            QString lineStyle = stuffElement.attribute("linestyle");
            bool boxVisible = stuffElement.attribute("box_visible").toInt();
            if(lineStyle == "solidline")
            {
                pWidget->setLineStyle(Qt::SolidLine);
            }
            else if(lineStyle == "dashline")
            {
                pWidget->setLineStyle(Qt::DashLine);
            }
            else if(lineStyle == "dotline")
            {
                pWidget->setLineStyle(Qt::DotLine);
            }
            else if(lineStyle == "dashdotline")
            {
                pWidget->setLineStyle(Qt::DashDotLine);
            }

            pWidget->setText(stuffElement.attribute("text"));
            pWidget->setFont(font);
            pWidget->setColor(QColor(stuffElement.attribute("color")));
            pWidget->setLineWidth(lineWidth);
            pWidget->setBoxVisible(boxVisible);
        }
        else if(stuffElement.attribute("what") == "movedwidget")
        {
            double x_new, y_new;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            parseDomValueNode2(newPosElement, x_new, y_new);
            size_t index = stuffElement.attribute("index").toInt();
            if(!mpParentContainerObject->mWidgetMap.contains(index))
            {
                this->clear("Undo stack attempted to access non-existing widget. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->mWidgetMap.find(index).value()->setPos(x_new, y_new);
        }
        stuffElement = stuffElement.nextSiblingElement("stuff");
    }

        //Create connectors after everything else, to make sure components are created before connectors
    QList<QDomElement>::iterator it;
    for(it=addedConnectorList.begin(); it!=addedConnectorList.end(); ++it)
    {
        loadConnector(*it, mpParentContainerObject, NoUndo);
    }

    for(it=modifiedConnectorList.begin(); it!=modifiedConnectorList.end(); ++it)
    {
        double tempX, tempY;
        QDomElement oldPosElement = (*it).firstChildElement("oldpos");
        parseDomValueNode2(oldPosElement, tempX, tempY);
        QPointF oldPos = QPointF(tempX, tempY);
        QDomElement newPosElement = (*it).firstChildElement("newpos");
        parseDomValueNode2(newPosElement, tempX, tempY);
        QPointF newPos = QPointF(tempX, tempY);

        int lineNumber = (*it).attribute("linenumber").toDouble();

        QDomElement connectorElement = (*it).firstChildElement(HMF_CONNECTORTAG);
        QString startComponent = connectorElement.attribute("startcomponent");
        QString startPort = connectorElement.attribute("startport");
        QString endComponent = connectorElement.attribute("endcomponent");
        QString endPort = connectorElement.attribute("endport");
        QPointF dXY = oldPos-newPos;
        if(!mpParentContainerObject->hasConnector(startComponent, startPort, endComponent, endPort))
        {
            this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
            return;
        }
        Connector *item = mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort);

        item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
        item->updateLine(lineNumber);
    }

        //Move all connectors that are connected between two components that has moved
    QList<Connector *>::iterator itc;
    for(itc=mpParentContainerObject->mSubConnectorList.begin(); itc!=mpParentContainerObject->mSubConnectorList.end(); ++itc)
    {
        //No error checking necessary
        if(movedObjects.contains((*itc)->getStartComponentName()) && movedObjects.contains((*itc)->getEndComponentName()))
        {
            //! @todo This will add an additional modifiedConneector in undo stack (solved by ugly hack in redo code), but there is no support for NOUNDO in these functions...
            (*itc)->moveAllPoints(dx, dy);
            (*itc)->drawConnector();
        }
    }

        //Reduce stack pointer if something was done (otherwise stack is at max position)
    if(!didSomething)
    {
        --mCurrentStackPosition;
    }

    gpUndoWidget->refreshList();
}


//! @brief Register function for deleted objects
//! @param item Pointer to the component about to be deleted
void UndoStack::registerDeletedObject(ModelObject *item)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    if(item->getTypeName() == "HopsanGUIContainerPort")
    {
        stuffElement.setAttribute("what", "deletedcontainerport");
    }
    else if(item->getTypeName() == "Subsystem")
    {
        stuffElement.setAttribute("what", "deletedsubsystem");
    }
    else
    {
        stuffElement.setAttribute("what", "deletedobject");
    }
    item->saveToDomElement(stuffElement);
    gpUndoWidget->refreshList();

    //qDebug() << mDomDocument.toString();
}


//! @brief Register function for connectors
//! @param item Pointer to the connector about to be deleted
void UndoStack::registerDeletedConnector(Connector *item)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "deletedconnector");
    item->saveToDomElement(stuffElement);
    gpUndoWidget->refreshList();
}


//! @brief Register function for added objects
//! @param itemName Name of the added object
void UndoStack::registerAddedObject(ModelObject *item)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    if(item->getTypeName() == HOPSANGUICONTAINERPORTTYPENAME)
    {
        stuffElement.setAttribute("what", "addedcontainerport");
    }
    else if(item->getTypeName() == HOPSANGUISYSTEMTYPENAME)
    {
        stuffElement.setAttribute("what", "addedsubsystem");
    }
    else
    {
        stuffElement.setAttribute("what", "addedobject");
    }
    item->saveToDomElement(stuffElement);

    gpUndoWidget->refreshList();
}


//! @brief Register function for added connectors
//! @param item Pointer to the added connector
void UndoStack::registerAddedConnector(Connector *item)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "addedconnector");
    item->saveToDomElement(stuffElement);
    gpUndoWidget->refreshList();
}


//! @brief Registser function for renaming an object
//! @param oldName Old object name
//! @param newName New object name
void UndoStack::registerRenameObject(QString oldName, QString newName)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "rename");
    stuffElement.setAttribute("oldname", oldName);
    stuffElement.setAttribute("newname", newName);
    gpUndoWidget->refreshList();
}


//! @brief Register function for modifying a line in a connector
//! @param oldPos Position of the line before it was moved
//! @param item Pointer to the connector
//! @param lineNumber Number of the line that was moved
void UndoStack::registerModifiedConnector(QPointF oldPos, QPointF newPos, Connector *item, int lineNumber)
{
    //! @todo This if statement is a very very very ugly hack...
    if(!(getCurrentPost().attribute("type") == "paste"))    //Connectors are modified when undoing paste operations, but this will modify them twice, so don't register it
    {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", "modifiedconnector");
        stuffElement.setAttribute("linenumber", lineNumber);
        appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
        appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
        item->saveToDomElement(stuffElement);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for moving an object
//! @param oldPos Position of the object before it was moved
//! @param objectName Name of the object
void UndoStack::registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "movedobject");
    stuffElement.setAttribute(HMF_NAMETAG, objectName);
    appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
    appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
    gpUndoWidget->refreshList();
}


//! @brief Register function for rotating an object
//! @param item Pointer to the object
void UndoStack::registerRotatedObject(const QString objectName, const qreal angle)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "rotate");
    stuffElement.setAttribute("objectname", objectName);
    setQrealAttribute(stuffElement, "angle", angle);
    gpUndoWidget->refreshList();
}


//! @brief Register function for vertical flip of an object
//! @param item Pointer to the object
void UndoStack::registerVerticalFlip(QString objectName)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "verticalflip");
    stuffElement.setAttribute("objectname", objectName);
    gpUndoWidget->refreshList();
}


//! @brief Register function for horizontal flip of an object
//! @param item Pointer to the object
void UndoStack::registerHorizontalFlip(QString objectName)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "horizontalflip");
    stuffElement.setAttribute("objectname", objectName);
    gpUndoWidget->refreshList();
}


//! @brief Registser function for changing parameters of an object
//! @param objectName Name of the object with the parameter
//! @param parameterName Name of the changed parameter
//! @param oldValueTxt Text string with old parameter value
//! @param newValueTxt Text string with new parameter value
void UndoStack::registerChangedParameter(QString objectName, QString parameterName, QString oldValueTxt, QString newValueTxt)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "changedparameter");
    stuffElement.setAttribute("parametername", parameterName);
    stuffElement.setAttribute("oldvalue", oldValueTxt);
    stuffElement.setAttribute("newvalue", newValueTxt);
    stuffElement.setAttribute("objectname", objectName);
    gpUndoWidget->refreshList();
}


////! @brief Registser function for changing the start values of an object
////! @param objectName Name of the object
////! @param portName Name of the port where start value has changed
////! @param parameterName Name of the changed start value
////! @param oldValueTxt Text string with old start value
////! @param newValueTxt Text string with new start value
//void UndoStack::registerChangedStartValue(QString objectName, QString portName, QString parameterName, QString oldValueTxt, QString newValueTxt)
//{
//    if(!mpParentContainerObject->isUndoEnabled())
//        return;
//    QDomElement currentPostElement = getCurrentPost();
//    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
//    stuffElement.setAttribute("what", "changedstartvalue");
//    stuffElement.setAttribute("parametername", parameterName);
//    stuffElement.setAttribute("oldvalue", oldValueTxt);
//    stuffElement.setAttribute("newvalue", newValueTxt);
//    stuffElement.setAttribute("objectname", objectName);
//    stuffElement.setAttribute("portname", portName);
//    gpUndoWidget->refreshList();
//}



//! @brief Register function for changing name visibility of an object
//! @param objectName Name of the object
//! @param isVisible Boolean that tells whether the object has become visible (true) or invisible (false)
void UndoStack::registerNameVisibilityChange(QString objectName, bool isVisible)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "namevisibilitychange");
    stuffElement.setAttribute("objectname", objectName);
    stuffElement.setAttribute("isvisible", isVisible);
    gpUndoWidget->refreshList();
}


void UndoStack::registerAddedWidget(Widget *item)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    if(item->mType == "TextBoxWidget")
        stuffElement.setAttribute("what", "addedtextboxwidget");
    stuffElement.setAttribute("index", item->getWidgetIndex());
    item->saveToDomElement(stuffElement);
    gpUndoWidget->refreshList();
}


void UndoStack::registerDeletedWidget(Widget *item)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    if(item->mType == "TextBoxWidget")
        stuffElement.setAttribute("what", "deletedtextboxwidget");
    stuffElement.setAttribute("index", item->getWidgetIndex());
    item->saveToDomElement(stuffElement);
    gpUndoWidget->refreshList();
}


//! @brief Register function for moving a GUI Widget
//! @param item Pointer to the moved GUI Widget
//! @param oldPos Previous position
//! @param newPos New Position
void UndoStack::registerMovedWidget(Widget *item, QPointF oldPos, QPointF newPos)
{
    qDebug() << "registerMovedWidget(), index = " << item->getWidgetIndex();

    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "movedwidget");
    stuffElement.setAttribute("index", item->getWidgetIndex());
    appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
    appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
    gpUndoWidget->refreshList();
}


//! @brief Register function for resizing a text box widget
//! Although this function handles position changes, it must not be confused with registerMovedWidget function. This functin only remembers the position because resizing a box will move the center position.
//! @param index Widget index of the resized box widget
//! @param w_old Previous width of the box widget
//! @param h_old Previous height of the box widget
//! @param w_new New width of the box widget
//! @param h_new New height of the box widget
//! @param oldPos Previous position of the box widget
//! @param newPos New position of the box widget
void UndoStack::registerResizedTextBoxWidget(const int index, const qreal w_old, const qreal h_old, const qreal w_new, const qreal h_new, const QPointF oldPos, const QPointF newPos)
{
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "resizedtextboxwidget");
    stuffElement.setAttribute("index", index);
    setQrealAttribute(stuffElement, "w_old", w_old);
    setQrealAttribute(stuffElement, "h_old", h_old);
    setQrealAttribute(stuffElement, "w_new", w_new);
    setQrealAttribute(stuffElement, "h_new", h_new);
    appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
    appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
    gpUndoWidget->refreshList();
}


void UndoStack::registerModifiedTextBoxWidget(int index, QString oldText, QFont oldFont, QColor oldColor, QString text, QFont font, QColor color, int oldLineWidth, Qt::PenStyle oldLineStyle, int lineWidth, Qt::PenStyle lineStyle, bool boxVisibleBefore, bool boxVisible)
{

    qDebug() << "registerModifiedTextBoxWidget()";
    if(!mpParentContainerObject->isUndoEnabled())
        return;
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");

    stuffElement.setAttribute("what", "modifiedtextboxwidget");
    stuffElement.setAttribute("index", index);
    stuffElement.setAttribute("text_old", oldText);
    stuffElement.setAttribute("text", text);
    stuffElement.setAttribute("font_old", oldFont.toString());
    stuffElement.setAttribute("font", font.toString());
    stuffElement.setAttribute("color_old", oldColor.name());
    stuffElement.setAttribute("color", color.name());
    stuffElement.setAttribute("linewidth_old", oldLineWidth);
    stuffElement.setAttribute("linewidth", lineWidth);
    stuffElement.setAttribute("box_visible_before", boxVisibleBefore);
    stuffElement.setAttribute("box_visible", boxVisible);

    if(lineStyle == Qt::SolidLine)
        stuffElement.setAttribute("linestyle", "solidline");
    else if(lineStyle == Qt::DashLine)
        stuffElement.setAttribute("linestyle", "dashline");
    else if(lineStyle == Qt::DotLine)
        stuffElement.setAttribute("linestyle", "dotline");
    else if(lineStyle == Qt::DashDotLine)
        stuffElement.setAttribute("linestyle", "dashdotline");

    if(oldLineStyle == Qt::SolidLine)
        stuffElement.setAttribute("linestyle_old", "solidline");
    else if(oldLineStyle == Qt::DashLine)
        stuffElement.setAttribute("linestyle_old", "dashline");
    else if(oldLineStyle == Qt::DotLine)
        stuffElement.setAttribute("linestyle_old", "dotline");
    else if(oldLineStyle == Qt::DashDotLine)
        stuffElement.setAttribute("linestyle_old", "dashdotline");

    gpUndoWidget->refreshList();
}



//! @brief Returns the DOM element for the current undo post
QDomElement UndoStack::getCurrentPost()
{
    QDomElement postElement = mUndoRoot.firstChildElement("post");
    while(!postElement.isNull())
    {
        if(postElement.attribute("number").toDouble() == mCurrentStackPosition)
        {
            return postElement;
        }
        postElement = postElement.nextSiblingElement("post");
    }
    return QDomElement();
}
