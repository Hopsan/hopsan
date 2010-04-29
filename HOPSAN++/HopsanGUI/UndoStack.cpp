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

class AppearanceData;

//! Constructor
UndoStack::UndoStack(GraphicsView *parentView)
{
    mpParentView = parentView;
    mCurrentStackPosition = 0;
    mStack.append(QStringList());
}


//! Destructor
UndoStack::~UndoStack()
{
    //Not implemented yet
}


//! Clears all contents in the stack
void UndoStack::clear()
{
    //Not implemented yet (not sure if it is needed)
}


//! Adds a new post to the stack
void UndoStack::newPost()
{
    ++mCurrentStackPosition;
    mStack.append(QStringList());
}


//! Will undo the changes registered in the last stack position, and switch stack pointer one step back
void UndoStack::undoOneStep()
{
    if(!(mCurrentStackPosition == 0 and mStack[0].empty()))
    {
    qDebug() << "Undo!" << mStack[mCurrentStackPosition].size() << " undo steps found.";
    string undoWord;
    for(int i = 0; i != mStack[mCurrentStackPosition].size(); ++i)
    {
        qDebug() << "Executing step " << i << ".";
        stringstream undoStream(mStack[mCurrentStackPosition][i].toStdString().c_str());
        if ( undoStream >> undoWord )
        {
            qDebug() << "Reading: " << undoWord.c_str();
            if ( undoWord == "COMPONENT" )
            {
                string componentType;
                undoStream >> componentType;
                string componentName;
                undoStream >> componentName;
                string tempString;
                undoStream >> tempString;
                int posX = QString(tempString.c_str()).toInt();
                undoStream >> tempString;
                int posY = QString(tempString.c_str()).toInt();
                undoStream >> tempString;
                qreal rotation = QString(tempString.c_str()).toDouble();
                undoStream >> tempString;
                int nameTextPos = QString(tempString.c_str()).toInt();

                //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
                AppearanceData appearanceData = *mpParentView->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData(QString(componentType.c_str()));
                mpParentView->addGUIObject(QString(componentType.c_str()), appearanceData, QPoint(posX, posY), 0, QString(componentName.c_str()));
                mpParentView->getGUIObject(QString(componentName.c_str()))->setNameTextPos(nameTextPos);
                while(mpParentView->getGUIObject(QString(componentName.c_str()))->rotation() != rotation)
                {
                    mpParentView->getGUIObject(QString(componentName.c_str()))->rotate();
                }
            }
        }
    }
    mStack.pop_back();
    --mCurrentStackPosition;
    mpParentView->setBackgroundBrush(mpParentView->mBackgroundColor);
    }
}


//! Will redo the previously undone changes if they exist, and switch stack pointer one step forward
void UndoStack::redoOneStep()
{
    //Not implemented yet
}


//! Store function for component
void UndoStack::store(GUIObject *item)
{
    newPost();
    QPointF pos = item->mapToScene(item->boundingRect().center());
    std::stringstream tempStringStream;
    //std::string tempString;
    tempStringStream << "COMPONENT " << item->getTypeName().toStdString() << " " << item->getName().toStdString()
            << " " << pos.x() << " " << pos.y() << " " << item->rotation() << " " << item->getNameTextPos();
    qDebug() << "Saving: " << tempStringStream.str().c_str();
    mStack[mCurrentStackPosition].append(QString(tempStringStream.str().c_str()));

    Component *mpCoreComponent = item->getHopsanCoreComponentPtr();
    vector<CompParameter> paramVector = mpCoreComponent->getParameterVector();
    std::vector<CompParameter>::iterator itp;
    for ( itp=paramVector.begin() ; itp !=paramVector.end(); ++itp )
    {
        tempStringStream.str("");
        tempStringStream << "PARAMETER " << item->getName().toStdString() << " " << itp->getName().c_str() << " " << itp->getValue() << "\n";
        qDebug() << "Saving: " << tempStringStream.str().c_str();
        mStack[mCurrentStackPosition].append(QString(tempStringStream.str().c_str()));
    }
}


//! Store function for connectors
void UndoStack::store(GUIConnector *item)
{
    //Not implemented yet
}
