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

GraphicsView::GraphicsView(QWidget *parent)
        : QGraphicsView(parent)
{
    this->setAcceptDrops(true);
    //this->setTransformationAnchor(QGraphicsView::NoAnchor);
}


GraphicsView::~GraphicsView()
{
    //delete guiComponent; //Skumt att ta bort en guiComponent?
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

//! @class ProjectTab
//! @brief The ProjectTab class is a Widget to contain a simulation model
//!
//! ProjectTab contains a drawing space to create models.
//!

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
    saveProjectTab(currentIndex());
}


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
    }
}


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

