//!
//! @file   ProjectTabWidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$

#ifndef PROJECTTABWIDGET_H
#define PROJECTTABWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTabWidget>
#include <map>

#include "MessageWidget.h"
#include "AppearanceData.h"


class GUIPort;
class GUIConnector;
class ProjectTab;

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    GraphicsScene(ProjectTab *parent = 0);
    qreal TestVar;

    ProjectTab *mpParentProjectTab;
};


class HopsanEssentials; //Forward declaration
class ComponentSystem;
class GUIComponent;
class GUIObject;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(HopsanEssentials *hopsan, ComponentSystem *model, ProjectTab *parent = 0);
    //~GraphicsView();
    bool mIsCreatingConnector;
    GUIObject *getGUIObject(QString name);
    GUIConnector *getTempConnector();
    HopsanEssentials *mpHopsan;
    ComponentSystem *mpModel;
    ProjectTab *mpParentProjectTab;
    QMap<QString, GUIObject *> mGUIObjectMap;
    QMap<QString, GUIConnector *> mConnectionMap;
    ComponentSystem *getModelPointer();
    QAction *systemPortAction;
    QMenu *menuInsert;
    QColor mBackgroundColor;
    QPen getPen(QString situation, QString type, QString style);

signals:
    void keyPressDelete();
    void keyPressR();
    void keyPressCtrlA();
    void keyPressCtrlS();
    void keyPressUp();
    void keyPressDown();
    void keyPressLeft();
    void keyPressRight();
    void viewClicked();
    void checkMessages();
    void systemPortSignal(QPoint position);

public slots:
    void addGUIObject(QString componentTypeName, AppearanceData appearanceData, QPoint position, qreal rotation = 0, QString name=QString(), bool startSelected=true);
    void deleteGUIObject(QString componentName);
    bool haveGUIObject(QString name);
    void renameGUIObject(QString oldName, QString newName);
    void addSystemPort();
    void addConnector(GUIPort *pPort);
    void removeConnector(GUIConnector* pConnector);
    void cutSelected();
    void copySelected();
    void paste();
    void selectAll();
    void setScale(const QString &scale);
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void hideNames();
    void showNames();


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
    QStringList mCopyData;
    QList<int> mCopyDataRot;
    QList<QPointF> mCopyDataPos;
    //! @todo QMap no good means problem if we rename need to loop around the rename like in coore
    void createActions();
    void createMenus();
    void addSystemPort(QPoint position, QString name=QString(), bool startSelected = false);
    QPen mPrimaryPenPowerUser;
    QPen mActivePenPowerUser;
    QPen mHoverPenPowerUser;
    QPen mPrimaryPenSignalUser;
    QPen mActivePenSignalUser;
    QPen mHoverPenSignalUser;
    QPen mPrimaryPenPowerIso;
    QPen mActivePenPowerIso;
    QPen mHoverPenPowerIso;
    QPen mPrimaryPenSignalIso;
    QPen mActivePenSignalIso;
    QPen mHoverPenSignalIso;
 };


class ProjectTabWidget; //Forward declaration

class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(ProjectTabWidget *parent = 0);
    bool mIsSaved;
    QString mModelFileName;
    bool useIsoGraphics;
    ProjectTabWidget *mpParentProjectTabWidget;
    ComponentSystem *mpComponentSystem;

    GraphicsView *mpGraphicsView;
    GraphicsScene *mpGraphicsScene;

public slots:
    void hasChanged();

signals:
    void checkMessages();

};


class MainWindow;

class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(MainWindow *parent = 0);

    ProjectTab *getCurrentTab();

    MainWindow *mpParentMainWindow;

    HopsanEssentials *mpHopsan;

    size_t mNumberOfUntitledTabs;

public slots:
    void addProjectTab(ProjectTab *projectTab, QString tabName="Untitled");
    void addNewProjectTab(QString tabName="Untitled");
    void saveProjectTab();
    void saveProjectTabAs();
    void saveProjectTab(int index, bool saveAs);
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();
    void simulateCurrent();
    void loadModel();
    void saveModel(bool saveAs);
    void setIsoGraphics(bool value);
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void hideNames();
    void showNames();

signals:
    void checkMessages();

};

#endif // PROJECTTABWIDGET_H
