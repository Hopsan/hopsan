//!
//! @file   UndoStack.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains classes for the undo stack and the undo widget (which displays the stack)
//!
//$Id$

#include <QtGui>
//
#include "UndoStack.h"

//#include <sstream>
#include <QDebug>
#include <QTextStream>

#include "GraphicsView.h"
//#include "GUIObject.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "MainWindow.h"
#include "ProjectTabWidget.h"
#include "LibraryWidget.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "loadObjects.h"
#include <assert.h>
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "MessageWidget.h"


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
        else if(stuffElement.attribute("what") == "rename")
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            if(!mpParentContainerObject->haveGUIModelObject(newName))
            {
                this->clear("Undo stack attempted to access non-existing connector. Stack was cleared to ensure stability.");
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
            if(!mpParentContainerObject->haveGUIModelObject(name))
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
            if(!mpParentContainerObject->haveGUIModelObject(name))
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
            if(!mpParentContainerObject->haveGUIModelObject(name))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(name)->flipVertical(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "horizontalflip")
        {
            QString name = stuffElement.attribute("objectname");
            if(!mpParentContainerObject->haveGUIModelObject(name))
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
            double oldValue = stuffElement.attribute("oldvalue").toDouble();
            if(!mpParentContainerObject->haveGUIModelObject(objectName))
            {
                this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
                return;
            }
            mpParentContainerObject->getGUIModelObject(objectName)->setParameterValue(parameterName, oldValue);
        }
        stuffElement = stuffElement.nextSiblingElement("stuff");
    }

        //Re-add connectors after components, to make sure start and end component exist
    QList<QDomElement>::iterator it;
    for(it=deletedConnectorList.begin(); it!=deletedConnectorList.end(); ++it)
    {
        QString startComponent = (*it).attribute("startcomponent");
        QString endComponent = (*it).attribute("endcomponent");
        if(!mpParentContainerObject->haveGUIModelObject(startComponent) || !mpParentContainerObject->haveGUIModelObject(endComponent))
        {
            this->clear("Undo stack attempted to access non-existing component. Stack was cleared to ensure stability.");
            return;
        }
        loadConnector(*it, mpParentContainerObject, NOUNDO);
    }

        //Remove objects after removing connectors, to make sure connectors don't lose their start and end components
    for(it = addedObjectList.begin(); it!=addedObjectList.end(); ++it)
    {
        QString name = (*it).attribute("name");
        if(!mpParentContainerObject->haveGUIModelObject(name))
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
            mpParentContainerObject->getGUIModelObject(name)->setPos(x, y);

            movedObjects.append(name);
            dx = x - x_old;
            dy = y - y_old;
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QString name = stuffElement.attribute("objectname");
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "verticalflip")
        {
            QString name = stuffElement.attribute("objectname");
            mpParentContainerObject->getGUIModelObject(name)->flipVertical(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "horizontalflip")
        {
            QString name = stuffElement.attribute("objectname");
            mpParentContainerObject->getGUIModelObject(name)->flipHorizontal(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "changedparameter")
        {
            QString objectName = stuffElement.attribute("objectname");
            QString parameterName = stuffElement.attribute("parametername");
            double newValue = stuffElement.attribute("newvalue").toDouble();
            mpParentContainerObject->getGUIModelObject(objectName)->setParameterValue(parameterName, newValue);
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
        GUIConnector *item = mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort);

        item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
        item->updateLine(lineNumber);
    }

        //Move all connectors that are connected between two components that has moved
    QList<GUIConnector *>::iterator itc;
    for(itc=mpParentContainerObject->mSubConnectorList.begin(); itc!=mpParentContainerObject->mSubConnectorList.end(); ++itc)
    {
        //GUIConnector test;

        if(movedObjects.contains((*itc)->getStartComponentName()) && movedObjects.contains((*itc)->getEndComponentName()))
        {
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
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for deleted objects
//! @param item Pointer to the component about to be deleted
void UndoStack::registerDeletedObject(GUIObject *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "deletedobject");
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();

    gpMainWindow->mpMessageWidget->printGUIInfoMessage(gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mDomDocument.toString());
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
void UndoStack::registerAddedObject(GUIObject *item)
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
    qDebug() << "registerModifiedConnector(" << oldPos << ", " << newPos << ", item, lineNumber)";

    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "modifiedconnector");
    stuffElement.setAttribute("linenumber", lineNumber);
    appendDomValueNode2(stuffElement, "oldpos", oldPos.x(), oldPos.y());
    appendDomValueNode2(stuffElement, "newpos", newPos.x(), newPos.y());
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
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


//void UndoStack::registerMovedConnector(double dx, double dy, GUIConnector *item)
//{
//    QDomElement currentPostElement = getCurrentPost();
//    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
//    stuffElement.setAttribute("what", "movedconnector");
//    stuffElement.setAttribute("dx", dx);
//    stuffElement.setAttribute("dy", dy);
//    item->saveToDomElement(stuffElement);
//    gpMainWindow->mpUndoWidget->refreshList();
//}


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
void UndoStack::registerChangedParameter(QString objectName, QString parameterName, double oldValue, double newValue)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "changedparameter");
    stuffElement.setAttribute("parametername", parameterName);
    stuffElement.setAttribute("oldvalue", oldValue);
    stuffElement.setAttribute("newvalue", newValue);
    stuffElement.setAttribute("objectname", objectName);
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


//! @brief Construtor for undo list widget
//! @param parent Pointer to the parent main window
UndoWidget::UndoWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("UndoWidget");
    this->resize(400,500);
    this->setWindowTitle("Undo History");

    mpRedoButton = new QPushButton(tr("&Redo"));
    mpRedoButton->setAutoDefault(false);
    mpRedoButton->setFixedHeight(30);
    QFont tempFont = mpRedoButton->font();
    tempFont.setBold(true);
    mpRedoButton->setFont(tempFont);

    mpUndoButton = new QPushButton(tr("&Undo"));
    mpUndoButton->setAutoDefault(false);
    mpUndoButton->setFixedHeight(30);
    mpUndoButton->setFont(tempFont);

    mpClearButton = new QPushButton(tr("&Clear"));
    mpClearButton->setAutoDefault(false);
    mpClearButton->setFixedHeight(30);
    mpClearButton->setFont(tempFont);

    mUndoTable = new QTableWidget(0,1);
    mUndoTable->setBaseSize(400, 500);
    mUndoTable->setColumnWidth(0, 400);
    mUndoTable->horizontalHeader()->setStretchLastSection(true);
    mUndoTable->horizontalHeader()->hide();

    mpLayout = new QGridLayout();
    mpLayout->addWidget(mUndoTable, 0, 0);
    mpLayout->addWidget(mpUndoButton, 1, 0);
    mpLayout->addWidget(mpRedoButton, 2, 0);
    mpLayout->addWidget(mpClearButton, 3, 0);
    setLayout(mpLayout);
}


