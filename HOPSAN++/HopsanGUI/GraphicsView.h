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
    bool mIsCreatingConnector;
    bool mPortsHidden;
    bool mIsRenamingObject;
    bool mUndoDisabled;
    GUIObject *getGUIObject(QString name);
    void resetBackgroundBrush();

    ProjectTab *mpParentProjectTab;
    typedef QMap<QString, GUIObject*> GUIObjectMapT;
    GUIObjectMapT mGUIObjectMap;
    QVector<GUIConnector *> mConnectorVector;
    QAction *systemPortAction;
    QMenu *menuInsert;
    QColor mBackgroundColor;
    UndoStack *mUndoStack;
    qreal mZoomFactor;
    GUIObject* addGUIObject(AppearanceData appearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, undoStatus undoSettings = UNDO);
    void deleteGUIObject(QString componentName, undoStatus undoSettings=UNDO);
    GUIConnector* findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    bool haveGUIObject(QString name);
    void renameGUIObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    void removeConnector(GUIConnector* pConnector, undoStatus undoSettings=UNDO);


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
    void checkMessages();
    void systemPortSignal(QPoint position);
    void deselectAllNameText();
    void deselectAllGUIObjects();
    void deselectAllGUIConnectors();

public slots:
    void addSystemPort();
    void addConnector(GUIPort *pPort, undoStatus undoSettings=UNDO);
    void cutSelected();
    void copySelected();
    void paste();
    void selectAll();
    void deselectAll();
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void hideNames();
    void showNames();
    void hidePorts(bool doIt);
    void undo();
    void redo();
    void clearUndo();
    void exportPDF();

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
    GUIObject *mpTempGUIObject;
    GUIConnector *mpTempConnector;
    QString *mpCopyData;
    //! @todo QMap no good means problem if we rename need to loop around the rename like in coore
    bool isObjectSelected();
    bool isConnectorSelected();
    void createActions();
    void createMenus();
    void addSystemPort(QPoint position, QString name=QString(), selectionStatus startSelected = DESELECTED);
    bool mJustStoppedCreatingConnector;
    bool mCtrlKeyPressed;
};

#endif // GRAPHICSVIEW_H
