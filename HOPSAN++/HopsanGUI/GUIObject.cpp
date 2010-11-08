//$Id$

#include <QVector>
#include <QtGui> //!< @todo maybe only need qtfile dialog

#include "common.h"

#include "GUIObject.h"
#include "GUISystem.h"
#include "GUIComponent.h"
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



//! @todo should not pSystem and pParent be teh same ?
GUIObject::GUIObject(QPoint pos, qreal rot, selectionStatus, GUISystem *pSystem, QGraphicsItem *pParent)
    : QGraphicsWidget(pParent)
{
    //Initi variables
    mHmfTagName = HMF_OBJECTTAG;
    mpSelectionBox = 0;

    mpParentSystem = pSystem;
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);

    //Set position orientation and other appearance stuff
    this->setCenterPos(pos);
    this->rotateTo(rot);
    this->setAcceptHoverEvents(true);
    mIsFlipped = false;
}


//! @brief Destructor for GUI Objects
GUIObject::~GUIObject()
{

}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int GUIObject::type() const
{
    return Type;
}

QPointF GUIObject::getCenterPos()
{
    return QPointF(this->pos().x()+this->boundingRect().width()/2.0, this->pos().y()+this->boundingRect().height()/2.0);
}

void GUIObject::setCenterPos(QPointF pos)
{
    this->setPos(pos.x()-this->boundingRect().width()/2.0, pos.y()-this->boundingRect().height()/2.0);
}


//! @brief Slot that deselects the object
void GUIObject::deselect()
{
    this->setSelected(false);
}


//! @brief Slot that selects the object
void GUIObject::select()
{
    this->setSelected(true);
}


//! @brief Defines what happens when mouse starts hovering the object
void GUIObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setHovered();
    }

    QGraphicsWidget::hoverEnterEvent(event);
}


//! @bried Defines what happens when mouse stops hovering the object
void GUIObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setPassive();
    }

    QGraphicsWidget::hoverLeaveEvent(event);
}


//! @brief Defines what happens if a mouse key is pressed while hovering an object
void GUIObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setFlag(QGraphicsItem::ItemIsMovable, true); //Make the component movable if not
    setFlag(QGraphicsItem::ItemIsSelectable, true); //Make the component selactable if not

        //Store old positions for all components, in case more than one is selected
    if(event->button() == Qt::LeftButton)
    {
        for(size_t i = 0; i < mpParentSystem->mSelectedGUIObjectsList.size(); ++i)
        {
            mpParentSystem->mSelectedGUIObjectsList[i]->mOldPos = mpParentSystem->mSelectedGUIObjectsList[i]->pos();
        }
    }

        //Objects shall not be selectable while creating a connector
    if(mpParentSystem->getIsCreatingConnector())
    {
        setFlag(QGraphicsItem::ItemIsMovable, false); //Make the component not movable during connection
        setFlag(QGraphicsItem::ItemIsSelectable, false); //Make the component not selactable during connection

        this->setSelected(false);
        this->setActive(false);
    }

    QGraphicsWidget::mousePressEvent(event);
}


//! @brief Defines what happens if a mouse key is released while hovering an object
void GUIObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
        //Objects shall not be selectable while creating a connector
    if(mpParentSystem->getIsCreatingConnector())
    {
        this->setSelected(false);
        this->setActive(false);
    }

    QGraphicsWidget::mouseReleaseEvent(event);
}


//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant GUIObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
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
            emit objectSelected();
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
//    else if (change == QGraphicsItem::ItemPositionHasChanged)
//    {
//        emit componentMoved();  //This signal must be emitted  before the snap code, because it updates the connectors which is used to determine whether or not to snap.

