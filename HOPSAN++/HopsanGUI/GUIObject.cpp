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

#include "common.h"

#include "GUIObject.h"
#include "GUISystem.h"
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

GUIObject::GUIObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUISystem *system, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    //remeber the scene ptr

    //Make a local copy of the appearance data (that can safely be modified if needed)
    mAppearanceData = *pAppearanceData;

    mpParentSystem = system;

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);

        //Set to null ptr initially
    mpIcon = 0;
    mpSelectionBox = 0;
    mpNameText = 0;
    //mIconType = USERGRAPHICS;

        //Setup appearance
    this->refreshAppearance();
    this->setPos(position.x()-mpIcon->boundingRect().width()/2,position.y()-mpIcon->boundingRect().height()/2);
    this->rotateTo(rotation);
    this->setSelected(startSelected);
    this->setIcon(gfxType);
    this->setZValue(10);
    this->setAcceptHoverEvents(true);
    mTextOffset = 5.0;
    mIsFlipped = false;

        //Create the textbox containing the name
    mpNameText = new GUIObjectDisplayName(this);
    mNameTextPos = 0;
    this->setNameTextPos(mNameTextPos);


        //Create connections
    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(fixTextPosition(QPointF)));
    if(mpParentSystem != 0)
    {
        connect(mpParentSystem->mpParentProjectTab->mpGraphicsView,SIGNAL(zoomChange()),this,SLOT(adjustTextPositionToZoom()));
        connect(mpParentSystem, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
        connect(mpParentSystem, SIGNAL(hideAllNameText()), this, SLOT(hideName()));
        connect(mpParentSystem, SIGNAL(showAllNameText()), this, SLOT(showName()));
        connect(mpParentSystem, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIcon(graphicsType)));
    }
}


GUIObject::~GUIObject()
{
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
            x1 = mpIcon->boundingRect().width()/2 - mpNameText->boundingRect().width()/2;
            y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
            x2 = mpIcon->boundingRect().width()/2 - mpNameText->boundingRect().width()/2;
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
            x1 = mpIcon->boundingRect().width()/2 - mpNameText->boundingRect().width()/2;
            y1 = mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
            x2 = mpIcon->boundingRect().width()/2 - mpNameText->boundingRect().width()/2;
            y2 = -mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
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

    if(mpParentSystem != 0)
    {
        mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
    }
}


void GUIObject::rememberConnector(GUIConnector *item)
{
    mpGUIConnectorPtrs.append(item);
    connect(this, SIGNAL(componentMoved()), item, SLOT(drawConnector()));
}


void GUIObject::forgetConnector(GUIConnector *item)
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
        mpNameText->setSelected(false);
        //Adjust the position of the text
        this->fixTextPosition(mpNameText->pos());
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

//void GUIObject::setName(QString newName, renameRestrictions renameSettings)
//{
//    QString oldName = getName();
//    //If name same as before do nothing
//    if (newName != oldName)
//    {
//        if (mpParentSystem != 0)
//        {
//            mpParentSystem->renameGUIObject(oldName, newName);
//        }
//        else
//        {
//            assert(false);
//        }
//    }
////    //Check if we want to avoid trying to rename in the graphics view map
////    if ( (renameSettings == CORERENAMEONLY) or (mpParentSystem == 0) )
////    {
////        mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->renameSubComponent(this->getName(), newName));
////        refreshDisplayName();
////    }
////    else
////    {
////        //Rename
////        mpParentSystem->renameGUIObject(oldName, newName);
////    }

////    mpParentSystem->mGUIObjectMap.erase(mpParentSystem->mGUIObjectMap.find(this->getName()));
////    mAppearanceData.setName(newName);
////    refreshDisplayName();
////    mpParentSystem->mGUIObjectMap.insert(this->getName(), this);
//}

void GUIObject::setDisplayName(QString name)
{
    mAppearanceData.setName(name);
    refreshDisplayName();
}


