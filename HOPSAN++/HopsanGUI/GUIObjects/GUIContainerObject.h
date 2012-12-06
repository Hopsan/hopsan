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
//#include "PlotWindow.h"
#include <QSharedPointer>

#include "GUIModelObject.h"
#include "CopyStack.h"

//Forward Declarations
class PlotWindow;
class ProjectTab;
class UndoStack;
class MainWindow;
class QGraphicsScene;
class Port;
class Widget;
class TextBoxWidget;

using namespace std;

typedef QSharedPointer<QVector<double> > SharedTimeVectorPtrT;
class UniqueSharedTimeVectorPtrHelper
{
public:
    SharedTimeVectorPtrT makeSureUnique(QVector<double> &rTimeVector);

private:
    QVector< SharedTimeVectorPtrT > mSharedTimeVecPointers;
};

QString makeConcatName(const QString componentName, const QString portName, const QString dataName);
void splitConcatName(const QString fullName, QString &rCompName, QString &rPortName, QString &rVarName);

//! @class VariableDescription
//! @brief Container class for strings describing a plot variable

class VariableDescription
{
public:
    enum VarTypeT {M, I, S, ST};
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QString mAliasName;
    VarTypeT mVarType;

    QString getFullName() const;
    void setFullName(const QString compName, const QString portName, const QString dataName);

    QString getTypeVarString() const;

    bool operator==(const VariableDescription &other) const;
};


//! @class PlotData
//! @brief Object containg all plot data and plot data function associated with a container object

class HopImpData
{
public:
      QString mDataName;
      double scale;
      double startvalue;
      QVector<double> mDataValues;
};

class LogVariableData;
class LogVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, LogVariableData*> GenerationMapT;

    //! @todo also need qucik constructor for creating a container with one generation directly
    LogVariableContainer(const VariableDescription &rVarDesc);
    ~LogVariableContainer();
    void addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData);
    void addDataGeneration(const int generation, const SharedTimeVectorPtrT time, const QVector<double> &rData);
    void removeDataGeneration(const int generation);
    void removeGenerationsOlderThen(const int gen);

    LogVariableData* getDataGeneration(const int gen=-1);
    bool hasDataGeneration(const int gen);
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;

    VariableDescription getVariableDescription() const;
    QString getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getComponentName() const;
    QString getPortName() const;
    QString getDataName() const;
    QString getDataUnit() const;


    void setAliasName(const QString alias);


    //std::string mVarTypeT;
    //QString mVariableType; //! @todo find better name, this is model, import, script, script_temp

signals:
    void nameChanged();

private slots:
    void forgetDataGeneration(int gen);

private:
    VariableDescription mVariableDescription;
    GenerationMapT mDataGenerations;
};

class LogVariableData : public QObject
{
    Q_OBJECT

public:
    //! @todo maybe have protected constructor, to avoid creating these objects manually (need to be friend with container)
    LogVariableData(const int generation, const QVector<double> &rTime, const QVector<double> &rData, LogVariableContainer *pParent);
    LogVariableData(const int generation, SharedTimeVectorPtrT time, const QVector<double> &rData, LogVariableContainer *pParent);
    ~LogVariableData();

    double mAppliedValueOffset;
    double mAppliedTimeOffset;
    int mGeneration;
    QVector<double> mDataVector;
    SharedTimeVectorPtrT mSharedTimeVectorPtr;

    VariableDescription getVariableDescription() const;
    QString getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getComponentName() const;
    QString getPortName() const;
    QString getDataName() const;
    QString getDataUnit() const;
    int getGeneration() const;
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    double getOffset() const;
    void addtoData(const LogVariableData *pOther);
    void addtoData(const double other);
    void subtoData(const LogVariableData *pOther);
    void subtoData(const double other);
    void multtoData(const LogVariableData *pOther);
    void multtoData(const double other);
    void divtoData(const LogVariableData *pOther);
    void divtoData(const double other);
    void assigntoData(const LogVariableData *pOther);
    bool poketoData(const int index, const double value);
    double peekFromData(const int index);
    //double getScale() const;


public slots:
    void setValueOffset(double offset);
    void setTimeOffset(double offset);
    //setScale(double scale);


signals:
    void beginDeleted(int gen);
    void dataChanged();
    void nameChanged();

private:
    LogVariableContainer *mpParentVariableContainer;

    QVector<double> mTimeVector; //! @todo should be smart pointer, or store by Time vector Id

};



class LogDataHandler : public QObject
{
    Q_OBJECT

public:
    LogDataHandler(ContainerObject *pParent);
    ~LogDataHandler();


