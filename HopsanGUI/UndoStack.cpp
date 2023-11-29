/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   UndoStack.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains classes for the undo stack and the undo widget (which displays the stack)
//!
//$Id$

#include <iostream>

#include "global.h"
#include "UndoStack.h"
#include "GUIConnector.h"
#include "loadFunctions.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIWidgets.h"
#include "MessageHandler.h"
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
UndoStack::UndoStack(SystemObject *parentSystem)
{
    mpParentSystemObject = parentSystem;
    mCurrentStackPosition = -1;
    mEnabled = true;
    clear();
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
        gpMessageHandler->addErrorMessage(errorMsg);
    }
}


//! @brief Adds a new empty post to the undo stack
//! @param type String used to specify a post type. This tells the undo widget to display the entire post as one item. For example UNDO_PASTE is used for paste operations, so that every added object, connector and widget is not shown in the undo widget list.
void UndoStack::newPost(QString type)
{
    if (mEnabled) {
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
}


//! @brief Will undo the changes registered in the current stack position and decrease stack position one step
//! @see redoOneStep()
void UndoStack::undoOneStep()
{
    bool didSomething = false;
    QList<QDomElement> deletedConnectorList;
    QList<QDomElement> addedConnectorList;
    QList<QDomElement> addedObjectList;
    QList<QDomElement> addedsystemportsList;
    QList<QDomElement> addedsubsystemsList;
    QStringList movedObjects;
    int dx=0, dy=0;
    QDomElement stuffElement = getCurrentPost().lastChildElement("stuff");
    while(!stuffElement.isNull())
    {
        didSomething = true;
        if(stuffElement.attribute("what") == UNDO_DELETEDOBJECT)
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            ModelObject* pObj = loadModelObject(componentElement, mpParentSystemObject, NoUndo);

            //Load parameter values
            QDomElement xmlParameters = componentElement.firstChildElement(HMF_PARAMETERS);
            QDomElement xmlParameter = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
            while (!xmlParameter.isNull())
            {
                loadParameterValue(xmlParameter, pObj, NoUndo);
                xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
            }
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDSYSTEMPORT)
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            loadSystemPortObject(systemPortElement, mpParentSystemObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDSUBSYSTEM)
        {
            QDomElement systemElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            loadModelObject(systemElement, mpParentSystemObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDOBJECT)
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            addedObjectList.append(componentElement);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDSYSTEMPORT)
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            addedsystemportsList.append(systemPortElement);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDSUBSYSTEM)
        {
            QDomElement systemElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            addedsubsystemsList.append(systemElement);
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDCONNECTOR)
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            deletedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDCONNECTOR)
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            addedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == UNDO_RENAME)
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            if(!mpParentSystemObject->hasModelObject(newName))
            {
                this->clear("Undo stack attempted to access non-existing conmponent. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->renameModelObject(newName, oldName, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_MODIFIEDCONNECTOR)
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
            if(!mpParentSystemObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            Connector *item = mpParentSystemObject->findConnector(startComponent, startPort, endComponent, endPort);
            QPointF dXY = newPos - oldPos;
            item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
            item->updateLine(lineNumber);
        }
        else if(stuffElement.attribute("what") == UNDO_MOVEDOBJECT)
        {
            double x, y, x_new, y_new;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(newPosElement, x_new, y_new);
            parseDomValueNode2(oldPosElement, x, y);
            QString name = stuffElement.attribute(HMF_NAMETAG);
            if(!mpParentSystemObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->getModelObject(name)->setPos(x, y);
            mpParentSystemObject->getModelObject(name)->rememberPos();
            movedObjects.append(name);
            dx = x_new - x;
            dy = y_new - y;
        }
        else if(stuffElement.attribute("what") == UNDO_MOVEDCONNECTOR)
        {
            double dx = stuffElement.attribute("dx").toDouble();
            double dy = stuffElement.attribute("dy").toDouble();

            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");

            if(!mpParentSystemObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->findConnector(startComponent, startPort, endComponent, endPort)->moveAllPoints(-dx, -dy);
        }
        else if(stuffElement.attribute("what") == UNDO_ROTATE)
        {
            QString name = stuffElement.attribute("objectname");
            double angle = stuffElement.attribute("angle").toDouble();
            if(!mpParentSystemObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            //double targetAngle = mpParentSystemObject->getGUIModelObject(name)->rotation()-angle;
            //if(targetAngle >= 360) { targetAngle = 0; }
            //if(targetAngle < 0) { targetAngle = 270; }
            mpParentSystemObject->getModelObject(name)->rotate(-angle, NoUndo);

        }
        else if(stuffElement.attribute("what") == UNDO_VERTICALFLIP)
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentSystemObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->getModelObject(name)->flipVertical(NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_HORIZONTALFLIP)
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentSystemObject->hasModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->getModelObject(name)->flipHorizontal(NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_CHANGEDPARAMETER)
        {
            QString objectName = stuffElement.attribute("objectname");
            QString parameterName = stuffElement.attribute("parametername");
            QString oldValue = stuffElement.attribute("oldvalue");
            if(!mpParentSystemObject->hasModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->getModelObject(objectName)->setParameterValue(parameterName, oldValue);
        }
        else if(stuffElement.attribute("what") == UNDO_NAMEVISIBILITYCHANGE)
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            if(isVisible)
            {
                mpParentSystemObject->getModelObject(objectName)->hideName(NoUndo);
            }
            else
            {
                mpParentSystemObject->getModelObject(objectName)->showName(NoUndo);
            }
        }
        else if(stuffElement.attribute("what") == UNDO_ALWAYSVISIBLECHANGE)
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            mpParentSystemObject->getModelObject(objectName)->setAlwaysVisible(!isVisible, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDTEXTBOXWIDGET)
        {
            removeTextboxWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDTEXTBOXWIDGET)
        {
            addTextboxwidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_RESIZEDTEXTBOXWIDGET)
        {
            size_t index = stuffElement.attribute("index").toInt();
            double w_old = stuffElement.attribute("w_old").toDouble();
            double h_old = stuffElement.attribute("h_old").toDouble();
            double x_old, y_old;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, x_old, y_old);
            TextBoxWidget *pTempWidget = qobject_cast<TextBoxWidget *>(mpParentSystemObject->getWidget(index));
            if (pTempWidget)
            {
                pTempWidget->setSize(w_old, h_old);
                pTempWidget->setPos(x_old, y_old);
            }
        }
        else if(stuffElement.attribute("what") == UNDO_MODIFIEDTEXTBOXWIDGET)
        {
            modifyTextboxWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDIMAGEWIDGET)
        {
            removeImageWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDIMAGEWIDGET)
        {
            addImageWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_MODIFIEDIMAGEWIDGET)
        {
            modifyImageWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_MOVEDWIDGET)
        {
            double x_old, y_old;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, x_old, y_old);
            int id = stuffElement.attribute("index").toInt();
            Widget *pWidget = mpParentSystemObject->getWidget(id);
            if(pWidget)
            {
                pWidget->setPos(x_old, y_old);
            }
            else
            {
                qDebug() << "Failed to find index in map: " << id;
                this->clear("Undo stack attempted to access non-existing widget. Stack was cleared to ensure stability.");
                return;
            }
        }
        else if(stuffElement.attribute("what") == UNDO_REMOVEDALIASES)
        {
            QDomElement xmlAlias = stuffElement.firstChildElement(HMF_ALIAS);
            while (!xmlAlias.isNull())
            {
                loadPlotAlias(xmlAlias, mpParentSystemObject);
                xmlAlias = xmlAlias.nextSiblingElement(HMF_ALIAS);
            }
        }
        stuffElement = stuffElement.previousSiblingElement("stuff");
    }

    // Re-add connectors after components, to make sure start and end component exist
    QList<QDomElement>::iterator it;
    for(it=deletedConnectorList.begin(); it!=deletedConnectorList.end(); ++it)
    {
        QString startComponent = (*it).attribute("startcomponent");
        QString endComponent = (*it).attribute("endcomponent");
        if(!mpParentSystemObject->hasModelObject(startComponent) || !mpParentSystemObject->hasModelObject(endComponent))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        loadConnector(*it, mpParentSystemObject, NoUndo);
    }

    // Remove connectors after modified connector action
    for(it=addedConnectorList.begin(); it!=addedConnectorList.end(); ++it)
    {
        QString startComponent = (*it).attribute("startcomponent");
        QString startPort = (*it).attribute("startport");
        QString endComponent = (*it).attribute("endcomponent");
        QString endPort = (*it).attribute("endport");
        auto pConnectorToRemove = mpParentSystemObject->findConnector(startComponent, startPort, endComponent, endPort);
        if(pConnectorToRemove) {
            mpParentSystemObject->removeSubConnector(pConnectorToRemove, NoUndo);
        }
        else {
            gpMessageHandler->addErrorMessage("Undo stack attempted to access non-existing connector.");
        }
    }

    // Remove objects after removing connectors, to make sure connectors don't lose their start and end components
    for(it = addedObjectList.begin(); it!=addedObjectList.end(); ++it)
    {
        QString name = (*it).attribute(HMF_NAMETAG);
        if(!mpParentSystemObject->hasModelObject(name))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        this->mpParentSystemObject->deleteModelObject(name, NoUndo);
    }

    // Remove system ports
    for(it = addedsystemportsList.begin(); it!=addedsystemportsList.end(); ++it)
    {
        QString name = (*it).attribute(HMF_NAMETAG);
        if(!mpParentSystemObject->hasModelObject(name))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        this->mpParentSystemObject->deleteModelObject(name, NoUndo);
    }

    // Remove subsystems
    for(it = addedsubsystemsList.begin(); it!=addedsubsystemsList.end(); ++it)
    {
        QString name = (*it).attribute(HMF_NAMETAG);
        SystemObject *pSystemToRemove = qobject_cast<SystemObject *>(mpParentSystemObject->getModelObject(name));

        // Update information about Subsystem in DOM tree, in case it has changed inside since registered in the undo stack
        QDomElement oldElement = (*it).parentNode().toElement();
        QDomElement parentElement = (*it).parentNode().parentNode().toElement();
        QDomElement newStuffElement = oldElement.ownerDocument().createElement("stuff");
        parentElement.insertAfter(newStuffElement, oldElement); //Note! cant use append, because then order will be changed and redo will fail some operations
        newStuffElement.setAttribute("what", UNDO_ADDEDSUBSYSTEM);
        pSystemToRemove->saveToDomElement(newStuffElement);
        parentElement.removeChild(oldElement);

        if(!mpParentSystemObject->hasModelObject(name))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        this->mpParentSystemObject->deleteModelObject(name, NoUndo);
    }

    // Move all connectors that are connected between two components that has moved (must be done after components have been moved)
    QList<Connector *>::iterator itc;
    for(itc=mpParentSystemObject->mSubConnectorList.begin(); itc!=mpParentSystemObject->mSubConnectorList.end(); ++itc)
    {
        //Error check should not be necessary for this action, because it was already done when moving the objects
        if(movedObjects.contains((*itc)->getStartComponentName()) && movedObjects.contains((*itc)->getEndComponentName()))
        {
            (*itc)->moveAllPoints(-dx, -dy);
            (*itc)->drawConnector();
        }
    }

    // Reduce stack position if something was done (otherwise stack is empty)
    if(didSomething)
    {
        mCurrentStackPosition--;
    }

    gpUndoWidget->refreshList();
}


//! @brief Will redo the previously undone changes if they exist, and increase stack position one step
//! @see undoOneStep()
void UndoStack::redoOneStep()
{
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
        if(stuffElement.attribute("what") == UNDO_DELETEDOBJECT)
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            QString name = componentElement.attribute(HMF_NAMETAG);
            if(mpParentSystemObject->hasModelObject(name)) {
                mpParentSystemObject->deleteModelObject(name, NoUndo);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+name);
            }
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDSYSTEMPORT)
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            QString name = systemPortElement.attribute(HMF_NAMETAG);
            if(mpParentSystemObject->hasModelObject(name)) {
                mpParentSystemObject->deleteModelObject(name, NoUndo);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+name);
            }

        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDSUBSYSTEM)
        {
            QDomElement systemElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            QString name = systemElement.attribute(HMF_NAMETAG);
            if(mpParentSystemObject->hasModelObject(name)) {
                mpParentSystemObject->deleteModelObject(name, NoUndo);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+name);
            }
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDOBJECT)
        {
            QDomElement componentElement = stuffElement.firstChildElement(HMF_COMPONENTTAG);
            loadModelObject(componentElement, mpParentSystemObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDSYSTEMPORT)
        {
            QDomElement systemPortElement = stuffElement.firstChildElement(HMF_SYSTEMPORTTAG);
            loadSystemPortObject(systemPortElement, mpParentSystemObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDSUBSYSTEM)
        {
            QDomElement systemElement = stuffElement.firstChildElement(HMF_SYSTEMTAG);
            loadModelObject(systemElement, mpParentSystemObject, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDCONNECTOR)
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");
            if(!mpParentSystemObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->removeSubConnector(mpParentSystemObject->findConnector(startComponent, startPort, endComponent, endPort), NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDCONNECTOR)
        {
            QDomElement connectorElement = stuffElement.firstChildElement(HMF_CONNECTORTAG);
            addedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == UNDO_RENAME)
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            if(mpParentSystemObject->hasModelObject(oldName)) {
                mpParentSystemObject->renameModelObject(oldName, newName, NoUndo);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+oldName);
            }

        }
        else if(stuffElement.attribute("what") == UNDO_MODIFIEDCONNECTOR)
        {
            QDomElement tempElement = stuffElement;
            modifiedConnectorList.append(tempElement);
        }
        else if(stuffElement.attribute("what") == UNDO_MOVEDOBJECT)
        {
            double x, y, x_old, y_old;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(newPosElement, x, y);
            parseDomValueNode2(oldPosElement, x_old, y_old);
            QString name = stuffElement.attribute(HMF_NAMETAG);
            if(mpParentSystemObject->hasModelObject(name)) {
                mpParentSystemObject->getModelObject(name)->setPos(x, y);
                mpParentSystemObject->getModelObject(name)->rememberPos();
                movedObjects.append(name);
                dx = x - x_old;
                dy = y - y_old;
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+name);
            }
        }
        else if(stuffElement.attribute("what") == UNDO_ROTATE)
        {
            QString name = stuffElement.attribute("objectname");
            double angle = stuffElement.attribute("angle").toDouble();
            if(mpParentSystemObject->hasModelObject(name)) {
                mpParentSystemObject->getModelObject(name)->rotate(angle, NoUndo);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+name);
            }

        }
        else if(stuffElement.attribute("what") == UNDO_VERTICALFLIP)
        {
            QString name = stuffElement.attribute("objectname");
            if(mpParentSystemObject->hasModelObject(name)) {
                mpParentSystemObject->getModelObject(name)->flipVertical(NoUndo);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+name);
            }

        }
        else if(stuffElement.attribute("what") == UNDO_HORIZONTALFLIP)
        {
            QString name = stuffElement.attribute("objectname");
            if(mpParentSystemObject->hasModelObject(name)) {
                mpParentSystemObject->getModelObject(name)->flipHorizontal(NoUndo);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+name);
            }

        }
        else if(stuffElement.attribute("what") == UNDO_CHANGEDPARAMETER)
        {
            QString objectName = stuffElement.attribute("objectname");
            QString parameterName = stuffElement.attribute("parametername");
            QString newValue = stuffElement.attribute("newvalue");
            if(mpParentSystemObject->hasModelObject(objectName)) {
                mpParentSystemObject->getModelObject(objectName)->setParameterValue(parameterName, newValue);
            }
            else{
                gpMessageHandler->addErrorMessage("Undo stack (redo) attempted to access non-existing component: "+objectName);
            }

        }
        else if(stuffElement.attribute("what") == UNDO_NAMEVISIBILITYCHANGE)
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            if(isVisible)
            {
                mpParentSystemObject->getModelObject(objectName)->showName(NoUndo);
            }
            else
            {
                mpParentSystemObject->getModelObject(objectName)->hideName(NoUndo);
            }
        }
        else if(stuffElement.attribute("what") == UNDO_ALWAYSVISIBLECHANGE)
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            mpParentSystemObject->getModelObject(objectName)->setAlwaysVisible(isVisible, NoUndo);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDTEXTBOXWIDGET)
        {
            addTextboxwidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDTEXTBOXWIDGET)
        {
            removeTextboxWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_RESIZEDTEXTBOXWIDGET)
        {
            size_t index = stuffElement.attribute("index").toInt();
            double w_new = stuffElement.attribute("w_new").toDouble();
            double h_new = stuffElement.attribute("h_new").toDouble();
            double x_new, y_new;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            parseDomValueNode2(newPosElement, x_new, y_new);
            TextBoxWidget *tempWidget = qobject_cast<TextBoxWidget *>(mpParentSystemObject->mWidgetMap.find(index).value());
            tempWidget->setSize(w_new, h_new);
            tempWidget->setPos(x_new, y_new);
        }
        else if(stuffElement.attribute("what") == UNDO_MODIFIEDTEXTBOXWIDGET)
        {
            modifyTextboxWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_ADDEDIMAGEWIDGET)
        {
            addImageWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_DELETEDIMAGEWIDGET)
        {
            removeImageWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_MODIFIEDIMAGEWIDGET)
        {
            modifyImageWidget(stuffElement);
        }
        else if(stuffElement.attribute("what") == UNDO_MOVEDWIDGET)
        {
            double x_new, y_new;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            parseDomValueNode2(newPosElement, x_new, y_new);
            size_t index = stuffElement.attribute("index").toInt();
            if(!mpParentSystemObject->mWidgetMap.contains(index))
            {
                this->clear("Undo stack attempted to access non-existing widget. Stack was cleared to ensure stability.");
                return;
            }
            mpParentSystemObject->mWidgetMap.find(index).value()->setPos(x_new, y_new);
        }
        stuffElement = stuffElement.nextSiblingElement("stuff");
    }

        //Create connectors after everything else, to make sure components are created before connectors
    QList<QDomElement>::iterator it;
    for(it=addedConnectorList.begin(); it!=addedConnectorList.end(); ++it)
    {
        loadConnector(*it, mpParentSystemObject, NoUndo);
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
        if(!mpParentSystemObject->hasConnector(startComponent, startPort, endComponent, endPort))
        {
            this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
            return;
        }
        Connector *item = mpParentSystemObject->findConnector(startComponent, startPort, endComponent, endPort);

        item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
        item->updateLine(lineNumber);
    }

        //Move all connectors that are connected between two components that has moved
    QList<Connector *>::iterator itc;
    for(itc=mpParentSystemObject->mSubConnectorList.begin(); itc!=mpParentSystemObject->mSubConnectorList.end(); ++itc)
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
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        if (currentPostElement.isNull())
            return;
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        if(item->getTypeName() == HOPSANGUISYSTEMPORTTYPENAME)
        {
            stuffElement.setAttribute("what", UNDO_DELETEDSYSTEMPORT);
        }
        else if(item->getTypeName() == HOPSANGUISYSTEMTYPENAME || item->getTypeName() == HOPSANGUICONDITIONALSYSTEMTYPENAME)
        {
            stuffElement.setAttribute("what", UNDO_DELETEDSUBSYSTEM);
        }
        else
        {
            stuffElement.setAttribute("what", UNDO_DELETEDOBJECT);
        }
        item->saveToDomElement(stuffElement);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for connectors
