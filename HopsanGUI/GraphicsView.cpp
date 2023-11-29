/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   GraphicsView.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GraphicsView class
//!
//$Id$

#include <QScrollBar>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QCompleter>
#include <QRect>
#include <QLineEdit>
#include <QStringListModel>

#include "global.h"
#include "common.h"
#include "GraphicsView.h"

#include "Configuration.h"
#include "GUIConnector.h"
#include "UndoStack.h"

#include "Widgets/ModelWidget.h"
#include "ModelHandler.h"
#include "MessageHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "LibraryHandler.h"

//Maybe we can remove these to when some cleanup has happened in the code later on (maybe even GUIPort.h)
#include "GUIPort.h"
#include "Widgets/LibraryWidget.h"

//! @class GraphicsView
//! @brief The GraphicsView class is a class which display the content of a scene of components.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsView::GraphicsView(ModelWidget *parent)
        : QGraphicsView(parent)
{
    mpParentModelWidget = parent;
    setContainerPtr(mpParentModelWidget->getTopLevelSystemContainer());

    mIgnoreNextContextMenuEvent = false;
    mIgnoreNextMouseReleaseEvent = false;
    mCtrlKeyPressed = false;
    mShiftKeyPressed = false;
    mLeftMouseButtonPressed = false;
    this->setDragMode(RubberBandDrag);
    this->setInteractive(true);
    this->setEnabled(true);
    this->setAcceptDrops(true);
    this->setMouseTracking(true);
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->setSceneRect(0,0,5000,5000);
    this->centerOn(this->sceneRect().center());

    mIsoColor = QColor("white");
    mZoomFactor = 1.0;

    mpAddComponentLineEdit = new QLineEdit(this);
    mpAddComponentLineEdit->setCompleter(new QCompleter(mpAddComponentLineEdit));
    mpAddComponentLineEdit->completer()->setCompletionMode(QCompleter::PopupCompletion);
    mpAddComponentLineEdit->completer()->setCaseSensitivity(Qt::CaseInsensitive);
#if QT_VERSION >= 0x050000
    mpAddComponentLineEdit->completer()->setFilterMode(Qt::MatchContains);
#endif
    connect(mpAddComponentLineEdit->completer(), SIGNAL(activated(QString)), this, SLOT(insertComponentFromLineEdit()));
    hideAddComponentLineEdit();

    this->updateViewPort();
    this->setRenderHint(QPainter::Antialiasing, gpConfig->getBoolSetting(CFG_ANTIALIASING));
}