void GUIObject::setIcon(graphicsType gfxType)
{
    qDebug() << "setIcon(" << gfxType << ")";

    QGraphicsSvgItem *tmp = mpIcon;
    if(gfxType && mAppearanceData.haveIsoIcon())
    {
        mpIcon = new QGraphicsSvgItem(mAppearanceData.getFullIconPath(ISOGRAPHICS) , this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mIconType = ISOGRAPHICS;
        //qDebug() << "Setting QString(ICONPATH) to " << mAppearanceData.getFullQString(ICONPATH)(true);
    }
    else
    {
        mpIcon = new QGraphicsSvgItem(mAppearanceData.getFullIconPath(USERGRAPHICS), this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mIconType = USERGRAPHICS;
        //qDebug() << "Setting QString(ICONPATH) to " << mAppearanceData.getFullQString(ICONPATH)(false);
    }

    //Delete old icon if it exist;
    if (tmp != 0)
    {
        delete(tmp);
    }

    if(mAppearanceData.getIconRotationBehaviour() == "ON")
        mIconRotation = true;
    else
        mIconRotation = false;

    if(!mIconRotation)
    {
        mpIcon->setRotation(-this->rotation());
        if(this->rotation() == 0)
        {
            mpIcon->setPos(0,0);
        }
        else if(this->rotation() == 90)
        {
            mpIcon->setPos(0,this->boundingRect().height());
        }
        else if(this->rotation() == 180)
        {
            mpIcon->setPos(this->boundingRect().width(),this->boundingRect().height());
        }
        else if(this->rotation() == 270)
        {
            mpIcon->setPos(this->boundingRect().width(),0);
        }
    }

}


//! Slots that deselects the object. Used for signal-slot connection.
void GUIObject::deselect()
{
    this->setSelected(false);
}


//! Slots that selects the object. Used for signal-slot connection.
void GUIObject::select()
{
    this->setSelected(true);
}


//! Returns the port with the specified name.
GUIPort *GUIObject::getPort(QString name)
{

    //! @todo use the a guiport map instead   (Is this really a good idea? The number of ports is probably too small to make it beneficial, and it would slow down everything else...)
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
    return 0;
}

QVector<QString> GUIObject::getParameterNames()
{
    cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
    return QVector<QString>();
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

void GUIObject::saveToDomDocument(QDomDocument &rDomDocument)
{
    //! @todo add code here
}


//! Event when mouse cursor enters component icon.
void GUIObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setHovered();
        //mpSelectionBox->setVisible(true);
    }
    this->showPorts(true);
    this->setZValue(12);
}


//! Event when mouse cursor leaves component icon.
void GUIObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setPassive();
    }
    this->showPorts(false);
    this->setZValue(10);
}


//! Defines what shall happen if a mouse key is pressed while hovering an object.
void GUIObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
        //Store old positions for all components, in case more than one is selected
    if(event->button() == Qt::LeftButton)
    {
        //QList<GUIObject *>::iterator it;
        for(size_t i = 0; i < mpParentSystem->mSelectedGUIObjectsList.size(); ++i)
        {
            mpParentSystem->mSelectedGUIObjectsList[i]->mOldPos = mpParentSystem->mSelectedGUIObjectsList[i]->pos();
        }
    }

        //Objects shall not be selectable while creating a connector
    if(mpParentSystem->mIsCreatingConnector)
    {
        this->setSelected(false);
        this->setActive(false);
    }
}


