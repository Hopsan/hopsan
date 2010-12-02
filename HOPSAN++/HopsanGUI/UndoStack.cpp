//!
//! @file   UndoStack.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains classes for the undo stack and the undo widget (which displays the stack)
//!
//$Id$

#include <QtGui>

#include <assert.h>

#include "UndoStack.h"
#include "MainWindow.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "loadObjects.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/UndoWidget.h"


//! @class UndoStack
//! @brief The UndoStack class is used as an XML-based storage for undo and redo operations.
//!
//! The stack consists of "undo posts", where each post can contain several actions. One undo post equal pressing ctrl-z once. To add a new post, use newPost().
//! New actions are registered to the stack with their respective register functions. To undo or redo, use the undoOneStep() and redoOneStep() functions.
//!


//! @brief Constructor for the undo stack
//! @param parentSystem Pointer to the current system
UndoStack::UndoStack(GUIContainerObject *parentSystem) : QObject()
{
    mpParentContainerObject = parentSystem;
    mCurrentStackPosition = -1;
}


//! @brief Clears all contents in the undo stack
//! @param errorMsg (optional) Error message that will be displayed in message widget
void UndoStack::clear(QString errorMsg)
{
    mCurrentStackPosition = -1;
    mUndoRoot.clear();
    mUndoRoot = mDomDocument.createElement("hopsanundo");
    mDomDocument.appendChild(mUndoRoot);
    QDomElement firstPost = appendDomElement(mUndoRoot, "post");
    firstPost.setAttribute("number", mCurrentStackPosition);

    gpMainWindow->mpUndoWidget->refreshList();

    if(!errorMsg.isEmpty())
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage(errorMsg);
    }
}


//! @brief Adds a new empty post to the undo stack
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

    gpMainWindow->mpUndoWidget->refreshList();

   //mpParentContainerObject->mpParentProjectTab->hasChanged();
}


