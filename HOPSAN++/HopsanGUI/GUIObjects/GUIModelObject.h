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
//! @file   GUIModelObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIModelObject class (The baseclass for all objects representing model parts)
//!
//$Id$

#ifndef GUIMODELOBJECT_H
#define GUIMODELOBJECT_H

#include "GUIObject.h"
#include "GUIModelObjectAppearance.h"
#include "CoreAccess.h"
#include <QGraphicsSvgItem>

class Connector;
class ModelObjectDisplayName;
class Port;
class SystemContainer;

class ModelObject : public WorkspaceObject
{
    Q_OBJECT

public:
    ModelObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT graphics = UserGraphics, ContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);
    virtual ~ModelObject();
    virtual void deleteInHopsanCore();

    virtual void setParentContainerObject(ContainerObject *pParentContainer);

    // Name methods
    virtual void setName(QString name);
    virtual QString getName() const;
    virtual void refreshDisplayName(const QString overrideName="");
    virtual QString getTypeName() const;
    QString getSubTypeName() const;
    void setSubTypeName(const QString subTypeName);
    virtual int getNameTextPos();
    virtual void setNameTextPos(int textPos);

    // CQS methods
    virtual QString getTypeCQS(){return "hasNoCqsType";} //Overloaded in GUISystem and GUIComponent

    // Appearance methods
    void setAppearanceDataBasePath(const QString basePath);
    virtual ModelObjectAppearance* getAppearanceData();
    virtual const ModelObjectAppearance *getLibraryAppearanceData() const;
    bool isVisible();
    QGraphicsSvgItem *getIcon();

    // Help methods
    QString getHelpPicture();
    QString getHelpText();

    // Parameter methods
    virtual QStringList getParameterNames();
    virtual void getParameters(QVector<CoreParameterData> &rParameterDataVec);
    virtual void getParameter(const QString paramName, CoreParameterData &rData);
    virtual QString getParameterValue(const QString paramName);

    virtual QString getDefaultParameterValue(const QString paramName) const;

    // VariableAlias method
    //! @todo parameters and portvaraibles should be more similar in the future, so that we do not need handle them separately
    virtual QVector< QPair<QString,QString> > getVariableAliasList();
    virtual void getVariableDataDescriptions(QVector<CoreVariableData> &rVarDataDescriptions);
    virtual void getVariameterDescriptions(QVector<CoreVariameterDescription> &rVariameterDescriptions);

    virtual bool setParameterValue(QString name, QString valueTxt, bool force=false);
    virtual QString getStartValueTxt(QString portName, QString variable);
    virtual bool setStartValue(QString portName, QString variable, QString sysParName);

    // Load and save methods
    virtual void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);
    virtual void loadFromDomElement(QDomElement &rDomElement);
    virtual void setModelFileInfo(QFile &rFile);

    // Connector methods
    QList<Connector*> getConnectorPtrs();
    void rememberConnector(Connector *item);
    void forgetConnector(Connector *item);

    // Port methods
    void showPorts(bool visible);
    Port *getPort(const QString &rName);
    QList<Port*> &getPortListPtrs();
    virtual Port* createRefreshExternalPort(QString portName);
    virtual void hideExternalDynamicParameterPort(QString portName);
    virtual void removeExternalPort(QString portName);

    enum { Type = ModelObjectType };
    int type() const;

    void getLosses(double &total, QMap<QString, double> domainSpecificLosses);
    bool isLossesDisplayVisible();

    QPointF getOldPos() const;
public slots:
    virtual void refreshAppearance();
    virtual void refreshExternalPortsAppearanceAndPosition();
    void deleteMe();
    virtual void rotate(qreal angle, UndoStatusEnumT undoSettings = Undo);
    virtual void flipVertical(UndoStatusEnumT undoSettings = Undo);
    virtual void flipHorizontal(UndoStatusEnumT undoSettings = Undo);
    void hideName(UndoStatusEnumT undoSettings = NoUndo);
    void showName(UndoStatusEnumT undoSettings = NoUndo);
    void setIcon(GraphicsTypeEnumT);
    void showLosses();
    void hideLosses();
    void setCppCode(QString code);
    void setCppInputs(int n);
    void setCppOutputs(int n);
    QString getCppCode();
    int getCppInputs();
    int getCppOutputs();
    void setModelicaCode(QString code);
    QString getModelicaCode();
    void redrawConnectors();

signals:
    void nameChanged();

protected:
    // Protected methods
    virtual void openPropertiesDialog(){}
    virtual QAction *buildBaseContextMenu(QMenu &rMenue, QGraphicsSceneContextMenuEvent* pEvent);

    // Reimplemented Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    // Save and load methods
    virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);

    // Protected members
    ModelObjectAppearance mModelObjectAppearance;

    QString mName;
    ModelObjectDisplayName *mpNameText;
    double mTextOffset;
    int mNameTextPos;

    GraphicsTypeEnumT mIconType;
    bool mIconRotation;
    QGraphicsSvgItem *mpIcon;
    QString mLastIconPath;
    qreal mLastIconScale;

    QMap<QString, QString> mDefaultParameterValues;
    QStringList mActiveDynamicParameterPortNames;

    QList<Port*> mPortListPtrs;
    QList<Connector*> mConnectorPtrs;

    QGraphicsTextItem *mpLossesDisplay;

    double mTotalLosses;
    QMap<QString, double> mDomainSpecificLosses;

    // Used by C++ components
    QString mCppCode;
    int mnCppInputs;
    int mnCppOutputs;

    // Used by Modelica components
    QString mModelicaCode;

    bool mDragCopying;
    QWidget *mpDialogParentWidget;

protected slots:
    void snapNameTextPosition(QPointF pos);
    void calcNameTextPositions(QVector<QPointF> &rPts);
    void setNameTextScale(qreal scale);
    void setIconZoom(const qreal zoom);

private:
    void refreshIconPosition();
};


class ModelObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT

public:
    ModelObjectDisplayName(ModelObject *pParent);

public slots:
    void deselect();

signals:
    void textMoved(QPointF pos);

protected:
    // Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    // Protected members
    ModelObject* mpParentModelObject;
};

#endif // GUIMODELOBJECT_H