//            //Snap component if it only has one connector and is dropped close enough (horizontal or vertical) to adjacent component
//        if(mpParentSystem != 0 && mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mSnapping && !mpParentSystem->mIsCreatingConnector)
//        {
//                //Vertical snap
//            if( (mpGUIConnectorPtrs.size() == 1) &&
//                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
//                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
//                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) )
//            {
//                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
//                {
//                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.last().x() - mpGUIConnectorPtrs.first()->mPoints.first().x(), 0);
//                }
//                else
//                {
//                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x(), 0);
//                }
//            }
////            else if( (mpGUIConnectorPtrs.size() == 2) &&
////                     (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
////                     (mpGUIConnectorPtrs.last()->getNumberOfLines() < 4) &&
////                     ( ( (this->rotation() == 0 || this->rotation() == 180) &&
////                       (mPortListPtrs.first()->pos().y() == mPortListPtrs.last()->pos().y()) ) ||
////                       ( (this->rotation() == 90 || this->rotation() == 270) &&
////                       (mPortListPtrs.first()->pos().x() == mPortListPtrs.last()->pos().x()) ) ) &&
////                     !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
////                     !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
////                     (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
////                     (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) &&
////                     !(mpGUIConnectorPtrs.last()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.last()->getNumberOfLines() == 2) &&
////                     !(mpGUIConnectorPtrs.last()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.last()->getNumberOfLines() > 1) &&
////                     (abs(mpGUIConnectorPtrs.last()->mPoints.first().x() - mpGUIConnectorPtrs.last()->mPoints.last().x()) < SNAPDISTANCE) &&
////                     (abs(mpGUIConnectorPtrs.last()->mPoints.first().x() - mpGUIConnectorPtrs.last()->mPoints.last().x()) > 0.0) )
////            {
////                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
////                {
////                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.last().x() - mpGUIConnectorPtrs.first()->mPoints.first().x(), 0);
////                }
////                else
////                {
////                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x(), 0);
////                }
////            }

//                //Horizontal snap
//            if( (mpGUIConnectorPtrs.size() == 1) &&
//                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
//                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
//                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 2) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) < SNAPDISTANCE) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) > 0.0) )
//            {
//                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
//                {
//                    this->moveBy(0, mpGUIConnectorPtrs.first()->mPoints.last().y() - mpGUIConnectorPtrs.first()->mPoints.first().y());
//                }
//                else
//                {
//                    this->moveBy(0, mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y());
//                }
//            }
//        }
//    }

    return  QGraphicsWidget::itemChange(change, value);
}


//! @brief Slot that rotates the object to a desired angle (NOT registered in undo stack!)
//! @param angle Angle to rotate to
//! @see rotate(undoStatus undoSettings)
//! @todo Add option to register this in undo stack - someone will want to do this sooner or later anyway
void GUIObject::rotateTo(qreal angle)
{
    while(this->rotation() != angle)
    {
        this->rotate(NOUNDO);
    }
}

//! @brief Rotates a component 90 degrees clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stsack
//! @see rotateTo(qreal angle);
void GUIObject::rotate(undoStatus undoSettings)
{
    this->setTransformOriginPoint(this->boundingRect().center());
    this->setRotation(this->rotation()+90);

    if (this->rotation() == 360)
    {
        this->setRotation(0);
    }

    if(undoSettings == UNDO)
    {
        mpParentSystem->mUndoStack->registerRotatedObject(this);
    }

    emit objectMoved();
}


//! @brief Slot that moves component one pixel upwards
//! @see moveDown()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveUp()
{
    //qDebug() << "Move up!";
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()-1);
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort(); //!< @todo If we have many objects selected this will probably call MANY updates of the viewport, maybe should do in some other way, same "problem" in other places
}


//! @brief Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveDown()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()+1);
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void GUIObject::moveLeft()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()-1, this->pos().y());
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void GUIObject::moveRight()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()+1, this->pos().y());
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Tells the component to ask its parent to delete it
//! @todo The name of the function is silly
//! @todo will not work with gui only objects like textboxes, as they ont have unique names
void GUIObject::deleteMe()
{
    //Should not be used
    assert(false);
}


bool GUIObject::isFlipped()
{
    return mIsFlipped;
}





//! @brief Constructor for GUI Objects
//! @param position Initial scene coordinates where object shall be placed
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to appearance data object
//! @param startSelected Initial selection status
//! @param gfxType Initial graphics type (user or iso)
//! @param system Pointer to the parent system
//! @param parent Pointer to parent object (not mandatory)
GUIModelObject::GUIModelObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUISystem *system, QGraphicsItem *parent)
        : GUIObject(position, rotation, startSelected, system, parent)
{
        //Initialize variables
    mpIcon = 0;
    mpNameText = 0;
    mTextOffset = 5.0;

        //Set the hmf save tag name
    mHmfTagName = HMF_OBJECTTAG; //!< @todo change this

    //! @todo Is this comment a todo?
    //remeber the scene ptr

        //Make a local copy of the appearance data (that can safely be modified if needed)
    mAppearanceData = *pAppearanceData;

        //Setup appearance
    this->refreshAppearance();
    this->setCenterPos(position);
    this->setIcon(gfxType);
    this->setZValue(10);
    this->setSelected(startSelected);

        //Create the textbox containing the name
    mpNameText = new GUIModelObjectDisplayName(this);
    mpNameText->setFlag(QGraphicsItem::ItemIsSelectable, false); //To minimize problems when move after copy and so on
    this->setNameTextPos(0); //Set initial name text position

        //Create connections
    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(snapNameTextPosition(QPointF)));
    if(mpParentSystem != 0)
    {
        connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(zoomChange(qreal)), this, SLOT(setNameTextScale(qreal)));
        connect(mpParentSystem, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
        connect(mpParentSystem, SIGNAL(hideAllNameText()), this, SLOT(hideName()));
        connect(mpParentSystem, SIGNAL(showAllNameText()), this, SLOT(showName()));
        connect(mpParentSystem, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIcon(graphicsType)));
    }
    else
    {
        //maybe something different
    }
}