//! Defines the right click menu event
void GraphicsView::contextMenuEvent ( QContextMenuEvent * event )
{
    hideAddComponentLineEdit();

    qDebug() << "GraphicsView::contextMenuEvent(), reason = " << event->reason();
    if(!mpContainerObject->isCreatingConnector() && !mIgnoreNextContextMenuEvent)
    {
        if (itemAt(event->pos()))
        {
            QGraphicsView::contextMenuEvent(event);
        }
        else
        {
            QGraphicsView::contextMenuEvent(event);
            QMenu menu(this);
            QAction *addTextBoxAction = menu.addAction("Add Text Box Widget");
            addTextBoxAction->setDisabled(mpParentModelWidget->isEditingLimited() || mpContainerObject->isLocallyLocked());
            QAction *addImageWidgetAction = menu.addAction("Add Image Widget");
            addImageWidgetAction->setDisabled(mpParentModelWidget->isEditingLimited() || mpContainerObject->isLocallyLocked());

            QCursor cursor;
            QAction *selectedAction = menu.exec(cursor.pos());

            if(selectedAction == addTextBoxAction)
            {
                mpContainerObject->getUndoStackPtr()->newPost();
                mpContainerObject->addTextBoxWidget(this->mapToScene(event->pos()).toPoint());
            }
            else if(selectedAction == addImageWidgetAction) {
                mpContainerObject->getUndoStackPtr()->newPost();
                mpContainerObject->addImageWidget(this->mapToScene(event->pos()).toPoint());
            }
        }
    }
    mIgnoreNextContextMenuEvent = false;
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
#if QT_VERSION >= 0x050000
    if(event->button() == Qt::LeftButton && !mpContainerObject->isCreatingConnector() &&
            mpContainerObject->getSelectedModelObjectPtrs().isEmpty() &&
            mpContainerObject->getSelectedGUIWidgetPtrs().isEmpty() &&
            !mpContainerObject->isConnectorSelected() &&
            this->rubberBandRect().isEmpty())
#else
    if(!mpContainerObject->isCreatingConnector() &&
            mpContainerObject->getSelectedModelObjectPtrs().isEmpty() &&
            mpContainerObject->getSelectedGUIWidgetPtrs().isEmpty() &&
            !mpContainerObject->isConnectorSelected())  //We cannot check for rubber band selection on Qt4, not sure how to solve this
#endif
    {
        QCursor cursor;
        QPointF pos = mapFromGlobal(cursor.pos());
        QRect tempRect(pos.x(), pos.y(),300,1);

        // Update lists with type names and display names for quick-add component function
        // (only if contents in library has changed)
        if(mTypeNames.size() != gpLibraryHandler->getLoadedTypeNames().size()) {
            mTypeNames.clear();
            mDisplayNames.clear();
            mTypeNames = gpLibraryHandler->getLoadedTypeNames();
            for(const QString &typeName : mTypeNames)
            {
                ComponentLibraryEntry entry = gpLibraryHandler->getEntry(typeName);
                mDisplayNames << entry.pAppearance->getDisplayName();
            }
        }
        mpAddComponentLineEdit->completer()->setModel(new QStringListModel(mDisplayNames,mpAddComponentLineEdit));
        mpAddComponentLineEdit->setGeometry(pos.x(), pos.y(), 200,30);
        mpAddComponentLineEdit->show();
        mpAddComponentLineEdit->setFocus();

        this->setIgnoreNextMouseReleaseEvent();
    }

    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicsView::insertComponentFromLineEdit()
{
    QString displayName = mpAddComponentLineEdit->text();
    if(!mDisplayNames.contains(displayName)) {
        return;
    }
    QString typeName = mTypeNames[mDisplayNames.indexOf(displayName)];
    gpMessageHandler->addInfoMessage("Adding: "+typeName);
    mpContainerObject->getUndoStackPtr()->newPost();
    mpContainerObject->addModelObject(typeName, mapToScene(mpAddComponentLineEdit->pos()));
    hideAddComponentLineEdit();
}


//! Defines what happens when moving an object in a GraphicsView.
//! @param event contains information of the drag operation.
//! @todo This function seems to do nothing. Can it be removed?
void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    qDebug() << "Accepting: " << event->mimeData()->text();

    event->accept();

//    if (event->mimeData()->hasText())
//    {
//        event->accept();
//    }
//    else
//    {
//        event->ignore();
//    }
}


//! Defines what happens when drop an object in a GraphicsView.
//! @param event contains information of the drop operation.
void GraphicsView::dropEvent(QDropEvent *event)
{
    if(mpParentModelWidget->isEditingLimited() || mpContainerObject->isLocallyLocked())
        return;

    //A HMF file was dropped in the graphics view, so try to open the model
    if(event->mimeData()->hasUrls())
    {
        for(int i=0; i<event->mimeData()->urls().size(); ++i)
        {
            if(event->mimeData()->urls().at(i).toString().endsWith(".hmf"))
            {
                gpModelHandler->loadModel(event->mimeData()->urls().at(i).toString().remove(0,8));
            }
        }
        return;
    }
    if (event->mimeData()->hasText())
    {
        mpParentModelWidget->hasChanged();
        QString text = event->mimeData()->text();

        //Dropped item is a drag copy operation
        if(text == "HOPSANDRAGCOPY")
        {
            //These booleans must be reset here, because they are not automatically reset when dropping things.
            //It doesn't really matter if it will be incorrect, because keeping the shift key pressed after a drop
            //and attempting to do more ctrl-stuff does not make any sense anyway.
            mShiftKeyPressed = false;
            mLeftMouseButtonPressed = false;

            //Paste the drag copy component
            mpContainerObject->paste(mpContainerObject->getDragCopyStackPtr());
            return;
        }

        //Check if dropped item is a plot data string, and attempt to open a plot window if so
        else if(text.startsWith("HOPSANPLOTDATA:"))
        {
            QStringList fields = text.split(":");
            if (fields.size() > 2)
            {
                getContainerPtr()->getLogDataHandler()->plotVariable("", fields[1], fields[2].toInt(), 0);
            }
            return;
        }


        //Dropped item is not a plot data string, so assume it is a component typename
        mpContainerObject->getUndoStackPtr()->newPost();
        event->accept();
        QPointF position = event->pos();
        mpContainerObject->addModelObject(text, this->mapToScene(position.toPoint()));
        this->setFocus();
    }

    this->setDragMode(NoDrag);
}


//! @brief Updates the viewport in case something has changed.
//! Also changes to the correct background color if it is not the correct one.
void GraphicsView::updateViewPort()
{
    if( (mpParentModelWidget->getTopLevelSystemContainer()->getGfxType() == UserGraphics) && (this->backgroundBrush().color() != gpConfig->getBackgroundColor()) )
    {
        this->setBackgroundBrush(gpConfig->getBackgroundColor());
    }
    else if( (mpParentModelWidget->getTopLevelSystemContainer()->getGfxType() == ISOGraphics) && (this->backgroundBrush().color() != mIsoColor) )
    {
        this->setBackgroundBrush(mIsoColor);
    }
    else
    {
        this->viewport()->update();
    }
}

