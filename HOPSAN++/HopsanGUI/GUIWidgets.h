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

private slots:
    void deleteMe();
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


#endif // GUIWIDGETS_H
