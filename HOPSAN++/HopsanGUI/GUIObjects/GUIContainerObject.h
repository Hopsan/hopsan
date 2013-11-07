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
//! @file   GUIContainerObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI Container class (base class for Systems and Groups)
//!
//$Id$

#ifndef GUICONTAINEROBJECT_H
#define GUICONTAINEROBJECT_H

#include <QSlider>

#include "GUIModelObject.h"
#include "CopyStack.h"
#include "LogDataHandler.h"

//Forward Declarations
class PlotWindow;
class ModelWidget;
class UndoStack;
class MainWindow;
class QGraphicsScene;
class Port;
class Widget;
class TextBoxWidget;
class QTableView;
class QRadioButton;

using namespace std;

class ContainerObject : public ModelObject
{
    friend class UndoStack;     //! @todo Not sure about this, but the alternative would be to have lots and lots of access functions only used by undo stack...
    Q_OBJECT
public:
    enum ContainerEdgeEnumT {RightEdge, BottomEdge, LeftEdge, TopEdge};
    ContainerObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics, ContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);
    virtual ~ContainerObject();

    void hasChanged();
    ModelWidget *mpModelWidget;  //!< @todo not public

    //Signal/slot connection methods
    virtual void makeMainWindowConnectionsAndRefresh();
    virtual void unmakeMainWindowConnectionsAndRefresh();

    //Scene and core access methods
    QGraphicsScene *getContainedScenePtr();
    virtual CoreSystemAccess *getCoreSystemAccessPtr();

    //GUIModelObjects and GUIWidgets methods
    void takeOwnershipOf(QList<ModelObject*> &rModeObjectlist, QList<Widget*> &rWidgetList);

    //GUIModelObject methods
    ModelObject *addModelObject(QString fullTypeName, QPointF position, qreal rotation=0, SelectionStatusEnumT startSelected = Deselected, NameVisibilityEnumT nameStatus = UseDefault, UndoStatusEnumT undoSettings = Undo);
    ModelObject *addModelObject(ModelObjectAppearance* pAppearanceData, QPointF position, qreal rotation=0, SelectionStatusEnumT startSelected = Deselected, NameVisibilityEnumT nameStatus = UseDefault, UndoStatusEnumT undoSettings = Undo);
    ModelObject *getModelObject(const QString modelObjectName);
    Port *getModelObjectPort(const QString modelObjectName, const QString portName);
    void deleteModelObject(QString componentName, UndoStatusEnumT undoSettings=Undo);
    void renameModelObject(QString oldName, QString newName, UndoStatusEnumT undoSettings=Undo);
    bool hasModelObject(QString name);
    void rememberSelectedModelObject(ModelObject *object);
    void forgetSelectedModelObject(ModelObject *object);
    QList<ModelObject *> getSelectedModelObjectPtrs();
    bool isSubObjectSelected();

    // Alias methods
    bool setVariableAlias(QString compName, QString portName, QString varName, QString alias);
    QString getFullNameFromAlias(const QString alias);
    QStringList getAliasNames();

    //GUIWidgets methods
    TextBoxWidget *addTextBoxWidget(QPointF position, UndoStatusEnumT undoSettings=Undo);
    void deleteWidget(Widget *pWidget, UndoStatusEnumT undoSettings=Undo);
    void rememberSelectedWidget(Widget *widget);
    void forgetSelectedWidget(Widget *widget);
    QList<Widget *> getSelectedGUIWidgetPtrs();

    // Parameter Methods
    virtual bool setParameterValue(QString name, QString value, bool force=false);
    virtual bool setOrAddParameter(const CoreParameterData &rParameter, bool force=false);
    virtual bool renameParameter(const QString oldName, const QString newName);

    //Handle connector methods
    bool hasConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    Connector *findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    void rememberSubConnector(Connector *pConnector);
    void removeSubConnector(Connector* pConnector, UndoStatusEnumT undoSettings=Undo);
    bool isConnectorSelected();
    void rememberSelectedSubConnector(Connector *pConnector);
    void forgetSelectedSubConnector(Connector *pConnector);
    bool isCreatingConnector();
    void cancelCreatingConnector();
    void addOneConnectorLine(QPointF pos);
    void removeOneConnectorLine(QPointF pos);
    void makeConnectorDiagonal(bool diagonal);
    void updateTempConnector(QPointF pos);

    //Handle container appearance
    QString getIconPath(const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType);
    void setIconPath(const QString path, const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType);
    ContainerEdgeEnumT findPortEdge(QPointF center, QPointF pt); //!< @todo maybe not public
    virtual void refreshAppearance();
    void refreshExternalPortsAppearanceAndPosition();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y); //!< @todo maybe not public

    //Plot and simulation results methods
    LogDataHandler *getLogDataHandler();
    void setLogDataHandler(LogDataHandler *pHandler);

    //Undo/redo methods
    UndoStack *getUndoStackPtr();

    //Copy/paste methods
    CopyStack *getDragCopyStackPtr();

    //These (overloaded versions) are used in containerPropertiesDialog by systems
    virtual size_t getNumberOfLogSamples();
    virtual void setNumberOfLogSamples(size_t nSamples);
    virtual double getLogStartTime() const;
    virtual void setLogStartTime(const double logStartT);

    //Model info
    void setModelInfo(const QString &author, const QString &email, const QString &affiliation, const QString &description);
    void getModelInfo(QString &author, QString &email, QString &affiliation, QString &description) const;

    //Model and script file methods
    void setModelFile(QString path);
    QFileInfo getModelFileInfo();
    void setScriptFile(QString path);
    QString getScriptFile();

    QStringList getModelObjectNames();
    QList<Widget*> getWidgets();

    void replaceComponent(QString name, QString newType);

    //Numbered sections methods
    void selectSection(int no, bool append=false);
    void assignSection(int no);

    //Losses methods
    bool areLossesVisible();
    QRadioButton *mpAvgPwrRadioButton;