//! @brief Will undo the changes registered in the current stack position and decrease stack position one step
//! @see redoOneStep()
void UndoStack::undoOneStep()
{
    bool didSomething = false;
    QList<QDomElement> deletedConnectorList;
    QList<QDomElement> addedConnectorList;
    QList<QDomElement> addedObjectList;
    QStringList movedObjects;
    QList<int> addedWidgetList;
    int dx, dy;
    QDomElement stuffElement = getCurrentPost().firstChildElement("stuff");
    while(!stuffElement.isNull())
    {
        didSomething = true;
        if(stuffElement.attribute("what") == "deletedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            loadGUIModelObject(componentElement, gpMainWindow->mpLibrary, mpParentContainerObject, NOUNDO);
        }
        else if(stuffElement.attribute("what") == "addedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            addedObjectList.append(componentElement);
        }
        else if(stuffElement.attribute("what") == "deletedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement("connect");
            deletedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "addedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement("connect");
            addedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "rename")
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            if(!mpParentContainerObject->hasGUIModelObject(newName))
            {
                this->clear("Undo stack attempted to access non-existing conmponent. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->renameGUIModelObject(newName, oldName, NOUNDO);
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
            QDomElement connectorElement = stuffElement.firstChildElement("connect");
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");
            if(!mpParentContainerObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            GUIConnector *item = mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort);
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
            QString name = stuffElement.attribute("name");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->setPos(x, y);
            movedObjects.append(name);
            dx = x_new - x;
            dy = y_new - y;
    }
        else if(stuffElement.attribute("what") == "movedconnector")
        {
            double dx = stuffElement.attribute("dx").toDouble();
            double dy = stuffElement.attribute("dy").toDouble();

            QDomElement connectorElement = stuffElement.firstChildElement("connect");
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
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            //! @todo Make a function for rotation clockwise, this is crazy!
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "verticalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->flipVertical(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "horizontalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->flipHorizontal(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "changedparameter")
        {
            QString objectName = stuffElement.attribute("objectname");
            QString parameterName = stuffElement.attribute("parametername");
            QString oldValue = stuffElement.attribute("oldvalue");
            if(!mpParentContainerObject->hasGUIModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(objectName)->setParameterValue(parameterName, oldValue);
        }
        else if(stuffElement.attribute("what") == "changedstartvalue")
        {
            QString objectName = stuffElement.attribute("objectname");
            QString portName = stuffElement.attribute("portname");
            QString parameterName = stuffElement.attribute("parametername");
            QString oldValue = stuffElement.attribute("oldvalue");
            if(!mpParentContainerObject->hasGUIModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(objectName)->setStartValue(portName, parameterName, oldValue);
        }
        else if(stuffElement.attribute("what") == "namevisibilitychange")
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            if(isVisible)
            {
                mpParentContainerObject->getGUIModelObject(objectName)->hideName(NOUNDO);
            }
            else
            {
                mpParentContainerObject->getGUIModelObject(objectName)->showName(NOUNDO);
            }
        }
        else if(stuffElement.attribute("what") == "addedboxwidget")
        {
             size_t index = stuffElement.attribute("index").toInt();
             mpParentContainerObject->mWidgetMap.find(index).value()->deleteMe(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "resizedboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            double w_old = stuffElement.attribute("w_old").toDouble();
            double h_old = stuffElement.attribute("h_old").toDouble();
            double x_old, y_old;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, x_old, y_old);
            GUIBoxWidget *tempWidget = qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value());
            tempWidget->setSize(w_old, h_old);
            tempWidget->setPos(x_old, y_old);
        }
        else if(stuffElement.attribute("what") == "modifiedboxwidgetstyle")
        {
            size_t index = stuffElement.attribute("index").toInt();
            int lineWidth = stuffElement.attribute("linewidth_old").toInt();
            QColor lineColor = QColor(stuffElement.attribute("linecolor_old"));
            QString lineStyle = stuffElement.attribute("linestyle_old");
            if(lineStyle == "solidline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::SolidLine);
            }
            else if(lineStyle == "dashline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::DashLine);
            }
            else if(lineStyle == "dotline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::DotLine);
            }
            else if(lineStyle == "dashdotline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::DashDotLine);
            }
            qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineColor(lineColor);
            qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineWidth(lineWidth);
        }
        else if(stuffElement.attribute("what") == "deletedboxwidget")
        {
             QDomElement boxElement = stuffElement.firstChildElement(HMF_BOXWIDGETTAG);
             loadBoxWidget(boxElement, mpParentContainerObject, NOUNDO);
             mpParentContainerObject->mWidgetMap.insert(stuffElement.attribute("index").toInt(), mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value());
             mpParentContainerObject->mWidgetMap.remove(mpParentContainerObject->mHighestWidgetIndex-1);
             mpParentContainerObject->mHighestWidgetIndex -= 1;
        }
        else if(stuffElement.attribute("what") == "addedtextwidget")
        {
             size_t index = stuffElement.attribute("index").toInt();
             addedWidgetList.append(index);
        }
        else if(stuffElement.attribute("what") == "deletedtextwidget")
        {
             QDomElement textElement = stuffElement.firstChildElement(HMF_TEXTWIDGETTAG);
             loadTextWidget(textElement, mpParentContainerObject, NOUNDO);
             mpParentContainerObject->mWidgetMap.insert(stuffElement.attribute("index").toInt(), mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value());
             mpParentContainerObject->mWidgetMap.remove(mpParentContainerObject->mHighestWidgetIndex-1);
             mpParentContainerObject->mHighestWidgetIndex -= 1;
        }
        else if(stuffElement.attribute("what") == "modifiedtextwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            QFont font;
            font.fromString(stuffElement.attribute("font_old"));
            qobject_cast<GUITextWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setText(stuffElement.attribute("text_old"));
            qobject_cast<GUITextWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setTextFont(font);
            qobject_cast<GUITextWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setTextColor(QColor(stuffElement.attribute("color_old")));
        }
        else if(stuffElement.attribute("what") == "movedwidget")
        {
            double x_old, y_old;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, x_old, y_old);
            size_t index = stuffElement.attribute("index").toInt();
            if(!mpParentContainerObject->mWidgetMap.contains(index))
            {
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
        if(!mpParentContainerObject->hasGUIModelObject(startComponent) || !mpParentContainerObject->hasGUIModelObject(endComponent))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        loadConnector(*it, mpParentContainerObject, NOUNDO);
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
        mpParentContainerObject->removeConnector(mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort), NOUNDO);
    }

        //Remove objects after removing connectors, to make sure connectors don't lose their start and end components
    for(it = addedObjectList.begin(); it!=addedObjectList.end(); ++it)
    {
        QString name = (*it).attribute("name");
        if(!mpParentContainerObject->hasGUIModelObject(name))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        this->mpParentContainerObject->deleteGUIModelObject(name, NOUNDO);
    }

        //Move all connectors that are connected between two components that has moved (must be done after components have been moved)
    QList<GUIConnector *>::iterator itc;
    for(itc=mpParentContainerObject->mSubConnectorList.begin(); itc!=mpParentContainerObject->mSubConnectorList.end(); ++itc)
    {
        //Error check should not be necessary for this action, because it was already done when moving the objects
        if(movedObjects.contains((*itc)->getStartComponentName()) && movedObjects.contains((*itc)->getEndComponentName()))
        {
            (*itc)->moveAllPoints(-dx, -dy);
            (*itc)->drawConnector();
        }
    }

    for(size_t i=0; i<addedWidgetList.size(); ++i)
    {
        mpParentContainerObject->mWidgetMap.find(addedWidgetList.at(i)).value()->deleteMe(NOUNDO);
    }

        //Reduce stack position if something was done (otherwise stack is empty)
    if(didSomething)
    {
        mCurrentStackPosition--;
    }

    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Will redo the previously undone changes if they exist, and increase stsack position one step
//! @see undoOneStep()
//! @todo Add error checking (like in undoOneStep()
void UndoStack::redoOneStep()
{
    bool didSomething = false;
    ++mCurrentStackPosition;
    QList<QDomElement> addedConnectorList;
    QList<QDomElement> modifiedConnectorList;
    QStringList movedObjects;
    int dx, dy;
    QDomElement stuffElement = getCurrentPost().firstChildElement("stuff");
    while(!stuffElement.isNull())
    {
        didSomething = true;
        if(stuffElement.attribute("what") == "deletedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            QString name = componentElement.attribute("name");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            this->mpParentContainerObject->deleteGUIModelObject(name, NOUNDO);
        }
        else if(stuffElement.attribute("what") == "addedobject")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            loadGUIModelObject(componentElement, gpMainWindow->mpLibrary, mpParentContainerObject, NOUNDO);
        }
        else if(stuffElement.attribute("what") == "deletedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement("connect");
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");
            if(!mpParentContainerObject->hasConnector(startComponent, startPort, endComponent, endPort))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->removeConnector(mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort), NOUNDO);
        }
        else if(stuffElement.attribute("what") == "addedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement("connect");
            addedConnectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "rename")
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            if(!mpParentContainerObject->hasGUIModelObject(oldName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->renameGUIModelObject(oldName, newName, NOUNDO);
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
            QString name = stuffElement.attribute("name");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->setPos(x, y);
            movedObjects.append(name);
            dx = x - x_old;
            dy = y - y_old;
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "verticalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->flipVertical(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "horizontalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->hasGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->flipHorizontal(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "changedparameter")
        {
            QString objectName = stuffElement.attribute("objectname");
            QString parameterName = stuffElement.attribute("parametername");
            QString newValue = stuffElement.attribute("newvalue");
            if(!mpParentContainerObject->hasGUIModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(objectName)->setParameterValue(parameterName, newValue);
        }
        else if(stuffElement.attribute("what") == "changedstartvalue")
        {
            QString objectName = stuffElement.attribute("objectname");
            QString portName = stuffElement.attribute("portname");
            QString parameterName = stuffElement.attribute("parametername");
            QString newValue = stuffElement.attribute("newvalue");
            if(!mpParentContainerObject->hasGUIModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(objectName)->setStartValue(portName, parameterName, newValue);
        }
        else if(stuffElement.attribute("what") == "namevisibilitychange")
        {
            QString objectName = stuffElement.attribute("objectname");
            bool isVisible = (stuffElement.attribute("isvisible").toInt() == 1);
            if(isVisible)
            {
                mpParentContainerObject->getGUIModelObject(objectName)->showName(NOUNDO);
            }
            else
            {
                mpParentContainerObject->getGUIModelObject(objectName)->hideName(NOUNDO);
            }
        }
        else if(stuffElement.attribute("what") == "addedboxwidget")
        {
            QDomElement boxElement = stuffElement.firstChildElement(HMF_BOXWIDGETTAG);
            gpMainWindow->mpMessageWidget->printGUIDebugMessage(boxElement.toDocument().toString());
            loadBoxWidget(boxElement, mpParentContainerObject, NOUNDO);
            mpParentContainerObject->mWidgetMap.insert(stuffElement.attribute("index").toInt(), mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value());
            mpParentContainerObject->mWidgetMap.remove(mpParentContainerObject->mHighestWidgetIndex-1);
            mpParentContainerObject->mHighestWidgetIndex -= 1;
        }
        else if(stuffElement.attribute("what") == "deletedboxwidget")
        {
             size_t index = stuffElement.attribute("index").toInt();
             mpParentContainerObject->mWidgetMap.find(index).value()->deleteMe(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "resizedboxwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            double w_new = stuffElement.attribute("w_new").toDouble();
            double h_new = stuffElement.attribute("h_new").toDouble();
            double x_new, y_new;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            parseDomValueNode2(newPosElement, x_new, y_new);
            GUIBoxWidget *tempWidget = qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value());
            tempWidget->setSize(w_new, h_new);
            tempWidget->setPos(x_new, y_new);
        }
        else if(stuffElement.attribute("what") == "addedtextwidget")
        {
            QDomElement textElement = stuffElement.firstChildElement(HMF_TEXTWIDGETTAG);
            loadTextWidget(textElement, mpParentContainerObject, NOUNDO);
            mpParentContainerObject->mWidgetMap.insert(stuffElement.attribute("index").toInt(), mpParentContainerObject->mWidgetMap.find(mpParentContainerObject->mHighestWidgetIndex-1).value());
            mpParentContainerObject->mWidgetMap.remove(mpParentContainerObject->mHighestWidgetIndex-1);
            mpParentContainerObject->mHighestWidgetIndex -= 1;
        }
        else if(stuffElement.attribute("what") == "deletedtextwidget")
        {
             size_t index = stuffElement.attribute("index").toInt();
             mpParentContainerObject->mWidgetMap.find(index).value()->deleteMe(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "modifiedtextwidget")
        {
            size_t index = stuffElement.attribute("index").toInt();
            QFont font;
            font.fromString(stuffElement.attribute("font"));
            qobject_cast<GUITextWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setText(stuffElement.attribute("text"));
            qobject_cast<GUITextWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setTextFont(font);
            qobject_cast<GUITextWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setTextColor(QColor(stuffElement.attribute("color")));
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
        else if(stuffElement.attribute("what") == "modifiedboxwidgetstyle")
        {
            size_t index = stuffElement.attribute("index").toInt();
            int lineWidth = stuffElement.attribute("linewidth").toInt();
            QColor lineColor = QColor(stuffElement.attribute("linecolor"));
            QString lineStyle = stuffElement.attribute("linestyle");
            if(lineStyle == "solidline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::SolidLine);
            }
            else if(lineStyle == "dashline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::DashLine);
            }
            else if(lineStyle == "dotline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::DotLine);
            }
            else if(lineStyle == "dashdotline")
            {
                qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineStyle(Qt::DashDotLine);
            }
            qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineColor(lineColor);
            qobject_cast<GUIBoxWidget *>(mpParentContainerObject->mWidgetMap.find(index).value())->setLineWidth(lineWidth);
        }
        stuffElement = stuffElement.nextSiblingElement("stuff");
    }

        //Create connectors after everything else, to make sure components are created before connectors
    QList<QDomElement>::iterator it;
    for(it=addedConnectorList.begin(); it!=addedConnectorList.end(); ++it)
    {
        loadConnector(*it, mpParentContainerObject, NOUNDO);
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

        QDomElement connectorElement = (*it).firstChildElement("connect");
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
        GUIConnector *item = mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort);

        item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
        item->updateLine(lineNumber);
    }

        //Move all connectors that are connected between two components that has moved
    QList<GUIConnector *>::iterator itc;
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

    //! @todo why dont I know of my own UndoWidget (should maybe have a pointer to it directly)
    //! This is because there is only one undo widget, while each loaded model has its own undo stack.
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for deleted objects
//! @param item Pointer to the component about to be deleted
void UndoStack::registerDeletedObject(GUIModelObject *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "deletedobject");
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();

    //gpMainWindow->mpMessageWidget->printGUIDebugMessage(gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mDomDocument.toString());
}


//! @brief Register function for connectors
//! @param item Pointer to the connector about to be deleted
void UndoStack::registerDeletedConnector(GUIConnector *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "deletedconnector");
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for added objects
//! @param itemName Name of the added object
void UndoStack::registerAddedObject(GUIModelObject *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "addedobject");
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for added connectors
//! @param item Pointer to the added connector
void UndoStack::registerAddedConnector(GUIConnector *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "addedconnector");
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Registser function for renaming an object
//! @param oldName Old object name
//! @param newName New object name
void UndoStack::registerRenameObject(QString oldName, QString newName)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "rename");
    stuffElement.setAttribute("oldname", oldName);
    stuffElement.setAttribute("newname", newName);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for modifying a line in a connector
//! @param oldPos Position of the line before it was moved
//! @param item Pointer to the connector
//! @param lineNumber Number of the line that was moved
void UndoStack::registerModifiedConnector(QPointF oldPos, QPointF newPos, GUIConnector *item, int lineNumber)
{
    if(!(getCurrentPost().attribute("type") == "paste"))    //Connectors are modified when undoing paste operations, but this will modify them twice, so don't register it
    {
        QDomElement currentPostElement = getCurrentPost();
        QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
        stuffElement.setAttribute("what", "modifiedconnector");
        stuffElement.setAttribute("linenumber", lineNumber);
        appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
        appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
        item->saveToDomElement(stuffElement);
        gpMainWindow->mpUndoWidget->refreshList();
    }
}


//! @brief Register function for moving an object
//! @param oldPos Position of the object before it was moved
//! @param objectName Name of the object
void UndoStack::registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "movedobject");
    stuffElement.setAttribute("name", objectName);
    appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
    appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for rotating an object
//! @param item Pointer to the object
void UndoStack::registerRotatedObject(QString objectName)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "rotate");
    stuffElement.setAttribute("objectname", objectName);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for vertical flip of an object
//! @param item Pointer to the object
void UndoStack::registerVerticalFlip(QString objectName)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "verticalflip");
    stuffElement.setAttribute("objectname", objectName);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for horizontal flip of an object
//! @param item Pointer to the object
//! @todo Maybe we should combine this and registerVerticalFlip to one function?
void UndoStack::registerHorizontalFlip(QString objectName)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "horizontalflip");
    stuffElement.setAttribute("objectname", objectName);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Registser function for changing parameters of an object
void UndoStack::registerChangedParameter(QString objectName, QString parameterName, QString oldValueTxt, QString newValueTxt)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "changedparameter");
    stuffElement.setAttribute("parametername", parameterName);
    stuffElement.setAttribute("oldvalue", oldValueTxt);
    stuffElement.setAttribute("newvalue", newValueTxt);
    stuffElement.setAttribute("objectname", objectName);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Registser function for changing parameters of an object
