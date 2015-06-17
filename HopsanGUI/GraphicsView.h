/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
#include "GraphicsViewPort.h"

//Forward Declarations
class ContainerObject;
class ModelWidget;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(ModelWidget *parent = 0);

    void updateViewPort();
    void getViewPort(double &rX, double &rY, double &rZoom) const;
    GraphicsViewPort getViewPort() const;
    void setViewPort(GraphicsViewPort vp);
    void setContainerPtr(ContainerObject *pContainer);
    ContainerObject *getContainerPtr();
    bool isCtrlKeyPressed();
    bool isShiftKeyPressed();
    bool isLeftMouseButtonPressed();
    void setIgnoreNextContextMenuEvent();
    void setZoomFactor(double zoomFactor);
    double getZoomFactor();
    void clearHighlights();

    ModelWidget *mpParentModelWidget;

signals:
    void keyPressDelete();
    void keyPressShiftK();
    void keyPressShiftL();
    void keyPressCtrlUp();
    void keyPressCtrlDown();
    void keyPressCtrlLeft();
    void keyPressCtrlRight();
    void zoomChange(double zoomfactor);
    void systemPortSignal(QPointF position);
    void hovered();
    void unHighlightAll();

public slots:
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void centerView();
    void print();
    void exportToPDF();
    void exportToPNG();

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
    QColor mIsoColor;
    bool mCtrlKeyPressed;
    bool mShiftKeyPressed;
    bool mLeftMouseButtonPressed;
    bool mIgnoreNextContextMenuEvent;
    double mZoomFactor;

    ContainerObject *mpContainerObject;
};


class AnimatedGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    AnimatedGraphicsView(QGraphicsScene *pScene, QWidget *pParent);

    void updateViewPort();
    void getViewPort(double &rX, double &rY, double &rZoom);
    bool isCtrlKeyPressed();
    bool isShiftKeyPressed();
    bool isLeftMouseButtonPressed();
    void setIgnoreNextContextMenuEvent();
    void setZoomFactor(double zoomFactor);
    double getZoomFactor();

signals:
    void keyPressDelete();
    void keyPressShiftK();
    void keyPressShiftL();
    void keyPressCtrlUp();
    void keyPressCtrlDown();
    void keyPressCtrlLeft();
    void keyPressCtrlRight();
    void zoomChange(double zoomfactor);
    void systemPortSignal(QPointF position);
    void hovered();

public slots:
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void centerView();

protected:
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void contextMenuEvent ( QContextMenuEvent * event );

private:
    QColor mIsoColor;
    bool mCtrlKeyPressed;
    bool mShiftKeyPressed;
    bool mLeftMouseButtonPressed;
    bool mIgnoreNextContextMenuEvent;
    double mZoomFactor;
};


#endif // GRAPHICSVIEW_H
