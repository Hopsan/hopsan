/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include <QLineEdit>

#include "common.h"
#include "GraphicsViewPort.h"

//Forward Declarations
class SystemObject;
class ModelWidget;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(ModelWidget *parent = 0);
    void setContainerPtr(SystemObject *pContainer);
    SystemObject *getContainerPtr();

    void updateViewPort();
    void getViewPort(double &rX, double &rY, double &rZoom) const;
    GraphicsViewPort getViewPort() const;
    void setViewPort(GraphicsViewPort vp);

    bool isCtrlKeyPressed();
    bool isShiftKeyPressed();
    bool isLeftMouseButtonPressed();
    void setIgnoreNextContextMenuEvent();
    void setIgnoreNextMouseReleaseEvent();
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
    void hideAddComponentLineEdit();

protected:
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void insertComponentFromLineEdit();

private:
    QColor mIsoColor;
    bool mCtrlKeyPressed;
    bool mShiftKeyPressed;
    bool mLeftMouseButtonPressed;
    bool mIgnoreNextContextMenuEvent;
    bool mIgnoreNextMouseReleaseEvent;
    double mZoomFactor;
    QLineEdit *mpAddComponentLineEdit;
    QStringList mTypeNames;
    QStringList mDisplayNames;

    SystemObject *mpContainerObject;
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
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QColor mIsoColor;
    bool mCtrlKeyPressed;
    bool mShiftKeyPressed;
    bool mLeftMouseButtonPressed;
    bool mIgnoreNextContextMenuEvent;
    double mZoomFactor;
};


#endif // GRAPHICSVIEW_H
