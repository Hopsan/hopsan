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
//! @file   MovePortsDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-12-24
//!
//! @brief Contains a class for moving ports at GUI model objects
//!
//$Id$

#ifndef MOVEPORTSDIALOG_H
#define MOVEPORTSDIALOG_H

#include <QtGui>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>

#include "GUIPortAppearance.h"
#include "common.h"

// Forward declarations
class DragPort;
class ModelObjectAppearance;

class MovePortsDialog : public QDialog
{
    Q_OBJECT

public:
    MovePortsDialog(ModelObjectAppearance *pComponentAppearance, graphicsType gfxType = USERGRAPHICS, QWidget *parent = 0);

public slots:
    bool okButtonPressed();
    bool cancelButtonPressed();
    void updatePortXPos(QString x);
    void updatePortYPos(QString y);
    void updatePortRotation(QString a);
    void updatePortInfo(QString portName, QString x, QString y, QString a);
    void updateZoom();

signals:
    void finished();

protected:
    QGraphicsSvgItem *mpSVGComponent;
    ModelObjectAppearance *mpCompAppearance;
    PortAppearanceMapT *mpPortAppearanceMap;
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

    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;

    QVector<QCheckBox*> mvPortEnable;
};


class DragPort : public QGraphicsWidget
{
    Q_OBJECT

public:
    DragPort(PortAppearance *pAppearance, QString name, QGraphicsItem *parentComponent);

    void setPosOnComponent(double x, double y, double rot);
    QPointF getPosOnComponent();
    double getPortRotation();
    PortAppearance *getPortAppearance();

public slots:
    void setEnable(int state);

signals:
    void activePort(QString portName, QString x, QString y, QString a);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void portMoved();

    PortAppearance *mpPortAppearance;
    QGraphicsItem *mpParentComponent;
    QGraphicsSvgItem *mpSvg;
    QGraphicsTextItem *mpName;
};


#endif // MOVEPORTSDIALOG_H
