#include <QtGui>
//#include <QObject>
#include "UndoStack.h"
#include "GUIObject.h"
#include "HopsanCore.h"
#include <sstream>
#include <QDebug>
#include "AppearanceData.h"
#include "mainwindow.h"
#include "LibraryWidget.h"
#include <assert.h>

class AppearanceData;

//! Constructor.
UndoStack::UndoStack(GraphicsView *parentView)
{
    mpParentView = parentView;
    clear();
}


//! Destructor.
UndoStack::~UndoStack()
{
    //Not implemented yet
}


//! Clears all contents in the stack.
void UndoStack::clear()
{
    mCurrentStackPosition = -1;
    mStack.clear();
    //QList<QStringList> tempList;
    //mStack.append(tempList);            //Appends an empty list, so that there is something to write to.
    newPost();
}


//! Adds a new post to the stack
void UndoStack::newPost()
{
    int tempSize = mStack.size()-1;
    for(int i = mCurrentStackPosition; i != tempSize; ++i)
    {
        qDebug() << i;
        mStack.pop_back();
    }

    if(mCurrentStackPosition < 0)
    {
        QList<QStringList> tempList;
        mStack.append(tempList);
        mCurrentStackPosition = 0;
    }
    else if(!mStack[mCurrentStackPosition].empty())
    {
        QList<QStringList> tempList;
        mStack.append(tempList);
        ++mCurrentStackPosition;
    }
}


