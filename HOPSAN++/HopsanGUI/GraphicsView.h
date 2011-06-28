/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

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
    bool isCtrlKeyPressed();
    bool isLeftMouseButtonPressed();

    ProjectTab *mpParentProjectTab;

    QAction *systemPortAction;
    QMenu *menuInsert;
    qreal mZoomFactor;



signals:
    void keyPressDelete();
    void keyPressCtrlR();
    void keyPressCtrlE();
    void keyPressShiftK();
    void keyPressShiftL();
    void keyPressCtrlUp();
    void keyPressCtrlDown();
    void keyPressCtrlLeft();
    void keyPressCtrlRight();
    void zoomChange(qreal zoomfactor);
    void systemPortSignal(QPointF position);

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
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void contextMenuEvent ( QContextMenuEvent * event );

private:
    void createActions();
    void createMenus();
    QColor mIsoColor;
    bool mCtrlKeyPressed;
    bool mLeftMouseButtonPressed;
    bool mIgnoreNextContextMenuEvent;

    GUIContainerObject *mpContainerObject;
};

#endif // GRAPHICSVIEW_H