public slots:

    //Selection slots
    void selectAll();
    void deselectAll();
    void deselectSelectedNameText();
    QPointF getCenterPointFromSelection();

    //Show/hide slots
    void hideNames();
    void showNames();
    void toggleNames(bool value);
    void toggleSignals(bool value);
    void showSubcomponentPorts(bool doShowThem);

    //Connector slots
    Connector* createConnector(Port *pPort, UndoStatusEnumT undoSettings=Undo);
    Connector* createConnector(Port *pPort1, Port *pPort2, UndoStatusEnumT undoSettings=Undo);

    //Section slots
    void groupSelected(QPointF pt);

    //Copy/paste slots
    void cutSelected(CopyStack *xmlStack = 0);
    void copySelected(CopyStack *xmlStack = 0);
    void paste(CopyStack *xmlStack = 0);

    //Alignment slots
    void alignX();
    void alignY();

    //Undo/redo slots
    void undo();
    void redo();
    void clearUndo();
    void setUndoDisabled(bool disabled, bool dontAskJustDoIt=false);
    void setUndoEnabled(bool enabled, bool dontAskJustDoIt=false);
    void setSaveUndo(bool save);
    bool isUndoEnabled();
    bool getSaveUndo();
    void updateMainWindowButtons();

    //Appearance slots
    void setGfxType(GraphicsTypeEnumT gfxType);
    GraphicsTypeEnumT getGfxType();
    bool areSubComponentPortsShown();
    bool areSubComponentNamesShown();
    bool areSignalsHidden();

    //Properties slots
    void openPropertiesDialogSlot();

    //Enter and exit a container object
    void enterContainer();
    void exitContainer();

    //Rotate and flip slots
    void rotateSubObjects90cw();
    void rotateSubObjects90ccw();
    void flipSubObjectsHorizontal();
    void flipSubObjectsVertical();

    //Plot data slots
    void collectPlotData(bool overWriteLastGeneration=false);

    //Losses
    void showLosses(bool show);
    void showLossesFromDialog();
    void hideLosses();

    //Simulation time measurements
    void measureSimulationTime();
    void plotMeasuredSimulationTime();

    //External/internal subsystems
    bool isAncestorOfExternalSubsystem();
    bool isExternal();

signals:

    //Selection signals
    void deselectAllNameText();
    void deselectAllGUIObjects();
    void selectAllGUIObjects();
    void deselectAllConnectors();
    void selectAllConnectors();

    //Hide/show name text
    void hideAllNameText();
    void showAllNameText();
    void showOrHideAllNameText(bool doShow); //!< @todo use this instead of two separate show hide

    // Hide/Show subcomponent ports
    void showOrHideAllSubComponentPorts(bool doShow);

    //Hide/show signals components
    void showOrHideSignals(bool doShow);

    //Other signals
    void checkMessages();
    void deleteSelected();
    void setAllGfxType(GraphicsTypeEnumT);
    void componentChanged();
    void connectorRemoved();
    void rotateSelectedObjectsRight();
    void rotateSelectedObjectsLeft();
    void flipSelectedObjectsHorizontal();
    void flipSelectedObjectsVertical();

    void systemParametersChanged();

    void aliasChanged(const QString &rFullName, const QString &rAlias);

protected:

    //Protected methods
    virtual Port* createRefreshExternalPort(QString portName);
    virtual void renameExternalPort(QString oldName, QString newName);
    virtual void openPropertiesDialog();
    void clearContents();
    void forgetSubConnector(Connector *pConnector);
    void refreshInternalContainerPortGraphics();

    //Help function for creating container ports
    virtual void addExternalContainerPortObject(ModelObject *pModelObject);

    // Helpfunctions for creating connectors
    void disconnectGroupPortFromItsRealPort(Port *pGroupPort, Port *pRealPort);

    //Protected overloaded Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    //Scene pointer member
    QGraphicsScene *mpScene;

    //Model and script file members
    QFileInfo mModelFileInfo;
    QString mScriptFilePath;

    //Model object members
    typedef QHash<QString, ModelObject*> ModelObjectMapT;
    ModelObjectMapT mModelObjectMap;
    ModelObject *mpTempGUIModelObject;
    QList<ModelObject *> mSelectedModelObjectsList;

    //Connector members
    QList<Connector *> mSelectedSubConnectorsList;
    QList<Connector *> mSubConnectorList;
    Connector *mpTempConnector;
    bool mIsCreatingConnector;

    //Widget members
    QMap<size_t, Widget *> mWidgetMap;
    QList<Widget *> mSelectedWidgetsList;
    size_t mHighestWidgetIndex;

    //Contained object appearance members
    bool mShowSubComponentPorts;
    bool mShowSubComponentNames;
    bool mSignalsHidden;
    GraphicsTypeEnumT mGfxType;

    //Plot members
    LogDataHandler *mpLogDataHandler;

    //Undo-redo members
    UndoStack *mpUndoStack;
    bool mUndoDisabled;
    bool mSaveUndoStack;

    //Copy-paste members
    CopyStack *mpDragCopyStack;
    double mPasteOffset;

    //Numbered sections members
    QList< QList<ModelObject *> > mSection;

    //Losses members
    bool mLossesVisible;
    QDialog *mpLossesDialog;
    QSlider *mpMinLossesSlider;
    QRadioButton *mpEnergyRadioButton;

    //Model information
    QString mAuthor;
    QString mEmail;
    QString mAffiliation;
    QString mDescription;

    //Time measurement dialog
    QTableView *mpComponentTable;
    QTableView *mpTypeTable;
};

#endif // GUICONTAINEROBJECT_H
