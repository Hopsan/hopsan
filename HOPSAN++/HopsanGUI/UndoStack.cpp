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
#include "GUIObjects/GUISystem.h"

//! @brief Constructor for the undo stack
//! @param parentSystem Pointer to the current system
UndoStack::UndoStack(GUISystem *parentSystem) : QObject()
{
    mpParentSystem = parentSystem;
    mCurrentStackPosition = -1;
    mStack.clear();
}


//! @brief Clears all contents in the undo stack
void UndoStack::clear()
{
    mCurrentStackPosition = -1;
    mStack.clear();
    newPost();
}


//! @brief Adds a new empty post to the undo stack
void UndoStack::newPost()
{
    int tempSize = mStack.size()-1;
    for(int i = mCurrentStackPosition; i != tempSize; ++i)
    {
        mStack.pop_back();
    }

    if(mCurrentStackPosition < 0)
    {
        QList<QString> tempList;
        mStack.append(tempList);
        mCurrentStackPosition = 0;
    }
    else if(!mStack[mCurrentStackPosition].empty())
    {
        QList<QString> tempList;
        mStack.append(tempList);
        ++mCurrentStackPosition;
    }
    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpUndoWidget->refreshList();
}


//! @brief Inserts an undo post to the current stack position
void UndoStack::insertPost(QString str)
{
    if(mCurrentStackPosition < 0)
    {
        this->clear();
    }
    if(!mpParentSystem->mUndoDisabled)
    {
        mStack[mCurrentStackPosition].insert(0,str);
        mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpUndoWidget->refreshList();
    }
}


//! @brief Will undo the changes registered in the last stack position, and switch stack pointer one step back
void UndoStack::undoOneStep()
{
    int undoPosition = mCurrentStackPosition;
    if(mCurrentStackPosition < 0)
    {
        undoPosition = -1;
    }
    else
    {
        while( mStack[undoPosition].empty() && (undoPosition != 0) )
        {
            --undoPosition;
        }
    }

    if(undoPosition > -1)
    {
        for(int i = 0; i < mStack[undoPosition].size(); ++i)
        {
            QString undoevent;
            QTextStream poststream(&mStack[undoPosition][i]);
            poststream >> undoevent;

            if( undoevent == "DELETEDOBJECT" )
            {
                //poststream >> junk; //Discard Component load command
                ////! @todo maybe we should not save it automatically in the guiobject maby let some other external save function add it
                loadGUIModelObject(poststream, mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary,  mpParentSystem, NOUNDO);
            }
            else if ( undoevent == "DELETEDCONNECTOR" )
            {
                loadConnector(poststream, mpParentSystem, NOUNDO);
            }
            else if( undoevent == "ADDEDOBJECT" )
            {
                //! @todo Maybe we only need to save the name
                readName(poststream); //Discard Type
                QString name = readName(poststream); //Store name
                mpParentSystem->deleteGUIModelObject(name, NOUNDO);
            }
            else if( undoevent == "ADDEDCONNECTOR" )
            {
                //! @todo Do we really need to save the entire connector for this
                QString startCompName = readName(poststream);
                QString startPortName = readName(poststream);
                QString endCompName = readName(poststream);
                QString endPortName = readName(poststream);
                GUIConnector *item = mpParentSystem->findConnector(startCompName, startPortName, endCompName, endPortName);
                mpParentSystem->removeConnector(item, NOUNDO);
            }
            else if( undoevent == "RENAMEDOBJECT" )     //! @todo This does not affect the GUIObject name! (but removes undo post...)
            {
                QString oldName = readName(poststream);
                QString newName = readName(poststream);
                mpParentSystem->renameGUIObject(newName, oldName, NOUNDO);
            }
            else if( undoevent == "MODIFIEDCONNECTOR" )
            {
                QPointF oldPt, newPt;
                int lineNumber;

                QString startComp = readName(poststream);
                QString startPort = readName(poststream);
                QString endComp = readName(poststream);
                QString endPort = readName(poststream);
                poststream >> oldPt.rx() >> oldPt.ry() >> newPt.rx() >> newPt.ry() >> lineNumber;
                GUIConnector *item = mpParentSystem->findConnector(startComp, startPort, endComp, endPort);

                QPointF dXY = newPt-oldPt;
                item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()-dXY);
                item->updateLine(lineNumber);
            }
            else if( undoevent == "MOVEDOBJECT" )
            {
                QPointF oldPt, newPt;
                QString name = readName(poststream);
                poststream >> oldPt.rx() >> oldPt.ry() >> newPt.rx() >> newPt.ry();
                mpParentSystem->getGUIModelObject(name)->setPos(oldPt);
            }
            else if( undoevent == "ROTATEDOBJECT" )
            {
                QString name = readName(poststream);
                //! @todo This feels wierd, why rotate three times
                mpParentSystem->getGUIModelObject(name)->rotate(NOUNDO);
                mpParentSystem->getGUIModelObject(name)->rotate(NOUNDO);
                mpParentSystem->getGUIModelObject(name)->rotate(NOUNDO);
            }
            else if( undoevent == "VERTICALFLIP" )
            {
                QString name = readName(poststream);
                mpParentSystem->getGUIModelObject(name)->flipVertical(NOUNDO);
            }
            else if( undoevent == "HORIZONTALFLIP" )
            {
                QString name = readName(poststream);
                mpParentSystem->getGUIModelObject(name)->flipHorizontal(NOUNDO);
            }
        }
        mCurrentStackPosition = undoPosition - 1;

        mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
    }
    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpUndoWidget->refreshList();
}


