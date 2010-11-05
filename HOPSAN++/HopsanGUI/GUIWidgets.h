//$Id$

#ifndef GUIWIDGETS_H
#define GUIWIDGETS_H

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsWidget>

#include "common.h"

#include "AppearanceData.h"
#include <assert.h>

#include "GUIObject.h"

#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>


class GUITextWidget : public GUIObject
{
    Q_OBJECT
public:
    GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent=0);
    void saveToDomElement(QDomElement &rDomElement);
    void setText(QString text);
    void setTextColor(QColor color);
    void setTextFont(QFont font);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void deleteMe();

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





class GUIBoxWidget : public GUIObject
{
    Q_OBJECT
public:
    GUIBoxWidget(QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent=0);
    //void saveToDomElement(QDomElement &rDomElement);

public slots:
    void deleteMe();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private slots:
    void openColorDialog();
    void updateWidgetFromDialog();

private:
    QGraphicsRectItem *mpRectItem;

    QDialog *mpEditBoxDialog;
    QLabel *mpWidthLabelInDialog;
    QSpinBox *mpWidthBoxInDialog;
    QPushButton *mpColorInDialogButton;
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