void UndoStack::registerChangedStartValue(QString objectName, QString portName, QString parameterName, QString oldValueTxt, QString newValueTxt)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "changedstartvalue");
    stuffElement.setAttribute("parametername", parameterName);
    stuffElement.setAttribute("oldvalue", oldValueTxt);
    stuffElement.setAttribute("newvalue", newValueTxt);
    stuffElement.setAttribute("objectname", objectName);
    stuffElement.setAttribute("portname", portName);
    gpMainWindow->mpUndoWidget->refreshList();
}



//! @brief Register function for changing name visibility of an object
void UndoStack::registerNameVisibilityChange(QString objectName, bool isVisible)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "namevisibilitychange");
    stuffElement.setAttribute("objectname", objectName);
    stuffElement.setAttribute("isvisible", isVisible);
    gpMainWindow->mpUndoWidget->refreshList();
}


void UndoStack::registerAddedBoxWidget(GUIBoxWidget *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "addedboxwidget");
    stuffElement.setAttribute("index", item->mWidgetIndex);
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


void UndoStack::registerDeletedBoxWidget(GUIBoxWidget *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "deletedboxwidget");
    stuffElement.setAttribute("index", item->mWidgetIndex);
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


void UndoStack::registerResizedBoxWidget(int index, double w_old, double h_old, double w_new, double h_new, QPointF oldPos, QPointF newPos)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "resizedboxwidget");
    stuffElement.setAttribute("index", index);
    stuffElement.setAttribute("w_old", w_old);
    stuffElement.setAttribute("h_old", h_old);
    stuffElement.setAttribute("w_new", w_new);
    stuffElement.setAttribute("h_new", h_new);
    appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
    appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
    gpMainWindow->mpUndoWidget->refreshList();

    gpMainWindow->mpMessageWidget->printGUIDebugMessage( mDomDocument.toString(4));
}


