/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$

#include <QVector>
#include <QtGui> //!< @todo maybe only need qtfile dialog

#include "GUIObject.h"

#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "ParameterDialog.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "GUIUtilities.h"
#include "UndoStack.h"
#include "MessageWidget.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "LibraryWidget.h"
#include "loadObjects.h"

using namespace std;

double dist(double x1,double y1, double x2, double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}

GUIObject::GUIObject(QPoint position, AppearanceData appearanceData, GraphicsScene *scene, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    //remeber the scene ptr
    //! @todo is this really necessary as the object might know th scen (after adding ourrselves)
    mpParentGraphicsScene = scene;

    //Make a local copy of the appearance data (that can safely be modified if needed)
    mAppearanceData = appearanceData;

    mpParentGraphicsScene->addItem(this);
    mpParentGraphicsView = mpParentGraphicsScene->mpParentProjectTab->mpGraphicsView;

    mTextOffset = 5.0;

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    //setFocusPolicy(Qt::StrongFocus);
    this->setAcceptHoverEvents(true);

    this->setZValue(10);

    //Set to null ptr initially
    mpIcon = 0;
    mpSelectionBox = 0;
    mpNameText = 0;
    mIconType = false;
//    setIcon(false); //Use user icon initially

//    setGeometry(0,0,mpIcon->boundingRect().width(),mpIcon->boundingRect().height());
//    mpSelectionBox = new GUIObjectSelectionBox(0,0,mpIcon->boundingRect().width(),mpIcon->boundingRect().height(),
//                                                  QPen(QColor("red"),2*1.6180339887499), QPen(QColor("darkRed"),2*1.6180339887499),this);
//    mpSelectionBox->setVisible(false);
    this->refreshAppearance();

    setPos(position.x()-mpIcon->boundingRect().width()/2,position.y()-mpIcon->boundingRect().height()/2);
    mIsFlipped = false;

    mpNameText = new GUIObjectDisplayName(this);
    mNameTextPos = 0;
    this->setNameTextPos(mNameTextPos);

    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(fixTextPosition(QPointF)));
    connect(mpParentGraphicsView,SIGNAL(zoomChange()),this,SLOT(adjustTextPositionToZoom()));
    //connect(this->mpParentGraphicsView,SIGNAL(keyPressDelete()),this,SLOT(deleteComponent()));



    //std::cout << "GUIObject: " << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
}


GUIObject::~GUIObject()
{
    //! @todo This will lead to crash when closing program since undo stack may not exist. Fix it.
    //delete widget;
    emit componentDeleted();
}


int GUIObject::type() const
{
    return Type;
}


void GUIObject::fixTextPosition(QPointF pos)
{
    double x1,x2,y1,y2;

    if(!mIsFlipped)
    {
        if(this->rotation() == 0)
        {
            x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
            y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
            x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
            y2 = mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
        }
        else if(this->rotation() == 180)
        {
            x1 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
            y1 = mpIcon->boundingRect().height() + mpNameText->boundingRect().height() + mTextOffset;
            x2 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
            y2 = -mTextOffset;
        }
        else if(this->rotation() == 90)
        {
            x1 = -mpNameText->boundingRect().height() - mTextOffset;
            y1 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
            x2 = mpIcon->boundingRect().width() + mTextOffset;
            y2 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
        }
        else if(this->rotation() == 270)
        {
            x1 = mpIcon->boundingRect().width() + mpNameText->boundingRect().height() + mTextOffset;
            y1 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
            x2 = -mTextOffset;
            y2 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
        }
    }
    else
    {
        if(this->rotation() == 0)
        {
            x1 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
            y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
            x2 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
            y2 = mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
        }
        else if(this->rotation() == 180)
        {
            x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
            y1 = mpIcon->boundingRect().height() + mpNameText->boundingRect().height() + mTextOffset;
            x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
            y2 = -mTextOffset;
        }
        else if(this->rotation() == 90)
        {
            x1 = -mpNameText->boundingRect().height() - mTextOffset;
            y1 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
            x2 = mpIcon->boundingRect().width() + mTextOffset;
            y2 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
        }
        else if(this->rotation() == 270)
        {
            x1 = mpIcon->boundingRect().width() + mpNameText->boundingRect().height() + mTextOffset;
            y1 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
            x2 = -mTextOffset;
            y2 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
        }
    }

    double x = mpNameText->mapToParent(pos).x();
    double y = mpNameText->mapToParent(pos).y();

    if (dist(x,y, x1,y1) > dist(x,y, x2,y2))
    {
        mpNameText->setPos(x2,y2);
        mNameTextPos = 0;
    }
    else
    {
        mpNameText->setPos(x1,y1);
        mNameTextPos = 1;
    }

    this->mpParentGraphicsView->resetBackgroundBrush();
}


void GUIObject::addConnector(GUIConnector *item)
{
    mpGUIConnectorPtrs.append(item);
    connect(this, SIGNAL(componentMoved()), item, SLOT(drawConnector()));
}


void GUIObject::removeConnector(GUIConnector *item)
{
    mpGUIConnectorPtrs.removeOne(item);
    disconnect(this, SIGNAL(componentMoved()), item, SLOT(drawConnector()));
}


QList<GUIConnector*> GUIObject::getGUIConnectorPtrs()
{
    return mpGUIConnectorPtrs;
}


//! This function refreshes the displayed name (HopsanCore may have changed it)
void GUIObject::refreshDisplayName()
{
    if (mpNameText != 0)
    {
        mpNameText->setPlainText(mAppearanceData.getName());
        //Adjust the position of the text
        this->fixTextPosition(this->mpNameText->pos());
    }
}


//! This function returns the current component name
QString GUIObject::getName()
{
    return mAppearanceData.getName();
}


QList<GUIPort*> GUIObject::getPortListPtrs()
{
    return mPortListPtrs;
}

void GUIObject::deleteInHopsanCore()
{
    cout << "Virtual dummy function" << endl;
    assert(false);
}

void GUIObject::setName(QString newName, bool doOnlyCoreRename)
{
    mpParentGraphicsView->mGUIObjectMap.erase(mpParentGraphicsView->mGUIObjectMap.find(this->getName()));
    mAppearanceData.setName(newName);
    refreshDisplayName();
    mpParentGraphicsView->mGUIObjectMap.insert(this->getName(), this);
}