//! @brief Will redo the previously undone changes if they exist, and re-add the undo command to the undo stack.
void UndoStack::redoOneStep()
{
    if( (mCurrentStackPosition != mStack.size()-1) && (!mStack[mCurrentStackPosition+1].empty()) )
    {
        ++mCurrentStackPosition;
        for(int i = mStack[mCurrentStackPosition].size()-1; i > -1; --i)
        {
            QString redoevent;
            QTextStream poststream (&mStack[mCurrentStackPosition][i]);
            poststream >> redoevent;

            //qDebug() << "REDO: " << redoevent;
            if( redoevent == "DELETEDOBJECT" )
            {
                readName(poststream); //Discard Type
                QString name = readName(poststream); //Store name
                mpParentSystem->deleteGUIModelObject(name, NOUNDO);
            }
            else if ( redoevent == "DELETEDCONNECTOR" )
            {
                //! @todo Do we really need to save the entire connector for this
                QString startCompName = readName(poststream);
                QString startPortName = readName(poststream);
                QString endCompName = readName(poststream);
                QString endPortName = readName(poststream);
                GUIConnector *item = mpParentSystem->findConnector(startCompName, startPortName, endCompName, endPortName);
                mpParentSystem->removeConnector(item, NOUNDO);
            }
            else if( redoevent == "ADDEDOBJECT" )
            {
                  loadGUIModelObject(poststream, mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary,  mpParentSystem, NOUNDO);
            }
            else if( redoevent == "ADDEDCONNECTOR" )
            {
                loadConnector(poststream, mpParentSystem, NOUNDO);
            }
            else if( redoevent == "RENAMEDOBJECT" )
            {
                QString oldName = readName(poststream);
                QString newName = readName(poststream);
                mpParentSystem->renameGUIObject(oldName, newName, NOUNDO);
            }
            else if( redoevent == "MODIFIEDCONNECTOR" )
            {
                QPointF oldPt, newPt;
                int lineNumber;
                QString startComp = readName(poststream);
                QString startPort = readName(poststream);
                QString endComp = readName(poststream);
                QString endPort = readName(poststream);
                poststream >> oldPt.rx() >> oldPt.ry() >> newPt.rx() >> newPt.ry() >> lineNumber;

                GUIConnector *item = mpParentSystem->findConnector(startComp, startPort, endComp, endPort);

                QPointF dXY = newPt-oldPt;
                item->getLine(lineNumber)->setPos(item->getLine(lineNumber)->pos()+dXY);
                item->updateLine(lineNumber);
            }
            else if( redoevent == "MOVEDOBJECT" )
            {
                QPointF oldPt, newPt;
                QString name = readName(poststream);
                poststream >> oldPt.rx() >> oldPt.ry() >> newPt.rx() >> newPt.ry();
                mpParentSystem->getGUIModelObject(name)->setPos(newPt);
            }
            else if( redoevent == "ROTATEDOBJECT" )
            {
                QString name;
                name = readName(poststream);
                //! @todo This feels wierd, why rotate one time
                mpParentSystem->getGUIModelObject(name)->rotate(NOUNDO);
            }
        }
        mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
    }
    //! @todo why dont I know of my own UndoWidget (should maybe have a pointer to it directly)
    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpUndoWidget->refreshList();
}


