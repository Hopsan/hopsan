//$Id$

#include "GUIModelObject.h"

#include "GUISystem.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../GraphicsView.h"
#include "../Utilities/GUIUtilities.h"
#include "../GUIConnector.h"
#include "../GUIPort.h"
#include "../UndoStack.h"
#include "../Configuration.h"
#include "../MainWindow.h"

#include <QSvgRenderer>

//! @brief Constructor for GUI Objects
//! @param position Initial scene coordinates where object shall be placed
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to appearance data object
//! @param startSelected Initial selection status
//! @param gfxType Initial graphics type (user or iso)
//! @param system Pointer to the parent system
//! @param parent Pointer to parent object (not mandatory)
GUIModelObject::GUIModelObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUIContainerObject *system, QGraphicsItem *parent)
        : GUIObject(position, rotation, startSelected, system, parent)
{
        //Initialize variables
    mpIcon = 0;
    mpNameText = 0;
    mTextOffset = 5.0;

        //Set the hmf save tag name
    mHmfTagName = HMF_OBJECTTAG; //!< @todo change this

        //Make a local copy of the appearance data (that can safely be modified if needed)
    if (pAppearanceData != 0)
    {
        mGUIModelObjectAppearance = *pAppearanceData;
    }

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
    if(mpParentContainerObject != 0)
    {
        connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(zoomChange(qreal)), this, SLOT(setNameTextScale(qreal)));
        connect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
        connect(mpParentContainerObject, SIGNAL(hideAllNameText()), this, SLOT(hideName()));
        connect(mpParentContainerObject, SIGNAL(showAllNameText()), this, SLOT(showName()));
        connect(mpParentContainerObject, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIcon(graphicsType)));
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

    if(mpParentContainerObject != 0)
    {
        mpParentContainerObject->mpParentProjectTab->mpGraphicsView->updateViewPort();
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
    mGUIConnectorPtrs.append(item);
    connect(this, SIGNAL(objectMoved()), item, SLOT(drawConnector()));
}


//! @brief Removes a connector pointer from the connector list
//! @param item Pointer to connector that shall be forgotten
void GUIModelObject::forgetConnector(GUIConnector *item)
{
    mGUIConnectorPtrs.removeOne(item);
    disconnect(this, SIGNAL(objectMoved()), item, SLOT(drawConnector()));
}

//! @param Returns a copy of the list with pointers to the connecetors connected to the object
QList<GUIConnector*> GUIModelObject::getGUIConnectorPtrs()
{
    return mGUIConnectorPtrs;
}


//! @brief Refreshes the displayed name (HopsanCore may have changed it)
void GUIModelObject::refreshDisplayName()
{
    if (mpNameText != 0)
    {
        mpNameText->setPlainText(mGUIModelObjectAppearance.getName());
        mpNameText->setSelected(false);
        //Adjust the position of the text
        this->snapNameTextPosition(mpNameText->pos());
    }
}


//! @brief Returns the name of the object
QString GUIModelObject::getName()
{
    return mGUIModelObjectAppearance.getName();
}