//! @param item Pointer to the connector about to be deleted
void UndoStack::registerDeletedConnector(Connector *item)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        //! @todo this check below is needed elsewhere also, to prevent crash under rare circumstances
        if (currentPostElement.isNull())
            return;
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_DELETEDCONNECTOR);
        item->saveToDomElement(stuffElement);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for added objects
//! @param itemName Name of the added object
void UndoStack::registerAddedObject(ModelObject *item)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        if(item->getTypeName() == HOPSANGUISYSTEMPORTTYPENAME)
        {
            stuffElement.setAttribute("what", UNDO_ADDEDSYSTEMPORT);
        }
        else if(item->getTypeName() == HOPSANGUISYSTEMTYPENAME || item->getTypeName() == HOPSANGUICONDITIONALSYSTEMTYPENAME)
        {
            stuffElement.setAttribute("what", UNDO_ADDEDSUBSYSTEM);
        }
        else
        {
            stuffElement.setAttribute("what", UNDO_ADDEDOBJECT);
        }
        item->saveToDomElement(stuffElement);

        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for added connectors
//! @param item Pointer to the added connector
void UndoStack::registerAddedConnector(Connector *pConnector)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_ADDEDCONNECTOR);
        pConnector->saveToDomElement(stuffElement);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for renaming an object