//! @brief Register function for deleted objects
//! @param item Pointer to the component about to be deleted
void UndoStack::registerDeletedObject(GUIObject *item)
{
    //qDebug() << "registerDeletedObject()";
    QString str;
    QTextStream stream(&str);
    item->saveToTextStream(stream, "DELETEDOBJECT");
    this->insertPost(str);
}


//! @brief Register function for connectors
//! @param item Pointer to the connector about to be deleted
void UndoStack::registerDeletedConnector(GUIConnector *item)
{
    qDebug() << "Entering: registerDeletedConnector()";
    QString str;
    QTextStream stream(&str);
    item->saveToTextStream(stream, "DELETEDCONNECTOR");
    this->insertPost(str);
    qDebug() << "Leaving: registerDeletedConnector()";
}


//! @brief Register function for added objects
//! @param itemName Name of the added object
void UndoStack::registerAddedObject(GUIObject *item)
{
    QString str;
    QTextStream stream(&str);

    item->saveToTextStream(stream, "ADDEDOBJECT"); //! @todo We now save the parameters also (not necessary as they are dafult, (does not do any harm however)
    this->insertPost(str);
}


//! @brief Register function for added connectors
//! @param item Pointer to the added connector
void UndoStack::registerAddedConnector(GUIConnector *item)
{
    //qDebug() << "registerAddedConnector()";
    QString str;
    QTextStream stream(&str);
    item->saveToTextStream(stream, "ADDEDCONNECTOR");
    this->insertPost(str);
}


//! @brief Registser function for renaming an object
//! @param oldName Old object name
//! @param newName New object name
void UndoStack::registerRenameObject(QString oldName, QString newName)
{
    QString str;
    QTextStream stream(&str);
    stream << "RENAMEDOBJECT " << addQuotes(oldName) << " " << addQuotes(newName);
    this->insertPost(str);
}


//! @brief Register function for modifying a line in a connector
//! @param oldPos Position of the line before it was moved
//! @param item Pointer to the connector
//! @param lineNumber Number of the line that was moved
void UndoStack::registerModifiedConnector(QPointF oldPos, QPointF newPos, GUIConnector *item, int lineNumber)
{
    QString str;
    QTextStream stream(&str);

    stream << "MODIFIEDCONNECTOR" << " " << addQuotes(item->getStartComponentName()) << " " << addQuotes(item->getStartPortName())
                                  << " " << addQuotes(item->getEndComponentName())   << " " << addQuotes(item->getEndPortName())
                                  << " " << oldPos.x() << " " << oldPos.y() << " " << newPos.x() << " " << newPos.y() << " " << lineNumber;
    this->insertPost(str);

}


//! @brief Register function for moving an object
//! @param oldPos Position of the object before it was moved
//! @param objectName Name of the object
void UndoStack::registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName)
{
    QString str;
    QTextStream stream(&str);
    stream << "MOVEDOBJECT " <<  addQuotes(objectName) << " " << oldPos.x() << " " << oldPos.y() << " " << newPos.x() << " " << newPos.y();
    this->insertPost(str);

}


//! @brief Register function for rotating an object
//! @param item Pointer to the object
void UndoStack::registerRotatedObject(GUIObject *item)
{
    QString str;
    QTextStream stream(&str);
    stream << "ROTATEDOBJECT " << addQuotes(item->getName());
    this->insertPost(str);
}


//! @brief Register function for vertical flip of an object
//! @param item Pointer to the object
void UndoStack::registerVerticalFlip(GUIObject *item)
{
    QString str;
    QTextStream stream(&str);
    stream << "VERTICALFLIP " << item->getName();
    this->insertPost(str);
}