//! Defines what shall happen if a mouse key is released while hovering an object.
void GUIObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QList<GUIObject *>::iterator it;

        //Loop through all selected objects and register changed positions in undo stack
    bool alreadyClearedRedo = false;
    for(it = mpParentSystem->mSelectedGUIObjectsList.begin(); it != mpParentSystem->mSelectedGUIObjectsList.end(); ++it)
    {
        if(((*it)->mOldPos != (*it)->pos()) && (event->button() == Qt::LeftButton))
        {
                //This check makes sure that only one undo post is created when moving several objects at once
            if(!alreadyClearedRedo)
            {
                mpParentSystem->mUndoStack->newPost();
                mpParentSystem->mpParentProjectTab->hasChanged();
                alreadyClearedRedo = true;
            }
            mpParentSystem->mUndoStack->registerMovedObject((*it)->mOldPos, (*it)->pos(), (*it)->getName());
        }
    }

    QGraphicsWidget::mouseReleaseEvent(event);

        //Objects shall not be selectable while creating a connector
    if(mpParentSystem->mIsCreatingConnector)
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
            mpParentSystem->mSelectedGUIObjectsList.append(this);
            mpSelectionBox->setActive();
            connect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            disconnect(mpParentSystem, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            connect(mpParentSystem, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
            emit componentSelected();
        }
        else
        {
            mpParentSystem->mSelectedGUIObjectsList.removeAll(this);
            disconnect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            connect(mpParentSystem, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            disconnect(mpParentSystem, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
            mpSelectionBox->setPassive();
        }
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit componentMoved();  //This signal must be emitted  before the snap code, because it updates the connectors which is used to determine whether or not to snap.

            //Snap component if it only has one connector and is dropped close enough (horizontal or vertical) to adjacent component
        if(mpParentSystem != 0 && mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mSnapping)
        {
                //Vertical snap
            if( (mpGUIConnectorPtrs.size() == 1) &&
                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) )
            {
                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
                {
                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.last().x() - mpGUIConnectorPtrs.first()->mPoints.first().x(), 0);
                }
                else
                {
                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x(), 0);
                }
            }

                //Horizontal snap
            if( (mpGUIConnectorPtrs.size() == 1) &&
                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 2) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) < SNAPDISTANCE) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) > 0.0) )
            {
                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
                {
                    this->moveBy(0, mpGUIConnectorPtrs.first()->mPoints.last().y() - mpGUIConnectorPtrs.first()->mPoints.first().y());
                }
                else
                {
                    this->moveBy(0, mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y());
                }
            }
        }
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
            if ((*i)->isConnected || mpParentSystem->mPortsHidden)
            {
                (*i)->hide();
            }
        }
}


////! Figures out the number of a component port by using a pointer to the port.
////! @see getPort(int number)
//int GUIObject::getPortNumber(GUIPort *port)
//{
//    for (int i = 0; i != mPortListPtrs.size(); ++i)
//    {
//        if(port == mPortListPtrs.value(i))
//        {
//            return i;
//        }
//    }
//    qDebug() << "Request for port number of non-existing port.";
//    assert(false);      /// @todo: Trough exception
//}


//! Rotates a component 90 degrees clockwise, and tells the connectors that the component has moved.
//! @see rotateTo(qreal angle);
void GUIObject::rotate(undoStatus undoSettings)
{
    this->setTransformOriginPoint(mpIcon->boundingRect().center());
    this->setRotation(this->rotation()+90);

    if (this->rotation() == 360)
    {
        this->setRotation(0);
    }

    int tempNameTextPos = mNameTextPos;
    mpNameText->rotate(-90);
    this->fixTextPosition(mpNameText->pos());
    setNameTextPos(tempNameTextPos);

    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(mPortListPtrs.value(i)->getPortDirection() == TOPBOTTOM)
            mPortListPtrs.value(i)->setPortDirection(LEFTRIGHT);
        else
            mPortListPtrs.value(i)->setPortDirection(TOPBOTTOM);
        if (mPortListPtrs.value(i)->getPortType() == "POWERPORT")
        {
            if(this->rotation() == 0 && !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(0);
            else if(this->rotation() == 0 && mIsFlipped)
                mPortListPtrs.value(i)->setRotation(180);
            else if(this->rotation() == 90 && !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(270);
            else if(this->rotation() == 90 && mIsFlipped)
                mPortListPtrs.value(i)->setRotation(90);
            else if(this->rotation() == 180 && !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(180);
            else if(this->rotation() == 180 && mIsFlipped)
                mPortListPtrs.value(i)->setRotation(0);
            else if(this->rotation() == 270 && !mIsFlipped)
                mPortListPtrs.value(i)->setRotation(90);
            else if(this->rotation() == 270 && mIsFlipped)
                mPortListPtrs.value(i)->setRotation(270);
        }
        //mPortListPtrs[i]->updatePosition();
    }

    if(!mIconRotation)
    {
        mpIcon->setRotation(-this->rotation());
        if(this->rotation() == 0)
        {
            mpIcon->setPos(0,0);
        }
        else if(this->rotation() == 90)
        {
            mpIcon->setPos(0,this->boundingRect().height());
        }
        else if(this->rotation() == 180)
        {
            mpIcon->setPos(this->boundingRect().width(),this->boundingRect().height());
        }
        else if(this->rotation() == 270)
        {
            mpIcon->setPos(this->boundingRect().width(),0);
        }

        //mpIcon->setPos(this->boundingRect().center());
    }

    if(undoSettings == UNDO)
    {
        mpParentSystem->mUndoStack->registerRotatedObject(this);
    }

    emit componentMoved();
}


//! Slot that rotates the object to a desired angle (this does NOT create and undo post)
//! @see rotate(undoStatus undoSettings)
void GUIObject::rotateTo(qreal angle)
{
    while(this->rotation() != angle)
    {
        this->rotate(NOUNDO);
    }
}


//! Slot that moves component one pixel upwards
//! @see moveDown()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveUp()
{
    //qDebug() << "Move up!";
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()-1);
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveDown()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()+1);
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void GUIObject::moveLeft()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()-1, this->pos().y());
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void GUIObject::moveRight()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()+1, this->pos().y());
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @todo Fix name text position when flipping components