//! @param oldName Old object name
//! @param newName New object name
void UndoStack::registerRenameObject(QString oldName, QString newName)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_RENAME);
        stuffElement.setAttribute("oldname", oldName);
        stuffElement.setAttribute("newname", newName);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for modifying a line in a connector
//! @param oldPos Position of the line before it was moved
//! @param item Pointer to the connector
//! @param lineNumber Number of the line that was moved
void UndoStack::registerModifiedConnector(QPointF oldPos, QPointF newPos, Connector *item, int lineNumber)
{
    if(mEnabled) {
        //! @todo This if statement is a very very very ugly hack...
        if(!(getCurrentPost().attribute("type") == UNDO_PASTE))    //Connectors are modified when undoing paste operations, but this will modify them twice, so don't register it
        {
            QDomElement currentPostElement = getCurrentPost();
            QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
            stuffElement.setAttribute("what", UNDO_MODIFIEDCONNECTOR);
            stuffElement.setAttribute("linenumber", lineNumber);
            appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
            appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
            item->saveToDomElement(stuffElement);
            gpUndoWidget->refreshList();
        }
    }
}


//! @brief Register function for moving an object
//! @param oldPos Position of the object before it was moved
//! @param objectName Name of the object
void UndoStack::registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_MOVEDOBJECT);
        stuffElement.setAttribute(HMF_NAMETAG, objectName);
        appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
        appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for rotating an object