//! Will undo the changes registered in the last stack position, and switch stack pointer one step back
void UndoStack::undoOneStep()
{
    int undoPosition = mCurrentStackPosition;
    if(mCurrentStackPosition < 0)
    {
        undoPosition = -1;
    }
    else if(mStack[mCurrentStackPosition].empty() and mCurrentStackPosition != 0)
    {
        undoPosition = mCurrentStackPosition - 1;
    }

    if(undoPosition > -1)
    {
        for(int i = 0; i != mStack[undoPosition].size(); ++i)
        {
            if( mStack[undoPosition][i][0] == "DELETEDOBJECT" )
            {
                QString componentType = mStack[undoPosition][i][1];
                QString componentName = mStack[undoPosition][i][2];
                int posX = mStack[undoPosition][i][3].toInt();
                int posY = mStack[undoPosition][i][4].toInt();
                qreal rotation = mStack[undoPosition][i][5].toDouble();
                int nameTextPos = mStack[undoPosition][i][6].toInt();

                //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
                AppearanceData appearanceData = *mpParentView->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData(componentType);
                mpParentView->addGUIObject(componentType, appearanceData, QPoint(posX, posY), 0, componentName, false, true);
                mpParentView->getGUIObject(componentName)->setNameTextPos(nameTextPos);
                while(mpParentView->getGUIObject(componentName)->rotation() != rotation)
                {
                    mpParentView->getGUIObject(componentName)->rotate();
                }
            }
            else if ( mStack[undoPosition][i][0] == "DELETEDCONNECTOR" )
            {
                QString startComponentName = mStack[undoPosition][i][1];
                int startPortNumber = mStack[undoPosition][i][2].toInt();
                QString endComponentName = mStack[undoPosition][i][3];;
                int endPortNumber = mStack[undoPosition][i][4].toInt();

                GUIPort *startPort = mpParentView->getGUIObject(startComponentName)->getPort(startPortNumber);
                GUIPort *endPort = mpParentView->getGUIObject(endComponentName)->getPort(endPortNumber);

                QVector<QPointF> tempPointVector;
                qreal tempX, tempY;
                for(int j = 5; j != mStack[undoPosition][i].size(); ++j)
                {
                    tempX = mStack[undoPosition][i][j].toDouble();
                    tempY = mStack[undoPosition][i][j+1].toDouble();
                    tempPointVector.push_back(QPointF(tempX, tempY));
                    ++j;
                }

                //! @todo: Store useIso bool in model file and pick the correct line styles when loading
                GUIConnector *pTempConnector;

                QString type, style;
                if((startPort->mpCorePort->getNodeType() == "NodeHydraulic") | (startPort->mpCorePort->getNodeType() == "NodeMechanic"))
                    type = "Power";
                else if(startPort->mpCorePort->getNodeType() == "NodeSignal")
                    type = "Signal";
                if(mpParentView->mpParentProjectTab->useIsoGraphics)
                    style = "Iso";
                else
                    style = "User";
                pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, mpParentView->getPen("Primary", type, style),
                                                  mpParentView->getPen("Active", type, style), mpParentView->getPen("Hover", type, style), mpParentView);

                mpParentView->scene()->addItem(pTempConnector);

                startPort->hide();
                endPort->hide();

                GUIObject::connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMe()));
                GUIObject::connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMe()));

                mpParentView->mConnectorVector.append(pTempConnector);
                bool success = mpParentView->getModelPointer()->connect(startPort->mpCorePort, endPort->mpCorePort);
                if (!success)
                {
                    qDebug() << "Unsuccessful connection try" << endl;
                    assert(false);
                }
            }
            else if( mStack[undoPosition][i][0] == "ADDEDOBJECT" )
            {
                QString itemName = mStack[undoPosition][i][2];

                GUIObject* item = mpParentView->mGUIObjectMap.find(itemName).value();
                mpParentView->mGUIObjectMap.erase(mpParentView->mGUIObjectMap.find(itemName));
                item->deleteInHopsanCore();
                mpParentView->scene()->removeItem(item);
                delete(item);
                mpParentView->setBackgroundBrush(mpParentView->mBackgroundColor);
            }
            else if( mStack[undoPosition][i][0] == "ADDEDCONNECTOR" )
            {
                QString startComponentName = mStack[undoPosition][i][1];
                int startPortNumber = mStack[undoPosition][i][2].toInt();
                QString endComponentName = mStack[undoPosition][i][3];
                int endPortNumber = mStack[undoPosition][i][4].toInt();

                    // Lookup the pointer to the connector to remove from the connector vector
                GUIConnector *item;
                for(int i = 0; i != mpParentView->mConnectorVector.size(); ++i)
                {
                    if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == startComponentName) and
                       (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == startPortNumber) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == endComponentName) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == endPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                    else if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == endComponentName) and
                            (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == endPortNumber) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == startComponentName) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == startPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                }
                mpParentView->removeConnector(item, true);
                //mStack[undoPosition].pop_front();
            }
            else if( mStack[undoPosition][i][0] == "RENAMEDOBJECT" )
            {
                QString oldName = mStack[undoPosition][i][1];
                QString newName = mStack[undoPosition][i][2];

                GUIObject* obj_ptr = mpParentView->mGUIObjectMap.find(newName).value();
                mpParentView->mGUIObjectMap.erase(mpParentView->mGUIObjectMap.find(newName));
                obj_ptr->setName(oldName, true);
                mpParentView->mGUIObjectMap.insert(obj_ptr->getName(), obj_ptr);
            }
            else if( mStack[undoPosition][i][0] == "MODIFIEDCONNECTOR" )
            {
                qreal oldX =                    mStack[undoPosition][i][1].toDouble();
                qreal oldY =                    mStack[undoPosition][i][2].toDouble();
                qreal newX =                    mStack[undoPosition][i][3].toDouble();
                qreal newY =                    mStack[undoPosition][i][4].toDouble();
                QString startComponentName =    mStack[undoPosition][i][5];
                int startPortNumber =           mStack[undoPosition][i][6].toInt();
                QString endComponentName =      mStack[undoPosition][i][7];
                int endPortNumber =             mStack[undoPosition][i][8].toInt();
                int lineNumber =                mStack[undoPosition][i][9].toInt();

                    //Fetch the pointer to the connector
                GUIConnector *item;
                for(int i = 0; i != mpParentView->mConnectorVector.size(); ++i)
                {
                    if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == startComponentName) and
                       (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == startPortNumber) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == endComponentName) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == endPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                    else if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == endComponentName) and
                            (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == endPortNumber) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == startComponentName) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == startPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                }
                qreal dX = newX-oldX;
                qreal dY = newY-oldY;

                item->getLine(lineNumber)->setPos(QPointF(item->getLine(lineNumber)->pos().x()-dX, item->getLine(lineNumber)->pos().y()-dY));
                item->updateLine(lineNumber);
            }
            else if( mStack[undoPosition][i][0] == "MOVEDOBJECT" )
            {
                qreal oldX = mStack[undoPosition][i][1].toDouble();
                qreal oldY = mStack[undoPosition][i][2].toDouble();
                qreal newX = mStack[undoPosition][i][3].toDouble();
                qreal newY = mStack[undoPosition][i][4].toDouble();
                QString objectName = mStack[undoPosition][i][5];

                mpParentView->getGUIObject(objectName)->setPos(QPointF(oldX, oldY));
            }
        }
        mCurrentStackPosition = undoPosition - 1;
        mpParentView->setBackgroundBrush(mpParentView->mBackgroundColor);
    }
}