//! Slot that flips the object vertically.
//! @see flipHorizontal()
void GUIObject::flipVertical(undoStatus undoSettings)
{
    this->rotate(NOUNDO);
    this->rotate(NOUNDO);
    this->flipHorizontal(NOUNDO);
    if(undoSettings == UNDO)
    {
        mpParentSystem->mUndoStack->registerVerticalFlip(this);

    }
}


//! Slot that flips the object horizontally.
//! @see flipVertical()
void GUIObject::flipHorizontal(undoStatus undoSettings)
{
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(mPortListPtrs[i]->getPortType() == "READPORT" ||  mPortListPtrs[i]->getPortType() == "WRITEPORT")
        {
            if(this->rotation() == 90 ||  this->rotation() == 270)
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

        //"Un-flip" the ports and name text
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(this->rotation() == 90 ||  this->rotation() == 270)
        {
            mPortListPtrs.value(i)->scale(1,-1);
            mPortListPtrs.value(i)->translate(0, -mPortListPtrs.value(i)->boundingRect().width());
            this->mpNameText->scale(1,-1);
        }
        else
        {
            mPortListPtrs.value(i)->scale(-1,1);
            mPortListPtrs.value(i)->translate(-mPortListPtrs.value(i)->boundingRect().width(), 0);
            this->mpNameText->scale(-1,1);
        }
    }
    //this->setNameTextPos(mNameTextPos);
    this->fixTextPosition(mpNameText->pos());


    if(undoSettings == UNDO)
    {
        mpParentSystem->mUndoStack->registerHorizontalFlip(this);
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
    mpNameText->setVisible(false);
}

void GUIObject::showName()
{
    mpNameText->setVisible(true);
}


//! Dummy
QString GUIObject::getTypeName()
{
    assert(false);
    return "";
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
                                                  QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
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
    mpParentGUIObject = pParent;
    this->setTextInteractionFlags(Qt::NoTextInteraction);
    //this->setTextInteractionFlags(Qt::TextEditable | Qt::TextSelectableByMouse);
    this->setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
}

void GUIObjectDisplayName::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit textMoved(this->pos());
    QGraphicsTextItem::mouseReleaseEvent(event);
}

void GUIObjectDisplayName::focusInEvent(QFocusEvent *event)
{
    qDebug() << "focusInEvent()";
    mpParentGUIObject->mpParentSystem->mIsRenamingObject = true;
    this->setSelected(true);
    QGraphicsTextItem::focusInEvent(event);
}


//! @todo Can we delete this reimplementation?
void GUIObjectDisplayName::focusOutEvent(QFocusEvent *event)
{
//    qDebug() << "focusOutEvent()";
//    //mpParentGUIComponent->mpParentSystem->mUndoStack->newPost();
//    mpParentGUIObject->mpParentSystem->mIsRenamingObject = false;
//        //Try to set the new name, the rename function in parent is used
//    mpParentGUIObject->mpParentSystem->renameGUIObject(mpParentGUIObject->getName(),toPlainText());
//        //Refresh the display name (it may be different from the one you wanted)
//    mpParentGUIObject->refreshDisplayName();

//    this->setSelected(false);

//    emit textMoved(pos());
    mpParentGUIObject->mpParentSystem->mIsRenamingObject = false;
    QGraphicsTextItem::focusOutEvent(event);
}


