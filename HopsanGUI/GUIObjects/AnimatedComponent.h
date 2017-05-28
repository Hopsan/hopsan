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

class AnimationWidget;
class AnimatedIcon;
class ModelObject;
class ModelObjectAppearance;
class PlotWindow;

class AnimatedComponent : public QObject
{
    Q_OBJECT

    friend class AnimatedIcon;
public:
    //Make room for some additional params passed form children
    AnimatedComponent(ModelObject* unanimatedComponent, AnimationWidget *parent);

    void draw();
    void updateAnimation();
    ModelObjectAnimationData *getAnimationDataPtr();
    int indexOfMovable(AnimatedIcon *pMovable);
    QPointF getPortPos(QString portName);
   // ModelObject* getOriginal();
    ModelObject *mpModelObject;

private slots:
    void textEdited();

private:
    void setupAnimationBase(QString basePath);
    void setupAnimationMovable(int m);
    void limitMovables();

    AnimationWidget *mpAnimationWidget;
    AnimatedIcon *mpBase;
    QList<AnimatedIcon *> mpMovables;
    QList<QList<QVector<double> > > *mpData;
    QList<QList<double*> > *mpNodeDataPtrs;

    QGraphicsTextItem *mpText;

    QMap<QString, QPointF> mPortPositions;

    ModelObjectAnimationData *mpAnimationData;

    bool mIsDisplay;
    bool mIsNumericalInput;

    PlotWindow *mpPlotWindow;
};



//////////////////////////////////////////////////////////////////////////////////////////////


class AnimatedIcon : public WorkspaceObject
{
    Q_OBJECT

    friend class AnimatedComponent;

public:
    AnimatedIcon(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, AnimatedComponent *pAnimatedComponent=0, ContainerObject *pParentContainer=0, int idx=-1, QGraphicsItem *pParent=0);

    virtual void loadFromDomElement(QDomElement domElement);
    virtual void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);

    AnimatedComponent *mpAnimatedComponent;
    int mIdx;

    // Type info
    enum { Type = AnimatedObjectType };
    int type() const;
    virtual QString getHmfTagName() const;

public slots:
    virtual void deleteMe(UndoStatusEnumT undoSettings=Undo);
    virtual void rotate(double angle, UndoStatusEnumT undoSettings=Undo);
    virtual void flipVertical(UndoStatusEnumT undoSettings=Undo);
    virtual void flipHorizontal(UndoStatusEnumT undoSettings=Undo);
    void refreshIconPosition();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

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
