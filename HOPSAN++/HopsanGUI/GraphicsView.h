#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QtGui>
#include <QGraphicsView>
#include <QObject>
#include <QMap>
#include <QVector>

#include "common.h"

#include "AppearanceData.h"
#include "GUIObject.h"

//Forward Declarations
class UndoStack;
class GUIPort;
class GUIObject;
class GUIConnector;
class ProjectTab;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(ProjectTab *parent = 0);

    void resetBackgroundBrush();

    ProjectTab *mpParentProjectTab;
    GUISystem *mpSystem;
    QAction *systemPortAction;
    QMenu *menuInsert;
    QColor mBackgroundColor;
    qreal mZoomFactor;
    bool mCtrlKeyPressed;


signals:
    void deleteSelected();
    void keyPressCtrlR();
    void keyPressShiftK();
    void keyPressShiftL();
    void keyPressCtrlUp();
    void keyPressCtrlDown();
    void keyPressCtrlLeft();
    void keyPressCtrlRight();
    void viewClicked();
    void zoomChange();
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
    //! @todo QMap no good means problem if we rename need to loop around the rename like in coore
    void createActions();
    void createMenus();
};

#endif // GRAPHICSVIEW_H
