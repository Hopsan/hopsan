#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QtGui>
#include <QGraphicsView>
#include <QObject>
#include <QMap>
#include <QVector>

#include "AppearanceData.h"

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
    //~GraphicsView();
    bool mIsCreatingConnector;
    bool mPortsHidden;
    bool mIsRenamingObject;
    GUIObject *getGUIObject(QString name);
    GUIConnector *getTempConnector();
    void resetBackgroundBrush();
    void deselectAllGUIObjects();
    void deselectAllConnectors();

    ProjectTab *mpParentProjectTab;
    typedef QMap<QString, GUIObject*> GUIObjectMapT;
    GUIObjectMapT mGUIObjectMap;
    QVector<GUIConnector *> mConnectorVector;
    QAction *systemPortAction;
    QMenu *menuInsert;
    QColor mBackgroundColor;
    //QPen getPen(QString situation, QString type, QString style);
    UndoStack *undoStack;
    qreal mZoomFactor;

signals:
    void keyPressDelete();
    void keyPressCtrlR();
    void keyPressShiftK();
    void keyPressShiftL();
    void keyPressCtrlUp();
    void keyPressCtrlDown();
    void keyPressCtrlLeft();
    void keyPressCtrlRight();
    void viewClicked();
    void checkMessages();
    void systemPortSignal(QPoint position);
    void zoomChange();

public slots:
    GUIObject* addGUIObject(AppearanceData appearanceData, QPoint position, qreal rotation=0, bool startSelected = true, bool doNotRegisterUndo = false);
    void deleteGUIObject(QString componentName, bool noUnDo=false);
    bool haveGUIObject(QString name);
    void renameGUIObject(QString oldName, QString newName, bool noUnDo=false);
    void addSystemPort();
    void addConnector(GUIPort *pPort, bool doNotRegisterUndo=false);
    void removeConnector(GUIConnector* pConnector, bool doNotRegisterUndo=false);
    GUIConnector* findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    void cutSelected();
    void copySelected();
    void paste();
    void selectAll();
    void deSelectAll();
    //void setScale(const QString &scale);
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

    void contextMenuEvent ( QContextMenuEvent * event );

private:
    GUIObject *mpTempGUIObject;
    GUIConnector *mpTempConnector;
    QString *mpCopyData;
    //! @todo QMap no good means problem if we rename need to loop around the rename like in coore
    bool isObjectSelected();
    bool isConnectorSelected();
    void createActions();
    void createMenus();
    void addSystemPort(QPoint position, QString name=QString(), bool startSelected = false);
    bool mJustStoppedCreatingConnector;
    bool mCtrlKeyPressed;
};

#endif // GRAPHICSVIEW_H