    typedef QMap<QString, LogVariableContainer*> DataMapT;
    typedef QMap<QString, LogVariableContainer*> AliasMapT;


    typedef QList<QVector<double> > TimeListT;

    typedef QList<VariableDescription> FavoriteListT;

    void collectPlotDataFromModel();
    void exportToPlo(QString filePath, QStringList variables);
    void importFromPlo();

    bool isEmpty();

    QVector<double> getTimeVector(int generation);
    QVector<double> getPlotDataValues(int generation, QString componentName, QString portName, QString dataName);
    QVector<double> getPlotDataValues(const QString fullName, int generation);
    LogVariableData *getPlotData(int generation, QString componentName, QString portName, QString dataName);
    LogVariableData *getPlotData(const QString fullName, const int generation);
    LogVariableData *getPlotDataByAlias(const QString alias, const int generation);
    QVector<LogVariableData*> getAllVariablesAtNewestGeneration();
    QVector<LogVariableData*> getOnlyVariablesAtGeneration(const int generation);
    int getLatestGeneration() const;
    QStringList getPlotDataNames();

    void definePlotAlias(QString fullName);
    bool definePlotAlias(const QString alias, const QString fullName);
    void undefinePlotAlias(QString alias);
    AliasMapT getPlotAliasMap();
    QString getFullNameFromAlias(QString alias);
    QString getAliasFromFullName(QString fullName);

    bool componentHasPlotGeneration(int generation, QString fullName);
    void limitPlotGenerations();

    void updateObjectName(QString oldName, QString newName);


    void incrementOpenPlotCurves();
    void decrementOpenPlotCurves();
    bool hasOpenPlotCurves();

    FavoriteListT getFavoriteVariableList();
    void setFavoriteVariable(QString componentName, QString portName, QString dataName, QString dataUnit);
    void removeFavoriteVariableByComponentName(QString componentName);

    PlotWindow *openNewPlotWindow(const QString fullName);
    PlotWindow *plotToWindow(const QString fullName, const int gen, int axis=0, PlotWindow *pPlotWindow=0, QColor color=QColor());

    LogVariableData *addVariableWithScalar(const LogVariableData *a, const double x);
    LogVariableData *subVariableWithScalar(const LogVariableData *a, const double x);
    LogVariableData *mulVariableWithScalar(const LogVariableData *a, const double x);
    LogVariableData *divVariableWithScalar(const LogVariableData *a, const double x);
    QString addVariableWithScalar(const QString &a, const double x);
    QString subVariableWithScalar(const QString &a, const double x);
    QString mulVariableWithScalar(const QString &a, const double x);
    QString divVariableWithScalar(const QString &a, const double x);

    LogVariableData* addVariables(const LogVariableData *a, const LogVariableData *b);
    LogVariableData* subVariables(const LogVariableData *a, const LogVariableData *b);
    LogVariableData* multVariables(const LogVariableData *a, const LogVariableData *b);
    LogVariableData* divVariables(const LogVariableData *a, const LogVariableData *b);
    LogVariableData* assignVariables(LogVariableData *a, const LogVariableData *b);
    bool pokeVariables(LogVariableData *a, const int index, const double value);
    LogVariableData* saveVariables(LogVariableData *a);
    void delVariables(LogVariableData *a);
    QString addVariables(const QString &a, const QString &b);
    QString subVariables(const QString &a, const QString &b);
    QString multVariables(const QString &a, const QString &b);
    QString divVariables(const QString &a, const QString &b);
    QString assignVariables(const QString &a, const QString &b);
    bool pokeVariables(const QString &a, const int index, const double value);
    double peekVariables(const QString &a, const int index);
    double peekVariables(LogVariableData *a, const int b);
    QString delVariables(const QString &a);    
    QString saveVariables(const QString &currName, const QString &newName);
    LogVariableData* defineTempVariable(const QString desiredname);
    LogVariableData* defineNewVariable(const QString desiredname);
    void removeTempVariable(const QString fullName);

signals:
    void newDataAvailable();


private:
    ContainerObject *mpParentContainerObject;



    DataMapT mAllPlotData;
    int mGenerationNumber;
    unsigned long int mTempVarCtr;

    QList<SharedTimeVectorPtrT> mTimeVectorPtrs;
    AliasMapT mPlotAliasMap;
    FavoriteListT mFavoriteVariables;
    int mnPlotCurves;
    //LogVariableData* tempVar;
};