void GUIModelObject::setName(QString /*name*/)
{
    //Must be overloaded
    assert(false);
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
////        mGUIModelObjectAppearance.setName(mpParentSystem->getCoreSystemAccessPtr()->renameSubComponent(this->getName(), newName));
////        refreshDisplayName();
////    }
////    else
////    {
////        //Rename
////        mpParentSystem->renameGUIObject(oldName, newName);
////    }

////    mpParentSystem->mGUIObjectMap.erase(mpParentSystem->mGUIObjectMap.find(this->getName()));
////    mGUIModelObjectAppearance.setName(newName);
////    refreshDisplayName();
////    mpParentSystem->mGUIObjectMap.insert(this->getName(), this);
//}


//! @brief Sets the name of the object (may be modified by HopsanCore if name already exists)
void GUIModelObject::setDisplayName(QString name)
{
    mGUIModelObjectAppearance.setName(name);
    refreshDisplayName();
}


//! @brief Updates the icon of the object to user or iso style
//! @param gfxType Graphics type that shall be used
void GUIModelObject::setIcon(graphicsType gfxType)
{
    QString iconPath;
    if(gfxType && mGUIModelObjectAppearance.haveIsoIcon())
    {
        iconPath = mGUIModelObjectAppearance.getFullIconPath(ISOGRAPHICS);
        mIconType = ISOGRAPHICS;
    }
    else
    {
        iconPath = mGUIModelObjectAppearance.getFullIconPath(USERGRAPHICS);
        mIconType = USERGRAPHICS;
    }
    //Check if specified file exist, else use unknown icon
    QFile iconFile(iconPath);
    if (!iconFile.exists())
    {
        iconPath = OBJECTICONPATH + QString("missingcomponenticon.svg");
    }

    //If we have no Icon, create one
    if (mpIcon == 0)
    {
        mpIcon = new QGraphicsSvgItem(iconPath, this);
    }
    else
    {
        //! @todo this seems to load new graphics in old scale, need to fix this
        //If we have an icon, change graphics, and redraww by calling hide and then show
        if (mpIcon->renderer()->load(iconPath))
        {
            mpIcon->hide();
            mpIcon->show();
        }
        else
        {
            qDebug() << "failed to swap icon to: " << iconPath;
        }
    }

    mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);

    if(mGUIModelObjectAppearance.getIconRotationBehaviour() == "ON")
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
double GUIModelObject::getParameterValue(QString /*name*/)
{
    //cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
    return 0;
}


//! @brief Virtual function that returns a vector with the names of the parameteres in the object
QVector<QString> GUIModelObject::getParameterNames()
{
    //cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
    return QVector<QString>();
}


//! @brief Virtual function that sets specified parameter to specified value
//! @param name Name of parameter
//! @param value New parameter value
void GUIModelObject::setParameterValue(QString /*name*/, double /*value*/)
{
    //cout << "This function should only be available in GUIComponent and  GUISubsystem" << endl;
    assert(false);
}


//! @brief Virtual function that sets specified startValue to specified value
//! @param portName Name of port
//! @param variable Name of variable in port
//! @param startValue New parameter value
void GUIModelObject::setStartValue(QString /*portName*/, QString /*variable*/, double /*startValue*/)
{
    //cout << "This function should only be available in GUIComponent and  GUISubsystem" << endl;
    assert(false);
}


void GUIModelObject::mapParameterToSystemParameter(QString /*parameterName*/, QString /*systesmParameterKey*/)
{
    assert(false);
}


//void GUIModelObject::rememberSystemParameterMapping(QString /*name*/)
//{
//    assert(false);
//}


//void GUIModelObject::forgetSystemParameterMapping(QString /*name*/)
//{
//    assert(false);
//}


//bool GUIModelObject::isParameterMappedToSystemParameter(QString /*parameterName*/)
//{
//    assert(false);
//}


//QString GUIModelObject::getSystemParameterKey(QString /*parameterName*/)
//{
//    assert(false);
//}

void GUIModelObject::setModelFileInfo(QFile &/*rFile*/)
{
    //Only available in GUISystem for now
    assert(false);
}


void GUIModelObject::saveToDomElement(QDomElement &rDomElement)
{
    QDomElement xmlObject = appendDomElement(rDomElement, mHmfTagName);
    saveCoreDataToDomElement(xmlObject);
    saveGuiDataToDomElement(xmlObject);
}

void GUIModelObject::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    rDomElement.setAttribute(HMF_TYPETAG, getTypeName());
    rDomElement.setAttribute(HMF_NAMETAG, getName());
}

QDomElement GUIModelObject::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    //Save GUI realted stuff
    QDomElement xmlGuiStuff = appendDomElement(rDomElement,HMF_HOPSANGUITAG);

    QPointF pos = mapToScene(boundingRect().center());

    appendPoseTag(xmlGuiStuff,pos.x(), pos.y(), rotation(), this->mIsFlipped);
    QDomElement nametext = appendDomElement(xmlGuiStuff, HMF_NAMETEXTTAG);
    nametext.setAttribute("position", getNameTextPos());
    nametext.setAttribute("visible", mpNameText->isVisible());

    return xmlGuiStuff;
}


void GUIModelObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    this->buildBaseContextMenu(menu, event->screenPos());

    QGraphicsItem::contextMenuEvent(event);
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
    QList<GUIModelObject *>::iterator it;

        //Loop through all selected objects and register changed positions in undo stack
    bool alreadyClearedRedo = false;
    for(it = mpParentContainerObject->mSelectedGUIObjectsList.begin(); it != mpParentContainerObject->mSelectedGUIObjectsList.end(); ++it)
    {
        if(((*it)->mOldPos != (*it)->pos()) && (event->button() == Qt::LeftButton))
        {
            emit objectMoved();
                //This check makes sure that only one undo post is created when moving several objects at once
            if(!alreadyClearedRedo)
            {
                if(mpParentContainerObject->mSelectedGUIObjectsList.size() > 1)
                {
                    mpParentContainerObject->mUndoStack->newPost("movedmultiple");
                }
                else
                {
                    mpParentContainerObject->mUndoStack->newPost();
                }
                mpParentContainerObject->mpParentProjectTab->hasChanged();
                alreadyClearedRedo = true;
            }
            mpParentContainerObject->mUndoStack->registerMovedObject((*it)->mOldPos, (*it)->pos(), (*it)->getName());
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

QAction *GUIModelObject::buildBaseContextMenu(QMenu &rMenu, QPointF pos)
{
    rMenu.addSeparator();
    QAction *groupAction;
    if (!this->scene()->selectedItems().empty())
        groupAction = rMenu.addAction(tr("Group components"));
    QAction *showNameAction = rMenu.addAction(tr("Show name"));
    showNameAction->setCheckable(true);
    showNameAction->setChecked(mpNameText->isVisible());
    QAction *parameterAction = rMenu.addAction(tr("Properties"));

    QAction *selectedAction = rMenu.exec(pos.toPoint());

    if (selectedAction == parameterAction)
    {
        openPropertiesDialog();
    }
    else if (selectedAction == showNameAction)
    {
        mpParentContainerObject->mUndoStack->newPost();
        if(mpNameText->isVisible())
        {
            this->hideName();
        }
        else
        {
            this->showName();
        }
    }
    else if (selectedAction == groupAction)
    {
        //! @todo fix this if possible
    }
    else
    {
        return selectedAction;
    }

    //Return 0 action if any of the above actions were triggered
    return 0;
}



//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant GUIModelObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    GUIObject::itemChange(change, value);   //This must be done BEFORE the snapping code to avoid an event loop. This is because snap uses "moveBy()", which triggers a new itemChange event.

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if(this->isSelected())
        {
            mpParentContainerObject->mSelectedGUIObjectsList.append(this);
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
        }
        else
        {
            mpParentContainerObject->mSelectedGUIObjectsList.removeAll(this);
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
        }
    }

    //Snap if objects have moved
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit objectMoved();  //This signal must be emitted  before the snap code, because it updates the connectors which is used to determine whether or not to snap.

            //Snap component if it only has one connector and is dropped close enough (horizontal or vertical) to adjacent component
        if(mpParentContainerObject != 0 && gConfig.getSnapping() &&
           !mpParentContainerObject->getIsCreatingConnector() && mpParentContainerObject->mSelectedGUIObjectsList.size() == 1)
        {
                //Vertical snap
            if( (mGUIConnectorPtrs.size() == 1) &&
                (mGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
                (abs(mGUIConnectorPtrs.first()->mPoints.first().x() - mGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
                (abs(mGUIConnectorPtrs.first()->mPoints.first().x() - mGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) )
            {
                if(this->mGUIConnectorPtrs.first()->getStartPort()->mpParentGuiModelObject == this)
                {
                    this->moveBy(mGUIConnectorPtrs.first()->mPoints.last().x() - mGUIConnectorPtrs.first()->mPoints.first().x(), 0);
                }
                else
                {
                    this->moveBy(mGUIConnectorPtrs.first()->mPoints.first().x() - mGUIConnectorPtrs.first()->mPoints.last().x(), 0);
                }
            }
//            else if( (mGUIConnectorPtrs.size() == 2) &&
//                     (mGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
//                     (mGUIConnectorPtrs.last()->getNumberOfLines() < 4) &&
//                     ( ( (this->rotation() == 0 || this->rotation() == 180) &&
//                       (mPortListPtrs.first()->pos().y() == mPortListPtrs.last()->pos().y()) ) ||
//                       ( (this->rotation() == 90 || this->rotation() == 270) &&
//                       (mPortListPtrs.first()->pos().x() == mPortListPtrs.last()->pos().x()) ) ) &&
//                     !(mGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
//                     !(mGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
//                     (abs(mGUIConnectorPtrs.first()->mPoints.first().x() - mGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
//                     (abs(mGUIConnectorPtrs.first()->mPoints.first().x() - mGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) &&
//                     !(mGUIConnectorPtrs.last()->isFirstAndLastDiagonal() && mGUIConnectorPtrs.last()->getNumberOfLines() == 2) &&
//                     !(mGUIConnectorPtrs.last()->isFirstOrLastDiagonal() && mGUIConnectorPtrs.last()->getNumberOfLines() > 1) &&
//                     (abs(mGUIConnectorPtrs.last()->mPoints.first().x() - mGUIConnectorPtrs.last()->mPoints.last().x()) < SNAPDISTANCE) &&
//                     (abs(mGUIConnectorPtrs.last()->mPoints.first().x() - mGUIConnectorPtrs.last()->mPoints.last().x()) > 0.0) )
//            {
//                if(this->mGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
//                {
//                    this->moveBy(mGUIConnectorPtrs.first()->mPoints.last().x() - mGUIConnectorPtrs.first()->mPoints.first().x(), 0);
//                }
//                else
//                {
//                    this->moveBy(mGUIConnectorPtrs.first()->mPoints.first().x() - mGUIConnectorPtrs.first()->mPoints.last().x(), 0);
//                }
//            }

                //Horizontal snap
            if( (mGUIConnectorPtrs.size() == 1) &&
                (mGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mGUIConnectorPtrs.first()->getNumberOfLines() > 2) &&
                (abs(mGUIConnectorPtrs.first()->mPoints.first().y() - mGUIConnectorPtrs.first()->mPoints.last().y()) < SNAPDISTANCE) &&
                (abs(mGUIConnectorPtrs.first()->mPoints.first().y() - mGUIConnectorPtrs.first()->mPoints.last().y()) > 0.0) )
            {
                if(this->mGUIConnectorPtrs.first()->getStartPort()->mpParentGuiModelObject == this)
                {
                    this->moveBy(0, mGUIConnectorPtrs.first()->mPoints.last().y() - mGUIConnectorPtrs.first()->mPoints.first().y());
                }
                else
                {
                    this->moveBy(0, mGUIConnectorPtrs.first()->mPoints.first().y() - mGUIConnectorPtrs.first()->mPoints.last().y());
                }
            }
        }
    }

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
            if ((*i)->isConnected() || mpParentContainerObject->mPortsHidden)
            {
                (*i)->hide();
            }
        }
}


//! @brief Rotates a component 90 degrees clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stsack
//! @see rotateTo(qreal angle);
//! @todo try to reuse the code in rotate guiobject
void GUIModelObject::rotate(undoStatus undoSettings)
{
    //qDebug() << "this->boundingrect(): " << this->boundingRect();
    //qDebug() << "this->mpIcon->boundingrect(): " << this->mpIcon->boundingRect();
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

        mPortListPtrs.value(i)->refreshPortOverlayPosition();
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
        mpParentContainerObject->mUndoStack->registerRotatedObject(this->getName());
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
        mpParentContainerObject->mUndoStack->registerVerticalFlip(this->getName());
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
        mPortListPtrs[i]->refreshPortOverlayPosition();
    }
    this->snapNameTextPosition(mpNameText->pos());

    if(undoSettings == UNDO)
    {
        mpParentContainerObject->mUndoStack->registerHorizontalFlip(this->getName());
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
void GUIModelObject::hideName(undoStatus undoSettings)
{
    bool previousStatus = mpNameText->isVisible();
    mpNameText->setVisible(false);
    if(undoSettings == UNDO && previousStatus == true)
    {
        mpParentContainerObject->mUndoStack->registerNameVisibilityChange(this->getName(), false);
    }
}


//! @brief Slots that makes the name text of the object visible
void GUIModelObject::showName(undoStatus undoSettings)
{
    bool previousStatus = mpNameText->isVisible();
    mpNameText->setVisible(true);
    if(undoSettings == UNDO && previousStatus == false)
    {
        mpParentContainerObject->mUndoStack->registerNameVisibilityChange(this->getName(), true);
    }
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
    mpParentContainerObject->deleteGUIModelObject(this->getName());
}


//! @brief Returns a pointer to the appearance data object
GUIModelObjectAppearance* GUIModelObject::getAppearanceData()
{
    return &mGUIModelObjectAppearance;
}


//! @brief Refreshes the appearance of the object
void GUIModelObject::refreshAppearance()
{
//    //! @todo maybe we can break (some of) this code out and run it in a base class maybe even guiobjekt
//    bool hasActiveSelectionBox = false;
//    if (mpSelectionBox != 0)
//    {
//        hasActiveSelectionBox = mpSelectionBox->isVisible(); //!< @todo This is a bit strange need to fix see todo bellow
//        delete mpSelectionBox;
//    }

    setIcon(mIconType);
    setGeometry(pos().x(), pos().y(), mpIcon->boundingRect().width(), mpIcon->boundingRect().height());

    //Resize the selection box
    mpSelectionBox->setSize(0.0, 0.0, mpIcon->boundingRect().width(), mpIcon->boundingRect().height());

//    //! @todo problem with hovered or active or passive selection box, should maybe make it possible to resize rather than to create a new selection box on refresh
//    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpIcon->boundingRect().width(), mpIcon->boundingRect().height(),
//                                                  QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
//    if (hasActiveSelectionBox)
//    {
//        mpSelectionBox->setActive();
//    }

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
            mpParentGUIModelObject->mpParentContainerObject->deselectSelectedNameText();
            connect(this->mpParentGUIModelObject->mpParentContainerObject, SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
        else
        {
            disconnect(this->mpParentGUIModelObject->mpParentContainerObject, SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
    }
    return value;
}


//! @brief Slot that deselects the name text
void GUIModelObjectDisplayName::deselect()
{
    this->setSelected(false);
}