//! @brief Destructor for GUI Objects
GUIModelObject::~GUIModelObject()
{
    emit objectDeleted();
}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int GUIModelObject::type() const
{
    return Type;
}

//! @brief Updates name text position
//! @param pos Position where name text was dropped
void GUIModelObject::snapNameTextPosition(QPointF pos)
{
    QVector<QPointF> pts;
    this->calcNameTextPositions(pts);

    QPointF  mtp_pos = mpNameText->mapToParent(pos);
    if ( dist(mtp_pos, pts[0]) < dist(mtp_pos, pts[1]) )
    {
        //We dont use this.setnamepos here as that would recaluclate the positions again
        mpNameText->setPos(pts[0]);
        mNameTextPos = 0;
    }
    else
    {
        //We dont use this.setnamepos here as that would recaluclate the positions again
        mpNameText->setPos(pts[1]);
        mNameTextPos = 1;
    }

    if(mpParentSystem != 0)
    {
        mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
    }
}

void GUIModelObject::calcNameTextPositions(QVector<QPointF> &rPts)
{
    rPts.clear();

    QPointF pt0, pt1, tWH;
    QPointF localCenter = this->boundingRect().center();
    QPointF localWH = this->boundingRect().bottomRight();

    //Create the transformation, and transform points
    QTransform transf;
    transf.rotate(-(this->rotation()));
    if (this->mIsFlipped)
    {
        transf.scale(-1.0,1.0);
    }

    //First we transform the height and width, (also take absolute values for width and height)
    tWH = transf*localWH;
    tWH.setX(fabs(tWH.x()));
    tWH.setY(fabs(tWH.y()));

    //qDebug() <<  " width: " << this->boundingRect().width() << "height: " << this->boundingRect().height()  << " lWH: " << localWH << " tWH: " << tWH;
    //Now we transforme the name text posistions
    //pt0 = top, pt1 = bottom, pts relative loacal center on object
    pt0.rx() = -mpNameText->boundingRect().width()/2.0;
    pt0.ry() = -(tWH.y()/2.0 + mpNameText->boundingRect().height() + mTextOffset);

    pt1.rx() = -mpNameText->boundingRect().width()/2.0;
    pt1.ry() = tWH.y()/2.0 + mTextOffset;

    //    qDebug() << "pt0: " << pt0;
    //    qDebug() << "pt1: " << pt1;
    pt0 = transf * pt0;
    pt1 =  transf * pt1;
    //    qDebug() << "tpt0: " << pt0;
    //    qDebug() << "tpt1: " << pt1;

    //Store transformed positions relative current local origo
    rPts.append(localCenter+pt0);
    rPts.append(localCenter+pt1);

//    qDebug() << "rPts0: " << rPts[0];
//    qDebug() << "rPts1: " << rPts[1];
//    qDebug() << "\n";
}


//! @brief Slot that scales the name text
void GUIModelObject::setNameTextScale(qreal scale)
{
    this->mpNameText->setScale(scale);
}


//! @brief Stores a connector pointer in the connector list
//! @param item Pointer to connector that shall be stored
void GUIModelObject::rememberConnector(GUIConnector *item)
{
    mpGUIConnectorPtrs.append(item);
    connect(this, SIGNAL(objectMoved()), item, SLOT(drawConnector()));
}


//! @brief Removes a connector pointer from the connector list
//! @param item Pointer to connector that shall be forgotten
void GUIModelObject::forgetConnector(GUIConnector *item)
{
    mpGUIConnectorPtrs.removeOne(item);
    disconnect(this, SIGNAL(componentMoved()), item, SLOT(drawConnector()));
}