void GUIObject::setIcon(bool useIso)
{
    QGraphicsSvgItem *tmp = mpIcon;
    if(useIso and mAppearanceData.haveIsoIcon())
    {
        mpIcon = new QGraphicsSvgItem(mAppearanceData.getFullIconPath(true) , this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mIconType = useIso;
        //qDebug() << "Setting iconpath to " << mAppearanceData.getFullIconPath(true);
    }
    else
    {
        mpIcon = new QGraphicsSvgItem(mAppearanceData.getFullIconPath(false), this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mIconType = !useIso;
        //qDebug() << "Setting iconpath to " << mAppearanceData.getFullIconPath(false);
    }

    //Delete old icon if it exist;
    if (tmp != 0)
    {
        delete(tmp);
    }

    if(mAppearanceData.getIconRotationBehaviour() == "ON")
        this->mIconRotation = true;
    else
        this->mIconRotation = false;

    if(!this->mIconRotation)
    {
        this->mpIcon->setRotation(-this->rotation());
        if(this->rotation() == 0)
        {
            this->mpIcon->setPos(0,0);
        }
        else if(this->rotation() == 90)
        {
            this->mpIcon->setPos(0,this->boundingRect().height());
        }
        else if(this->rotation() == 180)
        {
            this->mpIcon->setPos(this->boundingRect().width(),this->boundingRect().height());
        }
        else if(this->rotation() == 270)
        {
            this->mpIcon->setPos(this->boundingRect().width(),0);
        }
    }

}


//! Returns the port with the specified name.
GUIPort *GUIObject::getPort(QString name)
{
    //! @todo use the a guiport map instead
    for (int i=0; i<mPortListPtrs.size(); ++i)
    {
        if (mPortListPtrs[i]->getName() == name)
        {
            return mPortListPtrs[i];
        }
    }

    return 0;
}

double GUIObject::getParameterValue(QString name)
{
    cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
}

QVector<QString> GUIObject::getParameterNames()
{
    cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
}

void GUIObject::setParameterValue(QString name, double value)
{
    cout << "This function should only be available in GUIComponent and  GUISubsystem" << endl;
    assert(false);
}


//! @brief Save GuiObject to a text stream
void GUIObject::saveToTextStream(QTextStream &rStream, QString prepend)
{
    QPointF pos = mapToScene(boundingRect().center());
    if (!prepend.isEmpty())
    {
        rStream << prepend << " ";
    }
    rStream << addQuotes(getTypeName()) << " " << addQuotes(getName()) << " "
            << pos.x() << " " << pos.y() << " " << rotation() << " " << getNameTextPos() << " " << mpNameText->isVisible() << "\n";
}


//! Event when mouse cursor enters component icon.
void GUIObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setHovered();
        //this->mpSelectionBox->setVisible(true);
    }
    this->showPorts(true);
}


//! Event when mouse cursor leaves component icon.
void GUIObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setPassive();
    }
    this->showPorts(false);
}


//! Defines what shall happen if a mouse key is pressed while hovering an object.
void GUIObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
        //Store old positions for all components, in case more than one is selected
    if(event->button() == Qt::LeftButton)
    {
        QMap<QString, GUIObject *>::iterator it;
        for(it = mpParentGraphicsView->mGUIObjectMap.begin(); it != mpParentGraphicsView->mGUIObjectMap.end(); ++it)
        {
            it.value()->mOldPos = it.value()->pos();
            qDebug() << it.key();
        }
    }
    QGraphicsWidget::mousePressEvent(event);

        //Objects shall not be selectable while creating a connector
    if(this->mpParentGraphicsView->mIsCreatingConnector)
    {
        this->setSelected(false);
        this->setActive(false);
    }
}


//! Defines what shall happen if a mouse key is released while hovering an object.
void GUIObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QMap<QString, GUIObject *>::iterator it;
    bool alreadyClearedRedo = false;
    for(it = mpParentGraphicsView->mGUIObjectMap.begin(); it != mpParentGraphicsView->mGUIObjectMap.end(); ++it)
    {
        if((it.value()->mOldPos != it.value()->pos()) and (event->button() == Qt::LeftButton))
        {
            if(!alreadyClearedRedo)
            {
                mpParentGraphicsView->undoStack->newPost();
                mpParentGraphicsView->mpParentProjectTab->hasChanged();
                alreadyClearedRedo = true;
            }
            mpParentGraphicsView->undoStack->registerMovedObject(it.value()->mOldPos, it.value()->pos(), it.value()->getName());
        }
    }

    QGraphicsWidget::mouseReleaseEvent(event);

        //Objects shall not be selectable while creating a connector
    if(this->mpParentGraphicsView->mIsCreatingConnector)
    {
        this->setSelected(false);
        this->setActive(false);
    }
}


//! Handles item change events.
QVariant GUIObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QGraphicsWidget::itemChange(change, value);

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (this->isSelected())
        {
            this->mpSelectionBox->setActive();
            connect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            emit componentSelected();
        }
        else
        {
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            this->mpSelectionBox->setPassive();
        }
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit componentMoved();
    }
    return value;
}


//! Shows or hides the port, depending on the input boolean and whether or not they are connected.
void GUIObject::showPorts(bool visible)
{
    QList<GUIPort*>::iterator i;
    if(visible)
    {
        for (i = mPortListPtrs.begin(); i != mPortListPtrs.end(); ++i)
        {
            (*i)->show();
        }
    }
    else
        for (i = mPortListPtrs.begin(); i != mPortListPtrs.end(); ++i)
        {
            if ((*i)->isConnected or mpParentGraphicsView->mPortsHidden)
            {
                (*i)->hide();
            }
        }
}


//! Figures out the number of a component port by using a pointer to the port.
//! @see getPort(int number)
int GUIObject::getPortNumber(GUIPort *port)
{
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(port == mPortListPtrs.value(i))
        {
            return i;
        }
    }
    qDebug() << "Request for port number of non-existing port.";
    assert(false);      /// @todo: Trough exception
}


