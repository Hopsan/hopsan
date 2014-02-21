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
//! @file   GUIWidgets.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIWidgets classes
//!
//$Id$


#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDialog>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QGraphicsRectItem>

#include "global.h"
#include "GUIWidgets.h"
#include "GUISystem.h"
#include "Widgets/ModelWidget.h"
#include "Utilities/GUIUtilities.h"
#include "UndoStack.h"

Widget::Widget(QPointF pos, qreal rot, SelectionStatusEnumT startSelected, ContainerObject *pSystem, QGraphicsItem *pParent)
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


void Widget::setOldPos()
{
    mOldPos = this->pos();
}


void Widget::setWidgetIndex(int idx)
{
    mWidgetIndex = idx;
}


int Widget::getWidgetIndex()
{
    return mWidgetIndex;
}

QVariant Widget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if(this->isSelected())
        {
            mpParentContainerObject->rememberSelectedWidget(this);
        }
        else
        {
            mpParentContainerObject->forgetSelectedWidget(this);
        }
    }

    return WorkspaceObject::itemChange(change, value);
}


void Widget::deleteMe(UndoStatusEnumT /*undoSettings*/)
{
    // Should be overloaded
    qFatal("This function must be overloaded");
}


void Widget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QList<Widget *>::iterator it;

        //Loop through all selected widgets and register changed positions in undo stack
    bool alreadyClearedRedo = false;
    QList<Widget *> selectedWidgets = mpParentContainerObject->getSelectedGUIWidgetPtrs();
    for(int i=0; i<selectedWidgets.size(); ++i)
    {
        if((selectedWidgets[i]->mOldPos != selectedWidgets[i]->pos()) && (event->button() == Qt::LeftButton) && !selectedWidgets[i]->mIsResizing)
        {
                //This check makes sure that only one undo post is created when moving several objects at once
            if(!alreadyClearedRedo)
            {
                if(mpParentContainerObject->getSelectedGUIWidgetPtrs().size() > 1)
                {
                    mpParentContainerObject->getUndoStackPtr()->newPost("movedmultiplewidgets");
                }
                else
                {
                    mpParentContainerObject->getUndoStackPtr()->newPost();
                }
                mpParentContainerObject->mpModelWidget->hasChanged();
                alreadyClearedRedo = true;
            }

            mpParentContainerObject->getUndoStackPtr()->registerMovedWidget(selectedWidgets[i], selectedWidgets[i]->mOldPos, selectedWidgets[i]->pos());
        }
        selectedWidgets[i]->mIsResizing = false;
    }

    WorkspaceObject::mouseReleaseEvent(event);
}


