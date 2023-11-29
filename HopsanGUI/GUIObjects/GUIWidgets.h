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
#include <QDialog>
#include <QPointer>
#include <QGraphicsSvgItem>
#include <QFileInfo>

enum WidgetTypesEnumT {UndefinedWidgetType, TextBoxWidgetType, ImageWidgetType};

class Widget : public WorkspaceObject
{
    Q_OBJECT

public:
    Widget(QPointF pos, double rot, SelectionStatusEnumT startSelected, SystemObject *pSystem, QGraphicsItem *pParent=0);
    int getWidgetIndex();

    // Type info
    virtual WidgetTypesEnumT getWidgetType() const = 0;
    int type() const override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

public slots:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

protected:
    int mWidgetIndex;
    bool mIsResizing;
};


class TextBoxWidget : public Widget
{
    Q_OBJECT

public:
    TextBoxWidget(QString text, QPointF pos, double rot, SelectionStatusEnumT startSelected, SystemObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent=0);
    TextBoxWidget(const TextBoxWidget &other, SystemObject *pSystem);

    // Type info
    virtual WidgetTypesEnumT getWidgetType() const;
    virtual QString getHmfTagName() const;

    // Save and load
    void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);
    void loadFromDomElement(QDomElement domElement);

    void setText(QString text);
    void setFont(QFont font);
    void setTextColor(QColor color);
    void setLineWidth(int value);
    void setLineStyle(Qt::PenStyle style);
    void setLineColor(QColor color);
    void setSize(double w, double h);
    void setBoxVisible(bool boxVisible);

    void makeSureBoxNotToSmallForText();
    void resizeBoxToText();
    void resizeTextToBox();
    void setReflowText(bool doReflow);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void refreshSelectionBoxSize();

public slots:
    void deleteMe(UndoStatusEnumT undoSettings=Undo);
    virtual void flipVertical(UndoStatusEnumT undoSettings = Undo);
    virtual void flipHorizontal(UndoStatusEnumT undoSettings = Undo);

private slots:
    void updateWidgetFromDialog();
    void openFontDialog();
    void openTextColorDialog();
    void openLineColorDialog();

private:
    void refreshWidgetSize();

    QGraphicsTextItem *mpTextItem;
    QGraphicsRectItem *mpBorderItem;

    QPointer<QDialog> mpEditDialog;
    QTextEdit *mpDialogTextBox;
    QPushButton *mpDialogFontButton;
    QToolButton *mpDialogLineColorButton;
    QToolButton *mpDialogTextColorButton;
    QCheckBox *mpDialogReflowCheckBox;

    QCheckBox *mpDialogShowBorderCheckBox;
    QSpinBox *mpDialogLineWidth;
    QComboBox *mpDialogLineStyle;

    QColor mSelectedTextColor;
    QColor mSelectedLineColor;

    bool mReflowText;
    bool mResizeTop;
    bool mResizeBottom;
    bool mResizeLeft;
    bool mResizeRight;

    QPointF mPosBeforeResize;
    double mWidthBeforeResize;
    double mHeightBeforeResize;
};


class ImageWidget : public Widget
{
    Q_OBJECT

public:
    ImageWidget(QPointF pos, double rot, SelectionStatusEnumT startSelected, SystemObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent=0);
    ImageWidget(const ImageWidget &other, SystemObject *pSystem);

    // Type info
    virtual WidgetTypesEnumT getWidgetType() const;
    virtual QString getHmfTagName() const;

    // Save and load
    void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);
    void loadFromDomElement(QDomElement domElement);

    void setImage(const QString &path, double iconScale = 1.0);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void refreshSelectionBoxSize();

public slots:
    void deleteMe(UndoStatusEnumT undoSettings=Undo);
    virtual void flipVertical(UndoStatusEnumT undoSettings = Undo);
    virtual void flipHorizontal(UndoStatusEnumT undoSettings = Undo);

private slots:
    void browseForImageFile();
    void updateWidgetFromDialog();

private:
    QGraphicsSvgItem *mpImage = nullptr;
    QString mImagePath;
    QString mDefaultImage = QString(GRAPHICSPATH) + "hopsan-logo.svg";

    QPointer<QDialog> mpEditDialog;
    QLabel *mpEditDialogPathLabel;
    QLineEdit *mpEditDialogPathLineEdit;
    QToolButton *mpEditDialogBrowseButton;
    QLabel *mpEditDialogScaleLabel;
    QDoubleSpinBox *mpEditDialogScaleSpinBox;
    void refreshWidgetSize();
};

#endif // GUIWIDGETS_H