//! Rotates a component 90 degrees clockwise, and tells the connectors that the component has moved.
void GUIObject::rotate(bool doNotRegisterUndo)
{
    this->setTransformOriginPoint(this->mpIcon->boundingRect().center());
    this->setRotation(this->rotation()+90);

    if (this->rotation() == 360)
    {
        this->setRotation(0);
    }

    int tempNameTextPos = mNameTextPos;
    this->mpNameText->rotate(-90);
    this->fixTextPosition(this->mpNameText->pos());
    setNameTextPos(tempNameTextPos);

    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(mPortListPtrs.value(i)->getPortDirection() == PortAppearance::VERTICAL)
            mPortListPtrs.value(i)->setPortDirection(PortAppearance::HORIZONTAL);
        else
            mPortListPtrs.value(i)->setPortDirection(PortAppearance::VERTICAL);
        if (mPortListPtrs.value(i)->getPortType() == "POWERPORT")
        {
            if(this->rotation() == 0 and !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(0);
            else if(this->rotation() == 0 and mIsFlipped)
                mPortListPtrs.value(i)->setRotation(180);
            else if(this->rotation() == 90 and !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(270);
            else if(this->rotation() == 90 and mIsFlipped)
                mPortListPtrs.value(i)->setRotation(90);
            else if(this->rotation() == 180 and !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(180);
            else if(this->rotation() == 180 and mIsFlipped)
                mPortListPtrs.value(i)->setRotation(0);
            else if(this->rotation() == 270 and !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(90);
            else if(this->rotation() == 270 and mIsFlipped)
                mPortListPtrs.value(i)->setRotation(270);
        }
        //mPortListPtrs[i]->updatePosition();
    }

    if(!this->mIconRotation)
    {
        this->mpIcon->setRotation(-this->rotation());
        if(this->rotation() == 0)
        {
            this->mpIcon->setPos(0,0);
        }
        else if(this->rotation() == 90)
        {
            this->mpIcon->setPos(0,this->boundingRect().height());
        }
        else if(this->rotation() == 180)
        {
            this->mpIcon->setPos(this->boundingRect().width(),this->boundingRect().height());
        }
        else if(this->rotation() == 270)
        {
            this->mpIcon->setPos(this->boundingRect().width(),0);
        }

        //this->mpIcon->setPos(this->boundingRect().center());
    }

    if(!doNotRegisterUndo)
    {
        mpParentGraphicsView->undoStack->registerRotatedObject(this);
    }

    emit componentMoved();
}


//! Slot that moves component one pixel upwards
//! @see moveDown()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveUp()
{
    //qDebug() << "Move up!";
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()-1);
    mpParentGraphicsView->setBackgroundBrush(mpParentGraphicsView->mBackgroundColor);
}


//! Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveDown()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()+1);
    mpParentGraphicsView->setBackgroundBrush(mpParentGraphicsView->mBackgroundColor);
}


//! Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void GUIObject::moveLeft()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()-1, this->pos().y());
    mpParentGraphicsView->setBackgroundBrush(mpParentGraphicsView->mBackgroundColor);
}


//! Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void GUIObject::moveRight()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()+1, this->pos().y());
    mpParentGraphicsView->setBackgroundBrush(mpParentGraphicsView->mBackgroundColor);
}


//! @todo Fix name text position when flipping components

//! Slot that flips the object vertically.
//! @see flipHorizontal()
void GUIObject::flipVertical(bool doNotRegisterUndo)
{
    this->rotate(true);
    this->rotate(true);
    this->flipHorizontal(true);
    if(!doNotRegisterUndo)
    {
        mpParentGraphicsView->undoStack->registerVerticalFlip(this);

    }
}


//! Slot that flips the object horizontally.
//! @see flipVertical()
void GUIObject::flipHorizontal(bool doNotRegisterUndo)
{
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        //if(mPortListPtrs[i]->getPortTypeEnum() == Port::READPORT or mPortListPtrs[i]->getPortTypeEnum() == Port::WRITEPORT)
        if(mPortListPtrs[i]->getPortType() == "READPORT" or mPortListPtrs[i]->getPortType() == "WRITEPORT")
        {
            if(this->rotation() == 90 or this->rotation() == 270)
            {
                mPortListPtrs.value(i)->scale(1,-1);
                mPortListPtrs.value(i)->translate(0, -mPortListPtrs.value(i)->boundingRect().width());
            }
            else
            {
                mPortListPtrs.value(i)->scale(-1,1);
                mPortListPtrs.value(i)->translate(-mPortListPtrs.value(i)->boundingRect().width(), 0);
            }
        }
    }

    //Flip the entire widget
    this->scale(-1, 1);
    if(mIsFlipped)
    {
        this->moveBy(-this->boundingRect().width(),0);
        mIsFlipped = false;
    }
    else
    {
        this->moveBy(this->boundingRect().width(),0);
        mIsFlipped = true;
    }

    this->fixTextPosition(this->mpNameText->pos());

        //"Un-flip" the ports
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(this->rotation() == 90 or this->rotation() == 270)
        {
            mPortListPtrs.value(i)->scale(1,-1);
            mPortListPtrs.value(i)->translate(0, -mPortListPtrs.value(i)->boundingRect().width());
        }
        else
        {
            mPortListPtrs.value(i)->scale(-1,1);
            mPortListPtrs.value(i)->translate(-mPortListPtrs.value(i)->boundingRect().width(), 0);
        }
    }
    if(!doNotRegisterUndo)
    {
        mpParentGraphicsView->undoStack->registerHorizontalFlip(this);
    }
}


//! Returns an integer that describes the position of the component name text.
//! @see setNameTextPos(int textPos)
//! @see fixTextPosition(QPointF pos)
int GUIObject::getNameTextPos()
{
    return mNameTextPos;
}\


//! Updates the name text position, and moves the text to the correct position.
//! @see getNameTextPos()
//! @see fixTextPosition(QPointF pos)
void GUIObject::setNameTextPos(int textPos)
{
    mNameTextPos = textPos;

    double x1,x2,y1,y2;

    if(this->rotation() == 0)
    {
        x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
        x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y2 = mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
    }
    else if(this->rotation() == 180)
    {
        x1 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y1 = mpIcon->boundingRect().height() + mpNameText->boundingRect().height() + mTextOffset;
        x2 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y2 = -mTextOffset;
    }
    else if(this->rotation() == 90)
    {
        x1 = -mpNameText->boundingRect().height() - mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
        x2 = mpIcon->boundingRect().width() + mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
    }
    else if(this->rotation() == 270)
    {
        x1 = mpIcon->boundingRect().width() + mpNameText->boundingRect().height() + mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
        x2 = -mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
    }

    switch(textPos)
    {
    case 0:
        mpNameText->setPos(x2,y2);
        break;
    case 1:
        mpNameText->setPos(x1,y1);
        break;
    }
}

void GUIObject::hideName()
{
    this->mpNameText->setVisible(false);
}

void GUIObject::showName()
{
    this->mpNameText->setVisible(true);
}


//! Dummy
QString GUIObject::getTypeName()
{
    assert(false);
}

AppearanceData* GUIObject::getAppearanceData()
{
    return &mAppearanceData;
}


