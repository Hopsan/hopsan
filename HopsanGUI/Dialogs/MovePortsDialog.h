/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   MovePortsDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-12-24
//!
//! @brief Contains a class for moving ports at GUI model objects
//!
//$Id$

#ifndef MOVEPORTSDIALOG_H
#define MOVEPORTSDIALOG_H

#include <QDialog>
#include <QGraphicsWidget>

// Qt Forward Declarations
class QGraphicsSvgItem;
class QGraphicsView;
class QGridLayout;
class QSlider;
class QCheckBox;
class QLineEdit;
class QLabel;
class QGraphicsItem;
class QGraphicsTextItem;
class QGraphicsSceneMouseEvent;

#include "GUIPortAppearance.h"
#include "common.h"

// Forward declarations
class DragPort;
class ModelObjectAppearance;

class MovePortsDialog : public QDialog
{
    Q_OBJECT

public:
    MovePortsDialog(ModelObjectAppearance *pComponentAppearance, const ModelObjectAppearance *pLibraryComponentAppearance,
                    GraphicsTypeEnumT gfxType = UserGraphics, QWidget *parent = 0);

public slots:
    bool okButtonPressed();
    bool cancelButtonPressed();
    void updatePortInfo(DragPort *pDragPort=0);
    void setSelectPort();
    void disabledPort();
    void updateZoom();

signals:
    void finished();

protected:
    void clearPortInfo();

    QGraphicsSvgItem *mpSVGComponent;
    ModelObjectAppearance *mpCompAppearance;
    PortAppearanceMapT *mpActualPortAppearanceMap;
    QMap<QString,DragPort*> mDragPortMap;

    QGraphicsView *mpView;
    double mViewScale;
    QGridLayout *mpMainLayout;
    QGridLayout *mpPortEnableLayout;

    QLabel *mpPortNameLabel;

    QSlider *mpZoomSlider;
    QLineEdit *mpPortXLineEdit;
    QLineEdit *mpPortYLineEdit;
    QLineEdit *mpPortALineEdit;
    QCheckBox *mpPortAutoCheckBox;
    QPushButton *mpResetButton;

    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;
};


class DragPort : public QGraphicsWidget
{
    Q_OBJECT

public:
    DragPort(QString name, const PortAppearance &rAppearance, const SharedPortAppearanceT pOriginalAppearance, QGraphicsItem *parentComponent);

    void setPosOnComponent(double x, double y, double rot);
    QPointF getPosOnComponent();
    double getPortRotation();
    QString getName();
    const PortAppearance &getPortAppearance() const;
    const SharedPortAppearanceT getOriginalPortAppearance() const;

public slots:
    void reset();
    void setEnable(int state);
    void setPortXPos(QString x);
    void setPortYPos(QString y);
    void setPortRotation(QString a);
    void setPortAutoPlaced(bool ap);

signals:
    void portSelected();
    void portMoved();
    void portDisabled();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    void refreshLocalAppearanceData();

    PortAppearance mPortAppearance;
    const SharedPortAppearanceT mpOriginalPortAppearance;
    QGraphicsItem *mpParentComponent;
    QGraphicsSvgItem *mpSvg;
    QGraphicsTextItem *mpName;
};


#endif // MOVEPORTSDIALOG_H
