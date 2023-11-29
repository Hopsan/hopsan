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
//! @file   GUIWidgets.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIWidgets classes
//!
//$Id$

#include <float.h>

#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDialog>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QGraphicsRectItem>
#include <QFileDialog>

#include "global.h"
#include "GUIWidgets.h"
#include "GUIContainerObject.h"
#include "Widgets/ModelWidget.h"
#include "Utilities/GUIUtilities.h"
#include "UndoStack.h"
#include "GraphicsView.h"
#include "MessageHandler.h"
#include "Configuration.h"

Widget::Widget(QPointF pos, double rot, SelectionStatusEnumT startSelected, SystemObject *pSystem, QGraphicsItem *pParent)
    : WorkspaceObject(pos, rot, startSelected, pSystem, pParent)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    if(pSystem)
    {
        pSystem->getContainedScenePtr()->addItem(this);
    }
    this->setPos(pos);
    mIsResizing = false;        //Only used for resizable widgets
}


int Widget::getWidgetIndex()
{
    return mWidgetIndex;
}

int Widget::type() const
{
    return WidgetType;
}

QVariant Widget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if(this->isSelected())
        {
            mpParentSystemObject->rememberSelectedWidget(this);
        }
        else
        {
            mpParentSystemObject->forgetSelectedWidget(this);
        }
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        QVariant rvalue = WorkspaceObject::itemChange(change, value);
        // Restore position if locked
        if(isLocallyLocked() || (getModelLockLevel()!=NotLocked))
        {
            this->setPos(mPreviousPos);
            rvalue = mPreviousPos;
        }

        return rvalue;
    }
    return WorkspaceObject::itemChange(change, value);
}


void Widget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QList<Widget *>::iterator it;

        //Loop through all selected widgets and register changed positions in undo stack
    bool alreadyClearedRedo = false;
    QList<Widget *> selectedWidgets = mpParentSystemObject->getSelectedGUIWidgetPtrs();
    for(int i=0; i<selectedWidgets.size(); ++i)
    {
        if((selectedWidgets[i]->mPreviousPos != selectedWidgets[i]->pos()) && (event->button() == Qt::LeftButton) && !selectedWidgets[i]->mIsResizing)
        {
                //This check makes sure that only one undo post is created when moving several objects at once
            if(!alreadyClearedRedo)
            {
                if(mpParentSystemObject->getSelectedGUIWidgetPtrs().size() > 1)
                {
                    mpParentSystemObject->getUndoStackPtr()->newPost(UNDO_MOVEDMULTIPLEWIDGETS);
                }
                else
                {
                    mpParentSystemObject->getUndoStackPtr()->newPost();
                }
                mpParentSystemObject->mpModelWidget->hasChanged();
                alreadyClearedRedo = true;
            }

            mpParentSystemObject->getUndoStackPtr()->registerMovedWidget(selectedWidgets[i], selectedWidgets[i]->mPreviousPos, selectedWidgets[i]->pos());
        }
        selectedWidgets[i]->mIsResizing = false;
    }

    WorkspaceObject::mouseReleaseEvent(event);
}


TextBoxWidget::TextBoxWidget(QString text, QPointF pos, double rot, SelectionStatusEnumT startSelected, SystemObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent)
    : Widget(pos, rot, startSelected, pSystem, pParent)
{
    mWidgetIndex = widgetIndex;

    mpBorderItem = new QGraphicsRectItem(0, 0, 200, 100, this);
    QPen borderPen = mpBorderItem->pen();
    borderPen.setWidth(2);
    borderPen.setStyle(Qt::SolidLine);
    borderPen.setCapStyle(Qt::RoundCap);
    borderPen.setJoinStyle(Qt::RoundJoin);
    mpBorderItem->setPen(borderPen);
    mpBorderItem->setPos(mpBorderItem->pen().width()/2.0, mpBorderItem->pen().width()/2.0);
    mpBorderItem->show();

    mpTextItem = new QGraphicsTextItem(text, this);
    QFont textFont = mpTextItem->font();
    textFont.setPointSize(12);
    mpTextItem->setFont(textFont);
    mpTextItem->setPos(this->boundingRect().center());
    mpTextItem->show();
    mpTextItem->setAcceptHoverEvents(false);
    // Activate text reflow, to match border width
    mReflowText=true;
    mpTextItem->setTextWidth(mpBorderItem->boundingRect().width());

    setLineColor(QColor("darkolivegreen"));
    setTextColor(QColor("darkolivegreen"));

    refreshWidgetSize();

    setFlag(QGraphicsItem::ItemAcceptsInputMethod, true);

    mWidthBeforeResize = mpBorderItem->rect().width();
    mHeightBeforeResize = mpBorderItem->rect().height();
    mPosBeforeResize = this->pos();
}