//! @param item Pointer to the object
void UndoStack::registerRotatedObject(const QString objectName, const double angle)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_ROTATE);
        stuffElement.setAttribute("objectname", objectName);
        setQrealAttribute(stuffElement, "angle", angle);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for vertical flip of an object
//! @param item Pointer to the object
void UndoStack::registerVerticalFlip(QString objectName)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_VERTICALFLIP);
        stuffElement.setAttribute("objectname", objectName);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for horizontal flip of an object
//! @param item Pointer to the object
void UndoStack::registerHorizontalFlip(QString objectName)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_HORIZONTALFLIP);
        stuffElement.setAttribute("objectname", objectName);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for changing parameters of an object
//! @param objectName Name of the object with the parameter
//! @param parameterName Name of the changed parameter
//! @param oldValueTxt Text string with old parameter value
//! @param newValueTxt Text string with new parameter value
void UndoStack::registerChangedParameter(QString objectName, QString parameterName, QString oldValueTxt, QString newValueTxt)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_CHANGEDPARAMETER);
        stuffElement.setAttribute("parametername", parameterName);
        stuffElement.setAttribute("oldvalue", oldValueTxt);
        stuffElement.setAttribute("newvalue", newValueTxt);
        stuffElement.setAttribute("objectname", objectName);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for changing name visibility of an object