//! @brief Set the system that the view is representing
void GraphicsView::setContainerPtr(SystemObject *pContainer)
{
    mpContainerObject = pContainer;
    setScene(mpContainerObject->getContainedScenePtr());
}


//! @brief Returns a pointer to the container object in the graphics view
SystemObject *GraphicsView::getContainerPtr()
{
    return this->mpContainerObject;
}


//! @brief Returns whether or not ctrl key is pressed
bool GraphicsView::isCtrlKeyPressed()
{
    return mCtrlKeyPressed;
}


//! @brief Returns whether or not shift key is pressed
bool GraphicsView::isShiftKeyPressed()
{
    return mShiftKeyPressed;
}


bool GraphicsView::isLeftMouseButtonPressed()
{
    return mLeftMouseButtonPressed;
}


//! @brief Tells the graphics view to ignore the next context menu event
void GraphicsView::setIgnoreNextContextMenuEvent()
{
    mIgnoreNextContextMenuEvent = true;
}

void GraphicsView::setIgnoreNextMouseReleaseEvent()
{
    mIgnoreNextMouseReleaseEvent = true;
}


//! @brief Sets the zoom factor to specified value
void GraphicsView::setZoomFactor(double zoomFactor)
{
    resetZoom();
    scale(zoomFactor, zoomFactor);
    mZoomFactor = zoomFactor;
    emit zoomChange(zoomFactor);
}


//! @brief Returns the current zoom factor
double GraphicsView::getZoomFactor()
{
    return mZoomFactor;
}

void GraphicsView::clearHighlights()
{
    emit unHighlightAll();
}


//! @brief Returns the vieports center and zoom in the supplied reference variables
void GraphicsView::getViewPort(double &rX, double &rY, double &rZoom) const
{
    rX = (horizontalScrollBar()->value() + width()/2.0 - pos().x()) / mZoomFactor;
    rY = (verticalScrollBar()->value() + height()/2.0 - pos().y()) / mZoomFactor;
    rZoom = mZoomFactor;
}

GraphicsViewPort GraphicsView::getViewPort() const
{
    double x,y,z;
    getViewPort(x,y,z);
    return GraphicsViewPort(x,y,z);
}

void GraphicsView::setViewPort(GraphicsViewPort vp)
{
    setZoomFactor(vp.mZoom);
    centerOn(vp.mCenter);
}


//! Defines what happens when scrolling the mouse in a GraphicsView.
//! @param event contains information of the scrolling operation.
void GraphicsView::wheelEvent(QWheelEvent *event)
{
    // Get value from scroll wheel change
    double wheelDelta;
    if(gpConfig->getInvertWheel())
    {
        wheelDelta = event->delta();
    }
    else
    {
        wheelDelta = -event->delta();
    }

    // Zoom with wheel if ctrl or alt is pressed
    if (event->modifiers().testFlag(Qt::ControlModifier) ||  event->modifiers().testFlag(Qt::AltModifier))
    {
//        double factor = pow(1.41,(-wheelDelta/240.0));
//        this->scale(factor,factor);
//        mZoomFactor = mZoomFactor * factor;
//        emit zoomChange(mZoomFactor);
        if (wheelDelta < 0)
        {
            zoomIn();
        }
        else
        {
            zoomOut();
        }
    }
    // Scroll horizontally with wheel if shift is pressed
    else if(event->modifiers().testFlag(Qt::ShiftModifier))
    {
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()-wheelDelta);
    }
    // Scroll vertically with wheel by default
    else
    {
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value()+wheelDelta);
    }
}