TextBoxWidget::TextBoxWidget(const TextBoxWidget &other, SystemObject *pSystem)
    : Widget(other.pos(), other.rotation(), Deselected, pSystem, 0)
{
    mpBorderItem = new QGraphicsRectItem(other.mpBorderItem->rect(), this);
    if(other.mpBorderItem->isVisible())
    {
        mpBorderItem->show();
    }
    else
    {
        mpBorderItem->hide();
    }
    mpTextItem = new QGraphicsTextItem(other.mpTextItem->toPlainText(), this);
    mpTextItem->setFont(other.mpTextItem->font());
    mpTextItem->show();
    setLineColor(other.mpTextItem->defaultTextColor());
}

WidgetTypesEnumT TextBoxWidget::getWidgetType() const
{
    return TextBoxWidgetType;
}

QString TextBoxWidget::getHmfTagName() const
{
    return HMF_TEXTBOXWIDGETTAG;
}


void TextBoxWidget::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    Q_UNUSED(contents);

    QDomElement xmlObject = appendDomElement(rDomElement, getHmfTagName());

    //Save GUI related stuff
    QDomElement xmlGuiStuff = appendDomElement(xmlObject,HMF_HOPSANGUITAG);

    QPointF pos = mapToScene(boundingRect().topLeft());

    QDomElement xmlPose = appendDomElement(xmlGuiStuff, HMF_POSETAG);
    setQrealAttribute(xmlPose, "x", pos.x());
    setQrealAttribute(xmlPose, "y", pos.y());

    QDomElement xmlText = appendDomElement(xmlGuiStuff, "textobject");
    xmlText.setAttribute("text", mpTextItem->toPlainText());
    xmlText.setAttribute("font", mpTextItem->font().toString());
    xmlText.setAttribute("fontcolor", mpTextItem->defaultTextColor().name());
    xmlText.setAttribute("reflow", mReflowText);

    QDomElement xmlSize = appendDomElement(xmlGuiStuff, "size");
    setQrealAttribute(xmlSize, "width", mpBorderItem->rect().width());
    setQrealAttribute(xmlSize, "height", mpBorderItem->rect().height());

    QDomElement xmlLine = appendDomElement(xmlGuiStuff, "line");
    xmlLine.setAttribute("visible", mpBorderItem->isVisible());
    xmlLine.setAttribute("width", mpBorderItem->pen().width());
    xmlLine.setAttribute("color", mpBorderItem->pen().color().name());

    QString style;
    if(mpBorderItem->pen().style() == Qt::SolidLine)
        style = "solidline";
    else if(mpBorderItem->pen().style() == Qt::DashLine)
        style = "dashline";
    else if(mpBorderItem->pen().style() == Qt::DotLine)
        style = "dotline";
    else if(mpBorderItem->pen().style() == Qt::DashDotLine)
        style = "dashdotline";
    xmlLine.setAttribute(HMF_STYLETAG, style);
}

void TextBoxWidget::loadFromDomElement(QDomElement domElement)
{
    QFont font;
    QColor textColor, lineColor;

    // Read gui specific stuff
    QDomElement guiData = domElement.firstChildElement(HMF_HOPSANGUITAG);

    // Text
    QDomElement textObject = guiData.firstChildElement("textobject");
    QString text = textObject.attribute("text");
    font.fromString(textObject.attribute("font"));
    textColor.setNamedColor(textObject.attribute("fontcolor"));
    bool reflowText = parseAttributeBool(textObject, "reflow", false);

    // Box
    QDomElement poseTag = guiData.firstChildElement(HMF_POSETAG);
    QPointF point( parseAttributeQreal(poseTag,"x",0), parseAttributeQreal(poseTag,"y",0));
    QDomElement sizeTag = guiData.firstChildElement("size");
    double width = parseAttributeQreal(sizeTag, "width", 10);
    double height = parseAttributeQreal(sizeTag, "height", 10);
    QDomElement lineTag = guiData.firstChildElement("line");
    bool lineVisible = parseAttributeBool(lineTag, "visible", true);
    int linewidth = parseAttributeInt(lineTag, "width", 1);
    //! @todo this check is for backwards compatibility, remove in the future (added 20140224)
    if (linewidth == 1)
    {
        // Try double parsing
        linewidth = parseAttributeQreal(lineTag, "width", linewidth);
    }
    QString linestyle = lineTag.attribute(HMF_STYLETAG);
    lineColor.setNamedColor(lineTag.attribute("color",textColor.name()));

    setText(text);
    setFont(font);
    setTextColor(textColor);
    setReflowText(reflowText);
    setLineColor(lineColor);
    setLineWidth(linewidth);
    setBoxVisible(lineVisible);
    setSize(width, height);
    setPos(point);

    if(linestyle == "solidline")
        setLineStyle(Qt::SolidLine);
    if(linestyle == "dashline")
        setLineStyle(Qt::DashLine);
    if(linestyle == "dotline")
        setLineStyle(Qt::DotLine);
    if(linestyle == "dashdotline")
        setLineStyle(Qt::DashDotLine);

    // In case font is not correct this is nice to run
    makeSureBoxNotToSmallForText();
}

