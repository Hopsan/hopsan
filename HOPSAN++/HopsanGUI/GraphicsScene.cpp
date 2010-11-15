#include "GraphicsScene.h"

#include "ProjectTabWidget.h"

//! @class GraphicsScene
//! @brief The GraphicsScene class is a container for graphicsl components in a simulationmodel.
//!

//! @todo do we really need this class at all, i dont think so /Peter

//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsScene::GraphicsScene(ProjectTab *parent)
        :   QGraphicsScene(parent)
{
    mpParentProjectTab = parent;
    //setSceneRect(0.0, 0.0, 800.0, 600.0);   //! I think this is unnecessary, since we override it in GraphicsView...
    //connect(this, SIGNAL(changed( const QList<QRectF> & )),this->parent(), SLOT(hasChanged()));
}