void GUIObject::refreshAppearance()
{
    bool hasActiveSelectionBox = false;
    if (mpSelectionBox != 0)
    {
        hasActiveSelectionBox = mpSelectionBox->isVisible(); //!< @todo This is a bit strange need to fix see todo bellow
        delete mpSelectionBox;
    }

    setIcon(mIconType);
    qDebug() << pos();
    setGeometry(pos().x(), pos().y(), mpIcon->boundingRect().width(), mpIcon->boundingRect().height());
    qDebug() << pos();

    //! @todo problem with hovered or active or passive selection box, should maybe make it possible to resize rather than to create a new selection box on refresh
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpIcon->boundingRect().width(), mpIcon->boundingRect().height(),
                                                  QPen(QColor("red"),2*1.6180339887499), QPen(QColor("darkRed"),2*1.6180339887499), this);
    if (hasActiveSelectionBox)
    {
        mpSelectionBox->setActive();
    }

    this->refreshDisplayName();
}


void GUIObject::adjustTextPositionToZoom()
{
    this->fixTextPosition(mpNameText->pos());
}

GUIObjectDisplayName::GUIObjectDisplayName(GUIObject *pParent)
    :   QGraphicsTextItem(pParent)
{
    mpParentGUIComponent = pParent;
    setTextInteractionFlags(Qt::TextEditable | Qt::TextSelectableByMouse);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
}

void GUIObjectDisplayName::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit textMoved(event->pos());
    QGraphicsTextItem::mouseReleaseEvent(event);
}

void GUIObjectDisplayName::focusInEvent(QFocusEvent *event)
{
    mpParentGUIComponent->mpParentGraphicsView->mIsRenamingObject = true;
    QGraphicsTextItem::focusInEvent(event);
}

void GUIObjectDisplayName::focusOutEvent(QFocusEvent *event)
{
    mpParentGUIComponent->mpParentGraphicsView->mIsRenamingObject = false;
        //Try to set the new name, the rename function in parent view will be called
    mpParentGUIComponent->setName(toPlainText());
        //Refresh the display name (it may be different from the one you wanted)
    mpParentGUIComponent->refreshDisplayName();
    emit textMoved(pos());
    QGraphicsTextItem::focusOutEvent(event);
}




//! Constructor.
//! @param x1 is the x-coordinate of the top left corner of the parent component.
//! @param y1 is the y-coordinate of the top left corner of the parent component.
//! @param x2 is the x-coordinate of the bottom right corner of the parent component.
//! @param y2 is the y-coordinate of the bottom right corner of the parent component.
//! @param activePen defines the width and color of the box when the parent component is selected.
//! @param hoverPen defines the width and color of the box when the parent component is hovered by the mouse cursor.
//! @param *parent defines the parent object.
GUIObjectSelectionBox::GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent)
        : QGraphicsItemGroup(parent)
{
    mpParentGUIObject = parent;
    qreal b = 5;
    qreal a = 5;
    x1 = x1-3;
    y1 = y1-3;
    x2 = x2+3;
    y2 = y2+3;

    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    this->setPassive();

    QGraphicsLineItem *tempLine = new QGraphicsLineItem(x1,y1+b,x1,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y1,x1+a,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2-a,y1,x2,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y1,x2,y1+b,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1+a,y2,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y2-b,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2-b,x2,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2,x2-a,y2,this);
    mLines.push_back(tempLine);
}


//! Tells the box to become visible and use active style.
//! @see setPassive();
//! @see setHovered();
void GUIObjectSelectionBox::setActive()
{

    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mActivePen);
    }
}


//! Tells the box to become invisible.
//! @see setActive();
//! @see setHovered();
void GUIObjectSelectionBox::setPassive()
{
    this->setVisible(false);
}


//! Tells the box to become visible and use hovered style.
//! @see setActive();
//! @see setPassive();
void GUIObjectSelectionBox::setHovered()
{
    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mHoverPen);
    }
}

//! Tells the component to ask its parent to delete it.
void GUIObject::deleteMe()
{
    mpParentGraphicsView->deleteGUIObject(this->getName());
}


GUIComponent::GUIComponent(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
    : GUIObject(position, appearanceData, scene, parent)
{
    //Create the object in core, and get its default core name
    QString corename = this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.createComponent(mAppearanceData.getTypeName());

    if ( this->getName().isEmpty() )
    {
        //If the displayname has not been decided then use the name from core
        mAppearanceData.setName(corename);
    }
    else
    {
        //Lets rename the core object to the gui name that is set in the txt description file, we take the name theat this function returns
        mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.rename(corename, getName())); //Cant use setName here as that would call an aditional rename (of someone else)
    }

    //Sets the ports
    createPorts();

    refreshDisplayName(); //Make sure name window is correct size for center positioning

    std::cout << "GUIcomponent: " << mComponentTypeName.toStdString() << std::endl;
}



//!
//! @brief This function sets the desired component name
//! @param [in] newName The new name
//! @param [in] doOnlyCoreRename  Dont use this if you dont know what you are doing
//!
//! The desired new name will be sent to the the core component and may be modified. Rename will be called in the graphics view to make sure that the guicomponent map key value is up to date.
//! doOnlyCoreRename is a somewhat ugly hack, we need to be able to force setName without calling rename in some very special situations, it defaults to false
//!
void GUIComponent::setName(QString newName, bool doOnlyCoreRename)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        if (doOnlyCoreRename)
        {
            mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.rename(this->getName(), newName));
            refreshDisplayName();
        }
        else
        {
            //Rename
            mpParentGraphicsView->renameGUIObject(oldName, newName);
        }
    }
}


//! Event when double clicking on component icon.
void GUIComponent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    std::cout << "GUIComponent.cpp: " << "mouseDoubleClickEvent " << std::endl;

    openParameterDialog();

}


//! Returns a string with the component type.
QString GUIComponent::getTypeName()
{
    return mAppearanceData.getTypeName();
}

QString GUIComponent::getTypeCQS()
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getTypeCQS(this->getName());
}

//! @brief Get a vector with the names of the available parameters
QVector<QString> GUIComponent::getParameterNames()
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterNames(this->getName());
}

QString GUIComponent::getParameterUnit(QString name)
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterUnit(this->getName(), name);
}

QString GUIComponent::getParameterDescription(QString name)
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterDescription(this->getName(), name);
}

double GUIComponent::getParameterValue(QString name)
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterValue(this->getName(), name);
}

//! @brief Set a parameter value, wrapps hopsan core
void GUIComponent::setParameterValue(QString name, double value)
{
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.setParameter(this->getName(), name, value);
}

