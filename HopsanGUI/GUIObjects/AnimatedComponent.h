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
//! @file   AnimatedComponent.cpp
//! @author Pratik Deschpande <pratik661@gmail.com>
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-04-25
//!
//! @brief Contains a class for animated components
//!
//$Id$

#ifndef ANIMATEDCOMPONENT_H
#define ANIMATEDCOMPONENT_H

#include <QTransform>
#include <QList>
#include <QObject>
#include <QPen>
#include <QGraphicsSvgItem>
#include <QGraphicsColorizeEffect>

#include "GUIObject.h"
#include "GUIModelObjectAppearance.h"
#include "LogVariable.h"

class AnimationWidget;
class AnimatedIcon;
class ModelObject;
class ModelObjectAppearance;
class PlotWindow;
class Port;

class AnimatedComponent : public QObject
{
    Q_OBJECT

    friend class AnimatedIcon;
public:
    //Make room for some additional params passed form children
    AnimatedComponent(ModelObject* unanimatedComponent, AnimationWidget *parent);

    void draw();
    virtual void updateAnimation();
    ModelObjectAnimationData *getAnimationDataPtr();
    int indexOfMovable(AnimatedIcon *pMovable);
    QPointF getPortPos(QString portName);
   // ModelObject* getOriginal();
    ModelObject *mpModelObject;

private slots:
    void textEdited();

protected:
    void setupAnimationBase(QString basePath);
    void setupAnimationMovable(int m);
    void limitMovables();

    AnimationWidget *mpAnimationWidget;
    AnimatedIcon *mpBase;
    QList<AnimatedIcon *> mpMovables;
    QList<QList<QVector<double> > > *mpData;
    QList<QList<double*> > *mpNodeDataPtrs;

    //Display component members
    QGraphicsTextItem *mpText;
    QString mDescription, mUnit, mBackgroundColor, mTextColor, mHtml;
    double mUnitScaling;
    int mPrecision;

    QMap<QString, QPointF> mPortPositions;

    ModelObjectAnimationData *mpAnimationData;

    bool mIsDisplay;
    bool mIsNumericalInput;
};

struct AnimatedPlotData
{
    QPointer<Port> pModelObjectPort;
    SharedVectorVariableT animatedPlotData;
    SharedVectorVariableT logData;

    void clear();
};

class AnimatedScope : public AnimatedComponent
{
    Q_OBJECT
    friend class AnimatedIcon;
public:
    AnimatedScope(ModelObject* unanimatedScope, AnimationWidget *parent) : AnimatedComponent(unanimatedScope, parent) {}
    void openPlotwindow();
    void updatePlotwindow();
    void updateAnimation() override;
protected:
    QPointer<PlotWindow> mpPlotWindow;
    AnimatedPlotData mTimeData;
    AnimatedPlotData mBottomData;
    QVector<AnimatedPlotData> mLeftDatas;
    QVector<AnimatedPlotData> mRightDatas;

};

//////////////////////////////////////////////////////////////////////////////////////////////


class AnimatedIcon : public WorkspaceObject
{
    Q_OBJECT

    friend class AnimatedComponent;

public:
    AnimatedIcon(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, AnimatedComponent *pAnimatedComponent=0, SystemObject *pParentSystem=0, int idx=-1, QGraphicsItem *pParent=0);

    virtual void loadFromDomElement(QDomElement domElement) override;
    virtual void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel) override;

    AnimatedComponent *mpAnimatedComponent;
    int mIdx;

    // Type info
    virtual int type() const override;
    virtual QString getHmfTagName() const override;

public slots:
    virtual void deleteMe(UndoStatusEnumT undoSettings=Undo) override;
    virtual void rotate(double angle, UndoStatusEnumT undoSettings=Undo) override;
    virtual void flipVertical(UndoStatusEnumT undoSettings=Undo) override;
    virtual void flipHorizontal(UndoStatusEnumT undoSettings=Undo) override;
    void refreshIconPosition();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    //Protected members
    ModelObjectAppearance mModelObjectAppearance;

    GraphicsTypeEnumT mIconType;
    bool mIconRotation;
    QGraphicsSvgItem *mpIcon;
    QString mLastIconPath;
    double mLastIconScale;
    double *mpAdjustableNodeDataPtr;
    QGraphicsColorizeEffect *mpEffect;
};


#endif // ANIMATEDCOMPONENT_H
