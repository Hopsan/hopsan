/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   GraphicsView.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GraphicsView class
//!
//$Id$

#include "common.h"
#include "GraphicsView.h"

#include "MainWindow.h"
#include "Configuration.h"
#include "GUIConnector.h"
#include "UndoStack.h"

#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PlotWidget.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"

//Maybe we can remove these to when some cleanup has happend in the code later on (maybe even GUIPort.h)
#include "GUIPort.h"
#include "Widgets/LibraryWidget.h"

//! @class GraphicsView
//! @brief The GraphicsView class is a class which display the content of a scene of components.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsView::GraphicsView(ProjectTab *parent)
        : QGraphicsView(parent)
{
    mpParentProjectTab = parent;
    mpContainerObject = mpParentProjectTab->getSystem();

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
    this->setRenderHint(QPainter::Antialiasing, gConfig.getAntiAliasing());
}


//! Defines the right click menu event
void GraphicsView::contextMenuEvent ( QContextMenuEvent * event )
{
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

            QCursor cursor;
            QAction *selectedAction = menu.exec(cursor.pos());

            if(selectedAction == addTextBoxAction)
            {
                mpContainerObject->getUndoStackPtr()->newPost();
                this->mpContainerObject->addTextBoxWidget(this->mapToScene(event->pos()).toPoint());
            }
        }
    }
    mIgnoreNextContextMenuEvent = false;
}


//! Defines what happens when moving an object in a GraphicsView.
//! @param event contains information of the drag operation.
//! @todo This function seems to do nothing. Can it be removed?
void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
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
    if(!mpParentProjectTab->isEditingEnabled())
        return;

    //A HMF file was dropped in the graphics view, so try to open the model
    if(event->mimeData()->hasUrls())
    {
        for(int i=0; i<event->mimeData()->urls().size(); ++i)
        {
            if(event->mimeData()->urls().at(i).toString().endsWith(".hmf"))
            {
                gpMainWindow->mpProjectTabs->loadModel(event->mimeData()->urls().at(i).toString().remove(0,8));
            }
        }
        return;
    }
    if (event->mimeData()->hasText())
    {
        mpParentProjectTab->hasChanged();
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
        else if(text.startsWith("HOPSANPLOTDATA"))
        {
            gpMainWindow->mpPlotWidget->mpPlotVariableTree->createPlotWindow(text.section("\"", 1, 1), text.section("\"", 3, 3), text.section("\"", 5, 5), ""/*text.section("\"", 7, 7)*/);
            return;
        }


        //Dropped item is not a plot data string, so assume it is a component typename
        mpContainerObject->getUndoStackPtr()->newPost();
        event->accept();
        QPointF position = event->pos();
        mpContainerObject->addModelObject(text, this->mapToScene(position.toPoint()).toPoint());
        this->setFocus();
    }

    this->setDragMode(NoDrag);
}


//! @brief Updates the viewport in case something has changed.
//! Also changes to the correct background color if it is not the correct one.
void GraphicsView::updateViewPort()
{
    if( (mpParentProjectTab->getSystem()->getGfxType() == USERGRAPHICS) && (this->backgroundBrush().color() != gConfig.getBackgroundColor()) )
    {
        this->setBackgroundBrush(gConfig.getBackgroundColor());
    }
    else if( (mpParentProjectTab->getSystem()->getGfxType() == ISOGRAPHICS) && (this->backgroundBrush().color() != mIsoColor) )
    {
        this->setBackgroundBrush(mIsoColor);
    }
    else
    {
        this->viewport()->update();
    }
}

//! @brief Set the system that the view is representing
void GraphicsView::setContainerPtr(ContainerObject *pContainer)
{
    this->mpContainerObject = pContainer;
}


//! @brief Returns a pointer to the container object in the graphics view
ContainerObject *GraphicsView::getContainerPtr()
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


//! @brief Sets the zoom factor to specified value
void GraphicsView::setZoomFactor(double zoomFactor)
{
    resetZoom();
    scale(zoomFactor, zoomFactor);
    mZoomFactor = zoomFactor;
}


//! @brief Returns the current soom factor
double GraphicsView::getZoomFactor()
{
    return mZoomFactor;
}


