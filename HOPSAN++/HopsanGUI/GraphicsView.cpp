#include "common.h"

#include "GraphicsView.h"
#include "GUIUtilities.h"
#include "GUIObject.h"
#include "GUIPort.h"
#include "UndoStack.h"
#include "GUIConnector.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "MessageWidget.h"
#include "LibraryWidget.h"
#include "loadObjects.h"
#include "GUISystem.h"

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

    mZoomFactor = 1.0;
    mBackgroundColor = QColor(Qt::white);
    this->resetBackgroundBrush();

    this->createActions();
    this->createMenus();

        //! @todo Antialiasing could be an option for the user. It makes the view blurred, but will on the other hand makes it look better when zooming out.
    //this->setRenderHint(QPainter::Antialiasing);
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
    if(!mpSystem->mIsCreatingConnector and !mpSystem->mJustStoppedCreatingConnector)
    {
        if (QGraphicsItem *item = itemAt(event->pos()))
            QGraphicsView::contextMenuEvent(event);
        // Context menu when right-clicking:
//        else
//        {
//            QMenu menu(this);
//            menu.addMenu(menuInsert);
//            menu.exec(event->globalPos());
//        }
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
    if (event->mimeData()->hasText())               //! @todo We must check if it is the correct type of text in the drop object, otherwise it will crash if the user drops something that is not a gui object...
    {
        mpSystem->mUndoStack->newPost();
        mpParentProjectTab->hasChanged();

        //QByteArray *data = new QByteArray;
        //*data = event->mimeData()->data("application/x-text");

        QString datastr =  event->mimeData()->text();
        QTextStream stream(&datastr, QIODevice::ReadOnly);

        AppearanceData appearanceData;
        stream >> appearanceData;

        //! @todo Check if appearnaceData OK otherwihse do not add

        if(appearanceData.mIsOK)
        {
            event->accept();
            QPoint position = event->pos();
            mpSystem->addGUIObject(appearanceData, this->mapToScene(position).toPoint());
        }
    }
}


//! @brief dont really know what this is used for
//! @todo Ok this does not seem niceto refresh the view at all, but maybe some parts of the view, dont know realy
void GraphicsView::resetBackgroundBrush()
{
    this->setBackgroundBrush(mBackgroundColor);
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
    if (event->modifiers().testFlag(Qt::ControlModifier) or event->modifiers().testFlag(Qt::AltModifier))
    {
        qreal factor = pow(1.41,(-wheelDelta/240.0));
        this->scale(factor,factor);
        emit zoomChange();
        mZoomFactor = mZoomFactor * factor;
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

    if (event->key() == Qt::Key_Delete and !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected() or mpSystem->isConnectorSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit deleteSelected();
    }
    else if (ctrlPressed and event->key() == Qt::Key_R and !mpSystem->mIsRenamingObject)
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
        if(mpSystem->mIsCreatingConnector)
        {
            delete(mpSystem->mpTempConnector);
            mpSystem->mIsCreatingConnector = false;
        }
    }
    else if(shiftPressed and event->key() == Qt::Key_K and !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressShiftK();
    }
    else if(shiftPressed and event->key() == Qt::Key_L and !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressShiftL();
    }
    else if(ctrlPressed and event->key() == Qt::Key_Up)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
        }
        emit keyPressCtrlUp();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed and event->key() == Qt::Key_Down)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlDown();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed and event->key() == Qt::Key_Left and !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
        }
        emit keyPressCtrlLeft();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed and event->key() == Qt::Key_Right and !mpSystem->mIsRenamingObject)
    {
        if(mpSystem->isObjectSelected())
        {
            mpSystem->mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlRight();
        doNotForwardEvent = true;
    }
    else if (ctrlPressed and event->key() == Qt::Key_A and !mpSystem->mIsRenamingObject)
    {
        mpSystem->selectAll();
    }
    else if (ctrlPressed)
    {
        if (mpSystem->mIsCreatingConnector and !mpSystem->mpTempConnector->isMakingDiagonal())
        {
            mpSystem->mpTempConnector->makeDiagonal(true);
            mpSystem->mpTempConnector->drawConnector();
            this->resetBackgroundBrush();
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
    if(event->key() == Qt::Key_Control and mpSystem->mIsCreatingConnector)
    {
        mpSystem->mpTempConnector->makeDiagonal(false);
        mpSystem->mpTempConnector->drawConnector();
        this->resetBackgroundBrush();
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
    QGraphicsView::mouseMoveEvent(event);

    this->resetBackgroundBrush();     //Refresh the viewport

        //If creating connector, the end port shall be updated to the mouse position.
    if (mpSystem->mIsCreatingConnector)
    {
        mpSystem->mpTempConnector->updateEndPoint(this->mapToScene(event->pos()));
        mpSystem->mpTempConnector->drawConnector();
    }
}


//! Defines what happens when clicking in a GraphicsView.
//! @param event contains information of the mouse click operation.
void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    emit viewClicked();
    mpSystem->mJustStoppedCreatingConnector = false;

        //No rubber band during connecting:
    if (mpSystem->mIsCreatingConnector)
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

    if (event->button() == Qt::RightButton and mpSystem->mIsCreatingConnector)
    {
        if((mpSystem->mpTempConnector->getNumberOfLines() == 1 and mpSystem->mpTempConnector->isMakingDiagonal()) or (mpSystem->mpTempConnector->getNumberOfLines() == 2 and !mpSystem->mpTempConnector->isMakingDiagonal()))
        {
            mpSystem->mpTempConnector->getStartPort()->isConnected = false;
            mpSystem->mpTempConnector->getStartPort()->show();
            mpSystem->mIsCreatingConnector = false;
            mpSystem->mJustStoppedCreatingConnector = true;
        }
        mpSystem->mpTempConnector->removePoint(true);
        if(mpSystem->mIsCreatingConnector)
        {
            mpSystem->mpTempConnector->updateEndPoint(this->mapToScene(event->pos()));
            mpSystem->mpTempConnector->drawConnector();
            this->resetBackgroundBrush();
        }
        //qDebug() << "mIsCreatingConnector = " << mIsCreatingConnector;
    }
    else if  ((event->button() == Qt::LeftButton) && (mpSystem->mIsCreatingConnector))
    {
        mpSystem->mpTempConnector->addPoint(this->mapToScene(event->pos()));
    }
    QGraphicsView::mousePressEvent(event);
}




//! @todo This is not used anywhere and can probably be removed. Why would you want to do it like this?
//void GraphicsView::setScale(const QString &scale)
//{
//    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
//    QMatrix oldMatrix = this->matrix();
//    this->resetMatrix();
//    this->translate(oldMatrix.dx(), oldMatrix.dy());
//    this->scale(newScale, newScale);
//}


//! Resets zoom factor to 100%.
//! @see zoomIn()
//! @see zoomOut()
void GraphicsView::resetZoom()
{
    this->resetMatrix();
    mZoomFactor = 1.0;
    emit zoomChange();
}


//! Increases zoom factor by 15%.
//! @see resetZoom()
//! @see zoomOut()
void GraphicsView::zoomIn()
{
    this->scale(1.15, 1.15);
    mZoomFactor = mZoomFactor * 1.15;
    emit zoomChange();
}


//! Decreases zoom factor by 13.04% (1 - 1/1.15).
//! @see resetZoom()
//! @see zoomIn()
void GraphicsView::zoomOut()
{
    this->scale(1/1.15, 1/1.15);
    mZoomFactor = mZoomFactor / 1.15;
    emit zoomChange();
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