void TextBoxWidget::setText(QString text)
{
    mpTextItem->setPlainText(text);
    makeSureBoxNotToSmallForText();
    mpSelectionBox->setPassive();
}


void TextBoxWidget::setFont(QFont font)
{
    mpTextItem->setFont(font);
    makeSureBoxNotToSmallForText();
    mpSelectionBox->setPassive();
}

void TextBoxWidget::setTextColor(QColor color)
{
    mpTextItem->setDefaultTextColor(color);
}

void TextBoxWidget::setLineWidth(int value)
{
    QPen borderPen = mpBorderItem->pen();
    borderPen.setWidth(value);
    mpBorderItem->setPen(borderPen);
}


void TextBoxWidget::setLineStyle(Qt::PenStyle style)
{
    QPen borderPen = mpBorderItem->pen();
    borderPen.setStyle(style);
    mpBorderItem->setPen(borderPen);
}


void TextBoxWidget::setLineColor(QColor color)
{
    QPen borderPen = mpBorderItem->pen();
    borderPen.setColor(color);
    mpBorderItem->setPen(borderPen);
}


void TextBoxWidget::setSize(double w, double h)
{
    QPointF posBeforeResize = pos();
    mpBorderItem->setRect(mpBorderItem->rect().x(), mpBorderItem->rect().y(), w, h);
    setPos(posBeforeResize);
    mpBorderItem->setPos(mpBorderItem->pen().width()/2.0, mpBorderItem->pen().width()/2.0);

    if (mReflowText)
    {
        mpTextItem->setTextWidth(mpBorderItem->boundingRect().width());
    }
    else
    {
        mpTextItem->setTextWidth(-1);
    }

    refreshWidgetSize();
    mpSelectionBox->setActive();

    mWidthBeforeResize = mpBorderItem->rect().width();
    mHeightBeforeResize = mpBorderItem->rect().height();
    mPosBeforeResize = pos();
}


void TextBoxWidget::setBoxVisible(bool boxVisible)
{
    mpBorderItem->setVisible(boxVisible);
}

void TextBoxWidget::makeSureBoxNotToSmallForText()
{
    mpBorderItem->setRect(mpBorderItem->rect().united(mpTextItem->boundingRect()));
    refreshWidgetSize();
}