//! Will redo the previously undone changes if they exist, and re-add the undo command to the undo stack.
void UndoStack::redoOneStep()
{
    if( (mCurrentStackPosition != mStack.size()-1) and (!mStack[mCurrentStackPosition+1].empty()) )
    {
        ++mCurrentStackPosition;
        for(int i = 0; i != mStack[mCurrentStackPosition].size(); ++i)
        {
            if( mStack[mCurrentStackPosition][i][0] == "DELETEDOBJECT" )
            {
                QString componentName = mStack[mCurrentStackPosition][i][2];

                GUIObject* item = mpParentView->mGUIObjectMap.find(componentName).value();
                mpParentView->mGUIObjectMap.erase(mpParentView->mGUIObjectMap.find(componentName));
                item->deleteInHopsanCore();
                mpParentView->scene()->removeItem(item);
                delete(item);
                mpParentView->setBackgroundBrush(mpParentView->mBackgroundColor);
            }
            else if ( mStack[mCurrentStackPosition][i][0] == "DELETEDCONNECTOR" )
            {
                QString startComponentName = mStack[mCurrentStackPosition][i][1];
                int startPortNumber = mStack[mCurrentStackPosition][i][2].toInt();
                QString endComponentName = mStack[mCurrentStackPosition][i][3];
                int endPortNumber = mStack[mCurrentStackPosition][i][4].toInt();

                    // Lookup the pointer to the connector to remove from the connector vector
                GUIConnector *item;
                for(int i = 0; i != mpParentView->mConnectorVector.size(); ++i)
                {
                    if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == startComponentName) and
                       (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == startPortNumber) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == endComponentName) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == endPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                    else if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == endComponentName) and
                            (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == endPortNumber) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == startComponentName) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == startPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                }
                mpParentView->removeConnector(item, true);
                //mStack[undoPosition].pop_front();
            }
            else if( mStack[mCurrentStackPosition][i][0] == "ADDEDOBJECT" )
            {
                QString componentType = mStack[mCurrentStackPosition][i][1];
                QString componentName = mStack[mCurrentStackPosition][i][2];
                int posX = mStack[mCurrentStackPosition][i][3].toInt();
                int posY = mStack[mCurrentStackPosition][i][4].toInt();
                qreal rotation = mStack[mCurrentStackPosition][i][5].toDouble();
                int nameTextPos = mStack[mCurrentStackPosition][i][6].toInt();

                //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
                AppearanceData appearanceData = *mpParentView->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData(componentType);
                mpParentView->addGUIObject(componentType, appearanceData, QPoint(posX, posY), 0, componentName, false, true);
                mpParentView->getGUIObject(componentName)->setNameTextPos(nameTextPos);
                while(mpParentView->getGUIObject(componentName)->rotation() != rotation)
                {
                    mpParentView->getGUIObject(componentName)->rotate();
                }
            }
            else if( mStack[mCurrentStackPosition][i][0] == "ADDEDCONNECTOR" )
            {
                QString startComponentName = mStack[mCurrentStackPosition][i][1];
                int startPortNumber = mStack[mCurrentStackPosition][i][2].toInt();
                QString endComponentName = mStack[mCurrentStackPosition][i][3];;
                int endPortNumber = mStack[mCurrentStackPosition][i][4].toInt();
                GUIPort *startPort = mpParentView->getGUIObject(startComponentName)->getPort(startPortNumber);
                GUIPort *endPort = mpParentView->getGUIObject(endComponentName)->getPort(endPortNumber);

                QVector<QPointF> tempPointVector;
                qreal tempX, tempY;
                for(int j = 5; j != mStack[mCurrentStackPosition][i].size(); ++j)
                {
                    tempX = mStack[mCurrentStackPosition][i][j].toDouble();
                    tempY = mStack[mCurrentStackPosition][i][j+1].toDouble();
                    tempPointVector.push_back(QPointF(tempX, tempY));
                    ++j;
                }

                //! @todo: Store useIso bool in model file and pick the correct line styles when loading
                GUIConnector *pTempConnector;

                QString type, style;
                if((startPort->mpCorePort->getNodeType() == "NodeHydraulic") | (startPort->mpCorePort->getNodeType() == "NodeMechanic"))
                    type = "Power";
                else if(startPort->mpCorePort->getNodeType() == "NodeSignal")
                    type = "Signal";
                if(mpParentView->mpParentProjectTab->useIsoGraphics)
                    style = "Iso";
                else
                    style = "User";

                pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, mpParentView->getPen("Primary", type, style),
                                                  mpParentView->getPen("Active", type, style), mpParentView->getPen("Hover", type, style), mpParentView);

                mpParentView->scene()->addItem(pTempConnector);

                startPort->hide();
                endPort->hide();

                GUIObject::connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMe()));
                GUIObject::connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMe()));

                mpParentView->mConnectorVector.append(pTempConnector);
                bool success = mpParentView->getModelPointer()->connect(startPort->mpCorePort, endPort->mpCorePort);
                if (!success)
                {
                    qDebug() << "Unsuccessful connection try" << endl;
                    assert(false);
                }
            }
            else if( mStack[mCurrentStackPosition][i][0] == "RENAMEDOBJECT" )
            {
                QString oldName = mStack[mCurrentStackPosition][i][1];
                QString newName = mStack[mCurrentStackPosition][i][2];

                GUIObject* obj_ptr = mpParentView->mGUIObjectMap.find(oldName).value();
                mpParentView->mGUIObjectMap.erase(mpParentView->mGUIObjectMap.find(oldName));
                obj_ptr->setName(newName, true);
                mpParentView->mGUIObjectMap.insert(obj_ptr->getName(), obj_ptr);
            }
            else if( mStack[mCurrentStackPosition][i][0] == "MODIFIEDCONNECTOR" )
            {
                qreal oldX =                    mStack[mCurrentStackPosition][i][1].toDouble();
                qreal oldY =                    mStack[mCurrentStackPosition][i][2].toDouble();
                qreal newX =                    mStack[mCurrentStackPosition][i][3].toDouble();
                qreal newY =                    mStack[mCurrentStackPosition][i][4].toDouble();
                QString startComponentName =    mStack[mCurrentStackPosition][i][5];
                int startPortNumber =           mStack[mCurrentStackPosition][i][6].toInt();
                QString endComponentName =      mStack[mCurrentStackPosition][i][7];
                int endPortNumber =             mStack[mCurrentStackPosition][i][8].toInt();
                int lineNumber =                mStack[mCurrentStackPosition][i][9].toInt();

                    //Fetch the pointer to the connector
                GUIConnector *item;
                for(int i = 0; i != mpParentView->mConnectorVector.size(); ++i)
                {
                    if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == startComponentName) and
                       (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == startPortNumber) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == endComponentName) and
                       (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == endPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                    else if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == endComponentName) and
                            (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == endPortNumber) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == startComponentName) and
                            (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == startPortNumber))
                    {
                        item = mpParentView->mConnectorVector[i];
                    }
                }
                qreal dX = newX-oldX;
                qreal dY = newY-oldY;

                item->getLine(lineNumber)->setPos(QPointF(item->getLine(lineNumber)->pos().x()+dX, item->getLine(lineNumber)->pos().y()+dY));
                item->updateLine(lineNumber);
            }
            else if( mStack[mCurrentStackPosition][i][0] == "MOVEDOBJECT" )
            {
                qreal oldX = mStack[mCurrentStackPosition][i][1].toDouble();
                qreal oldY = mStack[mCurrentStackPosition][i][2].toDouble();
                qreal newX = mStack[mCurrentStackPosition][i][3].toDouble();
                qreal newY = mStack[mCurrentStackPosition][i][4].toDouble();
                QString objectName = mStack[mCurrentStackPosition][i][5];

                mpParentView->getGUIObject(objectName)->setPos(QPointF(newX, newY));
            }
        }
        mpParentView->setBackgroundBrush(mpParentView->mBackgroundColor);
    }

}


