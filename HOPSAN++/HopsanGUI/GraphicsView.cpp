#include "common.h"

#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
//#include "GUIObject.h"
#include "GUIPort.h"
#include "UndoStack.h"
#include "GUIConnector.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "MessageWidget.h"
#include "LibraryWidget.h"
#include "loadObjects.h"
#include "GUIObjects/GUISystem.h"

using namespace std;

//! @class GraphicsView
//! @brief The GraphicsView class is a class which display the content of a scene of components.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsView::GraphicsView(ProjectTab *parent)
        : QGraphicsView(parent)
{
    mpParentProjectTab = parent;
    mpSystem = mpParentProjectTab->mpSystem;

    mCtrlKeyPressed = false;
    this->setDragMode(RubberBandDrag);
    this->setInteractive(true);
    this->setEnabled(true);
    this->setAcceptDrops(true);

    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->setSceneRect(0,0,5000,5000);
    this->centerOn(this->sceneRect().center());

    mIsoColor = QColor("white");
    mZoomFactor = 1.0;

    this->updateViewPort();

    this->createActions();
    this->createMenus();

    this->setRenderHint(QPainter::Antialiasing, mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mAntiAliasing);
}


//! Creastes the menus
void GraphicsView::createMenus()
{
    menuInsert = new QMenu(this);
    menuInsert->setObjectName("menuInsert");
    menuInsert->setTitle("Insert");
    menuInsert->addAction(systemPortAction);
}


//! Defines the actions
void GraphicsView::createActions()
{
    systemPortAction = new QAction(this);
    systemPortAction->setText("System Port");
}


//! Defines the right click menu event
void GraphicsView::contextMenuEvent ( QContextMenuEvent * event )
{
    if(!mpSystem->getIsCreatingConnector() && !mpSystem->mJustStoppedCreatingConnector)
    {
        if (QGraphicsItem *item = itemAt(event->pos()))
        {
            QGraphicsView::contextMenuEvent(event);
        }
        else
        {
            QGraphicsView::contextMenuEvent(event);
            QMenu menu(this);
            QAction *addTextAction = menu.addAction("Add text widget");
            QAction *addBoxAction = menu.addAction("Add box widget");

            QCursor cursor;
            QAction *selectedAction = menu.exec(cursor.pos());

            if(selectedAction == addTextAction)
            {
                this->mpSystem->addTextWidget(this->mapToScene(event->pos()).toPoint());
            }

            if(selectedAction == addBoxAction)
            {
                this->mpSystem->addBoxWidget(this->mapToScene(event->pos()).toPoint());
            }
        }



        mpSystem->mJustStoppedCreatingConnector = true;
    }
}


//! Defines what happens when moving an object in a GraphicsView.
//! @param event contains information of the drag operation.
void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasText())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}


//! Defines what happens when drop an object in a GraphicsView.
//! @param event contains information of the drop operation.
void GraphicsView::dropEvent(QDropEvent *event)
{
    //if (event->mimeData()->hasFormat("application/x-text"))
    if (event->mimeData()->hasText())
    {
        mpSystem->mUndoStack->newPost();
        mpParentProjectTab->hasChanged();


        QString typestring = event->mimeData()->text();
        GUIModelObjectAppearance* pAppearanceData = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData(typestring);

        //Check if appearnaceData OK otherwihse do not add (usefull if you drag some crap text into the window)
        if(pAppearanceData != 0)
        {
            event->accept();
            QPoint position = event->pos();
            mpSystem->addGUIModelObject(pAppearanceData, this->mapToScene(position).toPoint());
        }
    }
}