//! @brief Returns the vieports center and zoom in the supplied reference variables
void GraphicsView::getViewPort(qreal &rX, qreal &rY, qreal &rZoom)
{
    rX = (horizontalScrollBar()->value() + width()/2.0 - pos().x()) / mZoomFactor;
    rY = (verticalScrollBar()->value() + height()/2.0 - pos().y()) / mZoomFactor;
    rZoom = mZoomFactor;
}


//! Defines what happens when scrolling the mouse in a GraphicsView.
//! @param event contains information of the scrolling operation.
void GraphicsView::wheelEvent(QWheelEvent *event)
{
        //Get value from scroll wheel change
    qreal wheelDelta;
    if(gConfig.getInvertWheel())
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
        qreal factor = pow(1.41,(-wheelDelta/240.0));
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
void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    bool doNotForwardEvent = false;
    bool ctrlPressed = event->modifiers().testFlag(Qt::ControlModifier);
    bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);
    //bool altPressed = event->modifiers().testFlag(Qt::AltModifier); //Commented because it is not used, to avoid compile warnings

    //qDebug() << "shiftPressed = " << shiftPressed;
    //qDebug() << "event->key() = " << event->key();

    if (event->key() == Qt::Key_Delete)
    {
        if(mpContainerObject->isSubObjectSelected() || mpContainerObject->isConnectorSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressDelete();
    }
    else if (event->key() == Qt::Key_Escape)
    {
        mpContainerObject->cancelCreatingConnector();
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
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Down)
    {
        if(mpContainerObject->isSubObjectSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlDown();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Left)
    {
        if(mpContainerObject->isSubObjectSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
        }
        emit keyPressCtrlLeft();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Right)
    {
        if(mpContainerObject->isSubObjectSelected())
        {
            mpContainerObject->getUndoStackPtr()->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlRight();
        doNotForwardEvent = true;
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
void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
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
    gpMainWindow->mpLibrary->mpComponentNameField->setText(QString());

        //If creating connector, the end port shall be updated to the mouse position.
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
    if(!mpParentProjectTab->isEditingEnabled())
        return;

    mLeftMouseButtonPressed = true;

    QCursor apa;
    qDebug() << "GraphicsView::mousePressEvent(), pos: " << this->mapToScene(this->mapFromGlobal(apa.pos()));

        //No rubber band during connecting:
    if (mpContainerObject->isCreatingConnector())
    {
        this->setDragMode(NoDrag);
    }
    else if(mCtrlKeyPressed)
    {
        this->setDragMode(ScrollHandDrag);
    }
    else
    {
        this->setDragMode(RubberBandDrag);
    }

    //! Remove one connector line if right clicking while creating a connector
    if (event->button() == Qt::RightButton && mpContainerObject->isCreatingConnector())
    {
        mpContainerObject->removeOneConnectorLine(mapToScene(event->pos()));
    }
    else if  ((event->button() == Qt::LeftButton) && (mpContainerObject->isCreatingConnector()))
    {
        mpContainerObject->addOneConnectorLine(this->mapToScene(event->pos()));
    }

    QGraphicsView::mousePressEvent(event);
}


void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    mLeftMouseButtonPressed = false;
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


//! Increases zoom factor by 15%.
//! @see resetZoom()
//! @see zoomOut()
void GraphicsView::zoomIn()
{
    this->scale(1.15, 1.15);
    mZoomFactor = mZoomFactor * 1.15;
    emit zoomChange(mZoomFactor);
}


//! Decreases zoom factor by 13.04% (1 - 1/1.15).
//! @see resetZoom()
//! @see zoomIn()
void GraphicsView::zoomOut()
{
    this->scale(1/1.15, 1/1.15);
    mZoomFactor = mZoomFactor / 1.15;
    emit zoomChange(mZoomFactor);
}


//! Tells the current tab to center the viewport
void GraphicsView::centerView()
{
    this->centerOn(this->sceneRect().center());
}


//! Exports the graphics view to PDF
//! @todo Check if it is possible to export to SVG instead. It appears as it is not possible with the current QT version, but I am not sure.
void GraphicsView::exportToPDF()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", gConfig.getModelGfxDir(),
        "Adobe PDF Documents (*.svg)");
    if ( !fileName.isEmpty() )
    {
        QFileInfo file(fileName);
        gConfig.setModelGfxDir(file.absolutePath());

        //Here we set A0, Landscape and Fullpage among other things to make sure that components get large enough to be treeted as vector graphics
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