//! Handles item change events.
QVariant GUIObjectDisplayName::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QGraphicsTextItem::itemChange(change, value);

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (this->isSelected())
        {
            connect(this->mpParentGUIObject->mpParentSystem, SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
        else
        {
            disconnect(this->mpParentGUIObject->mpParentSystem, SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
    }
    return value;
}



void GUIObjectDisplayName::deselect()
{
    this->setSelected(false);
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

    mActivePen = activePen;
    mHoverPen = hoverPen;
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
        mLines[i]->setPen(mActivePen);
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
        mLines[i]->setPen(mHoverPen);
    }
}

//! Tells the component to ask its parent to delete it.
void GUIObject::deleteMe()
{
    qDebug() << "deleteMe in " << this->getName();
    mpParentSystem->deleteGUIObject(this->getName());
}

GUIContainerObject::GUIContainerObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUISystem *system, QGraphicsItem *parent)
        : GUIObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    //Something
}

void GUIContainerObject::makeRootSystem()
{
    mContainerStatus = ROOT;
}

GUIContainerObject::CONTAINERSTATUS GUIContainerObject::getContainerStatus()
{
    return mContainerStatus;
}

GUIComponent::GUIComponent(AppearanceData* pAppearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
    : GUIObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    //Create the object in core, and get its default core name
    mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->createComponent(mAppearanceData.getTypeName(), mAppearanceData.getName()));

    //Sets the ports
    createPorts();

    refreshDisplayName(); //Make sure name window is correct size for center positioning

    std::cout << "GUIcomponent: " << this->mAppearanceData.getTypeName().toStdString() << std::endl;
}

GUIComponent::~GUIComponent()
{
    //Remove in core
    //! @todo maybe change to delte instead of remove with dodelete yes
    mpParentSystem->mpCoreSystemAccess->removeSubComponent(this->getName(), true);
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
////            mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->renameSubComponent(this->getName(), newName));
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
    return mpParentSystem->mpCoreSystemAccess->getSubComponentTypeCQS(this->getName());
}

//! @brief Get a vector with the names of the available parameters
QVector<QString> GUIComponent::getParameterNames()
{
    return mpParentSystem->mpCoreSystemAccess->getParameterNames(this->getName());
}

QString GUIComponent::getParameterUnit(QString name)
{
    return mpParentSystem->mpCoreSystemAccess->getParameterUnit(this->getName(), name);
}

QString GUIComponent::getParameterDescription(QString name)
{
    return mpParentSystem->mpCoreSystemAccess->getParameterDescription(this->getName(), name);
}

double GUIComponent::getParameterValue(QString name)
{
    return mpParentSystem->mpCoreSystemAccess->getParameterValue(this->getName(), name);
}

//! @brief Set a parameter value, wrapps hopsan core
void GUIComponent::setParameterValue(QString name, double value)
{
    mpParentSystem->mpCoreSystemAccess->setParameter(this->getName(), name, value);
}

void GUIComponent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QAction *groupAction;
    if (!this->scene()->selectedItems().empty())
    {
        groupAction = menu.addAction(tr("Group components"));
    }

    QAction *parameterAction = menu.addAction(tr("Change parameters"));
    //menu.insertSeparator(parameterAction);

    QAction *showNameAction = menu.addAction(tr("Show name"));
    showNameAction->setCheckable(true);
    showNameAction->setChecked(mpNameText->isVisible());

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
        GUIGroup *pGroup = new GUIGroup(this->scene()->selectedItems(), &appdata, mpParentSystem);
        this->scene()->addItem(pGroup);
    }
    else if (selectedAction == showNameAction)
    {
        if(mpNameText->isVisible())
        {
            this->hideName();
        }
        else
        {
            this->showName();
        }
    }
    QGraphicsItem::contextMenuEvent(event);
}