//! @param Returns the a list with pointers to the connecetors connected to the object
QList<GUIConnector*> GUIModelObject::getGUIConnectorPtrs()
{
    return mpGUIConnectorPtrs;
}


//! @brief Refreshes the displayed name (HopsanCore may have changed it)
void GUIModelObject::refreshDisplayName()
{
    if (mpNameText != 0)
    {
        mpNameText->setPlainText(mAppearanceData.getName());
        mpNameText->setSelected(false);
        //Adjust the position of the text
        this->snapNameTextPosition(mpNameText->pos());
    }
}


//! @brief Returns the name of the object
QString GUIModelObject::getName()
{
    return mAppearanceData.getName();
}


//! @brief Returns a list with pointers to the ports in the object
QList<GUIPort*> &GUIModelObject::getPortListPtrs()
{
    return mPortListPtrs;
}

//void GUIModelObject::setName(QString newName, renameRestrictions renameSettings)
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


//! @brief Sets the name of the object (may be modified by HopsanCore if name already exists)
void GUIModelObject::setDisplayName(QString name)
{
    mAppearanceData.setName(name);
    refreshDisplayName();
}


//! @brief Updates the icon of the object to user or iso style
//! @param gfxType Graphics type that shall be used
void GUIModelObject::setIcon(graphicsType gfxType)
{
    QGraphicsSvgItem *tmp = mpIcon;
    if(gfxType && mAppearanceData.haveIsoIcon())
    {
        mpIcon = new QGraphicsSvgItem(mAppearanceData.getFullIconPath(ISOGRAPHICS) , this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mIconType = ISOGRAPHICS;
    }
    else
    {
        mpIcon = new QGraphicsSvgItem(mAppearanceData.getFullIconPath(USERGRAPHICS), this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mIconType = USERGRAPHICS;
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
        //! @todo calulate this instead of if if if if if .......
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


////! @brief Slot that deselects the object
//void GUIModelObject::deselect()
//{
//    this->setSelected(false);
//}


////! @brief Slot that selects the object
//void GUIModelObject::select()
//{
//    this->setSelected(true);
//}


//! @brief Returns a pointer to the port with the specified name
GUIPort *GUIModelObject::getPort(QString name)
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


//! @brief Virtual function that returns the specified parameter value
//! @param name Name of the parameter to return value from
double GUIModelObject::getParameterValue(QString name)
{
    cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
    return 0;
}


//! @brief Virtual function that returns a vector with the names of the parameteres in the object
QVector<QString> GUIModelObject::getParameterNames()
{
    cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
    return QVector<QString>();
}


//! @brief Virtual function that sets specified parameter to specified value
//! @param name Name of parameter
//! @param value New parameter value
void GUIModelObject::setParameterValue(QString name, double value)
{
    cout << "This function should only be available in GUIComponent and  GUISubsystem" << endl;
    assert(false);
}


//! @brief Saves the GUIModelObject to a text stream
//! @param &rStream Text stream to save into
//! @param prepend String to prepend before object data
void GUIModelObject::saveToTextStream(QTextStream &rStream, QString prepend)
{
    QPointF pos = mapToScene(boundingRect().center());
    if (!prepend.isEmpty())
    {
        rStream << prepend << " ";
    }
    rStream << addQuotes(getTypeName()) << " " << addQuotes(getName()) << " "
            << pos.x() << " " << pos.y() << " " << rotation() << " " << getNameTextPos() << " " << mpNameText->isVisible() << "\n";
}


void GUIModelObject::saveToDomElement(QDomElement &rDomElement)
{
    QDomElement xmlObject = appendDomElement(rDomElement, mHmfTagName);
    saveCoreDataToDomElement(xmlObject);
    saveGuiDataToDomElement(xmlObject);
}

void GUIModelObject::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    appendDomTextNode(rDomElement, HMF_TYPETAG, getTypeName());
    appendDomTextNode(rDomElement, HMF_NAMETAG, getName());
}

QDomElement GUIModelObject::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    //Save GUI realted stuff
    QDomElement xmlGuiStuff = appendDomElement(rDomElement,HMF_HOPSANGUITAG);

    QPointF pos = mapToScene(boundingRect().center());
    appendDomValueNode3(xmlGuiStuff, HMF_POSETAG, pos.x(), pos.y(), rotation());
    appendDomValueNode(xmlGuiStuff, HMF_NAMETEXTPOSTAG, getNameTextPos());
    appendDomValueNode(xmlGuiStuff, HMF_VISIBLETAG, mpNameText->isVisible());
    return xmlGuiStuff;
}


//! @brief Defines what happens when mouse starts hovering the object
void GUIModelObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    GUIObject::hoverEnterEvent(event);
    this->setZValue(12);
    this->showPorts(true);
}


//! @bried Defines what happens when mouse stops hovering the object
void GUIModelObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    GUIObject::hoverLeaveEvent(event);
    this->setZValue(10);
    this->showPorts(false);
}