void TextBoxWidget::resizeBoxToText()
{
    mpBorderItem->setRect(mpBorderItem->rect().x(), mpBorderItem->rect().y(), mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    refreshWidgetSize();
}

void TextBoxWidget::resizeTextToBox()
{
    setReflowText(true);
    mpTextItem->setTextWidth(mpBorderItem->boundingRect().width());
}

void TextBoxWidget::setReflowText(bool doReflow)
{
    mReflowText = doReflow;
}


void TextBoxWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    if (mpEditDialog.isNull() && !mpParentSystemObject->mpModelWidget->getGraphicsView()->isCtrlKeyPressed())
    {
        // Open a dialog where line width and color can be selected
        mpEditDialog = new QDialog(gpMainWindowWidget);
        mpEditDialog->setWindowTitle("Edit TextBox Widget");
        mpEditDialog->setAttribute(Qt::WA_DeleteOnClose);

        mpDialogShowBorderCheckBox = new QCheckBox("Show box rectangle");
        mpDialogShowBorderCheckBox->setChecked(mpBorderItem->isVisible());

        mpDialogLineWidth = new QSpinBox();
        mpDialogLineWidth->setMinimum(1);
        mpDialogLineWidth->setMaximum(100);
        mpDialogLineWidth->setSingleStep(1);
        mpDialogLineWidth->setValue(mpBorderItem->pen().width());

        mpDialogLineColorButton = new QToolButton();
        mpDialogLineColorButton->setStyleSheet(QString("* { background-color: rgb(%1,%2,%3); }").arg(mpBorderItem->pen().color().red())
                                               .arg(mpBorderItem->pen().color().green())
                                               .arg(mpBorderItem->pen().color().blue()));
        mpDialogLineStyle = new QComboBox();
        mpDialogLineStyle->insertItem(0, "Solid Line");
        mpDialogLineStyle->insertItem(1, "Dashes");
        mpDialogLineStyle->insertItem(2, "Dots");
        mpDialogLineStyle->insertItem(3, "Dashes and Dots");
        if(mpBorderItem->pen().style() == Qt::SolidLine)
            mpDialogLineStyle->setCurrentIndex(0);
        if(mpBorderItem->pen().style() == Qt::DashLine)
            mpDialogLineStyle->setCurrentIndex(1);
        if(mpBorderItem->pen().style() == Qt::DotLine)
            mpDialogLineStyle->setCurrentIndex(2);
        if(mpBorderItem->pen().style() == Qt::DashDotLine)
            mpDialogLineStyle->setCurrentIndex(3);

        mpDialogTextBox = new QTextEdit();
        mpDialogTextBox->setPlainText(mpTextItem->toPlainText());
        mpDialogTextBox->setFont(mpTextItem->font());
        mpDialogFontButton = new QPushButton("Change Text Font");

        mpDialogTextColorButton = new QToolButton();
        mpDialogTextColorButton->setToolTip("Change text color");
        mpDialogTextColorButton->setStyleSheet(QString("* { background-color: rgb(%1,%2,%3); }").arg(mpTextItem->defaultTextColor().red())
                                               .arg(mpTextItem->defaultTextColor().green())
                                               .arg(mpTextItem->defaultTextColor().blue()));
        mpDialogReflowCheckBox = new QCheckBox(tr("Reflow text"));
        mpDialogReflowCheckBox->setChecked(mReflowText);

        mSelectedTextColor = mpTextItem->defaultTextColor();
        mSelectedLineColor = mpBorderItem->pen().color();

        QGridLayout *pEditGroupLayout = new QGridLayout();
        pEditGroupLayout->addWidget(mpDialogTextBox,                            0,0,1,3);
        pEditGroupLayout->addWidget(mpDialogFontButton,                         1,0);
        pEditGroupLayout->addWidget(mpDialogTextColorButton,                    1,1);
        pEditGroupLayout->addWidget(mpDialogReflowCheckBox,                     1,2);
        pEditGroupLayout->addWidget(mpDialogShowBorderCheckBox,                 2,0,1,3);
        pEditGroupLayout->addWidget(new QLabel("Line Width: ", mpEditDialog),   3,0);
        pEditGroupLayout->addWidget(mpDialogLineWidth,                          3,1,1,2);
        pEditGroupLayout->addWidget(new QLabel("Line Color: ", mpEditDialog),   4,0);
        pEditGroupLayout->addWidget(mpDialogLineColorButton,                    4,1,1,2);
        pEditGroupLayout->addWidget(new QLabel("Line Style: ", mpEditDialog),   5,0);
        pEditGroupLayout->addWidget(mpDialogLineStyle,                          5,1,1,2);

        QGroupBox *pEditGroupBox = new QGroupBox();
        pEditGroupBox->setLayout(pEditGroupLayout);

        QPushButton *pDialogDoneButton = new QPushButton("Done");
        QPushButton *pDialogCancelButton = new QPushButton("Cancel");
        QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
        pButtonBox->addButton(pDialogDoneButton, QDialogButtonBox::ActionRole);
        pButtonBox->addButton(pDialogCancelButton, QDialogButtonBox::ActionRole);

        QGridLayout *pDialogLayout = new QGridLayout();
        pDialogLayout->addWidget(pEditGroupBox,0,0);
        pDialogLayout->addWidget(pButtonBox,1,0);
        mpEditDialog->setLayout(pDialogLayout);
        mpEditDialog->show();

        this->setZValue(WidgetZValue);
        this->setFlag(QGraphicsItem::ItemStacksBehindParent, true);

        connect(mpDialogShowBorderCheckBox,    SIGNAL(toggled(bool)),  mpDialogLineWidth,       SLOT(setEnabled(bool)));
        connect(mpDialogShowBorderCheckBox,    SIGNAL(toggled(bool)),  mpDialogLineColorButton, SLOT(setEnabled(bool)));
        connect(mpDialogShowBorderCheckBox,    SIGNAL(toggled(bool)),  mpDialogLineStyle,       SLOT(setEnabled(bool)));
        connect(mpDialogFontButton,         SIGNAL(clicked()),      this,                   SLOT(openFontDialog()));
        connect(mpDialogTextColorButton,    SIGNAL(clicked()),      this,                   SLOT(openTextColorDialog()));
        connect(mpDialogLineColorButton,    SIGNAL(clicked()),      this,                   SLOT(openLineColorDialog()));
        connect(pDialogDoneButton,          SIGNAL(clicked()),      this,                   SLOT(updateWidgetFromDialog()));
        connect(pDialogCancelButton,        SIGNAL(clicked()),      mpEditDialog,           SLOT(close()));
    }
}

void TextBoxWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    WorkspaceObject::hoverMoveEvent(event);

    this->setCursor(Qt::ArrowCursor);
    mResizeLeft = false;
    mResizeRight = false;
    mResizeTop = false;
    mResizeBottom = false;

    int resLim = 5;

    if(event->pos().x() > boundingRect().left() && event->pos().x() < boundingRect().left()+resLim)
    {
        mResizeLeft = true;
    }
    if(event->pos().x() > boundingRect().right()-resLim && event->pos().x() < boundingRect().right())
    {
        mResizeRight = true;
    }
    if(event->pos().y() > boundingRect().top() && event->pos().y() < boundingRect().top()+resLim)
    {
        mResizeTop = true;
    }
    if(event->pos().y() > boundingRect().bottom()-resLim && event->pos().y() < boundingRect().bottom())
    {
        mResizeBottom = true;
    }

    if( (mResizeLeft && mResizeTop) || (mResizeRight && mResizeBottom) )
        this->setCursor(Qt::SizeFDiagCursor);
    else if( (mResizeTop && mResizeRight) || (mResizeBottom && mResizeLeft) )
        this->setCursor(Qt::SizeBDiagCursor);
    else if(mResizeLeft || mResizeRight)
        this->setCursor(Qt::SizeHorCursor);
    else if(mResizeTop || mResizeBottom)
        this->setCursor(Qt::SizeVerCursor);
}

void TextBoxWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(mResizeLeft || mResizeRight || mResizeTop || mResizeBottom)
    {
        mPosBeforeResize = this->pos();
        mWidthBeforeResize = this->mpBorderItem->rect().width();
        mHeightBeforeResize = this->mpBorderItem->rect().height();
        mIsResizing = true;
    }
    else
    {
        mIsResizing = false;
    }
    WorkspaceObject::mousePressEvent(event);
}


void TextBoxWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    WorkspaceObject::mouseMoveEvent(event);

    if (getModelLockLevel()==NotLocked)
    {
        if(mResizeLeft && mResizeTop)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), qMax(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), qMax(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            if(desiredRect.width() < mpTextItem->boundingRect().width())
                this->setX(mPosBeforeResize.x()+mWidthBeforeResize-mpBorderItem->boundingRect().width()+mpBorderItem->pen().widthF());
            if(desiredRect.height() < mpTextItem->boundingRect().height())
                this->setY(mPosBeforeResize.y()+mHeightBeforeResize-mpBorderItem->boundingRect().height()+mpBorderItem->pen().widthF());
        }
        else if(mResizeTop && mResizeRight)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), qMax(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), qMax(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            this->setX(mPosBeforeResize.x());
            if(desiredRect.height() < mpTextItem->boundingRect().height())
                this->setY(mPosBeforeResize.y()+mHeightBeforeResize-mpBorderItem->boundingRect().height()+mpBorderItem->pen().widthF());
        }
        else if(mResizeRight && mResizeBottom)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), qMax(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), qMax(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            this->setX(mPosBeforeResize.x());
            this->setY(mPosBeforeResize.y());
        }
        else if(mResizeBottom && mResizeLeft)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), qMax(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), qMax(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            this->setY(mPosBeforeResize.y());
            if(desiredRect.width() < mpTextItem->boundingRect().width())
                this->setX(mPosBeforeResize.x()+mWidthBeforeResize-mpBorderItem->boundingRect().width()+mpBorderItem->pen().widthF());
        }
        else if(mResizeLeft)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), qMax(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), mpBorderItem->rect().height());
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            this->setY(mPosBeforeResize.y());
            if(desiredRect.width() < mpTextItem->boundingRect().width())
                this->setX(mPosBeforeResize.x()+mWidthBeforeResize-mpBorderItem->boundingRect().width()+mpBorderItem->pen().widthF());
        }
        else if(mResizeRight)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), qMax(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), mpBorderItem->rect().height());
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            this->setPos(mPosBeforeResize);
        }
        else if(mResizeTop)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(),  mpBorderItem->rect().width(), qMax(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            this->setX(mPosBeforeResize.x());
            if(desiredRect.height() < mpTextItem->boundingRect().height())
                this->setY(mPosBeforeResize.y()+mHeightBeforeResize-mpBorderItem->boundingRect().height()+mpBorderItem->pen().widthF());
        }
        else if(mResizeBottom)
        {
            QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), mpBorderItem->rect().width(), qMax(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
            if (mReflowText)
            {
                mpTextItem->setTextWidth(desiredRect.width());
            }
            mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
            this->setPos(mPosBeforeResize);
        }

        mpBorderItem->setPos(mpBorderItem->pen().width()/2.0, mpBorderItem->pen().width()/2.0);

        refreshWidgetSize();
        mpSelectionBox->setActive();
    }
}


void TextBoxWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Widget::mouseReleaseEvent(event);
    if(mWidthBeforeResize != mpBorderItem->rect().width() || mHeightBeforeResize != mpBorderItem->rect().height())
    {
        mpParentSystemObject->getUndoStackPtr()->newPost();
        mpParentSystemObject->getUndoStackPtr()->registerResizedTextBoxWidget( mWidgetIndex, mWidthBeforeResize, mHeightBeforeResize, mpBorderItem->rect().width(), mpBorderItem->rect().height(), mPosBeforeResize, this->pos());
        mWidthBeforeResize = mpBorderItem->rect().width();
        mHeightBeforeResize = mpBorderItem->rect().height();
        mPosBeforeResize = this->pos();
    }
}

void TextBoxWidget::refreshSelectionBoxSize()
{
    mpSelectionBox->setSize(0.0, 0.0, boundingRect().width(), boundingRect().height());
}

void TextBoxWidget::deleteMe(UndoStatusEnumT undoSettings)
{
    if (!isLocallyLocked() && getModelLockLevel()==NotLocked)
    {
        mpParentSystemObject->deleteWidget(this, undoSettings);
    }
}

void TextBoxWidget::flipVertical(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings);
    // Nothing for now
}

void TextBoxWidget::flipHorizontal(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings);
    // Nothing for now
}


void TextBoxWidget::updateWidgetFromDialog()
{
    Qt::PenStyle selectedStyle;
    switch(mpDialogLineStyle->currentIndex())
    {
    case 0:
        selectedStyle = Qt::SolidLine;
        break;
    case 1:
        selectedStyle = Qt::DashLine;
        break;
    case 2:
        selectedStyle = Qt::DotLine;
        break;
    case 3:
        selectedStyle = Qt::DashDotLine;
        break;
    default:
        selectedStyle = Qt::SolidLine;  // This shall never happen
    }

    // Remember for UnDo
    mpParentSystemObject->getUndoStackPtr()->newPost("Modified TextBox");
    mpParentSystemObject->getUndoStackPtr()->registerModifiedWidget(this);

    // Update text
    mReflowText = mpDialogReflowCheckBox->isChecked();
    if (mReflowText)
    {
        mpTextItem->setTextWidth(boundingRect().width());
    }
    else
    {
        mpTextItem->setTextWidth(-1);
    }
    mpTextItem->setPlainText(mpDialogTextBox->toPlainText());
    mpTextItem->setFont(mpDialogTextBox->font());
    mpTextItem->setDefaultTextColor(mSelectedTextColor);

    // Update border box
    QPen borderPen = mpBorderItem->pen();
    borderPen.setColor(mSelectedLineColor);
    borderPen.setWidth(mpDialogLineWidth->value());
    borderPen.setStyle(selectedStyle);
    mpBorderItem->setPen(borderPen);
    mpBorderItem->setRect(mpBorderItem->rect().united(mpTextItem->boundingRect()));
    mpBorderItem->setVisible(mpDialogShowBorderCheckBox->isChecked());

    // Update the size
    refreshWidgetSize();

    if(this->isSelected())
    {
        mpSelectionBox->setActive();
    }

    mpParentSystemObject->mpModelWidget->hasChanged();

    mpEditDialog->close();
}


void TextBoxWidget::openFontDialog()
{
    bool ok;
    // We need to create a fontinfo object here to figure out what font is actually being used to render the text
    // The qfont object might not correspond exactly to the actual font used
    QFontInfo fi(mpDialogTextBox->font());
    qDebug()  << fi.family();
    qDebug()  << fi.styleName();
    qDebug()  << fi.style();
    qDebug()  << fi.styleHint();
    qDebug()  << fi.weight();
    QFont initialFont(fi.family(), fi.pointSize());
#ifdef _WIN32
    initialFont.setBold(fi.bold());
    initialFont.setItalic(fi.italic());
#else
    initialFont.setStyleName(fi.styleName());
#endif
    QFont font = QFontDialog::getFont(&ok, initialFont, gpMainWindowWidget);
    if (ok)
    {
        mpDialogTextBox->setFont(font);
    }
}

void TextBoxWidget::openTextColorDialog()
{
    QColor color = QColorDialog::getColor(mSelectedTextColor, gpMainWindowWidget);
    if (color.isValid())
    {
        mSelectedTextColor = color;
        mpDialogTextColorButton->setStyleSheet(QString("* { background-color: rgb(%1,%2,%3) }").arg(color.red()).arg(color.green()).arg(color.blue()));
    }
}

