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
#include <assert.h>

#include "GUIObject.h"
#include "../common.h"

#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QToolButton>


class GUIWidget : public GUIObject
{
    Q_OBJECT

public:
    GUIWidget(QPointF pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, QGraphicsItem *pParent=0);
    void setOldPos();
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


class GUITextWidget : public GUIWidget
{
    Q_OBJECT
public:
    GUITextWidget(QString text, QPointF pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent=0);
    void saveToDomElement(QDomElement &rDomElement);
    void setText(QString text);
    void setTextColor(QColor color);
    void setTextFont(QFont font);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

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
    QDialog *mpEditTextDialog;
    QPushButton *mpFontInDialogButton;
    QPushButton *mpColorInDialogButton;
    QPushButton *mpDoneInDialogButton;
    QPushButton *mpCancelInDialogButton;

    QTextEdit *mpTextBox;
    QFont mSelectedFont;
    QColor mSelectedColor;
    QLabel *mpExampleLabel;
};


class GUIBoxWidget : public GUIWidget
{
    Q_OBJECT
public:
    GUIBoxWidget(QPointF pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent=0);
    void saveToDomElement(QDomElement &rDomElement);
    void setLineWidth(int value);
    void setLineStyle(Qt::PenStyle style);
    void setLineColor(QColor color);
    void setSize(qreal w, qreal h);

public slots:
    void deleteMe(undoStatus undoSettings=UNDO);
    virtual void flipVertical(undoStatus /*undoSettings = UNDO*/){}
    virtual void flipHorizontal(undoStatus /*undoSettings = UNDO*/){}

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private slots:
    void openColorDialog();
    void updateWidgetFromDialog();

private:
    QGraphicsRectItem *mpRectItem;

    QDialog *mpEditBoxDialog;
    QLabel *mpWidthLabelInDialog;
    QSpinBox *mpWidthBoxInDialog;
    QLabel *mpColorLabelInDialog;
    QToolButton *mpColorInDialogButton;
    QLabel *mpStyleLabelInDialog;
    QComboBox *mpStyleBoxInDialog;
    QPushButton *mpDoneInDialogButton;
    QPushButton *mpCancelInDialogButton;

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