void GUIComponent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction;
        if (!mpParentGraphicsScene->selectedItems().empty())
            groupAction = menu.addAction(tr("Group components"));

        QAction *parameterAction = menu.addAction(tr("Change parameters"));
        //menu.insertSeparator(parameterAction);

        QAction *showNameAction = menu.addAction(tr("Show name"));
        showNameAction->setCheckable(true);
        showNameAction->setChecked(this->mpNameText->isVisible());

        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == parameterAction)
        {
            openParameterDialog();
        }
        else if (selectedAction == groupAction)
        {
            //groupComponents(mpParentGraphicsScene->selectedItems());
            AppearanceData appdata;
            appdata.setIconPathUser("subsystemtmp.svg");
            appdata.setBasePath("../../HopsanGUI/"); //!< @todo This is EXTREAMLY BAD
            GUIGroup *pGroup = new GUIGroup(mpParentGraphicsScene->selectedItems(), appdata, mpParentGraphicsScene);
            this->mpParentGraphicsScene->addItem(pGroup);
        }
        else if (selectedAction == showNameAction)
        {
            if(this->mpNameText->isVisible())
            {
                this->hideName();
            }
            else
            {
                this->showName();
            }
        }
}


void GUIComponent::openParameterDialog()
{
    ParameterDialog *dialog = new ParameterDialog(this,mpParentGraphicsView);
    dialog->exec();
}

//! @brief Help function to create ports in the component when it is created
void GUIComponent::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (not really necessary in guicomponents)
    QString cqsType = mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getTypeCQS(getName());
    PortAppearanceMapT::iterator i;
    for (i = mAppearanceData.getPortAppearanceMap().begin(); i != mAppearanceData.getPortAppearanceMap().end(); ++i)
    {
        QString nodeType = this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getNodeType(this->getName(), i.key());
        QString portType = this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getPortType(this->getName(), i.key());
        i.value().selectPortIcon(cqsType, portType, nodeType);

        qreal x = i.value().x;
        qreal y = i.value().y;

        GUIPort *pNewPort = new GUIPort(i.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this);
        mPortListPtrs.append(pNewPort);
    }
}


void GUIComponent::deleteInHopsanCore()
{
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.removeSubComponent(this->getName(), true);
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
    GUIObject::saveToTextStream(rStream, prepend);

    QVector<QString> parameterNames = mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterNames(this->getName());
    QVector<QString>::iterator pit;
    for(pit = parameterNames.begin(); pit != parameterNames.end(); ++pit)
    {
        //! @todo It is a bit strange that we can not control the parameter keyword, but then agian spliting this into a separate function with its own prepend variable would also be wierd
        rStream << "PARAMETER " << addQuotes(getName()) << " " << (*pit) << " " <<
                mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterValue(this->getName(), (*pit)) << "\n";
    }
}


GUISubsystem::GUISubsystem(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
        : GUIObject(position, appearanceData, scene, parent)
{
    //Set default values
    mLoadType = "Empty";
    mModelFilePath = "";

    QString corename = mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.createSubSystem();
    if ( getName().isEmpty() )
    {
        //If the displayname has not been decided then use the name from core
        mAppearanceData.setName(corename);
    }
    else
    {
        //Lets rename the core object to the gui name that is set in the txt description file, we take the name that this function returns
        mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.rename(corename, getName())); //Cant use setName here as thewould call an aditional rename (of someone else)
    }

    refreshDisplayName(); //Make sure name window is correct size for center positioning

    //! @todo Write some code here!

//    std::cout << "GUISubsystem: " << mComponentTypeName.toStdString() << std::endl;
}


//!
//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
//! @param [in] doOnlyCoreRename  Dont use this if you dont know what you are doing
//!
//! @todo This function is almost exactly identical to the one for GUIcomponents need to make sure that we dont dublicate functions like this, maybe this should be directly in GUIObject
//!
//! The desired new name will be sent to the the core component and may be modified. Rename will be called in the graphics view to make sure that the guicomponent map key value is up to date.
//! doOnlyCoreRename is a somewhat ugly hack, we need to be able to force setName without calling rename in some very special situations, it defaults to false
//!
void GUISubsystem::setName(QString newName, bool doOnlyCoreRename)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        if (doOnlyCoreRename)
        {
            mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.setSystemName(oldName, newName));
            refreshDisplayName();
        }
        else
        {
            //Rename
            mpParentGraphicsView->renameGUIObject(oldName, newName);
        }
    }
}


//! Returns a string with the sub system type.
QString GUISubsystem::getTypeName()
{
    //! @todo is this OK should really ask the subsystem but result should be subsystem i think
    return "Subsystem";
}

void GUISubsystem::setTypeCQS(QString typestring)
{
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.setSystemTypeCQS(this->getName(), typestring.toStdString()); //ehhh this will set the CQS type for the paren system (the root even) we want to set this partiular systems CQS type
}

QString GUISubsystem::getTypeCQS()
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getSystemTypeCQS(this->getName());  //ehhh this will get the CQS type for the paren system (the root even) we want this partiular systems CQS type
}

QVector<QString> GUISubsystem::getParameterNames()
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterNames(this->getName());
}

//void GUISubsystem::refreshAppearance();

//! @todo Maybe should be somewhere else and be called load subsystem
void GUISubsystem::loadFromFile(QTextStream &rFile)
{
    SystemAppearanceLoadData sysappdata;
    HeaderLoadData header;

    header.read(rFile);
    //qDebug() << "Header read";
    //! @todo check so that version OK!
    sysappdata.read(rFile);
    //qDebug() << "Sysapp data read";

    if (!sysappdata.usericon_path.isEmpty())
    {
        mAppearanceData.setIconPathUser(sysappdata.usericon_path);
    }
    if (!sysappdata.isoicon_path.isEmpty())
    {
        mAppearanceData.setIconPathISO(sysappdata.isoicon_path);
    }

    PortAppearanceMapT* portappmap = &(mAppearanceData.getPortAppearanceMap());
    for (int i=0; i<sysappdata.portnames.size(); ++i)
    {
        PortAppearance portapp;
        portapp.x = sysappdata.port_xpos[i];
        portapp.y = sysappdata.port_ypos[i];
        portapp.rot = sysappdata.port_angle[i];
        portapp.selectPortIcon("","",""); //!< @todo fix this

        portappmap->insert(sysappdata.portnames[i], portapp);
    }
    qDebug() << "Appearance set";

    //Load the contents of the subsystem from the external file
    //! @todo do this

    this->refreshAppearance();
}


int GUISubsystem::type() const
{
    return Type;
}


void GUISubsystem::deleteInHopsanCore()
{
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.removeSubComponent(this->getName(), true);
}