//! Register function for component
//! @param item is a pointer to the component about to be deleted.
void UndoStack::registerDeletedObject(GUIObject *item)
{

    QPointF pos = item->mapToScene(item->boundingRect().center());
    QStringList tempStringList;
    QString xPosString;
    QString yPosString;
    QString rotationString;
    QString nameTextPosString;
    xPosString.setNum(pos.x());
    yPosString.setNum(pos.y());
    rotationString.setNum(item->rotation());
    nameTextPosString.setNum(item->getNameTextPos());
    tempStringList << "DELETEDOBJECT" << item->getTypeName() << item->getName() << xPosString << yPosString << rotationString << nameTextPosString;
    mStack[mCurrentStackPosition].insert(0,tempStringList);

    //! @todo ugly quickhack for now dont save parameters for systemport or group
    //! @todo Group typename probably not correct
    //! @todo maybe the save functin should be part of every object (so it can write its own text)
    if ( (item->getTypeName() != "SystemPort") && (item->getTypeName() != "Group") )
    {
        Component *mpCoreComponent = item->getHopsanCoreComponentPtr();
        vector<CompParameter> paramVector = mpCoreComponent->getParameterVector();
        std::vector<CompParameter>::iterator itp;
        for ( itp=paramVector.begin() ; itp !=paramVector.end(); ++itp )
        {
            QString valueString;
            valueString.setNum(itp->getValue());
            tempStringList.clear();
            tempStringList << "PARAMETER" << item->getName() << QString(itp->getName().c_str()) << valueString;
            mStack[mCurrentStackPosition].insert(0,tempStringList);
        }
    }
}