//! @brief Reimplementation of show function. Updates the list every time before the widget is displayed.
void UndoWidget::show()
{
    refreshList();
    QDialog::show();
}


//! @brief Refresh function for the list. Reads from the current undo stack and displays the results in the table.
void UndoWidget::refreshList()
{
    if(mpParentMainWindow->mpProjectTabs->count() == 0)
    {
        return;
    }

    QTableWidgetItem *item;
    mUndoTable->clear();
    mUndoTable->setRowCount(0);


    //XML//

    QColor oddColor = QColor("white");
    QColor evenColor = QColor("whitesmoke");
    QColor activeColor = QColor("chartreuse");

    size_t pos = 0;
    bool found = true;
    //! @todo Give me a pointer to my undo stack, so I don't need to ask the main window about it!
    QDomElement undoRoot = gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mUndoRoot;
    QDomElement postElement = undoRoot.firstChildElement("post");
    while(found)
    {
        found = false;
        while(!postElement.isNull())
        {
            if(postElement.attribute("number").toInt() == pos)
            {
                //Found correct number, write it's contents to the list
                found = true;
                if(postElement.attribute("type") != QString())
                {
                    item = new QTableWidgetItem();
                    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                    item->setText(translateTag(postElement.attribute("type")));
                    if(pos == gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
                    {
                        item->setBackgroundColor(activeColor);
                    }
                    else if(pos%2 == 0)
                    {
                        item->setBackgroundColor(evenColor);
                    }
                    else
                    {
                        item->setBackgroundColor(oddColor);
                    }
                    mUndoTable->insertRow(0);
                    mUndoTable->setItem(0,0,item);
                }
                else
                {
                    QDomElement stuffElement = postElement.firstChildElement("stuff");
                    while(!stuffElement.isNull())
                    {
                        item = new QTableWidgetItem();
                        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        item->setText(translateTag(stuffElement.attribute("what")));
                        if(pos == gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
                        {
                            item->setBackgroundColor(activeColor);
                        }
                        else if(pos%2 == 0)
                        {
                            item->setBackgroundColor(evenColor);
                        }
                        else
                        {
                            item->setBackgroundColor(oddColor);
                        }
                        mUndoTable->insertRow(0);
                        mUndoTable->setItem(0,0,item);
                        stuffElement = stuffElement.nextSiblingElement("stuff");
                    }
                }
                break;
            }
            postElement = postElement.nextSiblingElement("post");
        }
        ++pos;
    }
    qDebug() << gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mDomDocument.toString();
}

QString UndoWidget::translateTag(QString tag)
{
    QMap<QString, QString> tagMap;
    tagMap.insert("addedobject",        "Added Object");
    tagMap.insert("addedconnector",     "Added Connector");
    tagMap.insert("deletedobject",      "Deleted Object");
    tagMap.insert("deletedconnector",   "Deleted Connector");
    tagMap.insert("movedobject",        "Moved Object");
    tagMap.insert("rename",             "Renamed Object");
    tagMap.insert("modifiedconnector",  "Modified Connector");
    tagMap.insert("rotate",             "Rotated Object");
    tagMap.insert("flipvertical",       "Flipped Vertical");
    tagMap.insert("fliphorizontal",     "Flipped Horizontal");
    tagMap.insert("paste",              "Paste");
    tagMap.insert("movedmultiple",      "Moved Objects");
    tagMap.insert("cut",                "Cut");
    tagMap.insert("changedparameters",  "Changed Parameter(s)");

    if(tagMap.contains(tag))
        return tagMap.find(tag).value();
    else
        return "Undefined Action";
}


//! @brief Returns a pointer to the undo button
QPushButton *UndoWidget::getUndoButton()
{
    return mpUndoButton;
}


//! @brief Returns a pointer to the redo button
QPushButton *UndoWidget::getRedoButton()
{
    return mpRedoButton;
}


//! @brief Returns a pointer to the clear button
QPushButton *UndoWidget::getClearButton()
{
    return mpClearButton;
}
