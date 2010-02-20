//!
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contains classes for Project Tabs
//!
//$Id$

#include "ProjectTabWidget.h"
#include "componentguiclass.h"
#include "HopsanCore.h"

#include <iostream>
#include <math.h>
#include <QBoxLayout>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QMessageBox>




//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsView::GraphicsView(QWidget *parent)
        : QGraphicsView(parent)
{
    this->setAcceptDrops(true);
    this->creatingConnector = false;
    //this->setTransformationAnchor(QGraphicsView::NoAnchor);
}


//! Destructor.
GraphicsView::~GraphicsView()
{
    //delete guiComponent; //Skumt att ta bort en guiComponent?
}


//! Defines what happens when moving an object in a GraphicsView.
//! @param event contains information of the drag operation.
void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-text"))
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
    if (event->mimeData()->hasFormat("application/x-text"))
    {
        QByteArray *data = new QByteArray;
        *data = event->mimeData()->data("application/x-text");

        QDataStream stream(data,QIODevice::ReadOnly);

        QStringList parameterData;
        stream >> parameterData;

        QString componentName = parameterData.at(0);
        QString iconDir = parameterData.at(1);

        event->accept();

        QCursor cursor;
        QPoint position = this->mapFromGlobal(cursor.pos());

        std::cout << "x=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "y=" << this->mapFromGlobal(cursor.pos()).y() << std::endl;

        ComponentGuiClass *guiComponent = new ComponentGuiClass(iconDir,componentName,position,this);

        guiComponent->setPos(event->pos());

        this->scene()->addItem(guiComponent);

        delete data;

    }
}


//! Defines what happens when scrolling the mouse in a GraphicsView.
//! @param event contains information of the scrolling operation.
void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        qreal factor = pow(1.41,(-event->delta()/240.0));
        this->scale(factor,factor);
    }
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        this->setDragMode(QGraphicsView::ScrollHandDrag);
    }
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    this->setDragMode(QGraphicsView::NoDrag);
}

//! Defines what happens when the mouse is moving in a GraphicsView.
//! @param event contains information of the mouse moving operation.
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QCursor cursor;
    std::cout << "X=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "Y=" << this->mapFromGlobal(cursor.pos()).y() << std::endl;
    this->setBackgroundBrush(Qt::NoBrush);

    if (this->creatingConnector)
    {
        //QPointF newPos = this->mapToScene(event->pos());
        line->drawLine(line->startPos, this->mapToScene(event->pos()));
        //qreal myLineWidth = 2.0;
        //QColor myLineColor = QColor("black");
         //= new GraphicsConnectorItem(lineH->startPos.x(), lineH->startPos.y(), lineH->startPos.x(), newPos.y(), myLineWidth, myLineColor);

        //this->scene()->addItem(lineV);
    }
}


void GraphicsView::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::RightButton)
    {
        line->removeLine();
    }

    if (event->button() != Qt::LeftButton)
    {
        return;
    }
    if (this->creatingConnector)
    {
        line->addLine();
    }

    //case InsertLine:
//        lineH = new QGraphicsLineItem(QLineF(event->x(), event->y(), event->x(), event->y()));
//        lineV = new QGraphicsLineItem(QLineF(event->x(), event->y(), event->x(), event->y()));
//        lineH->setPen(QPen(myLineColor, 2));
//        lineV->setPen(QPen(myLineColor, 2));
//        this->scene()->addItem(lineH);
//        this->scene()->addItem(lineV);
//        //break;

        QGraphicsView::mousePressEvent(event);
}


void GraphicsView::addConnector(GraphicsRectItem *rect)
{
    if (!creatingConnector)
    {
        std::cout << "Adding connector";
        QPointF oldPos = rect->mapToScene(rect->boundingRect().center());
        qreal myLineWidth = 2.0;
        QColor myLineColor = QColor("red");
        line = new GraphicsConnectorItem(oldPos.x(), oldPos.y(), oldPos.x(), oldPos.y(), myLineWidth, myLineColor, rect, this->scene());
        //GraphicsConnectorItem *lineV = new GraphicsConnectorItem(oldPos.x(), 0.0, 0.0, 0.0, myLineWidth, myLineColor, rect);
        //this->scene()->addItem(line);
        //this->scene()->addItem(lineV);
        this->creatingConnector = true;
        line->setStartPort(rect);
        //Connecta med componenet och kolla dess movement event istället kansake funkar??
    }
    else
    {
        QPointF newPos = rect->mapToScene(rect->boundingRect().center());
        line->drawLine(line->startPos, newPos);
        rect->getComponent()->addConnector(line);
        line->setEndPort(rect);

        creatingConnector = false;
        //HÄR SKA CONNECTSATSEN LIGGA
    }
}