//! @param objectName Name of the object
//! @param isVisible Boolean that tells whether the object has become visible (true) or invisible (false)
void UndoStack::registerNameVisibilityChange(QString objectName, bool isVisible)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_NAMEVISIBILITYCHANGE);
        stuffElement.setAttribute("objectname", objectName);
        stuffElement.setAttribute("isvisible", isVisible);
        gpUndoWidget->refreshList();
    }
}

void UndoStack::registerRemovedAliases(QStringList &aliases)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_REMOVEDALIASES);

        //! @todo need one function that gets both alias and full maybe
        for (int i=0; i<aliases.size(); ++i)
        {
            QDomElement alias = appendDomElement(stuffElement, HMF_ALIAS);
            alias.setAttribute(HMF_TYPE, "variable"); //!< @todo not manual type
            alias.setAttribute(HMF_NAMETAG, aliases[i]);
            QString fullName = mpParentSystemObject->getFullNameFromAlias(aliases[i]);
            appendDomTextNode(alias, "fullname",fullName );
        }
    }
}

void UndoStack::registerAlwaysVisibleChange(QString objectName, bool isVisible)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_ALWAYSVISIBLECHANGE);
        stuffElement.setAttribute("objectname", objectName);
        stuffElement.setAttribute("isvisible", isVisible);
        gpUndoWidget->refreshList();
    }
}