////! @brief Defines what happens if a mouse key is pressed while hovering an object
//void GUIModelObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//        //Store old positions for all components, in case more than one is selected
//    if(event->button() == Qt::LeftButton)
//    {
//        for(size_t i = 0; i < mpParentSystem->mSelectedGUIObjectsList.size(); ++i)
//        {
//            mpParentSystem->mSelectedGUIObjectsList[i]->mOldPos = mpParentSystem->mSelectedGUIObjectsList[i]->pos();
//        }
//    }

//        //Objects shall not be selectable while creating a connector
//    if(mpParentSystem->mIsCreatingConnector)
//    {
//        this->setSelected(false);
//        this->setActive(false);
//    }
//}


//! @brief Defines what happens if a mouse key is released while hovering an object
void GUIModelObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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

    GUIObject::mouseReleaseEvent(event);
}
//    }

//    QGraphicsWidget::mouseReleaseEvent(event);

//        //Objects shall not be selectable while creating a connector
//    if(mpParentSystem->mIsCreatingConnector)
//    {
//        this->setSelected(false);
//        this->setActive(false);
//    }
//}


//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant GUIModelObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    GUIObject::itemChange(change, value);   //This must be done BEFORE the snapping code to avoid an event loop. This is because snap uses "moveBy()", which triggers a new itemChange event.

    //Snap if objects have moved
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit objectMoved();  //This signal must be emitted  before the snap code, because it updates the connectors which is used to determine whether or not to snap.

            //Snap component if it only has one connector and is dropped close enough (horizontal or vertical) to adjacent component
        if(mpParentSystem != 0 && mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mSnapping &&
           !mpParentSystem->getIsCreatingConnector() && mpParentSystem->mSelectedGUIObjectsList.size() == 1)
        {
                //Vertical snap
            if( (mpGUIConnectorPtrs.size() == 1) &&
                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) )
            {
                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiModelObject == this)
                {
                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.last().x() - mpGUIConnectorPtrs.first()->mPoints.first().x(), 0);
                }
                else
                {
                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x(), 0);
                }
            }
//            else if( (mpGUIConnectorPtrs.size() == 2) &&
//                     (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
//                     (mpGUIConnectorPtrs.last()->getNumberOfLines() < 4) &&
//                     ( ( (this->rotation() == 0 || this->rotation() == 180) &&
//                       (mPortListPtrs.first()->pos().y() == mPortListPtrs.last()->pos().y()) ) ||
//                       ( (this->rotation() == 90 || this->rotation() == 270) &&
//                       (mPortListPtrs.first()->pos().x() == mPortListPtrs.last()->pos().x()) ) ) &&
//                     !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
//                     !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
//                     (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
//                     (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) &&
//                     !(mpGUIConnectorPtrs.last()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.last()->getNumberOfLines() == 2) &&
//                     !(mpGUIConnectorPtrs.last()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.last()->getNumberOfLines() > 1) &&
//                     (abs(mpGUIConnectorPtrs.last()->mPoints.first().x() - mpGUIConnectorPtrs.last()->mPoints.last().x()) < SNAPDISTANCE) &&
//                     (abs(mpGUIConnectorPtrs.last()->mPoints.first().x() - mpGUIConnectorPtrs.last()->mPoints.last().x()) > 0.0) )
//            {
//                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
//                {
//                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.last().x() - mpGUIConnectorPtrs.first()->mPoints.first().x(), 0);
//                }
//                else
//                {
//                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x(), 0);
//                }
//            }

                //Horizontal snap
            if( (mpGUIConnectorPtrs.size() == 1) &&
                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 2) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) < SNAPDISTANCE) &&
                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) > 0.0) )
            {
                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiModelObject == this)
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
//    else if (change == QGraphicsItem::ItemScaleHasChanged)
//    {
//        qDebug() << "item scale has changed";
//    }

//    qDebug() << "change: " << change;

    return value;
}