//! Register function for connectors
//! @param item is a pointer to the connector about to be deleted.
void UndoStack::registerDeletedConnector(GUIConnector *item)
{
    QStringList tempStringList;
    QString startPortNumberString;
    QString endPortNumberString;
    startPortNumberString.setNum(item->getStartPort()->getPortNumber());
    endPortNumberString.setNum(item->getEndPort()->getPortNumber());
    tempStringList << "DELETEDCONNECTOR" << item->getStartPort()->getGuiObject()->getName() << startPortNumberString <<
                                   item->getEndPort()->getGuiObject()->getName() << endPortNumberString;
    QString xString, yString;
    for(size_t j = 0; j != item->getPointsVector().size(); ++j)
    {
        xString.setNum(item->getPointsVector()[j].x());
        yString.setNum(item->getPointsVector()[j].y());
        tempStringList << xString << yString;
    }
    mStack[mCurrentStackPosition].insert(0,tempStringList);
}


//! Register function for added objects.
//! @param itemName is the name of the added object
void UndoStack::registerAddedObject(GUIObject *item)
{
    QPointF pos = item->mapToScene(item->boundingRect().center());
    QStringList tempStringList;
    QString xPosString;
    QString yPosString;
    QString rotationString;
    QString nameTextPosString;
    xPosString.setNum(pos.x());
    yPosString.setNum(pos.y());
    rotationString.setNum(item->rotation());
    nameTextPosString.setNum(item->getNameTextPos());
    tempStringList << "ADDEDOBJECT" << item->getTypeName() << item->getName() << xPosString << yPosString << rotationString << nameTextPosString;
    mStack[mCurrentStackPosition].insert(0,tempStringList);
}


