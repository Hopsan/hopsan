//!
//! @file   GUIContainerObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI Container class (base class for Systems and Groups)
//!
//$Id$

#ifndef GUICONTAINEROBJECT_H
#define GUICONTAINEROBJECT_H

#include "GUIModelObject.h"
#include "GUIWidgets.h"
#include "../CoreAccess.h"

//Forward Declarations
class ProjectTab;
class UndoStack;
class MainWindow;
class QGraphicsScene;

class GUIContainerObject : public GUIModelObject
{
    Q_OBJECT
public:
    enum CONTAINEREDGE {RIGHTEDGE, BOTTOMEDGE, LEFTEDGE, TOPEDGE};
    GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, GUIContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);
    virtual ~GUIContainerObject();

    void connectMainWindowActions();
    void disconnectMainWindowActions();

    //Scene and Core access
    QGraphicsScene *getContainedScenePtr();
    virtual CoreSystemAccess *getCoreSystemAccessPtr();

    //Handle GuiModelObjects and GuiWidgets
    void addTextWidget(QPoint position, undoStatus undoSettings=UNDO);
    void addBoxWidget(QPoint position, undoStatus undoSettings=UNDO);
    GUIModelObject *addGUIModelObject(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, nameVisibility nameStatus = USEDEFAULT, undoStatus undoSettings = UNDO);
    GUIModelObject *getGUIModelObject(QString name);
    void deleteGUIModelObject(QString componentName, undoStatus undoSettings=UNDO);
    void renameGUIModelObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    bool hasGUIModelObject(QString name);
    void takeOwnershipOf(QList<GUIModelObject*> &rModeObjectlist, QList<GUIWidget*> &rWidgetList);

    //Handle connectors
    GUIConnector *findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    bool hasConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    void removeConnector(GUIConnector* pConnector, undoStatus undoSettings=UNDO);
    void setIsCreatingConnector(bool isCreatingConnector);
    bool getIsCreatingConnector();
    void forgetContainedConnector(GUIConnector *pConnector); //! @todo maybe can be protected, other container (and self) should be able to access it, noone else needs to

    //Handle container appearance
    QString getIsoIconPath();
    QString getUserIconPath();
    void setIsoIconPath(QString path);
    void setUserIconPath(QString path);
    CONTAINEREDGE findPortEdge(QPointF center, QPointF pt);
    void refreshExternalPortsAppearanceAndPosition();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y); //!< @todo maybe not public

    bool isObjectSelected();
    bool isConnectorSelected();

    //These (overloaded versions) are used in containerPropertiesDialog by systems
    virtual void setTypeCQS(QString /*typestring*/){assert(false);}
    virtual size_t getNumberOfLogSamples(){assert(false);}
    virtual void setNumberOfLogSamples(size_t /*nSamples*/){assert(false);}

    //Public member variable
    //!< @todo make this private later
    QFileInfo mModelFileInfo; //!< @todo should not be public
    UndoStack *mUndoStack;
    ProjectTab *mpParentProjectTab;

    QList<GUIConnector *> mSelectedSubConnectorsList;
    QList<GUIConnector *> mSubConnectorList;

    //SHOULD BE PROTECTED
    typedef QHash<QString, GUIModelObject*> GUIModelObjectMapT;
    GUIModelObjectMapT mGUIModelObjectMap;
    QList<GUITextWidget *> mTextWidgetList; //! @todo we should really have one common list for all guiwidgets, or maybe only have the guiwidget map bellow
    QList<GUIBoxWidget *> mBoxWidgetList;
    QList<GUIModelObject *> mSelectedGUIModelObjectsList;
    QList<GUIWidget *> mSelectedGUIWidgetsList;

    bool mPortsHidden;
    bool mNamesHidden;
    bool mUndoDisabled;
    bool mIsRenamingObject;
    bool mJustStoppedCreatingConnector;

    GUIModelObject *mpTempGUIModelObject;
    GUIConnector *mpTempConnector;
    graphicsType mGfxType;

    size_t mHighestWidgetIndex;
    QMap<size_t, GUIWidget *> mWidgetMap;

    QList<QStringList> getFavoriteParameters();
    void setFavoriteParameter(QString componentName, QString portName, QString dataName, QString dataUnit);
    void removeFavoriteParameterByComponentName(QString componentName);

    QList<QStringList> mFavoriteParameters;

public slots:
        //Selection
    void selectAll();
    void deselectAll();
    void deselectSelectedNameText();
    QPointF getCenterPointFromSelection();
        //show/hide
    void hideNames();
    void showNames();
    void toggleNames(bool value);
    void hidePorts(bool doIt);
        //Create and remove
    void createConnector(GUIPort *pPort, undoStatus undoSettings=UNDO);
    void groupSelected(QPointF pt);
        //CopyPaste
    void cutSelected(CopyStack *xmlStack = 0);
    void copySelected(CopyStack *xmlStack = 0);
    void paste(CopyStack *xmlStack = 0);
        //Align
    void alignX();
    void alignY();
        //UndoRedo
    void undo();
    void redo();
    void clearUndo();
    void disableUndo();
    void updateUndoStatus();
        //Appearance and settings
    void setGfxType(graphicsType gfxType);
    void openPropertiesDialogSlot();
        //Enter and exit a container object
    void enterContainer();
    void exitContainer();
        //Rotate objects
    void rotateRight();
    //void rotateLeft();

signals:
        //Selection
    void deselectAllNameText();
    void deselectAllGUIObjects();
    void selectAllGUIObjects();
    void deselectAllGUIConnectors();
    void selectAllGUIConnectors();
        //HideShow
    void hideAllNameText();
    void showAllNameText();
        //Other
    void checkMessages();
    void deleteSelected();
    void setAllGfxType(graphicsType);
    void componentChanged();
    void rotateObjectsRight();


protected:
        //Protected methods
    virtual void createPorts();
    virtual void createExternalPort(QString portName);
    virtual void removeExternalPort(QString portName);
    virtual void renameExternalPort(QString oldName, QString newName);
    virtual void openPropertiesDialog();
    void clearContents();
        //Protected overloaded Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

protected slots:


private:
    bool mIsCreatingConnector;
    QGraphicsScene *mpScene;
    double mPasteOffset;

};

#endif // GUICONTAINEROBJECT_H
