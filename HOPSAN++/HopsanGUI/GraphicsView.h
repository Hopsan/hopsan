//!
//! @file   GraphicsView.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GraphicsView class
//!
//$Id$

#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QMenu>
#include <QGraphicsView>

#include "common.h"

//Forward Declarations
class ProjectTab;
class GUIContainerObject;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(ProjectTab *parent = 0);

    void updateViewPort();
    void getViewPort(qreal &rX, qreal &rY, qreal &rZoom);
    void setContainerPtr(GUIContainerObject *pContainer);
    GUIContainerObject *getContainerPtr();

    ProjectTab *mpParentProjectTab;

    QAction *systemPortAction;
    QMenu *menuInsert;
    qreal mZoomFactor;
    bool mCtrlKeyPressed;


signals:
    void keyPressDelete();
    void keyPressCtrlR();
    void keyPressShiftK();
    void keyPressShiftL();
    void keyPressCtrlUp();
    void keyPressCtrlDown();
    void keyPressCtrlLeft();
    void keyPressCtrlRight();
    void zoomChange(qreal zoomfactor);
    void systemPortSignal(QPoint position);

public slots:
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void centerView();
    void exportToPDF();

protected:
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void contextMenuEvent ( QContextMenuEvent * event );

private:
    void createActions();
    void createMenus();
    QColor mIsoColor;

    GUIContainerObject *mpContainerObject;
};

#endif // GRAPHICSVIEW_H
