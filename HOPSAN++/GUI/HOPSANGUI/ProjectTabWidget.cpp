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


GraphicsView::GraphicsView(QWidget *parent)
        : QGraphicsView(parent)
{
    this->setAcceptDrops(true);
    //this->setTransformationAnchor(QGraphicsView::NoAnchor);
}


GraphicsView::~GraphicsView()
{
    delete guiComponent;
}


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
    }
}


void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        qreal factor = pow(1.41,(-event->delta()/240.0));
        this->scale(factor,factor);
    }
}


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


ProjectTab::ProjectTab(QWidget *parent)
    : QWidget(parent)
{
    isSaved = false;

    GraphicsScene *scene = new GraphicsScene();
    GraphicsView  *view  = new GraphicsView();

    view->setScene(scene);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->addWidget(view);
//    addStretch(1);

    setLayout(tabLayout);

}


ProjectTabWidget::ProjectTabWidget(QWidget *parent)
        :   QTabWidget(parent)
{
    setTabsClosable(true);
    numberOfUntitledTabs = 0;

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));

}


void ProjectTabWidget::addProjectTab()
{
    //    std::cout << count() << std::endl;
    QString tabName;
    tabName.setNum(numberOfUntitledTabs);
    tabName = QString("Untitled").append(tabName).append(QString("*"));
    addTab(new ProjectTab(), tabName);

    numberOfUntitledTabs += 1;
}


void ProjectTabWidget::saveProjectTab()
{
    ProjectTab *currentTab = qobject_cast<ProjectTab *>(widget(currentIndex()));
    QString tabName = tabText(currentIndex());

    if (currentTab->isSaved)
    {
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" is already saved")));
    }
    else
    {
        tabName.chop(1);
        setTabText(currentIndex(), tabName);
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" saved")));
        currentTab->isSaved = true;
    }
}


void ProjectTabWidget::closeProjectTab(int index)
{
    if (!(qobject_cast<ProjectTab *>(widget(index))->isSaved))
    {
        //statusBar->showMessage(QString("Project: ").append(projectTabs->tabText(index)).append(QString(" can not be closed since it is not saved")));
    }
    else
    {
        std::cout << "Closing project: " << qPrintable(tabText(index)) << std::endl;
        //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
        removeTab(index);
    }
}