//! Defines what shall happen when various keys or key combinations are pressed.
//! @param event contains information about the key press event.
void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    emit unHighlightAll();

    bool doForwardEvent = true;
    const bool ctrlPressed = event->modifiers().testFlag(Qt::ControlModifier);
    const bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);
    const bool altPressed = event->modifiers().testFlag(Qt::AltModifier);

    //qDebug() << "shiftPressed = " << shiftPressed;
    //qDebug() << "event->key() = " << event->key();

    if (event->key() == Qt::Key_Delete)
    {
        bool allSelectedLocked = true;
        for(ModelObject* pObj : mpContainerObject->getSelectedModelObjectPtrs()) {
            if(!pObj->isLocallyLocked()) {
                allSelectedLocked = false;
                break;
            }
        }
        if((mpContainerObject->isSubObjectSelected() && !allSelectedLocked) || mpContainerObject->isConnectorSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
            mpParentModelWidget->hasChanged();
        }
        emit keyPressDelete();
    }
    else if (event->key() == Qt::Key_Escape)
    {
        mpContainerObject->cancelCreatingConnector();
        hideAddComponentLineEdit();
    }
    else if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        insertComponentFromLineEdit();
        hideAddComponentLineEdit();
    }
    else if (ctrlPressed && event->key() == Qt::Key_0)
    {
        mpContainerObject->assignSection(0);
    }
    else if (ctrlPressed && event->key() == Qt::Key_1)
    {
        mpContainerObject->assignSection(1);
    }
    else if (ctrlPressed && event->key() == Qt::Key_2)
    {
        mpContainerObject->assignSection(2);
    }
    else if (ctrlPressed && event->key() == Qt::Key_3)
    {
        mpContainerObject->assignSection(3);
    }
    else if (ctrlPressed && event->key() == Qt::Key_4)
    {
        mpContainerObject->assignSection(4);
    }
    else if (ctrlPressed && event->key() == Qt::Key_5)
    {
        mpContainerObject->assignSection(5);
    }
    else if (ctrlPressed && event->key() == Qt::Key_6)
    {
        mpContainerObject->assignSection(6);
    }
    else if (ctrlPressed && event->key() == Qt::Key_7)
    {
        mpContainerObject->assignSection(7);
    }
    else if (ctrlPressed && event->key() == Qt::Key_8)
    {
        mpContainerObject->assignSection(8);
    }
    else if (ctrlPressed && event->key() == Qt::Key_9)
    {
        mpContainerObject->assignSection(9);
    }
    else if (event->key() == Qt::Key_0)
    {
        mpContainerObject->selectSection(0);
    }
    else if (event->key() == Qt::Key_1)
    {
        mpContainerObject->selectSection(1);
    }
    else if (event->key() == Qt::Key_2)
    {
        mpContainerObject->selectSection(2);
    }
    else if (event->key() == Qt::Key_3)
    {
        mpContainerObject->selectSection(3);
    }
    else if (event->key() == Qt::Key_4)
    {
        mpContainerObject->selectSection(4);
    }
    else if (event->key() == Qt::Key_5)
    {
        mpContainerObject->selectSection(5);
    }
    else if (event->key() == Qt::Key_6)
    {
        mpContainerObject->selectSection(6);
    }
    else if (event->key() == Qt::Key_7)
    {
        mpContainerObject->selectSection(7);
    }
    else if (event->key() == Qt::Key_8)
    {
        mpContainerObject->selectSection(8);
    }
    else if (event->key() == Qt::Key_9)
    {
        mpContainerObject->selectSection(9);
    }
    else if(ctrlPressed && event->key() == Qt::Key_Up)
    {
        if(mpContainerObject->isSubObjectSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
        }
        emit keyPressCtrlUp();
        doForwardEvent = false;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Down)
    {
        if(mpContainerObject->isSubObjectSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
            mpParentModelWidget->hasChanged();
        }
        emit keyPressCtrlDown();
        doForwardEvent = false;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Left)
    {
        if(mpContainerObject->isSubObjectSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
        }
        emit keyPressCtrlLeft();
        doForwardEvent = false;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Right)
    {
        if(mpContainerObject->isSubObjectSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
            mpParentModelWidget->hasChanged();
        }
        emit keyPressCtrlRight();
        doForwardEvent = false;
    }
    else if (ctrlPressed && event->key() == Qt::Key_A)
    {
        mpContainerObject->selectAll();
    }
    else if (ctrlPressed)
    {
        if(mpContainerObject->isCreatingConnector())
        {
            mpContainerObject->makeConnectorDiagonal(true);
        }
        else
        {
            mCtrlKeyPressed = true;
            this->setDragMode(RubberBandDrag);
        }
    }
    else if(shiftPressed && altPressed && event->key() == Qt::Key_C)
    {
        for(const auto pMO : mpContainerObject->getModelObjects()) {
            if(pMO->getTypeCQS() == "C") {
                pMO->highlight();
            }
        }
    }
    else if(shiftPressed && altPressed && event->key() == Qt::Key_Q)
    {
        for(const auto pMO : mpContainerObject->getModelObjects()) {
            if(pMO->getTypeCQS() == "Q") {
                pMO->highlight();
            }
        }
    }
    else if(shiftPressed && altPressed && event->key() == Qt::Key_S)
    {
        for(const auto pMO : mpContainerObject->getModelObjects()) {
            if(pMO->getTypeCQS() == "S") {
                pMO->highlight();
            }
        }
    }
    else if (shiftPressed)
    {
        mShiftKeyPressed = true;
    }

    if(doForwardEvent) {
        QGraphicsView::keyPressEvent ( event );
    }
}