//! @brief Slot that opens the parameter dialog for the component
void GUIComponent::openParameterDialog()
{
    ParameterDialog *dialog = new ParameterDialog(this);
    dialog->exec();
}


//! @brief Help function to create ports in the component when it is created
void GUIComponent::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (not really necessary in guicomponents)
    QString cqsType = mpParentSystem->mpCoreSystemAccess->getSubComponentTypeCQS(getName());
    PortAppearanceMapT::iterator i;
    for (i = mAppearanceData.getPortAppearanceMap().begin(); i != mAppearanceData.getPortAppearanceMap().end(); ++i)
    {
        QString nodeType = mpParentSystem->mpCoreSystemAccess->getNodeType(this->getName(), i.key());
        QString portType = mpParentSystem->mpCoreSystemAccess->getPortType(this->getName(), i.key());
        i.value().selectPortIcon(cqsType, portType, nodeType);

        qreal x = i.value().x;
        qreal y = i.value().y;

        GUIPort *pNewPort = new GUIPort(i.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this);
        mPortListPtrs.append(pNewPort);
    }
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

    QVector<QString> parameterNames = mpParentSystem->mpCoreSystemAccess->getParameterNames(this->getName());
    QVector<QString>::iterator pit;
    for(pit = parameterNames.begin(); pit != parameterNames.end(); ++pit)
    {
        //! @todo It is a bit strange that we can not control the parameter keyword, but then agian spliting this into a separate function with its own prepend variable would also be wierd
        rStream << "PARAMETER " << addQuotes(getName()) << " " << addQuotes(*pit) << " " <<
                mpParentSystem->mpCoreSystemAccess->getParameterValue(this->getName(), (*pit)) << "\n";
    }
}


GUISystemPort::GUISystemPort(AppearanceData* pAppearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
        : GUIObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    //Sets the ports
    createPorts();
    refreshDisplayName();
}

GUISystemPort::~GUISystemPort()
{
    //! @todo delete systemport in core
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

        mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->addSystemPort(i.key()));

        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        mpGuiPort = new GUIPort(mAppearanceData.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this, mpParentSystem->mpCoreSystemAccess);
        mPortListPtrs.append(mpGuiPort);
    }
}


//! Returns a string with the GUIObject type.
QString GUISystemPort::getTypeName()
{
    return "SystemPort";
}

//! Set the name of a system port
void GUISystemPort::setName(QString newName, renameRestrictions renameSettings)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        if (renameSettings == CORERENAMEONLY)
        {
            //Set name in core component, Also set the current name to the resulting one (might have been changed)
            mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->renameSystemPort(oldName, newName));
            refreshDisplayName();
            mpGuiPort->setDisplayName(mAppearanceData.getName()); //change the actual gui port name
        }
        else
        {
            //Rename
            mpParentSystem->renameGUIObject(oldName, newName);
        }
    }
}


