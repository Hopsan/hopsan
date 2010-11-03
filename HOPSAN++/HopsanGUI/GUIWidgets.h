//$Id$

#ifndef GUIWIDGETS_H
#define GUIWIDGETS_H

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsWidget>

#include "common.h"

#include "AppearanceData.h"
#include <assert.h>

#include "GUIObject.h"

//class ProjectTabWidget;
//class GraphicsScene;
//class GraphicsView;
//class GUIConnector;
//class GUIModelObjectDisplayName;
//class GUIObjectSelectionBox;
//class GUIPort;
//class GUISystem;

class GUITextWidget : public GUIObject
{
    Q_OBJECT
public:
    GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent=0);

public slots:
    //void deselect();

protected:
    //virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    //virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    QGraphicsTextItem *mpTextItem;
};


#endif // GUIWIDGETS_H
