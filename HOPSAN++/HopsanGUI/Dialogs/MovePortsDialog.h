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

    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;
};


class DragPort : public QGraphicsWidget
{
    Q_OBJECT

public:
    DragPort(const PortAppearance &rAppearance, QString name, QGraphicsItem *parentComponent);

    void setPosOnComponent(double x, double y, double rot);
    QPointF getPosOnComponent();
    double getPortRotation();
    QString getName();
    const PortAppearance &getPortAppearance() const;

public slots:
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
    QGraphicsItem *mpParentComponent;
    QGraphicsSvgItem *mpSvg;
    QGraphicsTextItem *mpName;
};


#endif // MOVEPORTSDIALOG_H