void GUISubsystem::openSubsystemFile()
{
    QDir fileDialog;
    QString modelFileName = QFileDialog::getOpenFileName(mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget, tr("Choose Subsystem File"),
                                                         fileDialog.currentPath() + QString("/../../Models"),
                                                         tr("Hopsan Model Files (*.hmf)"));
    if (modelFileName.isEmpty())
        return;

    QFile file(modelFileName);   //Create a QFile object
    QFileInfo fileInfo(file);

    for(int t=0; t!=mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->count(); ++t)
    {
        if( (mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == fileInfo.fileName()) or (mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == (fileInfo.fileName() + "*")) )
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget, tr("Error"), tr("Unable to load model. File is already open."));
            return;
        }
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file or not a text file: " + modelFileName;
        return;
    }
    QTextStream textStreamFile(&file); //Converts to QTextStream
    mModelFilePath = modelFileName;
    loadFromFile(textStreamFile);
}

//! @todo Maybe should try to reduce multiple copys of same functions with other GUIObjects
void GUISubsystem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction;
        if (!mpParentGraphicsScene->selectedItems().empty())
            groupAction = menu.addAction(tr("Group components"));

        QAction *parameterAction = menu.addAction(tr("Change parameters"));
        //menu.insertSeparator(parameterAction);

        QAction *showNameAction = menu.addAction(tr("Show name"));
        showNameAction->setCheckable(true);
        showNameAction->setChecked(this->mpNameText->isVisible());

        QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
        if(!mModelFilePath.isEmpty()) loadAction->setDisabled(true);

        QAction *selectedAction = menu.exec(event->screenPos());



        if (selectedAction == parameterAction)
        {
            openParameterDialog();
        }
        else if (selectedAction == groupAction)
        {
            //groupComponents(mpParentGraphicsScene->selectedItems());
            AppearanceData appdata;
            appdata.setIconPathUser("subsystemtmp.svg");
            appdata.setBasePath("../../HopsanGUI/"); //!< @todo This is EXTREAMLY BAD
            GUIGroup *pGroup = new GUIGroup(mpParentGraphicsScene->selectedItems(), appdata, mpParentGraphicsScene);
            this->mpParentGraphicsScene->addItem(pGroup);
        }
        else if (selectedAction == showNameAction)
        {
            if(this->mpNameText->isVisible())
            {
                this->hideName();
            }
            else
            {
                this->showName();
            }
        }
        else if (selectedAction == loadAction)
        {
            openSubsystemFile();
        }
    }



void GUISubsystem::openParameterDialog()
{
    ParameterDialog *dialog = new ParameterDialog(this, mpParentGraphicsView);
    dialog->exec();
}

void GUISubsystem::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (in case we reload, but maybe we can discard old system and create new in that case)
    //Create the graphics for the ports but do NOT create new ports, use the system ports within the subsystem
    PortAppearanceMapT::iterator it;
    for (it = mAppearanceData.getPortAppearanceMap().begin(); it != mAppearanceData.getPortAppearanceMap().end(); ++it)
    {
        //! @todo fix this
//        QString nodeType = this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getNodeType(this->getName(), i.key());
//        QString portType = this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getPortType(this->getName(), i.key());
//        i.value().selectPortIcon(cqsType, portType, nodeType);

//        qreal x = i.value().x;
//        qreal y = i.value().y;

//        GUIPort *pNewPort = new GUIPort(i.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this);
//        mPortListPtrs.append(pNewPort);
    }
}

//! @brief Save GuiSubsystem to a text stream
//! @todo here we are NOT using the save function in the guiobject base class becouse subsystems are saved completely differently, need to make this more uniform in the future
void GUISubsystem::saveToTextStream(QTextStream &rStream, QString prepend)
{
    QPointF pos = mapToScene(boundingRect().center());
    if (!prepend.isEmpty())
    {
        rStream << prepend << " ";
    }

    if (!mModelFilePath.isEmpty())
    {
        mLoadType = "EXTERNAL";
    }
    else
    {
        mLoadType = "EMBEDED";
    }

    rStream << addQuotes(mLoadType) << " " << addQuotes(getName()) << " " << addQuotes(getTypeCQS()) << " " << addQuotes(mModelFilePath) << " "
            << pos.x() << " " << pos.y() << " " << rotation() << " " << getNameTextPos() << " " << mpNameText->isVisible() << "\n";
}

void GUISubsystem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(mModelFilePath.isEmpty())
    {
        openSubsystemFile();
    }
    else
    {
        return;
    }
}

GUISystemPort::GUISystemPort(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
        : GUIObject(position, appearanceData, scene, parent)
{
    //Sets the ports
    createPorts();
}

//! @brief Help function to create ports in the component when it is created
void GUISystemPort::createPorts()
{
    //! @todo Only one port in system ports could simplify this
    PortAppearanceMapT::iterator i;
    for (i = mAppearanceData.getPortAppearanceMap().begin(); i != mAppearanceData.getPortAppearanceMap().end(); ++i)
    {
        qreal x = i.value().x;
        qreal y = i.value().y;

        i.value().selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)

        mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.addSystemPort(i.key()));

        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        mpGuiPort = new GUIPort(mAppearanceData.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this, &(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem));
        mPortListPtrs.append(mpGuiPort);
    }
}


//! Returns a string with the GUIObject type.
QString GUISystemPort::getTypeName()
{
    return "SystemPort";
}

//! Set the name of a system port
void GUISystemPort::setName(QString newName, bool doOnlyCoreRename)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        if (doOnlyCoreRename)
        {
            //Set name in core component, Also set the current name to the resulting one (might have been changed)
            mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.renameSystemPort(oldName, newName));
            refreshDisplayName();
            mpGuiPort->setDisplayName(mAppearanceData.getName()); //change the actual gui port name
        }
        else
        {
            //Rename
            mpParentGraphicsView->renameGUIObject(oldName, newName);
        }
    }
}


int GUISystemPort::type() const
{
    return Type;
}


//! Delete the system port in the core
void GUISystemPort::deleteInHopsanCore()
{
    //qDebug() << "In GUISystemPort::deleteInHopsanCore";
    this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.deleteSystemPort(mAppearanceData.getName());
}


//! @class GUIGroup
//! @brief The GUIGroup class implement a class to group components graphically
//!
//! The grouping is an alternative to make subcomponents. A group is a graphical operation ONLY, no changes in the core.
//!


int GUIGroup::type() const
{
    return Type;
}


QString GUIGroup::getTypeName()
{
    return "";
}