void UndoStack::registerAddedWidget(Widget *item)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        if(item->getWidgetType() == TextBoxWidgetType)
            stuffElement.setAttribute("what", UNDO_ADDEDTEXTBOXWIDGET);
        if(item->getWidgetType() == ImageWidgetType)
            stuffElement.setAttribute("what", UNDO_ADDEDIMAGEWIDGET);
        stuffElement.setAttribute("index", item->getWidgetIndex());
        item->saveToDomElement(stuffElement);
        gpUndoWidget->refreshList();
    }
}


void UndoStack::registerDeletedWidget(Widget *item)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        if(item->getWidgetType() == TextBoxWidgetType)
            stuffElement.setAttribute("what", UNDO_DELETEDTEXTBOXWIDGET);
        if(item->getWidgetType() == ImageWidgetType)
            stuffElement.setAttribute("what", UNDO_DELETEDIMAGEWIDGET);
        stuffElement.setAttribute("index", item->getWidgetIndex());
        item->saveToDomElement(stuffElement);
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for moving a GUI Widget
//! @param item Pointer to the moved GUI Widget
//! @param oldPos Previous position
//! @param newPos New Position
void UndoStack::registerMovedWidget(Widget *item, QPointF oldPos, QPointF newPos)
{
    qDebug() << "registerMovedWidget(), index = " << item->getWidgetIndex();

    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_MOVEDWIDGET);
        stuffElement.setAttribute("index", item->getWidgetIndex());
        appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
        appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
        gpUndoWidget->refreshList();
    }
}