//! Defines what shall happen when a key is released.
//! @param event contains information about the keypress operation.
void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    emit unHighlightAll();

    // Releasing ctrl key while creating a connector means return from diagonal mode to orthogonal mode.
    if(event->key() == Qt::Key_Control)
    {
        mpContainerObject->makeConnectorDiagonal(false);
    }

    if(event->key() == Qt::Key_Control)
    {
        mCtrlKeyPressed = false;
        this->setDragMode(RubberBandDrag);
    }
    else if(event->key() == Qt::Key_Shift)
    {
        mShiftKeyPressed = false;
    }

    QGraphicsView::keyReleaseEvent(event);
}


//! Defines what happens when the mouse is moving in a GraphicsView.
//! @param event contains information of the mouse moving operation.
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    //! @todo This is a stupid solution, graphics view need to remove the text from the library because mouse move event in library is too slow...
    emit hovered();

    // If creating connector, the end port shall be updated to the mouse position.
    if (mpContainerObject->isCreatingConnector())
    {
        mpContainerObject->updateTempConnector(mapToScene(event->pos()));
    }

    QGraphicsView::mouseMoveEvent(event);
}


//! Defines what happens when clicking in a GraphicsView.
//! @param event contains information of the mouse click operation.
void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    emit unHighlightAll();

    if(!(mpParentModelWidget->isEditingLimited() || mpContainerObject->isLocallyLocked()))
    {
        mLeftMouseButtonPressed = (event->button() == Qt::LeftButton);

        QPointF viewPos = mapFromGlobal(QCursor::pos());

        // No rubber band during connecting:
        if (mpContainerObject->isCreatingConnector())
        {
            this->setDragMode(NoDrag);
        }
        else if(mCtrlKeyPressed)
        {
            hideAddComponentLineEdit();
            // Only enable drag mode when left clicking and not clicking on a workspace object
            if (mLeftMouseButtonPressed && (itemAt(viewPos.toPoint()) == nullptr)) {
                this->setDragMode(ScrollHandDrag);
            }
            else {
                this->setDragMode(NoDrag);
            }
        }
        else
        {
            this->setDragMode(RubberBandDrag);
        }

        // Remove one connector line if right clicking while creating a connector
        if ((event->button() == Qt::RightButton) && mpContainerObject->isCreatingConnector())
        {
            mpContainerObject->removeOneConnectorLine(mapToScene(event->pos()));
        }
        else if ((event->button() == Qt::LeftButton) && mpContainerObject->isCreatingConnector())
        {
            mpContainerObject->addOneConnectorLine(this->mapToScene(event->pos()));
        }
    }

    QGraphicsView::mousePressEvent(event);
}


void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if(mIgnoreNextMouseReleaseEvent) {
        QGraphicsView::mouseReleaseEvent(event);
        mIgnoreNextMouseReleaseEvent = false;
        return;
    }
    if(mpAddComponentLineEdit->isVisible())
    {
        hideAddComponentLineEdit();
    }

    bool createdUndoPost=false;

    for(ModelObject* pSelectedMO : mpContainerObject->getSelectedModelObjectPtrs())
    {
        QPointF currentMOPosition = pSelectedMO->pos();
        if(pSelectedMO->getPreviousPos() != currentMOPosition)
        {
            mpParentModelWidget->hasChanged();
            if(!createdUndoPost)
            {
                mpContainerObject->getUndoStackPtr()->newPost(UNDO_MOVEDMULTIPLE);
                createdUndoPost = true;
            }
            mpContainerObject->getUndoStackPtr()->registerMovedObject(pSelectedMO->getPreviousPos(), pSelectedMO->pos(), pSelectedMO->getName());
            pSelectedMO->rememberPos();
        }
    }
    mLeftMouseButtonPressed = false;
    //qDebug() << "GraphicsView QGraphicsView::mouseReleaseEvent(event)";
    QGraphicsView::mouseReleaseEvent(event);
}


//! Resets zoom factor to 100%.
//! @see zoomIn()
//! @see zoomOut()
void GraphicsView::resetZoom()
{
    this->resetMatrix();
    mZoomFactor = 1.0;
    emit zoomChange(mZoomFactor);
}


//! Increases zoom factor
//! @see resetZoom()
//! @see zoomOut()
void GraphicsView::zoomIn()
{
    const double zoomStep = 1.0+gpConfig->getZoomStep()/100.0;
    this->scale(zoomStep, zoomStep);
    mZoomFactor = mZoomFactor * zoomStep;
    emit zoomChange(mZoomFactor);
}


