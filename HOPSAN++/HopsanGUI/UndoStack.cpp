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
void UndoStack::clear()
{
    mCurrentStackPosition = -1;
    mUndoRoot.clear();
    mUndoRoot = mDomDocument.createElement("hopsanundo");
    mDomDocument.appendChild(mUndoRoot);
    QDomElement firstPost = appendDomElement(mUndoRoot, "post");
    firstPost.setAttribute("number", mCurrentStackPosition);
}


//! @brief Adds a new empty post to the undo stack
void UndoStack::newPost()
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

    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Will undo the changes registered in the last stack position, and switch stack pointer one step back
void UndoStack::undoOneStep()
{
    bool didSomething = false;
    QList<QDomElement> connectorList;
    QList<QDomElement> deletedList;
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
            deletedList.append(componentElement);
        }
        else if(stuffElement.attribute("what") == "deletedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement("connect");
            connectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "addedconnector")
        {
            QDomElement connectorElement = stuffElement.firstChildElement("connect");
            QString startComponent = connectorElement.attribute("startcomponent");
            QString startPort = connectorElement.attribute("startport");
            QString endComponent = connectorElement.attribute("endcomponent");
            QString endPort = connectorElement.attribute("endport");
            mpParentContainerObject->removeConnector(mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort), NOUNDO);
        }
        else if(stuffElement.attribute("what") == "rename")
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
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

            QPointF dXY = newPos-oldPos;
            GUIConnector *item = mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort);

            item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
            item->updateLine(lineNumber);
        }
        else if(stuffElement.attribute("what") == "movedobject")
        {
            double x, y;
            QDomElement oldPosElement = stuffElement.firstChildElement("oldpos");
            parseDomValueNode2(oldPosElement, x, y);
            QString name = stuffElement.attribute("name");
            mpParentContainerObject->getGUIModelObject(name)->setPos(x, y);
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            QString name = componentElement.attribute("name");
            //! @todo Make a function for rotation clockwise, this is crazy!
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "verticalflip")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            QString name = componentElement.attribute("name");
            mpParentContainerObject->getGUIModelObject(name)->flipVertical(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            QString name = componentElement.attribute("name");
            mpParentContainerObject->getGUIModelObject(name)->flipHorizontal(NOUNDO);
        }
        stuffElement = stuffElement.nextSiblingElement("stuff");
    }

        //Create connectors after everything else, to make sure components are created before connectors
    QList<QDomElement>::iterator it;
    for(it=connectorList.begin(); it!=connectorList.end(); ++it)
    {
        loadConnector(*it, mpParentContainerObject, NOUNDO);
    }

    for(it = deletedList.begin(); it!=deletedList.end(); ++it)
    {
        QString name = (*it).attribute("name");
        this->mpParentContainerObject->deleteGUIModelObject(name, NOUNDO);
    }

        //Reduce stack position if something was done (otherwise stack is empty)
    if(didSomething)
    {
        mCurrentStackPosition--;
    }

    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Will redo the previously undone changes if they exist, and re-add the undo command to the undo stack.
void UndoStack::redoOneStep()
{
    bool didSomething = false;
    ++mCurrentStackPosition;
    QList<QDomElement> connectorList;
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
            connectorList.append(connectorElement);
        }
        else if(stuffElement.attribute("what") == "rename")
        {
            QString newName = stuffElement.attribute("newname");
            QString oldName = stuffElement.attribute("oldname");
            mpParentContainerObject->renameGUIModelObject(oldName, newName, NOUNDO);
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

            QPointF dXY = oldPos-newPos;
            GUIConnector *item = mpParentContainerObject->findConnector(startComponent, startPort, endComponent, endPort);

            item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
            item->updateLine(lineNumber);
        }
        else if(stuffElement.attribute("what") == "movedobject")
        {
            double x, y;
            QDomElement newPosElement = stuffElement.firstChildElement("newpos");
            parseDomValueNode2(newPosElement, x, y);
            QString name = stuffElement.attribute("name");
            mpParentContainerObject->getGUIModelObject(name)->setPos(x, y);
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            QString name = componentElement.attribute("name");
            mpParentContainerObject->getGUIModelObject(name)->rotate(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "verticalflip")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            QString name = componentElement.attribute("name");
            mpParentContainerObject->getGUIModelObject(name)->flipVertical(NOUNDO);
        }
        else if(stuffElement.attribute("what") == "rotate")
        {
            QDomElement componentElement = stuffElement.firstChildElement("component");
            QString name = componentElement.attribute("name");
            mpParentContainerObject->getGUIModelObject(name)->flipHorizontal(NOUNDO);
        }
        stuffElement = stuffElement.nextSiblingElement("stuff");
    }

        //Create connectors after everything else, to make sure components are created before connectors
    QList<QDomElement>::iterator it;
    for(it=connectorList.begin(); it!=connectorList.end(); ++it)
    {
        loadConnector(*it, mpParentContainerObject, NOUNDO);
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


//! @brief Register function for rotating an object
//! @param item Pointer to the object
void UndoStack::registerRotatedObject(GUIObject *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "rotate");
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for vertical flip of an object
//! @param item Pointer to the object
void UndoStack::registerVerticalFlip(GUIObject *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "verticalflip");
    item->saveToDomElement(stuffElement);
    gpMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for horizontal flip of an object
//! @param item Pointer to the object
//! @todo Maybe we should combine this and registerVerticalFlip to one function?
void UndoStack::registerHorizontalFlip(GUIObject *item)
{
    QDomElement currentPostElement = getCurrentPost();
    QDomElement stuffElement = appendDomElement(currentPostElement, "stuff");
    stuffElement.setAttribute("what", "horizontalflip");
    item->saveToDomElement(stuffElement);
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
    QColor evenColor = QColor("antiquewhite");
    QColor activeColor = QColor("gold");

    size_t pos = 0;
    bool found = true;
    //! @todo Give me a pointer to my undo stack, so I don't need to ask the main window about it!
    QDomElement undoRoot = gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mUndoRoot;
    QDomElement postElement = undoRoot.firstChildElement("post");
    while(found)
    {
        found = false;
        //stuffList.append(QStringList());
        while(!postElement.isNull())
        {
            if(postElement.attribute("number").toInt() == pos)
            {
                //Found correct number, write it's contents to the list
                found = true;
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
                break;
            }
            postElement = postElement.nextSiblingElement("post");
        }
        ++pos;
    }
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