void UndoStack::registerModifiedBoxWidgetStyle(int index, int oldLineWidth, Qt::PenStyle oldLineStyle, QColor oldLineColor, int lineWidth, Qt::PenStyle lineStyle, QColor lineColor)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "modifiedboxwidgetstyle");
    stuffElement.setAttribute("index", index);
    stuffElement.setAttribute("linewidth", lineWidth);
    stuffElement.setAttribute("linewidth_old", oldLineWidth);
    stuffElement.setAttribute("linecolor", lineColor.name());
    stuffElement.setAttribute("linecolor_old", oldLineColor.name());

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

    gpMainWindow->mpUndoWidget->refreshList();
}


void UndoStack::registerAddedTextWidget(GUITextWidget *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "addedtextwidget");
    stuffElement.setAttribute("index", item->mWidgetIndex);
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


void UndoStack::registerDeletedTextWidget(GUITextWidget *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "deletedtextwidget");
    stuffElement.setAttribute("index", item->mWidgetIndex);
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


void UndoStack::registerModifiedTextWidget(int index, QString oldText, QFont oldFont, QColor oldColor, QString text, QFont font, QColor color)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "modifiedtextwidget");
    stuffElement.setAttribute("index", index);
    stuffElement.setAttribute("text_old", oldText);
    stuffElement.setAttribute("text", text);
    stuffElement.setAttribute("font_old", oldFont.toString());
    stuffElement.setAttribute("font", font.toString());
    stuffElement.setAttribute("color_old", oldColor.name());
    stuffElement.setAttribute("color", color.name());
    gpMainWindow->mpUndoWidget->refreshList();
}


void UndoStack::registerMovedWidget(GUIWidget *item, QPointF oldPos, QPointF newPos)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "movedwidget");
    stuffElement.setAttribute("index", item->mWidgetIndex);
    appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
    appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
    gpMainWindow->mpUndoWidget->refreshList();
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