//! Decreases zoom factor
//! @see resetZoom()
//! @see zoomIn()
void GraphicsView::zoomOut()
{
    const double zoomStep = 1.0/(1.0+gpConfig->getZoomStep()/100.0);
    this->scale(zoomStep, zoomStep);
    mZoomFactor = mZoomFactor * zoomStep;
    emit zoomChange(mZoomFactor);
}


//! Tells the current tab to center the viewport
void GraphicsView::centerView()
{
    this->centerOn(this->sceneRect().center());
}


//! Exports the graphics view to PDF
//! @todo Check if it is possible to export to SVG instead. It appears as it is not possible with the current QT version, but I am not sure.
void GraphicsView::print()
{
    QPrinter printer;
    if (QPrintDialog(&printer).exec() == QDialog::Accepted)
    {
        printer.setPaperSize(QPrinter::A4);
        printer.setOrientation(QPrinter::Landscape);
        printer.setFullPage(false);
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        this->render(&painter);
    }
}


//! Exports the graphics view to PDF
//! @todo Check if it is possible to export to SVG instead. It appears as it is not possible with the current QT version, but I am not sure.
void GraphicsView::exportToPDF()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", gpConfig->getStringSetting(CFG_MODELGFXDIR),
        "Adobe PDF Documents (*.pdf)");
    if ( !fileName.isEmpty() )
    {
        QFileInfo file(fileName);
        gpConfig->setStringSetting(CFG_MODELGFXDIR, file.absolutePath());

        //Here we set A0, Landscape and Fullpage among other things to make sure that components get large enough to be treated as vector graphics
        //Some bug or "feature" makes small objects be converted to bitmaps (ugly)
        //! @todo Try to find out why this happens (se comment above)
        QPrinter *printer = new QPrinter(QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::A4);
        printer->setOrientation(QPrinter::Landscape);
        printer->setFullPage(false);
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(fileName);
        QPainter *painter = new QPainter(printer);
        this->render(painter);
        painter->end();


//The following code will export to Svg instead
        //! @todo Implement a SVG function, and figure out how to always make vector graphics work

//        QSvgGenerator *gen = new QSvgGenerator();
//        gen->setResolution(600000);
//        gen->setFileName(fileName);
//        gen->setSize(QSize(200000, 200000));
//        gen->setViewBox(QRect(0, 0, 200000, 200000));
//        gen->setTitle(tr("SVG Generator Example Drawing"));
//        gen->setDescription(tr("An SVG drawing created by the SVG Generator "
//                                        "Example provided with Qt."));
//        QPainter *svgPainter = new QPainter(gen);
//        this->render(svgPainter);
//        svgPainter->end();
    }
}


//! Exports the graphics view to PNG
void GraphicsView::exportToPNG()
{
    QGraphicsScene *pScene = this->getContainerPtr()->getContainedScenePtr();
    pScene->clearSelection();

    //Ask user for resolution scaling
    QDialog ed(gpMainWindowWidget);
    ed.setWindowTitle(tr("Export to PNG"));
    QGridLayout *pLayout = new QGridLayout();
    ed.setLayout(pLayout);
    pLayout->addWidget(new QLabel(QString("Original size:  %1 x %2")
                                  .arg(pScene->itemsBoundingRect().width())
                                  .arg(pScene->itemsBoundingRect().height())), 0, 0, 1, 2);
    pLayout->addWidget(new QLabel("Resolution scale:", &ed), 1, 0, 1, 1);
    QDoubleSpinBox *pScaleEdit = new QDoubleSpinBox(&ed);
    pScaleEdit->setRange(0.1, 10.0);
    pScaleEdit->setDecimals(3);
    pScaleEdit->setValue(1.0);
    pLayout->addWidget(pScaleEdit, 1, 1, 1, 1);
    pLayout->addWidget(new QLabel("Use white background:", &ed), 2, 0, 1, 1);
    QCheckBox *pWhiteBG = new QCheckBox(&ed);
    pLayout->addWidget(pWhiteBG, 2, 1, 1, 1);
    auto *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &ed);
    pLayout->addWidget(pButtonBox, 3, 0, 1, 2);
    connect(pButtonBox, SIGNAL(accepted()), &ed, SLOT(accept()));
    connect(pButtonBox, SIGNAL(rejected()), &ed, SLOT(reject()));

    //! @todo we really need a help class to build this kind of simple grid based dialog boxes

    // Wait for input, Abort if user pressed cancel
    if (ed.exec() != QDialog::Accepted)
        return;

    double res = pScaleEdit->text().toDouble();

    // Open save dialog to get the file name
    QString fileName = QFileDialog::getSaveFileName(this, "Export File Name", gpConfig->getStringSetting(CFG_MODELGFXDIR), "Portable Network Graphics (*.png)");

    // Attempt to save if user did select a filename
    if(!fileName.isEmpty())
    {
        QFileInfo file(fileName);
        gpConfig->setStringSetting(CFG_MODELGFXDIR, file.absolutePath());

        pScene->setSceneRect(pScene->itemsBoundingRect());
        qDebug() << "itemsBoundingRect(): " << pScene->itemsBoundingRect().width() << "*" << pScene->itemsBoundingRect().height();
        qDebug() << "Desired size: " << pScene->sceneRect().width()*res << "*" << pScene->sceneRect().height()*res;
        QImage image(pScene->sceneRect().width()*res, pScene->sceneRect().height()*res, QImage::Format_ARGB32);
        qDebug() << "Image size: " << image.width() << "*" << image.height();
        if (pWhiteBG->isChecked())
        {
            image.fill(Qt::white);
        }
        else
        {
            image.fill(Qt::transparent);
        }

        QPainter painter(&image);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setWorldTransform(QTransform::fromScale(1,1));
        pScene->render(&painter);
        if(!image.save(fileName))
        {
            gpMessageHandler->addErrorMessage("Failed to export PNG file: " +fileName);
        }
        else
        {
            gpMessageHandler->addInfoMessage("Successfully exported PNG to: " +fileName);
        }
    }
}