int GUISystemPort::type() const
{
    return Type;
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
GUIGroup::GUIGroup(QList<QGraphicsItem*> compList, AppearanceData* pAppearanceData, GUISystem *system, QGraphicsItem *parent)
    :   GUIContainerObject(QPoint(0.0,0.0), 0, pAppearanceData, DESELECTED, USERGRAPHICS, system, parent)
{

    this->setDisplayName(QString("Grupp_test"));

    MessageWidget *pMessageWidget = mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget;
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
                if((GUIConnectorPtrs[i]->getStartComponentName() == pComponent->getName()) ||
                   (GUIConnectorPtrs[i]->getEndComponentName() == pComponent->getName()))
                {
                    if((compList.contains(GUIConnectorPtrs[i]->getStartPort()->getGuiObject())) &&
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
    mpGroupScene = new GraphicsScene(mpParentSystem->mpParentProjectTab);

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
        appData = *(mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData("SystemPort"));
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
        pGroupPortComponent = new GUIGroupPort(&appData, groupPortPoint, mpParentSystem);
        GUIPort *pPort = pGroupPortComponent->getPort("sysp");
        QString portName;
        if(pPort)
        {
            pGroupPortComponent->setOuterGuiPort(pPortBoundaryOutside);
            portName = pTransitConnector->getStartPortName();

            QVector<QPointF> points;
            points.append(pPortBoundaryInside->mapToScene(pPortBoundaryInside->boundingRect().center()));
            points.append(pPort->mapToScene(pPort->boundingRect().center())); //! @todo GUIConnector should handle any number of points e.g. 0, 1 or 2
            points.append(pPort->mapToScene(pPort->boundingRect().center()));
            GUIConnector *pInsideConnector = new GUIConnector(pPortBoundaryInside, pPort, points, mpParentSystem);
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
        GUIPortAppearance portAppearance;
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
        GUIConnector *tmpConnector = new GUIConnector(pGuiPort, pPortBoundaryOutside,pTransitConnector->getPointsVector(), mpParentSystem);
        mpParentScene->addItem(tmpConnector);
        this->showPorts(false);
        tmpConnector->drawConnector();

        //Delete the old connector
        delete pTransitConnector;
    }

    //Show this scene
    mpParentSystem->mpParentProjectTab->mpGraphicsView->setScene(mpGroupScene);
    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->show();

    //Draw a cross in the center of the group component icon (debug)
//    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,-10), this->rect().center()-QPointF(10,10)),this);
//    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,10), this->rect().center()-QPointF(10,-10)),this);

    //Scale up the ports and so on
    //! @todo Add a method to mpSelectionBox so the lines could be scaled
    QList<GUIPort*>::iterator it;
    for (it = mPortListPtrs.begin(); it != mPortListPtrs.end(); ++it)
    {
        (*it)->setTransformOriginPoint((*it)->boundingRect().center());
        (*it)->setScale(1.0/scale);
    }

    connect(this->mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

}


GUIGroup::~GUIGroup()
{
    qDebug() << "GUIGroup destructor";
    QHash<QString, GUIObject *>::iterator itm;
    for(itm = mpParentSystem->mGUIObjectMap.begin(); itm != mpParentSystem->mGUIObjectMap.end(); ++itm)
    {
        qDebug() << "GUIObjectMap: " << itm.key();
    }


    QList<QGraphicsItem*> objectsInScenePtrs = mpGroupScene->items();
    QList<QGraphicsItem*>::iterator it;
    for(it=objectsInScenePtrs.begin(); it != objectsInScenePtrs.end(); ++it)
    {
        //! @todo Will cause crash when closing program if the GUIObject has already been deleted by the scene.
        mpParentSystem->deleteGUIObject(this->getName());
        GUIComponent *pGUIComponent = qgraphicsitem_cast<GUIComponent*>(*it);
        mpGroupScene->removeItem((*it));

        if(pGUIComponent)
        {
            qDebug() << "Add this to parent scene: " << pGUIComponent->getName();
            mpParentScene->addItem(pGUIComponent);
        }
        //mpParentScene->addItem((*it));
    }
    qDebug() << "mpParentSystem->deleteGUIObject(this->getName()), getName:" << this->getName();
    //delete mpGroupScene;
}


//! Shows the parent scene. Should be called to exit a group.
void GUIGroup::showParent()
{
    mpParentSystem->mpParentProjectTab->mpGraphicsView->setScene(mpParentScene);

    disconnect(mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->hide();

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
    mpParentSystem->mpParentProjectTab->mpGraphicsView->setScene(mpGroupScene);

    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->show();

    connect(mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

}

//QGraphicsColorizeEffect *graphicsColor = new QGraphicsColorizeEffect;
//graphicsColor ->setColor(Qt::red);
//graphicsColor->setEnabled(true);
//mpIcon->setGraphicsEffect(graphicsColor);


GUIGroupPort::GUIGroupPort(AppearanceData* pAppearanceData, QPoint position, GUISystem *system, QGraphicsItem *parent)
    : GUIObject(position, 0, pAppearanceData, DESELECTED, USERGRAPHICS, system, parent)

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

//        mAppearanceData.setName(mpParentSystem->mpParentProjectTab->mGUIRootSystem.addSystemPort(i.key()));
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
        mpParentSystem->renameGUIObject(oldName, newName);
    }
}


int GUIGroupPort::type() const
{
    return Type;
}