class ContainerObject : public ModelObject
{
    friend class UndoStack;     //! @todo Not sure about this, but the alternative would be to have lots and lots of access functions only used by undo stack...
    Q_OBJECT
public:
    enum ContainerEdgeT {RIGHTEDGE, BOTTOMEDGE, LEFTEDGE, TOPEDGE};
    ContainerObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, ContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);
    virtual ~ContainerObject();

    void hasChanged();
    ProjectTab *mpParentProjectTab;  //!< @todo not public

    //Signal/slot connection methods
    virtual void connectMainWindowActions();
    virtual void disconnectMainWindowActions();

    //Scene and core access methods
    QGraphicsScene *getContainedScenePtr();
    virtual CoreSystemAccess *getCoreSystemAccessPtr();

    //GUIModelObjects and GUIWidgets methods
    void takeOwnershipOf(QList<ModelObject*> &rModeObjectlist, QList<Widget*> &rWidgetList);

    //GUIModelObject methods
    ModelObject *addModelObject(QString fullTypeName, QPointF position, qreal rotation=0, selectionStatus startSelected = DESELECTED, nameVisibility nameStatus = USEDEFAULT, undoStatus undoSettings = UNDO);
    ModelObject *addModelObject(ModelObjectAppearance* pAppearanceData, QPointF position, qreal rotation=0, selectionStatus startSelected = DESELECTED, nameVisibility nameStatus = USEDEFAULT, undoStatus undoSettings = UNDO);
    ModelObject *getModelObject(const QString modelObjectName);
    Port *getModelObjectPort(const QString modelObjectName, const QString portName);
    void deleteModelObject(QString componentName, undoStatus undoSettings=UNDO);
    void renameModelObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    bool hasModelObject(QString name);
    void rememberSelectedModelObject(ModelObject *object);
    void forgetSelectedModelObject(ModelObject *object);
    QList<ModelObject *> getSelectedModelObjectPtrs();
    bool isSubObjectSelected();

    // Alias methods
    void setVariableAlias(QString compName, QString portName, QString varName, QString alias);
    QString getFullNameFromAlias(const QString alias);
    QStringList getAliasNames();

    //GUIWidgets methods
    TextBoxWidget *addTextBoxWidget(QPointF position, undoStatus undoSettings=UNDO);
    void deleteWidget(Widget *pWidget, undoStatus undoSettings=UNDO);
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
    void removeSubConnector(Connector* pConnector, undoStatus undoSettings=UNDO);
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
    QString getIconPath(const graphicsType gfxType, const AbsoluteRelativeT absrelType);
    void setIconPath(const QString path, const graphicsType gfxType, const AbsoluteRelativeT absrelType);
    ContainerEdgeT findPortEdge(QPointF center, QPointF pt); //!< @todo maybe not public
    virtual void refreshAppearance();
    void refreshExternalPortsAppearanceAndPosition();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y); //!< @todo maybe not public

    //Plot and simulation results methods
    LogDataHandler *getPlotDataPtr();

    //Undo/redo methods
    UndoStack *getUndoStackPtr();

    //Copy/paste methods
    CopyStack *getDragCopyStackPtr();

    //These (overloaded versions) are used in containerPropertiesDialog by systems
    virtual size_t getNumberOfLogSamples();
    virtual void setNumberOfLogSamples(size_t nSamples);

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
    Connector* createConnector(Port *pPort, undoStatus undoSettings=UNDO);
    Connector* createConnector(Port *pPort1, Port *pPort2, undoStatus undoSettings=UNDO);

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
    void setGfxType(graphicsType gfxType);
    graphicsType getGfxType();
    bool areSubComponentPortsHidden();
    bool areSubComponentNamesHidden();
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
    void collectPlotData();

    //Losses
    void showLosses(bool show);
    void showLossesFromDialog();
    void hideLosses();

    //Simulation time measurements
    void measureSimulationTime();

    //External/internal subsystems
    bool isAncestorOfExternalSubsystem();
    bool isExternal();

    void recompileCppComponents(ModelObject *pComponent=0);

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
    void setAllGfxType(graphicsType);
    void componentChanged();
    void connectorRemoved();
    void rotateSelectedObjectsRight();
    void rotateSelectedObjectsLeft();
    void flipSelectedObjectsHorizontal();
    void flipSelectedObjectsVertical();

    void systemParametersChanged();

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
    void startConnector(Port *startPort);
    bool finilizeConnector(Port *endPort);
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
    bool mSubComponentPortsHidden;
    bool mSubComponentNamesHidden;
    bool mSignalsHidden;
    graphicsType mGfxType;

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
};

#endif // GUICONTAINEROBJECT_H