//! @brief Shows or hides the port, depending on the input boolean and whether or not they are connected
//! @param visible Tells whether the ports shall be shown or hidden
void GUIModelObject::showPorts(bool visible)
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
            if ((*i)->isConnected() || mpParentSystem->mPortsHidden)
            {
                (*i)->hide();
            }
        }
}


////! Figures out the number of a component port by using a pointer to the port.
////! @see getPort(int number)
//int GUIModelObject::getPortNumber(GUIPort *port)
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


//! @brief Rotates a component 90 degrees clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stsack
//! @see rotateTo(qreal angle);
//! @todo try to reuse the code in rotate guiobject
void GUIModelObject::rotate(undoStatus undoSettings)
{
    qDebug() << "this->boundingrect(): " << this->boundingRect();
    qDebug() << "this->mpIcon->boundingrect(): " << this->mpIcon->boundingRect();
    this->setTransformOriginPoint(this->boundingRect().center());
    this->setRotation(normDeg360(this->rotation()+90));

    //Set to zero if 360 (becouse of normDeg above it will not be larger than 360, we just check to be sure no real rounding issus occure in comparisson)
    if (this->rotation() >= 360.0)
    {
        this->setRotation(0.0);
    }


    int tempNameTextPos = mNameTextPos;
    //mpNameText->rotate(-90);
    this->snapNameTextPosition(mpNameText->pos());
    setNameTextPos(tempNameTextPos);

//    for (int i = 0; i != mPortListPtrs.size(); ++i)
//    {
//        if(mPortListPtrs.value(i)->getPortDirection() == TOPBOTTOM)
//            mPortListPtrs.value(i)->setPortDirection(LEFTRIGHT);
//        else
//            mPortListPtrs.value(i)->setPortDirection(TOPBOTTOM);
//        if (mPortListPtrs.value(i)->getPortType() == "POWERPORT")
//        {
//            if(this->rotation() == 0 && !mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(0);
//            else if(this->rotation() == 0 && mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(180);
//            else if(this->rotation() == 90 && !mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(270);
//            else if(this->rotation() == 90 && mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(90);
//            else if(this->rotation() == 180 && !mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(180);
//            else if(this->rotation() == 180 && mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(0);
//            else if(this->rotation() == 270 && !mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(90);
//            else if(this->rotation() == 270 && mIsFlipped)
//                mPortListPtrs.value(i)->setRotation(270);
//        }
//    }

    //! @todo myabe use signals and slots instead
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
//        //! @todo make sure this is not needed, use the actual port heading instead
//        if(mPortListPtrs.value(i)->getPortDirection() == TOPBOTTOM)
//            mPortListPtrs.value(i)->setPortDirection(LEFTRIGHT);
//        else
//            mPortListPtrs.value(i)->setPortDirection(TOPBOTTOM);

        mPortListPtrs.value(i)->refreshPortOverlayRotation();
    }

    //! @todo danger real == real
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

    emit objectMoved();
}


////! @brief Slot that rotates the object to a desired angle (NOT registered in undo stack!)
////! @param angle Angle to rotate to
////! @see rotate(undoStatus undoSettings)
////! @todo Add option to register this in undo stack - someone will want to do this sooner or later anyway
//void GUIModelObject::rotateTo(qreal angle)
//{
//    while(this->rotation() != angle)
//    {
//        this->rotate(NOUNDO);
//    }
//}







//! @brief Slot that flips the object vertically
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see flipHorizontal()
//! @todo Fix name text position when flipping components
void GUIModelObject::flipVertical(undoStatus undoSettings)
{
    this->flipHorizontal(NOUNDO);
    this->rotate(NOUNDO);
    this->rotate(NOUNDO);
    if(undoSettings == UNDO)
    {
        mpParentSystem->mUndoStack->registerVerticalFlip(this);
    }
}