//! Constructor.
//! @param compList is a list for the QGraphicsItems that should be in the group.
//! @param appearanceData defines the appearance for the group.
//! @param scene is the scene which should contain the group.
//! @param parent is the parent QGraphicsItem for the group, default = 0.
GUIGroup::GUIGroup(QList<QGraphicsItem*> compList, AppearanceData appearanceData, GraphicsScene *scene, QGraphicsItem *parent)
    :   GUIObject(QPoint(0.0,0.0), appearanceData, scene, parent)
{
    mpParentScene = scene;

    this->setName(QString("Grupp_test"));
    this->refreshDisplayName();

    MessageWidget *pMessageWidget = scene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget;
    pMessageWidget->printGUIMessage("Group selected components (implementing in progress...) Selected components: ");

    for (int i=0; i < compList.size(); ++i)
    {
        GUIComponent *pComponent = qgraphicsitem_cast<GUIComponent*>(compList.at(i));
        if (pComponent)
        {
            //Adds the component pComponent to a list of components whose make up the group
            mGUICompList.append(pComponent);
            pMessageWidget->printGUIMessage(pComponent->getName());

            QList<GUIConnector*> GUIConnectorPtrs = pComponent->getGUIConnectorPtrs();
            for(int i = 0; i != GUIConnectorPtrs.size(); ++i)
            {
                //Loop trough the GUIConnectors that are connected to pComponent
                if((GUIConnectorPtrs[i]->getStartPort()->getGuiObject()->getName() == pComponent->getName()) or
                   (GUIConnectorPtrs[i]->getEndPort()->getGuiObject()->getName() == pComponent->getName()))
                {
                    if((compList.contains(GUIConnectorPtrs[i]->getStartPort()->getGuiObject())) and
                       (compList.contains(GUIConnectorPtrs[i]->getEndPort()->getGuiObject())))
                    {
                        //Add the connections which have both ends among selected components for grouping in a list for connections
                        mGUIConnList.append(GUIConnectorPtrs[i]);
                    }
                    else
                    {
                        //Add the connections that go trough the group boundary to a list
                        mGUITransitConnList.append(GUIConnectorPtrs[i]);
                    }
                }
                if(GUIConnectorPtrs.empty())
                {
                    break;
                }
            }
        }
    }

    //Constructs a new scene for the group
    mpGroupScene = new GraphicsScene(this->mpParentGraphicsScene->mpParentProjectTab);

    double xMin = mGUICompList.at(0)->x()+mGUICompList.at(0)->rect().width()/2.0,
           xMax = mGUICompList.at(0)->x()+mGUICompList.at(0)->rect().width()/2.0,
           yMin = mGUICompList.at(0)->y()+mGUICompList.at(0)->rect().height()/2.0,
           yMax = mGUICompList.at(0)->y()+mGUICompList.at(0)->rect().height()/2.0;
    for (int i=0; i < mGUICompList.size(); ++i)
    {
        //Add the components in the group to the group scene
        mpGroupScene->addItem(mGUICompList.at(i));

        //Find the rect for the selscted items (the group)
        if (mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0 < xMin)
            xMin = mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0;
        if (mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0 > xMax)
            xMax = mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0;
        if (mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0 < yMin)
            yMin = mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0;
        if (mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0 > yMax)
            yMax = mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0;
    }
    //Fix the position for the group item
    this->setPos((xMax+xMin)/2.0-this->rect().width()/2.0,(yMax+yMin)/2.0-this->rect().height()/2.0);

    for (int i=0; i < mGUIConnList.size(); ++i)
    {
        //Add the connections in the group to the group scene
        mpGroupScene->addItem(mGUIConnList.at(i));
    }

    mpGroupScene->setSceneRect(0,0,0,0); //Dirty(?) fix to re-calculate the correct scenerect
    QPointF sceneCenterPointF = mpGroupScene->sceneRect().center();

    //Draw a cross in the center of the scene (just for debugging)
//    mpGroupScene->addLine(-10+sceneCenterPointF.x(), -10+sceneCenterPointF.y(), 10+sceneCenterPointF.x(), 10+sceneCenterPointF.y());
//    mpGroupScene->addLine(10+sceneCenterPointF.x(), -10+sceneCenterPointF.y(), -10+sceneCenterPointF.x(), 10+sceneCenterPointF.y());
//    qDebug() << "Center: " << sceneCenterPointF << mpGroupScene->sceneRect();

    //Adjusts the size of the group component icon
    double scale = 1.0;//.75*min(mpGroupScene->sceneRect().width()/this->boundingRect().width(),mpGroupScene->sceneRect().height()/this->boundingRect().height());
    this->setTransformOriginPoint(this->boundingRect().center());
    this->setScale(scale);

    //Take care of the boundary connections of the group
    for(int i=0; i < mGUITransitConnList.size(); ++i)
    {
        GUIConnector *pTransitConnector = mGUITransitConnList[i];

        //Get the right appearance data for the group port
        AppearanceData appData;
        appData = *(mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData("SystemPort"));
        appData.setName("aPaApA-port");

        GUIGroupPort *pGroupPortComponent;

        GUIComponent *startComp;
        GUIComponent *endComp;
        startComp = qgraphicsitem_cast<GUIComponent*>(pTransitConnector->getStartPort()->getGuiObject());
        endComp   = qgraphicsitem_cast<GUIComponent*>(pTransitConnector->getEndPort()->getGuiObject());

        QPoint groupPortPoint;
        GUIPort *pPortBoundaryInside; //Inside the group
        GUIPort *pPortBoundaryOutside; //Outside the group
        if((startComp) && (mGUICompList.contains(startComp)))
        {
            //Find the right point for the group boundary port (in this case the boundary is at the connector start point)
            pPortBoundaryInside = pTransitConnector->getStartPort();
            pPortBoundaryOutside = pTransitConnector->getEndPort();
        }
        if((endComp) && (mGUICompList.contains(endComp)))
        {
            //Find the right point for the group boundary port (in this case the boundary is at the connector end point)
            pPortBoundaryInside = pTransitConnector->getEndPort();
            pPortBoundaryOutside = pTransitConnector->getStartPort();
        }
        groupPortPoint = getOffsetPointfromPort(pPortBoundaryInside).toPoint();
        groupPortPoint += QPoint(2.0*groupPortPoint.x(), 2.0*groupPortPoint.y());
        groupPortPoint += pPortBoundaryInside->mapToScene(pPortBoundaryInside->boundingRect().center()).toPoint();

        //Add a new group port for the boundary at the boundary connector
        pGroupPortComponent = new GUIGroupPort(appData, groupPortPoint, mpGroupScene);
        GUIPort *pPort = pGroupPortComponent->getPort("sysp");
        QString portName;
        if(pPort)
        {
            pGroupPortComponent->setOuterGuiPort(pPortBoundaryOutside);
            portName = pTransitConnector->getStartPort()->getName();

            QVector<QPointF> points;
            points.append(pPortBoundaryInside->mapToScene(pPortBoundaryInside->boundingRect().center()));
            points.append(pPort->mapToScene(pPort->boundingRect().center())); //! @todo GUIConnector should handle any number of points e.g. 0, 1 or 2
            points.append(pPort->mapToScene(pPort->boundingRect().center()));
            GUIConnector *pInsideConnector = new GUIConnector(pPortBoundaryInside, pPort, points, this->mpParentGraphicsView);
            mpGroupScene->addItem(pInsideConnector);

//            pGroupPortComponent->addConnector(pInsideConnector);
            mpGroupScene->addItem(pGroupPortComponent);
            pGroupPortComponent->showPorts(false);

        }

        //A line from center to port, used to determine the angle
        QLineF line(QPointF(sceneCenterPointF.x(), sceneCenterPointF.y()), QPointF(groupPortPoint.x(), groupPortPoint.y()));
//        mpGroupScene->addLine(line); //(just for debugging)
        //Determine the placement of the ports on the group icon
        double vinkel=line.angle()*3.141592/180.0;
        double b = mpIcon->boundingRect().width()/2.0;
        double h = mpIcon->boundingRect().height()/2.0;
        double x, y;
        calcSubsystemPortPosition(b, h, vinkel, x, y);

        qDebug() << portName << " vinkel: " << tan(vinkel) << " x: " << x << " ber x: " << h/tan(vinkel) << " b: " << b << " y: " << y << " ber y: " << b*tan(vinkel) << " h: " << h;
        //Make ports on the group system icon
        PortAppearance portAppearance;
        portAppearance.selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)
        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        GUIPort *pGuiPort = new GUIPort(pPortBoundaryInside->getGUIComponentName().append(", ").append(portName),
                                        mpIcon->boundingRect().center().x()+x,
                                        mpIcon->boundingRect().center().y()-y,
                                        &(portAppearance),
                                        this);
        mPortListPtrs.append(pGuiPort);

        //Make connectors to the group component
        GUIConnector *tmpConnector = new GUIConnector(pGuiPort, pPortBoundaryOutside,pTransitConnector->getPointsVector(), this->mpParentGraphicsView);
        this->mpParentScene->addItem(tmpConnector);
        this->showPorts(false);
        tmpConnector->drawConnector();

        //Delete the old connector
        delete pTransitConnector;
    }

    //Show this scene
    this->mpParentGraphicsView->setScene(mpGroupScene);
    this->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->show();

    //Draw a cross in the center of the group component icon (debug)