//! @brief Register function for horizontal flip of an object
//! @param item Pointer to the object
//! @todo Maybe we should combine this and registerVerticalFlip to one function?
void UndoStack::registerHorizontalFlip(GUIObject *item)
{
    QString str;
    QTextStream stream(&str);
    stream << "HORIZONTALFLIP " << item->getName();
    this->insertPost(str);
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
    mTempStack = mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mStack;
    QTableWidgetItem *item;

    mUndoTable->clear();
    mUndoTable->setRowCount(0);

    int x = 0;

    if(mTempStack.empty())
    {
        item = new QTableWidgetItem();
        //! @todo what the heck is this suposed to mean !ENUM, item->setFlags(!Qt::ItemIsEditable);
        item->setText("No undo history found.");
        item->setBackgroundColor(QColor("white"));
        mUndoTable->insertRow(x);
        mUndoTable->setItem(x,0,item);
        ++x;
        mUndoTable->verticalHeader()->hide();
        item->setTextAlignment(Qt::AlignCenter);
    }
    else if(mTempStack[0].empty())
    {
        item = new QTableWidgetItem();
        //! @todo What the heck does this todo mean?
        //! @todo what the heck is this suposed to mean !ENUM, item->setFlags(!Qt::ItemIsEditable);
        item->setText("No undo history found.");
        item->setBackgroundColor(QColor("white"));
        mUndoTable->insertRow(x);
        mUndoTable->setItem(x,0,item);
        ++x;
        mUndoTable->verticalHeader()->hide();
        item->setTextAlignment(Qt::AlignCenter);
    }
    else
    {
        mUndoTable->verticalHeader()->show();
    }

    for(int i = mTempStack.size()-1; i != -1; --i)
    {
        for(int j = mTempStack[i].size()-1; j != -1; --j)
        {
            item = new QTableWidgetItem();
            //! @todo what the heck is this suposed to mean !ENUM, item->setFlags(!Qt::ItemIsEditable);

                // Translate the command words into better looking explanations
            QTextStream stream(&mTempStack[i][j]);
            QString commandword;
            stream >> commandword;

            if (commandword == "DELETEDOBJECT")
            {
                item->setText("Deleted Object");
            }
            else if (commandword == "DELETEDCONNECTOR")
            {
                item->setText("Deleted Connector");
            }
            else if (commandword == "ADDEDOBJECT")
            {
                item->setText("Added Object");
            }
            else if (commandword == "ADDEDCONNECTOR")
            {
                item->setText("Added Connector");
            }
            else if (commandword == "RENAMEDOBJECT")
            {
                item->setText("Renamed Object");
            }
            else if (commandword == "MODIFIEDCONNECTOR")
            {
                item->setText("Modified Connector");
            }
            else if (commandword == "MOVEDOBJECT")
            {
                item->setText("Moved Object");
            }
            else if (commandword == "ROTATEDOBJECT")
            {
                item->setText("Rotated Object");
            }
            else if (commandword == "VERTICALFLIP")
            {
                item->setText("Flipped Vertical");
            }
            else if (commandword == "HORIZONTALFLIP")
            {
                item->setText("Flipped Horizontal");
            }
            else
            {
                item->setText(commandword);
            }

                // Figure out which color to use.
            if(i > mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
            {
                if (i%2 == 0)
                {
                    item->setBackgroundColor(QColor("white"));
                }
                else
                {
                    item->setBackgroundColor(QColor("antiquewhite"));
                }
            }
            else if(i < mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
            {
                if (i%2 == 0)
                {
                    item->setBackgroundColor(QColor("white"));
                    item->setTextColor(QColor("black"));
                }
                else
                {
                    item->setBackgroundColor(QColor("antiquewhite"));
                    item->setTextColor(QColor("black"));
                }
            }
            else
            {
                item->setBackgroundColor(QColor("lightgreen"));
                item->setTextColor(QColor("black"));
                QFont tempFont = item->font();
                tempFont.setBold(true);
                item->setFont(tempFont);
            }

                //Insert a new line in the table and display the action
            if(commandword != "PARAMETER")                              //Don't show parameter lines, because they "belong" to the object lines.
            {
                mUndoTable->insertRow(x);
                mUndoTable->setItem(x,0,item);
                ++x;
            }
        }
    }
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