//! @brief Slot that flips the object horizontally
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see flipVertical()
void GUIModelObject::flipHorizontal(undoStatus undoSettings)
{
//    for (int i = 0; i != mPortListPtrs.size(); ++i)
//    {
//        if(mPortListPtrs[i]->getPortType() == "READPORT" ||  mPortListPtrs[i]->getPortType() == "WRITEPORT")
//        {
//            if(this->rotation() == 90 ||  this->rotation() == 270)
//            {
//                mPortListPtrs.value(i)->scale(1,-1);
//                mPortListPtrs.value(i)->translate(0, -mPortListPtrs.value(i)->boundingRect().width());
//            }
//            else
//            {
//                mPortListPtrs.value(i)->scale(-1,1);
//                mPortListPtrs.value(i)->translate(-mPortListPtrs.value(i)->boundingRect().width(), 0);
//            }
//        }
//    }

//    //Flip the entire widget
//    this->scale(-1, 1);
//    if(mIsFlipped)
//    {
//        this->moveBy(-this->boundingRect().width(),0);
//        mIsFlipped = false;
//    }
//    else
//    {
//        this->moveBy(this->boundingRect().width(),0);
//        mIsFlipped = true;
//    }

//        //"Un-flip" the ports and name text
//    for (int i = 0; i != mPortListPtrs.size(); ++i)
//    {
//        if(this->rotation() == 90 ||  this->rotation() == 270)
//        {
//            mPortListPtrs.value(i)->scale(1,-1);
//            mPortListPtrs.value(i)->translate(0, -mPortListPtrs.value(i)->boundingRect().width());
//            this->mpNameText->scale(1,-1);
//        }
//        else
//        {
//            mPortListPtrs.value(i)->scale(-1,1);
//            mPortListPtrs.value(i)->translate(-mPortListPtrs.value(i)->boundingRect().width(), 0);
//            this->mpNameText->scale(-1,1);
//        }
//    }
//    //this->setNameTextPos(mNameTextPos);
//    this->fixTextPosition(mpNameText->pos());


    //Flip the entire widget
//    QGraphicsScale horFlip(this);
//    horFlip.setOrigin(QVector3D(this->boundingRect().center().x(), this->boundingRect().center().y(), this->zValue()));
//    horFlip.setXScale(-1.0);
//    QMatrix4x4 horFlipM;
//    horFlip.applyTo(&horFlipM);
//    QGraphicsItem::setTransform(horFlipM.toTransform(),true);

//    qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,transformOriginPoint: " << this->transformOriginPoint();
//    qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,boundingrectcenter: " << this->boundingRect().center();
//    qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,scenboundingcenter: " << this->sceneBoundingRect().center();

//    qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,transformOriginPoint: " << this->transformOriginPoint();

    QTransform transf;
    transf.reset();
    transf.scale(-1.0, 1.0); //Horizontal flip matrix
    //! @todo transformationorigin point seems to have no effect here for some reason
    //this->setTransformOriginPoint(this->boundingRect().center()); //Make sure we transform around center point
    this->setTransform(transf,true);

    if(mIsFlipped)
    {
        mIsFlipped = false;
        this->moveBy(-this->boundingRect().width(),0);
    }
    else
    {
        mIsFlipped = true;
        this->moveBy(this->boundingRect().width(),0);
    }

    //Refresh port overlay and nametext
    //! @todo myabe use signals and slots instead
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        mPortListPtrs[i]->refreshPortOverlayRotation();
    }
    this->snapNameTextPosition(mpNameText->pos());




    if(undoSettings == UNDO)
    {
        mpParentSystem->mUndoStack->registerHorizontalFlip(this);
    }
}


//! @brief Returns an number of the current name text position
//! @see setNameTextPos(int textPos)
//! @see fixTextPosition(QPointF pos)
int GUIModelObject::getNameTextPos()
{
    return mNameTextPos;
}


//! @brief Moves the name text to the specified name text position
//! @param textPos Number of the desired text position
//! @see getNameTextPos()
//! @see fixTextPosition(QPointF pos)
void GUIModelObject::setNameTextPos(int textPos)
{
    QVector<QPointF> pts;
    this->calcNameTextPositions(pts);
    mNameTextPos = textPos;
    mpNameText->setPos(pts[textPos]);
}


//! @brief Slots that hides the name text of the object
void GUIModelObject::hideName()
{
    mpNameText->setVisible(false);
}


//! @brief Slots that makes the name text of the object visible
void GUIModelObject::showName()
{
    mpNameText->setVisible(true);
}


//! @brief Virtual dummy function that returns the type name of the object (must be reimplemented by children)
QString GUIModelObject::getTypeName()
{
    assert(false);
    return "";
}


void GUIModelObject::deleteMe()
{
    qDebug() << "deleteMe in " << this->getName();
    mpParentSystem->deleteGUIModelObject(this->getName());
}


//! @brief Returns a pointer to the appearance data object
AppearanceData* GUIModelObject::getAppearanceData()
{
    return &mAppearanceData;
}