void GraphicsView::hideAddComponentLineEdit()
{
    mpAddComponentLineEdit->hide();
    mpAddComponentLineEdit->clear();
}



/////////////////////////////////////////////////////////////////////////////7








//! Constructor.
//! @param parent defines a parent to the new instanced object.
AnimatedGraphicsView::AnimatedGraphicsView(QGraphicsScene *pScene, QWidget *pParent)
        : QGraphicsView(pScene, pParent)
{
    mIgnoreNextContextMenuEvent = false;
    mCtrlKeyPressed = false;
    mShiftKeyPressed = false;
    mLeftMouseButtonPressed = false;
    this->setDragMode(RubberBandDrag);
    this->setInteractive(true);
    this->setEnabled(true);
    this->setAcceptDrops(true);
    this->setMouseTracking(true);
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->setSceneRect(0,0,5000,5000);
    this->centerOn(this->sceneRect().center());

    mIsoColor = QColor("white");
    mZoomFactor = 1.0;

    this->updateViewPort();
    this->setRenderHint(QPainter::Antialiasing, gpConfig->getBoolSetting(CFG_ANTIALIASING));

    connect(this, SIGNAL(hovered()), gpLibraryWidget, SLOT(clearHoverEffects()));
    //connect(this, SIGNAL(hovered()), gpPlotWidget, SLOT(clearHoverEffects()));
}


//! Defines the right click menu event
void AnimatedGraphicsView::contextMenuEvent ( QContextMenuEvent * event )
{
    QGraphicsView::contextMenuEvent(event);
}


//! Defines what happens when moving an object in a AnimatedGraphicsView.
//! @param event contains information of the drag operation.
//! @todo This function seems to do nothing. Can it be removed?
void AnimatedGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    event->ignore();
}




//! @brief Updates the viewport in case something has changed.
//! Also changes to the correct background color if it is not the correct one.
void AnimatedGraphicsView::updateViewPort()
{
    this->setBackgroundBrush(gpConfig->getBackgroundColor());
}



//! @brief Returns whether or not ctrl key is pressed
bool AnimatedGraphicsView::isCtrlKeyPressed()
{
    return mCtrlKeyPressed;
}


//! @brief Returns whether or not shift key is pressed
bool AnimatedGraphicsView::isShiftKeyPressed()
{
    return mShiftKeyPressed;
}


bool AnimatedGraphicsView::isLeftMouseButtonPressed()
{
    return mLeftMouseButtonPressed;
}


//! @brief Tells the graphics view to ignore the next context menu event
void AnimatedGraphicsView::setIgnoreNextContextMenuEvent()
{
    mIgnoreNextContextMenuEvent = true;
}


//! @brief Sets the zoom factor to specified value
void AnimatedGraphicsView::setZoomFactor(double zoomFactor)
{
    resetZoom();
    scale(zoomFactor, zoomFactor);
    mZoomFactor = zoomFactor;
}


//! @brief Returns the current zoom factor
double AnimatedGraphicsView::getZoomFactor()
{
    return mZoomFactor;
}


//! @brief Returns the vieports center and zoom in the supplied reference variables
void AnimatedGraphicsView::getViewPort(double &rX, double &rY, double &rZoom)
{
    rX = (horizontalScrollBar()->value() + width()/2.0 - pos().x()) / mZoomFactor;
    rY = (verticalScrollBar()->value() + height()/2.0 - pos().y()) / mZoomFactor;
    rZoom = mZoomFactor;
}