TextBoxWidget::TextBoxWidget(QString text, QPointF pos, qreal rot, SelectionStatusEnumT startSelected, ContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent)
    : Widget(pos, rot, startSelected, pSystem, pParent)
{
    mType="TextBoxWidget";
    mHmfTagName = HMF_TEXTBOXWIDGETTAG;
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


TextBoxWidget::TextBoxWidget(const TextBoxWidget &other, ContainerObject *pSystem)
    : Widget(other.pos(), other.rotation(), Deselected, pSystem, 0)
{
    mType = other.mType;
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


void TextBoxWidget::saveToDomElement(QDomElement &rDomElement)
{
    QDomElement xmlObject = appendDomElement(rDomElement, mHmfTagName);

    //Save GUI realted stuff
    QDomElement xmlGuiStuff = appendDomElement(xmlObject,HMF_HOPSANGUITAG);

    QPointF pos = mapToScene(boundingRect().topLeft());

    QDomElement xmlPose = appendDomElement(xmlGuiStuff, HMF_POSETAG);
    setQrealAttribute(xmlPose, "x", pos.x());
    setQrealAttribute(xmlPose, "y", pos.y());

    QDomElement xmlText = appendDomElement(xmlGuiStuff, "textobject");
    xmlText.setAttribute("text", mpTextItem->toPlainText());
    xmlText.setAttribute("font", mpTextItem->font().toString());
    xmlText.setAttribute("fontcolor", mpTextItem->defaultTextColor().name());

    QDomElement xmlSize = appendDomElement(xmlGuiStuff, "size");
    setQrealAttribute(xmlSize, "width", mpBorderItem->rect().width());
    setQrealAttribute(xmlSize, "height", mpBorderItem->rect().height());

    QDomElement xmlLine = appendDomElement(xmlGuiStuff, "line");
    xmlLine.setAttribute("visible", mpBorderItem->isVisible());
    setQrealAttribute(xmlLine, "width", mpBorderItem->pen().width());
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


void TextBoxWidget::setSize(qreal w, qreal h)
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
    reflowText(true);
    mpTextItem->setTextWidth(mpBorderItem->boundingRect().width());
}

void TextBoxWidget::reflowText(bool doReflow)
{
    mReflowText = doReflow;
}


void TextBoxWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

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

    mSelectedFont = mpTextItem->font();
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
    this->setFlags(QGraphicsItem::ItemStacksBehindParent);

    connect(mpDialogShowBorderCheckBox,    SIGNAL(toggled(bool)),  mpDialogLineWidth,       SLOT(setEnabled(bool)));
    connect(mpDialogShowBorderCheckBox,    SIGNAL(toggled(bool)),  mpDialogLineColorButton, SLOT(setEnabled(bool)));
    connect(mpDialogShowBorderCheckBox,    SIGNAL(toggled(bool)),  mpDialogLineStyle,       SLOT(setEnabled(bool)));
    connect(mpDialogFontButton,         SIGNAL(clicked()),      this,                   SLOT(openFontDialog()));
    connect(mpDialogTextColorButton,    SIGNAL(clicked()),      this,                   SLOT(openTextColorDialog()));
    connect(mpDialogLineColorButton,    SIGNAL(clicked()),      this,                   SLOT(openLineColorDialog()));
    connect(pDialogDoneButton,          SIGNAL(clicked()),      this,                   SLOT(updateWidgetFromDialog()));
    connect(pDialogCancelButton,        SIGNAL(clicked()),      mpEditDialog,           SLOT(close()));
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

    if(mResizeLeft && mResizeTop)
    {
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), max(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), max(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
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
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), max(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), max(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
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
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), max(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), max(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
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
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), max(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), max(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
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
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), max(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), mpBorderItem->rect().height());
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
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), max(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), mpBorderItem->rect().height());
        if (mReflowText)
        {
            mpTextItem->setTextWidth(desiredRect.width());
        }
        mpBorderItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setPos(mPosBeforeResize);
    }
    else if(mResizeTop)
    {
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(),  mpBorderItem->rect().width(), max(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
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
        QRectF desiredRect = QRectF(mpBorderItem->rect().x(), mpBorderItem->rect().y(), mpBorderItem->rect().width(), max(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
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


void TextBoxWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Widget::mouseReleaseEvent(event);
    if(mWidthBeforeResize != mpBorderItem->rect().width() || mHeightBeforeResize != mpBorderItem->rect().height())
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        mpParentContainerObject->getUndoStackPtr()->registerResizedTextBoxWidget( mWidgetIndex, mWidthBeforeResize, mHeightBeforeResize, mpBorderItem->rect().width(), mpBorderItem->rect().height(), mPosBeforeResize, this->pos());
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
    mpParentContainerObject->deleteWidget(this, undoSettings);
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
    //! @todo undo settings should use save to xml instead, also missing reflow setting
    mpParentContainerObject->getUndoStackPtr()->newPost();
    mpParentContainerObject->getUndoStackPtr()->registerModifiedTextBoxWidget(mWidgetIndex, mpTextItem->toPlainText(), mpTextItem->font(), mpTextItem->defaultTextColor(),
                                                                              mpDialogTextBox->toPlainText(), mSelectedFont, mSelectedTextColor,
                                                                              mpBorderItem->pen().width(), mpBorderItem->pen().style(), mpDialogLineWidth->value(), selectedStyle,
                                                                              mpBorderItem->isVisible(), mpDialogShowBorderCheckBox->isChecked());

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
    mpTextItem->setFont(mSelectedFont);
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

    mpParentContainerObject->mpModelWidget->hasChanged();

    mpEditDialog->close();
}


void TextBoxWidget::openFontDialog()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, mpDialogTextBox->font(), gpMainWindowWidget);
    if (ok)
    {
        mSelectedFont = font;
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