void TextBoxWidget::openLineColorDialog()
{
    QColor color = QColorDialog::getColor(mSelectedTextColor, gpMainWindowWidget);
    if (color.isValid())
    {
        mSelectedLineColor = color;
        QString style = QString("* { background-color: rgb(%1,%2,%3) }").arg(color.red()).arg(color.green()).arg(color.blue());
        mpDialogLineColorButton->setStyleSheet(style);
    }
}

void TextBoxWidget::refreshWidgetSize()
{
    resize(mpBorderItem->boundingRect().width(), mpBorderItem->boundingRect().height());
    refreshSelectionBoxSize();
}

ImageWidget::ImageWidget(QPointF pos, double rot, SelectionStatusEnumT startSelected, SystemObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent)
    : Widget(pos, rot, startSelected, pSystem, pParent)
{
    mWidgetIndex = widgetIndex;
    setImage("");

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setZValue(WidgetZValue);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    refreshWidgetSize();
}

ImageWidget::ImageWidget(const ImageWidget &other, SystemObject *pSystem)
 : Widget(other.pos(), other.rotation(), Deselected, pSystem, 0)
{
    mpImage = new QGraphicsSvgItem(other.mpImage);
    mpImage->show();
}

void ImageWidget::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    Q_UNUSED(contents);

    QDomElement xmlObject = appendDomElement(rDomElement, getHmfTagName());

    //Save GUI related stuff
    QDomElement xmlGuiStuff = appendDomElement(xmlObject,HMF_HOPSANGUITAG);

    QPointF pos = mapToScene(boundingRect().topLeft());

    QDomElement xmlPose = appendDomElement(xmlGuiStuff, HMF_POSETAG);
    setQrealAttribute(xmlPose, hmf::x, pos.x());
    setQrealAttribute(xmlPose, hmf::y, pos.y());

    QDomElement xmlImage = appendDomElement(xmlGuiStuff, hmf::image);
    xmlImage.setAttribute(hmf::path, mImagePath);
    xmlImage.setAttribute(hmf::scale, mpImage->scale());
}

WidgetTypesEnumT ImageWidget::getWidgetType() const
{
    return ImageWidgetType;
}

QString ImageWidget::getHmfTagName() const
{
    return hmf::imagewidget;
}

void ImageWidget::setImage(const QString &path, double iconScale)
{
    mImagePath = path;

    if (mpImage != 0)
    {
        mpImage->deleteLater(); //Schedule previous icon for deletion
    }

    if(path.isEmpty()) {
        mpImage = new QGraphicsSvgItem(mDefaultImage, this);
    }
    else {
        mpImage = new QGraphicsSvgItem(path, this);
    }
    mpImage->setFlags(QGraphicsItem::ItemStacksBehindParent);

    mpImage->setPos(this->boundingRect().topLeft());
    mpImage->show();
    mpImage->setAcceptHoverEvents(false);
    mpImage->setScale(iconScale);

    this->prepareGeometryChange();
    this->resize(mpImage->boundingRect().width(), mpImage->boundingRect().height());  //Resize modelobject
    mpSelectionBox->setSize(0.0, 0.0, mpImage->boundingRect().width(), mpImage->boundingRect().height()); //Resize selection box

    this->setTransformOriginPoint(this->boundingRect().center());

    mpImage->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
}

void ImageWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    if (mpEditDialog.isNull() && !mpParentSystemObject->mpModelWidget->getGraphicsView()->isCtrlKeyPressed())
    {
        // Open a dialog where image can be selected and scaled
        mpEditDialog = new QDialog(gpMainWindowWidget);
        mpEditDialog->setWindowTitle("Edit Image Widget");
        mpEditDialog->setAttribute(Qt::WA_DeleteOnClose);

        mpEditDialogPathLabel = new QLabel(tr("Path:"), mpEditDialog);
        mpEditDialogPathLineEdit = new QLineEdit(mpEditDialog);
        mpEditDialogPathLineEdit->setText(mImagePath);
        mpEditDialogBrowseButton = new QToolButton(mpEditDialog);
        mpEditDialogBrowseButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Open.svg"));
        mpEditDialogScaleLabel = new QLabel(tr("Scale:"), mpEditDialog);
        mpEditDialogScaleSpinBox = new QDoubleSpinBox(mpEditDialog);
        mpEditDialogScaleSpinBox->setValue(mpImage->scale());
        mpEditDialogScaleSpinBox->setMinimum(DBL_MIN);
        mpEditDialogScaleSpinBox->setMaximum(DBL_MAX);
        mpEditDialogScaleSpinBox->setSingleStep(0.01);


        QHBoxLayout *pHorizontalLayout1 = new QHBoxLayout();
        pHorizontalLayout1->addWidget(mpEditDialogPathLabel);
        pHorizontalLayout1->addWidget(mpEditDialogPathLineEdit);
        pHorizontalLayout1->addWidget(mpEditDialogBrowseButton);
        pHorizontalLayout1->setStretch(1,1);

        QHBoxLayout *pHorizontalLayout2 = new QHBoxLayout();
        pHorizontalLayout2->addWidget(mpEditDialogScaleLabel);
        pHorizontalLayout2->addWidget(mpEditDialogScaleSpinBox);
        pHorizontalLayout2->setStretch(1,1);

        QPushButton *pDialogDoneButton = new QPushButton("Done");
        QPushButton *pDialogCancelButton = new QPushButton("Cancel");
        QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
        pButtonBox->addButton(pDialogDoneButton, QDialogButtonBox::ActionRole);
        pButtonBox->addButton(pDialogCancelButton, QDialogButtonBox::ActionRole);

        QVBoxLayout *pVerticalLayout = new QVBoxLayout(mpEditDialog);
        pVerticalLayout->addLayout(pHorizontalLayout1);
        pVerticalLayout->addLayout(pHorizontalLayout2);
        pVerticalLayout->addWidget(pButtonBox,1,0);

        mpEditDialog->show();

        connect(mpEditDialogBrowseButton,   SIGNAL(clicked()),      this,           SLOT(browseForImageFile()));
        connect(pDialogDoneButton,          SIGNAL(clicked()),      this,           SLOT(updateWidgetFromDialog()));
        connect(pDialogCancelButton,        SIGNAL(clicked()),      mpEditDialog,   SLOT(close()));
    }
}

void ImageWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    WorkspaceObject::hoverMoveEvent(event);
}

void ImageWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    WorkspaceObject::mousePressEvent(event);
}


void ImageWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    WorkspaceObject::mouseMoveEvent(event);

    if (getModelLockLevel()==NotLocked)
    {
        mpSelectionBox->setActive();
    }
}


void ImageWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Widget::mouseReleaseEvent(event);
}

void ImageWidget::refreshSelectionBoxSize()
{
    mpSelectionBox->setSize(0.0, 0.0, boundingRect().width(), boundingRect().height());
}

void ImageWidget::deleteMe(UndoStatusEnumT undoSettings)
{
    if (!isLocallyLocked() && getModelLockLevel()==NotLocked)
    {
        mpParentSystemObject->deleteWidget(this, undoSettings);
    }
}

void ImageWidget::flipVertical(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings);
}

void ImageWidget::flipHorizontal(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings);
}

void ImageWidget::browseForImageFile()
{
    QString filePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Select Image File"),
                                                    gpConfig->getStringSetting(CFG_MODELGFXDIR),
                                                    tr("Scalar Vector Graphics (*.svg)"));
    if(!filePath.isEmpty()) {
        mpEditDialogPathLineEdit->setText(filePath);
    }
}

void ImageWidget::updateWidgetFromDialog()
{
    // Remember for UnDo
    mpParentSystemObject->getUndoStackPtr()->newPost("Modified Image Widget");
    mpParentSystemObject->getUndoStackPtr()->registerModifiedWidget(this);

    QString path = mpEditDialogPathLineEdit->text();
    if(!path.isEmpty() && !QFileInfo::exists(path)) {
        gpMessageHandler->addErrorMessage("Specified image file does not exist.");
        path = "";
    }
    setImage(mpEditDialogPathLineEdit->text(), mpEditDialogScaleSpinBox->value());
    refreshWidgetSize();
    mpEditDialog->close();
}


void ImageWidget::loadFromDomElement(QDomElement domElement)
{
    // Read gui specific stuff
    QDomElement guiData = domElement.firstChildElement(HMF_HOPSANGUITAG);

    //Pose
    QDomElement poseObject = guiData.firstChildElement(HMF_POSETAG);
    double x = parseAttributeQreal(poseObject, hmf::x, 0);
    double y = parseAttributeQreal(poseObject, hmf::y, 0);
    this->setPos(x, y);

    // Image
    QDomElement imageObject = guiData.firstChildElement(hmf::image);
    QString path = imageObject.attribute(hmf::path);
    double scale = parseAttributeQreal(imageObject, hmf::scale, 1.0);

    setImage(path, scale);
    refreshWidgetSize();
}

void ImageWidget::refreshWidgetSize()
{
    resize(mpImage->boundingRect().width()*mpImage->scale(), mpImage->boundingRect().height()*mpImage->scale());
    refreshSelectionBoxSize();
}
