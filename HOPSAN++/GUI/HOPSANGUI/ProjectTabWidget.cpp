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

        QString iconDir;
        stream >> iconDir;

        event->accept();

        QCursor cursor;
        QPoint position = this->mapFromGlobal(cursor.pos());

        std::cout << "x=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "y=" << this->mapFromGlobal(cursor.pos()).y() << std::endl;

        guiComponent = new ComponentGuiClass(iconDir,position);
        this->scene()->addItem(guiComponent);

        delete data;

        (qobject_cast<ProjectTab *>(parent()))->hasChanged(); //Ugly!!!
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


//! Defines what happens when the mouse is moving in a GraphicsView.
//! @param event contains information of the mouse moving operation.
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QCursor cursor;
    std::cout << "X=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "Y=" << this->mapFromGlobal(cursor.pos()).y() << std::endl;
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
//    std::cout << "Släpper en komponent: " << qPrintable(event->mimeData()->text()) << std::endl;
//
//    QString componentName = event->mimeData()->text();
//
//    Component *comp = new Component(componentName);
//    this->scene()->addItem(comp);
//    comp->setPos(event->pos() + QPoint(-comp->boundingRect().width()/2, -comp->boundingRect().height()/2)); //Funkar inge vidare...
//
//}

//! Constructor.
GraphicsScene::GraphicsScene()
{

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

    myParent = (qobject_cast<ProjectTabWidget *>(parent)); //Ugly!!!

    GraphicsScene *scene = new GraphicsScene();
    GraphicsView  *view  = new GraphicsView();

    view->setScene(scene);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->addWidget(view);
//    addStretch(1);

//    setWindowModified(true);

    setLayout(tabLayout);

}


//! Should be called when a model has changed in some sense, e.g. a component added or a sinnection has changed.
void ProjectTab::hasChanged()
{
    if (isSaved)
    {
        QString tabName = myParent->tabText(myParent->currentIndex()); //Ugly!!!

        tabName.append("*");
        myParent->setTabText(myParent->currentIndex(), tabName);

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
    setTabsClosable(true);
    numberOfUntitledTabs = 0;

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));

}


//! Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addProjectTab()
{
    //    std::cout << count() << std::endl;
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
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" is already saved")));
    }
    else
    {
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
//! @see closeProjectTab(int index)
//! @return true if closing went ok. false if the user canceled the operation.
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