//    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,-10), this->rect().center()-QPointF(10,10)),this);
//    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,10), this->rect().center()-QPointF(10,-10)),this);

    //Scale up the ports and so on
    //! @todo Add a method to this->mpSelectionBox so the lines could be scaled
    QList<GUIPort*>::iterator it;
    for (it = mPortListPtrs.begin(); it != mPortListPtrs.end(); ++it)
    {
        (*it)->setTransformOriginPoint((*it)->boundingRect().center());
        (*it)->setScale(1.0/scale);
    }

    connect(this->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

}


GUIGroup::~GUIGroup()
{
    qDebug() << "GUIGroup destructor";
    QMap<QString, GUIObject *>::iterator itm;
    for(itm = mpParentGraphicsView->mGUIObjectMap.begin(); itm != mpParentGraphicsView->mGUIObjectMap.end(); ++itm)
    {
        qDebug() << "GUIObjectMap: " << itm.key();
    }


    QList<QGraphicsItem*> objectsInScenePtrs = this->mpGroupScene->items();
    QList<QGraphicsItem*>::iterator it;
    for(it=objectsInScenePtrs.begin(); it != objectsInScenePtrs.end(); ++it)
    {
        this->mpParentGraphicsView->deleteGUIObject(this->getName());
        GUIComponent *pGUIComponent = qgraphicsitem_cast<GUIComponent*>(*it);
        this->mpGroupScene->removeItem((*it));

        if(pGUIComponent)
        {
            qDebug() << "Add this to parent scene: " << pGUIComponent->getName();
            mpParentScene->addItem(pGUIComponent);
        }
        //mpParentScene->addItem((*it));
    }
    qDebug() << "this->mpParentGraphicsView->deleteGUIObject(this->getName()), getName:" << this->getName();
    //delete mpGroupScene;
}


//! Shows the parent scene. Should be called to exit a group.
void GUIGroup::showParent()
{
    this->mpParentGraphicsView->setScene(mpParentScene);

    disconnect(this->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

    this->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->hide();

}


void GUIGroup::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction;

        groupAction = menu.addAction(tr("Un-group components"));

        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == groupAction)
        {
            delete this;
        }
}


//! A slot that makes an entrance into a group at double clicks.
//! @param event contain information of the doubleclick event.
void GUIGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    this->mpParentGraphicsView->setScene(mpGroupScene);

    this->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->show();

    connect(this->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

}

//QGraphicsColorizeEffect *graphicsColor = new QGraphicsColorizeEffect;
//graphicsColor ->setColor(Qt::red);
//graphicsColor->setEnabled(true);
//this->mpIcon->setGraphicsEffect(graphicsColor);


GUIGroupPort::GUIGroupPort(AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
        : GUIObject(position, appearanceData, scene, parent)

{
    //Sets the ports
    //! @todo Only one port in group ports could simplify this
    PortAppearanceMapT::iterator i;
    for (i = mAppearanceData.getPortAppearanceMap().begin(); i != mAppearanceData.getPortAppearanceMap().end(); ++i)
    {
        qDebug() << "DEBUG: " << i.key();
        qreal x = i.value().x;
        qreal y = i.value().y;

        i.value().selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)

//        mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.addSystemPort(i.key()));
        mAppearanceData.setName(i.key());

        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        mpGuiPort = new GUIPort(mAppearanceData.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this);
        mpOuterGuiPort = 0;
        mPortListPtrs.append(mpGuiPort);
    }
}


void GUIGroupPort::setOuterGuiPort(GUIPort *pPort)
{
    mpOuterGuiPort = pPort;
}


//! Returns a string with the GUIObject type.
QString GUIGroupPort::getTypeName()
{
    return "GroupPort";
}

//! Set the name of a group port
void GUIGroupPort::setName(QString newName)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        //Rename
        mpParentGraphicsView->renameGUIObject(oldName, newName);
    }
}


int GUIGroupPort::type() const
{
    return Type;
}
