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
//! @file   GUIWidgets.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIWidgets classes
//!
//$Id$

#ifndef GUIWIDGETS_H
#define GUIWIDGETS_H

#include <QGraphicsWidget>
#include <QObject>

#include "GUIObject.h"
#include "common.h"

#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QToolButton>
#include <QCheckBox>


class Widget : public WorkspaceObject
{
    Q_OBJECT

public:
    Widget(QPointF pos, qreal rot, selectionStatus startSelected, ContainerObject *pSystem, QGraphicsItem *pParent=0);
    QString mType;
    void setOldPos();
    void setWidgetIndex(int idx);
    int getWidgetIndex();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

public slots:
    virtual void deleteMe(undoStatus undoSettings=UNDO);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void flipVertical(undoStatus /*undoSettings = UNDO*/){}
    virtual void flipHorizontal(undoStatus /*undoSettings = UNDO*/){}

protected:
    int mWidgetIndex;
    bool mIsResizing;
};


class TextBoxWidget : public Widget
{
    Q_OBJECT

public:
    TextBoxWidget(QString text, QPointF pos, qreal rot, selectionStatus startSelected, ContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent=0);
    void saveToDomElement(QDomElement &rDomElement);

    void setText(QString text);
    void setFont(QFont font);
    void setLineWidth(int value);
    void setLineStyle(Qt::PenStyle style);
    void setColor(QColor color);
    void setSize(qreal w, qreal h);
    void setBoxVisible(bool boxVisible);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void deleteMe(undoStatus undoSettings=UNDO);
    virtual void flipVertical(undoStatus /*undoSettings = UNDO*/){}
    virtual void flipHorizontal(undoStatus /*undoSettings = UNDO*/){}

private slots:
    void updateWidgetFromDialog();
    void openFontDialog();
    void openColorDialog();

private:
    QGraphicsTextItem *mpTextItem;
    QGraphicsRectItem *mpRectItem;

    QDialog *mpEditDialog;
    QCheckBox *mpShowBoxCheckBoxInDialog;
    QLabel *mpWidthLabelInDialog;
    QSpinBox *mpWidthBoxInDialog;
    QLabel *mpColorLabelInDialog;
    QPushButton *mpFontInDialogButton;
    QToolButton *mpColorInDialogButton;
    QLabel *mpStyleLabelInDialog;
    QComboBox *mpStyleBoxInDialog;
    QPushButton *mpDoneInDialogButton;
    QPushButton *mpCancelInDialogButton;

    QTextEdit *mpTextBoxInDialog;
    QFont mSelectedFont;
    QColor mSelectedColor;

    bool mResizeTop;
    bool mResizeBottom;
    bool mResizeLeft;
    bool mResizeRight;
    QPointF mPosBeforeResize;
    qreal mWidthBeforeResize;
    qreal mHeightBeforeResize;
};

#endif // GUIWIDGETS_H