//! @brief Register function for resizing a text box widget
//! Although this function handles position changes, it must not be confused with registerMovedWidget function. This function only remembers the position because resizing a box will move the center position.
//! @param index Widget index of the resized box widget
//! @param w_old Previous width of the box widget
//! @param h_old Previous height of the box widget
//! @param w_new New width of the box widget
//! @param h_new New height of the box widget
//! @param oldPos Previous position of the box widget
//! @param newPos New position of the box widget
void UndoStack::registerResizedTextBoxWidget(const int index, const double w_old, const double h_old, const double w_new, const double h_new, const QPointF oldPos, const QPointF newPos)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", UNDO_RESIZEDTEXTBOXWIDGET);
        stuffElement.setAttribute("index", index);
        setQrealAttribute(stuffElement, "w_old", w_old);
        setQrealAttribute(stuffElement, "h_old", h_old);
        setQrealAttribute(stuffElement, "w_new", w_new);
        setQrealAttribute(stuffElement, "h_new", h_new);
        appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
        appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
        gpUndoWidget->refreshList();
    }
}

void UndoStack::registerModifiedWidget(Widget *pItem)
{
    if(mEnabled) {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        if(pItem->getWidgetType() == TextBoxWidgetType) {
            stuffElement.setAttribute("what", UNDO_MODIFIEDTEXTBOXWIDGET);
        }
        else if(pItem->getWidgetType() == ImageWidgetType) {
            stuffElement.setAttribute("what", UNDO_MODIFIEDIMAGEWIDGET);
        }

        stuffElement.setAttribute("index", pItem->getWidgetIndex());

        // Save the old text box widget
        pItem->saveToDomElement(stuffElement);

        gpUndoWidget->refreshList();
    }
}

