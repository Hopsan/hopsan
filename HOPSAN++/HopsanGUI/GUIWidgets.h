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

//class ProjectTabWidget;
//class GraphicsScene;
//class GraphicsView;
//class GUIConnector;
//class GUIModelObjectDisplayName;
//class GUIObjectSelectionBox;
//class GUIPort;
//class GUISystem;


class GUITextWidget : public GUIObject
{
    Q_OBJECT
public:
    GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent=0);
    void saveToDomElement(QDomElement &rDomElement);
    void setText(QString text);

public slots:
    //void deselect();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    //virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private slots:
    void setTextFromDialog();
    void getFont();
    void getColor();

private:
    QGraphicsTextItem *mpTextItem;
    QDialog *mpEditTextDialog;
    QPushButton *mpFontInDialogButton;
    QPushButton *mpColorInDialogButton;
    QPushButton *mpDoneInDialogButton;
    QPushButton *mpCancelInDialogButton;

    QTextEdit *mpTextBox;
    QFont mpSelectedFont;
    QColor mpSelectedColor;
    QLabel *mpFontLabel;
};


#endif // GUIWIDGETS_H
