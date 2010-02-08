#include <iostream>
#include "ProjectTab.h"
#include <QtGui/QBoxLayout>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>

GraphicsView::GraphicsView(QWidget *parent)
        : QGraphicsView(parent)
{
    this->setAcceptDrops(true);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    std::cout << "Drar runt lite med en komponent: " << qPrintable(event->mimeData()->text()) << std::endl;
}


void GraphicsView::dropEvent(QDropEvent *event)
{
    std::cout << "Släpper en komponent: " << qPrintable(event->mimeData()->text()) << std::endl;

    QString componentName = event->mimeData()->text();

    Component *comp = new Component(componentName);
    this->scene()->addItem(comp);
    comp->setPos(event->pos() + QPoint(-comp->boundingRect().width()/2, -comp->boundingRect().height()/2)); //Funkar inge vidare...

}


GraphicsScene::GraphicsScene()
{

}

Component::Component(QString componentName, QGraphicsItem *parent)
    :   QGraphicsWidget(parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    QGraphicsSvgItem *icon = new QGraphicsSvgItem("../../../cool.svg", this);
    icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));

    QGraphicsTextItem *text = new QGraphicsTextItem(componentName, this);
    text->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    text->setPos(QPointF(-text->boundingRect().width()/2, icon->boundingRect().height()/2));
}


ProjectTab::ProjectTab(QWidget *parent)
    : QWidget(parent)
{
    isSaved = false;

    GraphicsScene *scene = new GraphicsScene();
    GraphicsView  *view  = new GraphicsView();

    view->setScene(scene);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->addWidget(view);
//    tabLayout->addStretch(1);

    setLayout(tabLayout);

}