void UndoStack::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

void UndoStack::addTextboxwidget(const QDomElement &rStuffElement)
{
    QDomElement textBoxElement = rStuffElement.firstChildElement(HMF_TEXTBOXWIDGETTAG);
    int id = parseAttributeInt(textBoxElement, "index", 0);
    TextBoxWidget *pWidget = mpParentSystemObject->addTextBoxWidget(QPointF(1,1), id, NoUndo);
    pWidget->loadFromDomElement(textBoxElement);
}

void UndoStack::removeTextboxWidget(const QDomElement &rStuffElement)
{
    size_t id = rStuffElement.attribute("index").toInt();
    mpParentSystemObject->deleteWidget(id, NoUndo);
}

void UndoStack::modifyTextboxWidget(QDomElement &rStuffElement)
{
    size_t index = rStuffElement.attribute("index").toInt();
    TextBoxWidget *pWidget = qobject_cast<TextBoxWidget *>(mpParentSystemObject->getWidget(index));
    if (pWidget)
    {
        pWidget->saveToDomElement(rStuffElement);

        QString tagname(HMF_TEXTBOXWIDGETTAG);
        pWidget->loadFromDomElement(rStuffElement.firstChildElement(tagname));

        // Now remember the prevData in case we want to redo/undo again
        rStuffElement.replaceChild(rStuffElement.lastChildElement(tagname), rStuffElement.firstChildElement(tagname));
    }
}


void UndoStack::addImageWidget(const QDomElement &rStuffElement)
{
    QDomElement imageElement = rStuffElement.firstChildElement(hmf::imagewidget);
    int id = parseAttributeInt(imageElement, hmf::index, 0);
    ImageWidget *pWidget = mpParentSystemObject->addImageWidget(QPointF(1,1), id, NoUndo);
    pWidget->loadFromDomElement(imageElement);
}


void UndoStack::removeImageWidget(const QDomElement &rStuffElement)
{
    size_t id = rStuffElement.attribute("index").toInt();
    mpParentSystemObject->deleteWidget(id, NoUndo);
}


void UndoStack::modifyImageWidget(QDomElement &rStuffElement)
{
    size_t index = rStuffElement.attribute("index").toInt();
    ImageWidget *pWidget = qobject_cast<ImageWidget *>(mpParentSystemObject->getWidget(index));
    if (pWidget) {
        pWidget->saveToDomElement(rStuffElement);

        QString tagname(hmf::imagewidget);
        pWidget->loadFromDomElement(rStuffElement.firstChildElement(tagname));

        // Now remember the prevData in case we want to redo/undo again
        rStuffElement.replaceChild(rStuffElement.lastChildElement(tagname), rStuffElement.firstChildElement(tagname));
    }
}


//! @brief Returns the DOM element for the current undo post
QDomElement UndoStack::getCurrentPost()
{
    QDomElement postElement = mUndoRoot.firstChildElement("post");
    while(!postElement.isNull())
    {
        if(postElement.attribute("number").toInt() == mCurrentStackPosition)
        {
            return postElement;
        }
        postElement = postElement.nextSiblingElement("post");
    }
    return QDomElement();
}