//! Updates the viewport, used when something has changed. Also changes to the correct background color if it is not the right one.
void GraphicsView::updateViewPort()
{
    MainWindow *pMainWindow = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;

    if( (mpParentProjectTab->mpSystem->mGfxType == USERGRAPHICS) && (this->backgroundBrush().color() != pMainWindow->mBackgroundColor) )
    {
        this->setBackgroundBrush(pMainWindow->mBackgroundColor);
    }
    else if( (mpParentProjectTab->mpSystem->mGfxType == ISOGRAPHICS) && (this->backgroundBrush().color() != mIsoColor) )
    {
        this->setBackgroundBrush(mIsoColor);
    }
    else
    {
        this->viewport()->update();
    }
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
    if(mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mInvertWheel)
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
    //bool altPressed = event->modifiers().testFlag(Qt::AltModifier);       //Commented because it is not used, to avoid compile warnings

    if (event->key() == Qt::Key_Delete && !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected() || mpSystem->isConnectorSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressDelete();
    }
    else if (ctrlPressed && event->key() == Qt::Key_R && !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlR();
    }
    else if (event->key() == Qt::Key_Escape)
    {
        if(mpSystem->getIsCreatingConnector())
        {
            delete(mpSystem->mpTempConnector);
            mpSystem->setIsCreatingConnector(false);
        }
    }
    else if(shiftPressed && event->key() == Qt::Key_K && !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressShiftK();
    }
    else if(shiftPressed && event->key() == Qt::Key_L && !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressShiftL();
    }
    else if(ctrlPressed && event->key() == Qt::Key_Up)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
        }
        emit keyPressCtrlUp();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Down)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlDown();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Left && !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
        }
        emit keyPressCtrlLeft();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed && event->key() == Qt::Key_Right && !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlRight();
        doNotForwardEvent = true;
    }
    else if (ctrlPressed && event->key() == Qt::Key_A && !mpSystem->mIsRenamingObject)
    {
        mpSystem->selectAll();
    }
    else if (ctrlPressed)
    {
        if (mpSystem->getIsCreatingConnector() && !mpSystem->mpTempConnector->isMakingDiagonal())
        {
            mpSystem->mpTempConnector->makeDiagonal(true);
            mpSystem->mpTempConnector->drawConnector();
            this->updateViewPort();
        }
        else
        {
            mCtrlKeyPressed = true;
            this->setDragMode(RubberBandDrag);
        }
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
    if(event->key() == Qt::Key_Control && mpSystem->getIsCreatingConnector())
    {
        mpSystem->mpTempConnector->makeDiagonal(false);
        mpSystem->mpTempConnector->drawConnector();
        this->updateViewPort();
    }

    if(event->key() == Qt::Key_Control)
    {
        mCtrlKeyPressed = false;
        this->setDragMode(RubberBandDrag);
    }

    QGraphicsView::keyReleaseEvent(event);
}


//! Defines what happens when the mouse is moving in a GraphicsView.
//! @param event contains information of the mouse moving operation.
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    //this->updateViewPort();     //Refresh the viewport
        //If creating connector, the end port shall be updated to the mouse position.
    if (mpSystem->getIsCreatingConnector())
    {
        mpSystem->mpTempConnector->updateEndPoint(this->mapToScene(event->pos()));
        mpSystem->mpTempConnector->drawConnector();
    }
    QGraphicsView::mouseMoveEvent(event);
}


//! Defines what happens when clicking in a GraphicsView.
//! @param event contains information of the mouse click operation.
void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    mpSystem->mJustStoppedCreatingConnector = false;

        //No rubber band during connecting:
    if (mpSystem->getIsCreatingConnector())
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

    if (event->button() == Qt::RightButton && mpSystem->getIsCreatingConnector())
    {
        if((mpSystem->mpTempConnector->getNumberOfLines() == 1 && mpSystem->mpTempConnector->isMakingDiagonal()) ||  (mpSystem->mpTempConnector->getNumberOfLines() == 2 && !mpSystem->mpTempConnector->isMakingDiagonal()))
        {
            mpSystem->mpTempConnector->getStartPort()->setIsConnected(false);
            mpSystem->mpTempConnector->getStartPort()->show();
            mpSystem->mpTempConnector->getStartPort()->getGuiModelObject()->forgetConnector(mpSystem->mpTempConnector);
            mpSystem->setIsCreatingConnector(false);
            mpSystem->mJustStoppedCreatingConnector = true;
        }
        mpSystem->mpTempConnector->removePoint(true);
        if(mpSystem->getIsCreatingConnector())
        {
            mpSystem->mpTempConnector->updateEndPoint(this->mapToScene(event->pos()));
            mpSystem->mpTempConnector->drawConnector();
            this->updateViewPort();
        }
        //qDebug() << "mIsCreatingConnector = " << mIsCreatingConnector;
    }
    else if  ((event->button() == Qt::LeftButton) && (mpSystem->getIsCreatingConnector()))
    {
        qDebug() << "Adding connector point: " << event->pos();
        mpSystem->mpTempConnector->addPoint(this->mapToScene(event->pos()));
    }
    QGraphicsView::mousePressEvent(event);
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
        this, "Export File Name", QString(),
        "Adobe PDF Documents (*.pdf)");
    if ( !fileName.isEmpty() )
    {
        //Here we set A0, Landscape and Fullpage among other things to make sure that components get large enough to be treeted as vector graphics
        //Some bug or "feature" makes small objects be converted to bitmaps (ugly)
        //! @todo Try to find out why this happens (se comment above)
        QPrinter *printer = new QPrinter(QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::A0);
        printer->setOrientation(QPrinter::Landscape);
        printer->setFullPage(true);
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(fileName);
        QPainter *painter = new QPainter(printer);
        this->render(painter);
        painter->end();
    }
}
