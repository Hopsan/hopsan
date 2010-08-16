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
    bool mIsCreatingConnector;                  //Till subsystem
    bool mPortsHidden;                          //Till subsystem
    bool mIsRenamingObject;                     //Till subsystem
    bool mUndoDisabled;                         //Till subsystem
    GUIObject *getGUIObject(QString name);      //Till subsystem
    void resetBackgroundBrush();

    ProjectTab *mpParentProjectTab;             //Behövs den längre?
    typedef QMap<QString, GUIObject*> GUIObjectMapT;        //Till subsystem
    GUIObjectMapT mGUIObjectMap;                            //Till subsystem
    QVector<GUIConnector *> mConnectorVector;               //Till subsystem
    QAction *systemPortAction;
    QMenu *menuInsert;
    QColor mBackgroundColor;
    UndoStack *mUndoStack;                      //Till subsystem
    qreal mZoomFactor;
    GUIObject* addGUIObject(AppearanceData appearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, undoStatus undoSettings = UNDO);  //Till subsystem
    void deleteGUIObject(QString componentName, undoStatus undoSettings=UNDO);  //Till subsystem
    GUIConnector* findConnector(QString startComp, QString startPort, QString endComp, QString endPort);    //Till subsystem
    bool haveGUIObject(QString name);       //Till subsystem
    void renameGUIObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);   //Till subsystem
    void removeConnector(GUIConnector* pConnector, undoStatus undoSettings=UNDO);           //Till subsystem


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
    void deselectAllNameText();             //Till subsystem
    void deselectAllGUIObjects();           //Till subsystem
    void deselectAllGUIConnectors();        //Till subsystem

public slots:
    void addSystemPort();                   //Till subsystem
    void addConnector(GUIPort *pPort, undoStatus undoSettings=UNDO);    //Till subsystem
    void cutSelected();                     //Till subsystem
    void copySelected();                    //Till subsystem
    void paste();                           //Till subsystem
    void selectAll();                       //Till subsystem
    void deselectAll();                     //Till subsystem
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void hideNames();                       //Till subsystem
    void showNames();                       //Till subsystem
    void hidePorts(bool doIt);              //Till subsystem
    void undo();                            //Till subsystem
    void redo();                            //Till subsystem
    void clearUndo();                       //Till subsystem
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
    GUIObject *mpTempGUIObject;             //Till subsystem
    GUIConnector *mpTempConnector;          //Till subsystem
    QString *mpCopyData;                    //Till subsystem
    //! @todo QMap no good means problem if we rename need to loop around the rename like in coore
    bool isObjectSelected();                //Till subsystem
    bool isConnectorSelected();             //Till subsystem
    void createActions();
    void createMenus();
    void addSystemPort(QPoint position, QString name=QString(), selectionStatus startSelected = DESELECTED);    //Till subsystem
    bool mJustStoppedCreatingConnector;     //Till subsystem
    bool mCtrlKeyPressed;                   //Till subsystem
};

#endif // GRAPHICSVIEW_H
