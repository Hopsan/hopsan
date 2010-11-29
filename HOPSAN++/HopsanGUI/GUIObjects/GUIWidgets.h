//$Id$

#ifndef GUIWIDGETS_H
#define GUIWIDGETS_H

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsWidget>

#include "../common.h"

//#include "GUIModelObjectAppearance.h"
#include <assert.h>

#include "GUIObject.h"

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
    GUIWidget(QPoint pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, QGraphicsItem *pParent=0);
    size_t mWidgetIndex;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

public slots:
    virtual void deleteMe(undoStatus undoSettings=UNDO);
};


class GUITextWidget : public GUIWidget
{
    Q_OBJECT
public:
    GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent=0);
    void saveToDomElement(QDomElement &rDomElement);
    void setText(QString text);
    void setTextColor(QColor color);
    void setTextFont(QFont font);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void deleteMe(undoStatus undoSettings=UNDO);

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
    GUIBoxWidget(QPoint pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent=0);
    void saveToDomElement(QDomElement &rDomElement);
    void setLineWidth(int value);
    void setLineStyle(Qt::PenStyle style);
    void setLineColor(QColor color);
    void setSize(qreal w, qreal h);

public slots:
    void deleteMe(undoStatus undoSettings=UNDO);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

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