//! Register function for added connectors.
//! @param item is a pointer to the added connector.
void UndoStack::registerAddedConnector(GUIConnector *item)
{
    QStringList tempStringList;
    int i;
    for(i = 0; i != mpParentView->mConnectorVector.size(); ++i)
    {
        if(mpParentView->mConnectorVector[i] == item)
        {
            break;
        }
    }
    QString startPortNumberString;
    QString endPortNumberString;
    startPortNumberString.setNum(item->getStartPort()->getPortNumber());
    endPortNumberString.setNum(item->getEndPort()->getPortNumber());
    tempStringList << "ADDEDCONNECTOR" << item->getStartPort()->getGuiObject()->getName() << startPortNumberString <<
                                      item->getEndPort()->getGuiObject()->getName() << endPortNumberString;
    QString xString, yString;
    for(size_t j = 0; j != item->getPointsVector().size(); ++j)
    {
        xString.setNum(item->getPointsVector()[j].x());
        yString.setNum(item->getPointsVector()[j].y());
        tempStringList << xString << yString;
    }
    mStack[mCurrentStackPosition].insert(0,tempStringList);
}


//! Registser function for renaming an object.
//! @param oldName is a string with the old name.
//! @param newName is a string with the new name.
void UndoStack::registerRenameObject(QString oldName, QString newName)
{
    QStringList tempStringList;
    tempStringList << "RENAMEDOBJECT" << oldName << newName;
    mStack[mCurrentStackPosition].insert(0,tempStringList);
}


//! Register function for moving a line in a connector.
//! @param oldPos is the position before the line was moved.
//! @param item is a pointer to the connector.
//! @param lineNumber is the number of the line that was moved.
void UndoStack::registerModifiedConnector(QPointF oldPos, QPointF newPos, GUIConnector *item, int lineNumber)
{
    QString oldXString;
    QString oldYString;
    QString newXString;
    QString newYString;
    QString lineNumberString;
    QString startPortNumberString;
    QString endPortNumberString;
    oldXString.setNum(oldPos.x());
    oldYString.setNum(oldPos.y());
    newXString.setNum(newPos.x());
    newYString.setNum(newPos.y());

    lineNumberString.setNum(lineNumber);
    startPortNumberString.setNum(item->getStartPort()->getPortNumber());
    endPortNumberString.setNum(item->getEndPort()->getPortNumber());

    QStringList tempStringList;
    tempStringList << "MODIFIEDCONNECTOR" << oldXString << oldYString << newXString << newYString << item->getStartPort()->getGuiObject()->getName() << startPortNumberString <<
                      item->getEndPort()->getGuiObject()->getName() << endPortNumberString << lineNumberString;
    mStack[mCurrentStackPosition].insert(0,tempStringList);
}


//! Register function for moving an object.
//! @param oldPos is the position of the object before it was moved.
//! @param objectName is the name of the object.
void UndoStack::registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName)
{
    QString oldXString;
    QString oldYString;
    QString newXString;
    QString newYString;
    oldXString.setNum(oldPos.x());
    oldYString.setNum(oldPos.y());
    newXString.setNum(newPos.x());
    newYString.setNum(newPos.y());

    QStringList tempStringList;

    tempStringList << "MOVEDOBJECT" << oldXString << oldYString << newXString << newYString << objectName;
    mStack[mCurrentStackPosition].insert(0,tempStringList);
}