//! Defines what happens when scrolling the mouse in a AnimatedGraphicsView.
//! @param event contains information of the scrolling operation.
void AnimatedGraphicsView::wheelEvent(QWheelEvent *event)
{
        //Get value from scroll wheel change
    double wheelDelta;
    if(gpConfig->getInvertWheel())
    {
        wheelDelta = event->delta();
    }
    else
    {
        wheelDelta = -event->delta();
    }

        //Zoom with wheel if ctrl or alt is pressed
    if (event->modifiers().testFlag(Qt::ControlModifier) ||  event->modifiers().testFlag(Qt::AltModifier))
    {
        double factor = pow(1.41,(-wheelDelta/240.0));
        this->scale(factor,factor);
        mZoomFactor = mZoomFactor * factor;
        emit zoomChange(mZoomFactor);
    }
        //Scroll horizontally with wheel if shift is pressed
    else if(event->modifiers().testFlag(Qt::ShiftModifier))
    {
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()-wheelDelta);
    }
        //Scroll vertically with wheel by default
    else
    {
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value()+wheelDelta);
    }
}


//! Defines what shall happen when various keys or key combinations are pressed.
//! @param event contains information about the key press event.
void AnimatedGraphicsView::keyPressEvent(QKeyEvent *event)
{
    bool doNotForwardEvent = false;
    bool ctrlPressed = event->modifiers().testFlag(Qt::ControlModifier);
    bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);

    if(ctrlPressed && event->key() == Qt::Key_Up)
    {
        emit keyPressCtrlUp();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Down)
    {
        emit keyPressCtrlDown();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Left)
    {
        emit keyPressCtrlLeft();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Right)
    {
        emit keyPressCtrlRight();
        doNotForwardEvent = true;
    }
    else if (ctrlPressed)
    {
        mCtrlKeyPressed = true;
        this->setDragMode(RubberBandDrag);
    }
    else if (shiftPressed)
    {
        mShiftKeyPressed = true;
    }

    if(!doNotForwardEvent)
    {
        QGraphicsView::keyPressEvent ( event );
    }
}


//! Defines what shall happen when a key is released.
//! @param event contains information about the keypress operation.
void AnimatedGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        mCtrlKeyPressed = false;
        this->setDragMode(RubberBandDrag);
    }
    else if(event->key() == Qt::Key_Shift)
    {
        mShiftKeyPressed = false;
    }

    QGraphicsView::keyReleaseEvent(event);
}


//! Defines what happens when the mouse is moving in a AnimatedGraphicsView.
//! @param event contains information of the mouse moving operation.
void AnimatedGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    emit hovered();

    QGraphicsView::mouseMoveEvent(event);
}


//! Defines what happens when clicking in a AnimatedGraphicsView.
//! @param event contains information of the mouse click operation.
void AnimatedGraphicsView::mousePressEvent(QMouseEvent *event)
{
    mLeftMouseButtonPressed = true;

    if(mCtrlKeyPressed)
    {
        this->setDragMode(ScrollHandDrag);
    }
    else
    {
        this->setDragMode(RubberBandDrag);
    }

    QGraphicsView::mousePressEvent(event);
}


void AnimatedGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if(!mCtrlKeyPressed && this->dragMode() != RubberBandDrag)
    {
        this->setDragMode(NoDrag);
        QCursor cursor;
        cursor.setShape(Qt::ArrowCursor);
    }

    mLeftMouseButtonPressed = false;
    QGraphicsView::mouseReleaseEvent(event);
}


//! Resets zoom factor to 100%.
//! @see zoomIn()
//! @see zoomOut()
void AnimatedGraphicsView::resetZoom()
{
    this->resetMatrix();
    mZoomFactor = 1.0;
    emit zoomChange(mZoomFactor);
}


//! Increases zoom factor by 15%.
//! @see resetZoom()
//! @see zoomOut()
void AnimatedGraphicsView::zoomIn()
{
    this->scale(1.15, 1.15);
    mZoomFactor = mZoomFactor * 1.15;
    emit zoomChange(mZoomFactor);
}


//! Decreases zoom factor by 13.04% (1 - 1/1.15).
//! @see resetZoom()
//! @see zoomIn()
void AnimatedGraphicsView::zoomOut()
{
    this->scale(1/1.15, 1/1.15);
    mZoomFactor = mZoomFactor / 1.15;
    emit zoomChange(mZoomFactor);
}


//! Tells the current tab to center the viewport
void AnimatedGraphicsView::centerView()
{
    this->centerOn(this->sceneRect().center());
}