//! @brief Refreshes the appearance of the object
void GUIModelObject::refreshAppearance()
{
    bool hasActiveSelectionBox = false;
    if (mpSelectionBox != 0)
    {
        hasActiveSelectionBox = mpSelectionBox->isVisible(); //!< @todo This is a bit strange need to fix see todo bellow
        delete mpSelectionBox;
    }

    setIcon(mIconType);
    //qDebug() << pos();
    setGeometry(pos().x(), pos().y(), mpIcon->boundingRect().width(), mpIcon->boundingRect().height());
    //qDebug() << pos();

    //! @todo problem with hovered or active or passive selection box, should maybe make it possible to resize rather than to create a new selection box on refresh
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpIcon->boundingRect().width(), mpIcon->boundingRect().height(),
                                                  QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    if (hasActiveSelectionBox)
    {
        mpSelectionBox->setActive();
    }

    this->refreshDisplayName();
}





//! @brief Construtor for the name text object
//! @param pParent Pointer to the object which the name text belongs to
GUIModelObjectDisplayName::GUIModelObjectDisplayName(GUIModelObject *pParent)
    :   QGraphicsTextItem(pParent)
{
    mpParentGUIModelObject = pParent;
    this->setTextInteractionFlags(Qt::NoTextInteraction);
    this->setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIgnoresTransformations);
}


//! @brief Defines what happens when a mouse button is released (used to update position when text has moved)
void GUIModelObjectDisplayName::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit textMoved(this->pos());
    QGraphicsTextItem::mouseReleaseEvent(event);
}


//! @brief Defines what happens when selection status of name text has changed
//! @param change Type of change (only ItemSelectedHasChanged is used)
QVariant GUIModelObjectDisplayName::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QGraphicsTextItem::itemChange(change, value);

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        qDebug() << "ItemSelectedHasChanged";
        if (this->isSelected())
        {
            mpParentGUIModelObject->mpParentSystem->deselectSelectedNameText();
            connect(this->mpParentGUIModelObject->mpParentSystem, SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
        else
        {
            disconnect(this->mpParentGUIModelObject->mpParentSystem, SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
    }
    return value;
}


//! @brief Slot that deselects the name text
void GUIModelObjectDisplayName::deselect()
{
    this->setSelected(false);
}



//! @brief Constructor for GUI object selection box
//! @param x1 Initial X-coordinate of the top left corner of the parent component
//! @param y1 InitialY-coordinate of the top left corner of the parent component
//! @param x2 InitialX-coordinate of the bottom right corner of the parent component
//! @param y2 InitialY-coordinate of the bottom right corner of the parent component
//! @param activePen Width and color of the box when the parent component is selected.
//! @param hoverPen Width and color of the box when the parent component is hovered by the mouse cursor.
//! @param *parent Pointer to the parent object.
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


//! @brief Makes the box visible and makes it use "active" style
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


//! @brief Makes the box invisible
//! @see setActive();
//! @see setHovered();
void GUIObjectSelectionBox::setPassive()
{
    this->setVisible(false);
}


//! @brief Makes the box visible and makes it use "hovered" style
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


GUIContainerObject::GUIContainerObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUISystem *system, QGraphicsItem *parent)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    //Something
}

void GUIContainerObject::makeRootSystem()
{
    mContainerStatus = ROOT;
}

void GUIContainerObject::updateExternalPortPositions()
{
    //Nothing for now
}

GUIContainerObject::CONTAINERSTATUS GUIContainerObject::getContainerStatus()
{
    return mContainerStatus;
}

//! @brief Use this function to calculate the placement of the ports on a subsystem icon.
//! @param[in] w width of the subsystem icon
//! @param[in] h heigth of the subsystem icon
//! @param[in] angle the angle in radians of the line between center and the actual port
//! @param[out] x the new calculated horizontal placement for the port
//! @param[out] y the new calculated vertical placement for the port
//! @todo rename this one and maybe change it a bit as it is now included in this class, it should be common for subsystems and groups
void GUIContainerObject::calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y)
{
    //! @todo make common PI declaration, maybe also PIhalf or include math.h and use M_PI
    if(angle>3.1415*3.0/2.0)
    {
        x=-max(min(h/tan(angle), w), -w);
        y=max(min(w*tan(angle), h), -h);
    }
    else if(angle>3.1415)
    {
        x=-max(min(h/tan(angle), w), -w);
        y=-max(min(w*tan(angle), h), -h);
    }
    else if(angle>3.1415/2.0)
    {
        x=max(min(h/tan(angle), w), -w);
        y=-max(min(w*tan(angle), h), -h);
    }
    else
    {
        x=max(min(h/tan(angle), w), -w);
        y=max(min(w*tan(angle), h), -h);
    }
}