//GraphicsView::GraphicsView(QWidget *parent)
//        : QGraphicsView(parent)
//{
//    this->setAcceptDrops(true);
//}
//
//void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
//{
//    std::cout << "Drar runt lite med en komponent: " << qPrintable(event->mimeData()->text()) << std::endl;
//}
//
//
//void GraphicsView::dropEvent(QDropEvent *event)
//{
//    std::cout << "SlĂ¤pper en komponent: " << qPrintable(event->mimeData()->text()) << std::endl;
//view
//    QString componentName = event->mimeData()->text();
//
//    Component *comp = new Component(componentName);
//    this->scene()->addItem(comp);
//    comp->setPos(event->pos() + QPoint(-comp->boundingRect().width()/2, -comp->boundingRect().height()/2)); //Funkar inge vidare...
//
//}

//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsScene::GraphicsScene(QObject *parent)
        :   QGraphicsScene(parent)
{
    connect(this, SIGNAL(changed( const QList<QRectF> & )),this->parent(), SLOT(hasChanged()));
}


//Component::Component(QString componentName, QGraphicsItem *parent)
//    :   QGraphicsWidget(parent)
//{
//    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
//
//    QGraphicsSvgItem *icon = new QGraphicsSvgItem("../../../cool.svg", this);
//    icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
//
//    QGraphicsTextItem *text = new QGraphicsTextItem(componentName, this);
//    text->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
//    text->setPos(QPointF(-text->boundingRect().width()/2, icon->boundingRect().height()/2));
//}

//! @class ProjectTab
//! @brief The ProjectTab class is a Widget to contain a simulation model
//!
//! ProjectTab contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTab::ProjectTab(QWidget *parent)
    : QWidget(parent)
{
    isSaved = false;

    pTabContainer = (qobject_cast<ProjectTabWidget *>(parent)); //Ugly!!!

    GraphicsScene *scene = new GraphicsScene(this);
    GraphicsView  *view  = new GraphicsView();

    view->setScene(scene);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->addWidget(view);
//    addStretch(1);

//    setWindowModified(true);

    setLayout(tabLayout);

}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ProjectTab::hasChanged()
{
    if (isSaved)
    {
        QString tabName = pTabContainer->tabText(pTabContainer->currentIndex()); //Ugly!!!

        tabName.append("*");
        pTabContainer->setTabText(pTabContainer->currentIndex(), tabName);

        isSaved = false;
    }
}


//! @class ProjectTabWidget
//! @brief The ProjectTabWidget class is a container class for ProjectTab class
//!
//! ProjectTabWidget contains ProjectTabWidget widgets.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTabWidget::ProjectTabWidget(QWidget *parent)
        :   QTabWidget(parent)
{
    HopsanEssentials* pHopsan = HopsanEssentials::getInstance();

    setTabsClosable(true);
    numberOfUntitledTabs = 0;

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));

}


//! Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addProjectTab()
{
    QString tabName;
    tabName.setNum(numberOfUntitledTabs);
    tabName = QString("Untitled").append(tabName).append(QString("*"));
    addTab(new ProjectTab(this), tabName);

    numberOfUntitledTabs += 1;
}


//! Saves current project.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTab()
{
    saveProjectTab(currentIndex());
}


//! Saves project at index.
//! @param index defines which project to save.
//! @see saveProjectTab()
void ProjectTabWidget::saveProjectTab(int index)
{
    ProjectTab *currentTab = qobject_cast<ProjectTab *>(widget(index));
    QString tabName = tabText(index);

    if (currentTab->isSaved)
    {
        //Nothing to do
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" is already saved")));
    }
    else
    {
        /*Add some "saving code" in the future:
         *
         *
         *
         */
        tabName.chop(1);
        setTabText(index, tabName);
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" saved")));
        std::cout << qPrintable(QString("Project: ").append(tabName).append(QString(" saved"))) << std::endl;
        currentTab->isSaved = true;
    }
}


//! Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(qobject_cast<ProjectTab *>(widget(index))->isSaved))
    {
        QString modelName;
        modelName = tabText(index);
        modelName.chop(1);
        QMessageBox msgBox;
        msgBox.setText(QString("The model '").append(modelName).append("'").append(QString(" is not saved.")));
        msgBox.setInformativeText("Do you want to save your changes before closing?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Save:
            // Save was clicked
            std::cout << "Save and close" << std::endl;
            saveProjectTab(index);
            removeTab(index);
            return true;
        case QMessageBox::Discard:
            // Don't Save was clicked
            removeTab(index);
            return true;
        case QMessageBox::Cancel:
            // Cancel was clicked
            std::cout << "Cancel closing" << std::endl;
            return false;
        default:
            // should never be reached
            return false;
        }
    }
    else
    {
        std::cout << "Closing project: " << qPrintable(tabText(index)) << std::endl;
        //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
        removeTab(index);
        return true;
    }
}


//! Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeProjectTab(int index)
//! @see saveProjectTab()
bool ProjectTabWidget::closeAllProjectTabs()
{
    while(count() > 0)
    {
        setCurrentIndex(count()-1);
        if (!closeProjectTab(count()-1))
        {
            return false;
        }
    }
    return true;
}

