//$Id$

#include "common.h"

#include "GUIWidgets.h"
#include "GUISystem.h"
#include "GUIObject.h"
#include "GraphicsScene.h"


using namespace std;


GUITextWidget::GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent)
    : GUIObject(pos, rot, startSelected, pSystem, pParent)
{
    //! @todo This should not be necessary to do in all GUIWidgets...

    pSystem->scene()->addItem(this);
    this->setPos(pos);
    mpTextItem = new QGraphicsTextItem(text, this);
    mpTextItem->setPos(0,0);
    mpTextItem->show();

    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                                  QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);

}
