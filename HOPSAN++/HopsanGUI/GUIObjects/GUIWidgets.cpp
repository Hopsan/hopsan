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
#include <cassert>


#include "GUIWidgets.h"
#include "GUISystem.h"
#include "Widgets/ProjectTabWidget.h"
#include "MainWindow.h"
#include "Utilities/GUIUtilities.h"
#include "UndoStack.h"

using namespace std;


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
    assert(1 == 2);
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

    this->mHmfTagName = HMF_TEXTBOXWIDGETTAG;

    mpRectItem = new QGraphicsRectItem(0, 0, 100, 100, this);
    QPen tempPen = mpRectItem->pen();
    tempPen.setWidth(2);
    tempPen.setStyle(Qt::SolidLine);//Qt::DotLine);
    tempPen.setCapStyle(Qt::RoundCap);
    tempPen.setJoinStyle(Qt::RoundJoin);
    mpRectItem->setPen(tempPen);
    mpRectItem->setPos(mpRectItem->pen().width()/2.0, mpRectItem->pen().width()/2.0);
    mpRectItem->show();

    mpTextItem = new QGraphicsTextItem(text, this);
    QFont tempFont = mpTextItem->font();
    tempFont.setPointSize(12);
    mpTextItem->setFont(tempFont);
    mpTextItem->setPos(this->boundingRect().center());
    mpTextItem->show();
    mpTextItem->setAcceptHoverEvents(false);

    this->setColor(QColor("darkolivegreen"));

    this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    mpSelectionBox->setSize(0.0, 0.0, max(mpTextItem->boundingRect().width(), mpRectItem->boundingRect().width()),
                                      max(mpTextItem->boundingRect().height(), mpRectItem->boundingRect().height()));

    this->resize(max(mpTextItem->boundingRect().width(), mpRectItem->boundingRect().width()),
                 max(mpTextItem->boundingRect().height(), mpRectItem->boundingRect().height()));

      this->setFlag(QGraphicsItem::ItemAcceptsInputMethod, true);

    mWidthBeforeResize = mpRectItem->rect().width();
    mHeightBeforeResize = mpRectItem->rect().height();
    mPosBeforeResize = this->pos();

    mWidgetIndex = widgetIndex;
}


TextBoxWidget::TextBoxWidget(const TextBoxWidget &other, ContainerObject *pSystem)
    : Widget(other.pos(), other.rotation(), Deselected, pSystem, 0)
{
    mType = other.mType;
    mpRectItem = new QGraphicsRectItem(other.mpRectItem->rect(), this, pSystem->getContainedScenePtr());
    if(other.mpRectItem->isVisible())
    {
        mpRectItem->show();
    }
    else
    {
        mpRectItem->hide();
    }
    mpTextItem = new QGraphicsTextItem(other.mpTextItem->toPlainText(), this, pSystem->getContainedScenePtr());
    mpTextItem->setFont(other.mpTextItem->font());
    mpTextItem->show();
    setColor(other.mpTextItem->defaultTextColor());
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
    setQrealAttribute(xmlSize, "width", mpRectItem->rect().width());
    setQrealAttribute(xmlSize, "height", mpRectItem->rect().height());

    QDomElement xmlLine = appendDomElement(xmlGuiStuff, "line");
    xmlLine.setAttribute("visible", mpRectItem->isVisible());
    setQrealAttribute(xmlLine, "width", mpRectItem->pen().width());

    QString style;
    if(mpRectItem->pen().style() == Qt::SolidLine)
        style = "solidline";
    else if(mpRectItem->pen().style() == Qt::DashLine)
        style = "dashline";
    else if(mpRectItem->pen().style() == Qt::DotLine)
        style = "dotline";
    else if(mpRectItem->pen().style() == Qt::DashDotLine)
        style = "dashdotline";
    xmlLine.setAttribute(HMF_STYLETAG, style);
}

void TextBoxWidget::setText(QString text)
{
    mpTextItem->setPlainText(text);
    mpRectItem->setRect(mpRectItem->rect().united(mpTextItem->boundingRect()));

    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
    mpSelectionBox->setPassive();
}


void TextBoxWidget::setFont(QFont font)
{
    mpTextItem->setFont(font);
    mpRectItem->setRect(mpRectItem->rect().united(mpTextItem->boundingRect()));
    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
    mpSelectionBox->setPassive();
    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
}


void TextBoxWidget::setLineWidth(int value)
{
    QPen tempPen;
    tempPen = mpRectItem->pen();
    tempPen.setWidth(value);
    mpRectItem->setPen(tempPen);
}


void TextBoxWidget::setLineStyle(Qt::PenStyle style)
{
    QPen tempPen;
    tempPen = mpRectItem->pen();
    tempPen.setStyle(style);
    mpRectItem->setPen(tempPen);
}


void TextBoxWidget::setColor(QColor color)
{
    mpTextItem->setDefaultTextColor(color);

    QPen tempPen;
    tempPen = mpRectItem->pen();
    tempPen.setColor(color);
    mpRectItem->setPen(tempPen);
}


void TextBoxWidget::setSize(qreal w, qreal h)
{
    QPointF posBeforeResize = this->pos();
    mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), w, h);
    this->setPos(posBeforeResize);

    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    mpSelectionBox->setActive();
    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
    mpRectItem->setPos(mpRectItem->pen().width()/2.0, mpRectItem->pen().width()/2.0);

    mWidthBeforeResize = mpRectItem->rect().width();
    mHeightBeforeResize = mpRectItem->rect().height();
    mPosBeforeResize = this->pos();
}


void TextBoxWidget::setBoxVisible(bool boxVisible)
{
    mpRectItem->setVisible(boxVisible);
}


void TextBoxWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
{
    //! @todo Make a separate file for this dialog

        //Open a dialog where line width and color can be selected
    mpEditDialog = new QDialog(gpMainWindow);
    mpEditDialog->setWindowTitle("Edit Box Widget");

    mpShowBoxCheckBoxInDialog = new QCheckBox("Show box rectangle");
    mpShowBoxCheckBoxInDialog->setChecked(mpRectItem->isVisible());

    mpWidthLabelInDialog = new QLabel("Line Width: ");
    mpWidthBoxInDialog = new QSpinBox();
    mpWidthBoxInDialog->setMinimum(1);
    mpWidthBoxInDialog->setMaximum(100);
    mpWidthBoxInDialog->setSingleStep(1);
    mpWidthBoxInDialog->setValue(mpRectItem->pen().width());
    mpColorLabelInDialog = new QLabel("Line Color: ");
    mpColorInDialogButton = new QToolButton();
    QString redString, greenString, blueString;
    redString.setNum(mpRectItem->pen().color().red());
    greenString.setNum(mpRectItem->pen().color().green());
    blueString.setNum(mpRectItem->pen().color().blue());
    mpColorInDialogButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mpColorInDialogButton->setAutoRaise(true);

    mpStyleLabelInDialog = new QLabel("Line Style: ");
    mpStyleBoxInDialog = new QComboBox();
    mpStyleBoxInDialog->insertItem(0, "Solid Line");
    mpStyleBoxInDialog->insertItem(1, "Dashes");
    mpStyleBoxInDialog->insertItem(2, "Dots");
    mpStyleBoxInDialog->insertItem(3, "Dashes and Dots");

    if(mpRectItem->pen().style() == Qt::SolidLine)
        mpStyleBoxInDialog->setCurrentIndex(0);
    if(mpRectItem->pen().style() == Qt::DashLine)
        mpStyleBoxInDialog->setCurrentIndex(1);
    if(mpRectItem->pen().style() == Qt::DotLine)
        mpStyleBoxInDialog->setCurrentIndex(2);
    if(mpRectItem->pen().style() == Qt::DashDotLine)
        mpStyleBoxInDialog->setCurrentIndex(3);

    mpTextBoxInDialog = new QTextEdit();
    mpTextBoxInDialog->setPlainText(mpTextItem->toPlainText());
    mpTextBoxInDialog->setTextColor(mSelectedColor);
    mpTextBoxInDialog->setFont(mSelectedFont);
    mpFontInDialogButton = new QPushButton("Change Font");

    QGridLayout *pEditGroupLayout = new QGridLayout();
    pEditGroupLayout->addWidget(mpTextBoxInDialog,          0,0,1,2);
    pEditGroupLayout->addWidget(mpFontInDialogButton,       1,0,1,2);
    pEditGroupLayout->addWidget(mpShowBoxCheckBoxInDialog,  2,0,1,2);
    pEditGroupLayout->addWidget(mpWidthLabelInDialog,       3,0);
    pEditGroupLayout->addWidget(mpWidthBoxInDialog,         3,1);
    pEditGroupLayout->addWidget(mpColorLabelInDialog,       4,0);
    pEditGroupLayout->addWidget(mpColorInDialogButton,      4,1);
    pEditGroupLayout->addWidget(mpStyleLabelInDialog,       5,0);
    pEditGroupLayout->addWidget(mpStyleBoxInDialog,         5,1);

    QGroupBox *pEditGroupBox = new QGroupBox();
    pEditGroupBox->setLayout(pEditGroupLayout);

    mSelectedFont = mpTextItem->font();
    mSelectedColor = mpTextItem->defaultTextColor();

    mpDoneInDialogButton = new QPushButton("Done");
    mpCancelInDialogButton = new QPushButton("Cancel");
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpDoneInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpCancelInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout();
    pDialogLayout->addWidget(pEditGroupBox,0,0);
    pDialogLayout->addWidget(pButtonBox,1,0);
    mpEditDialog->setLayout(pDialogLayout);
    mpEditDialog->show();

    this->setZValue(WidgetZValue);
    this->setFlags(QGraphicsItem::ItemStacksBehindParent);

    mSelectedColor = mpRectItem->pen().color();

    connect(mpShowBoxCheckBoxInDialog, SIGNAL(toggled(bool)), mpWidthLabelInDialog, SLOT(setEnabled(bool)));
    connect(mpShowBoxCheckBoxInDialog, SIGNAL(toggled(bool)), mpWidthBoxInDialog, SLOT(setEnabled(bool)));
    connect(mpShowBoxCheckBoxInDialog, SIGNAL(toggled(bool)), mpStyleLabelInDialog, SLOT(setEnabled(bool)));
    connect(mpShowBoxCheckBoxInDialog, SIGNAL(toggled(bool)), mpStyleBoxInDialog, SLOT(setEnabled(bool)));
    connect(mpFontInDialogButton,SIGNAL(clicked()),this,SLOT(openFontDialog()));
    connect(mpColorInDialogButton,SIGNAL(clicked()),this,SLOT(openColorDialog()));
    connect(mpDoneInDialogButton,SIGNAL(clicked()),this,SLOT(updateWidgetFromDialog()));
    connect(mpCancelInDialogButton,SIGNAL(clicked()),mpEditDialog,SLOT(close()));
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
        mWidthBeforeResize = this->mpRectItem->rect().width();
        mHeightBeforeResize = this->mpRectItem->rect().height();
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
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(), max(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), max(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        if(desiredRect.width() < mpTextItem->boundingRect().width())
            this->setX(mPosBeforeResize.x()+mWidthBeforeResize-mpRectItem->boundingRect().width()+mpRectItem->pen().widthF());
        if(desiredRect.height() < mpTextItem->boundingRect().height())
            this->setY(mPosBeforeResize.y()+mHeightBeforeResize-mpRectItem->boundingRect().height()+mpRectItem->pen().widthF());
    }
    else if(mResizeTop && mResizeRight)
    {
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(), max(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), max(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setX(mPosBeforeResize.x());
        if(desiredRect.height() < mpTextItem->boundingRect().height())
            this->setY(mPosBeforeResize.y()+mHeightBeforeResize-mpRectItem->boundingRect().height()+mpRectItem->pen().widthF());
    }
    else if(mResizeRight && mResizeBottom)
    {
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(), max(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), max(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setX(mPosBeforeResize.x());
        this->setY(mPosBeforeResize.y());
    }
    else if(mResizeBottom && mResizeLeft)
    {
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(), max(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), max(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setY(mPosBeforeResize.y());
        if(desiredRect.width() < mpTextItem->boundingRect().width())
            this->setX(mPosBeforeResize.x()+mWidthBeforeResize-mpRectItem->boundingRect().width()+mpRectItem->pen().widthF());
    }
    else if(mResizeLeft)
    {
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(), max(0.0, mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x()), mpRectItem->rect().height());
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setY(mPosBeforeResize.y());
        if(desiredRect.width() < mpTextItem->boundingRect().width())
            this->setX(mPosBeforeResize.x()+mWidthBeforeResize-mpRectItem->boundingRect().width()+mpRectItem->pen().widthF());
    }
    else if(mResizeRight)
    {
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(), max(0.0, mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x()), mpRectItem->rect().height());
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setPos(mPosBeforeResize);
    }
    else if(mResizeTop)
    {
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(),  mpRectItem->rect().width(), max(0.0, mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y()));
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setX(mPosBeforeResize.x());
        if(desiredRect.height() < mpTextItem->boundingRect().height())
            this->setY(mPosBeforeResize.y()+mHeightBeforeResize-mpRectItem->boundingRect().height()+mpRectItem->pen().widthF());
    }
    else if(mResizeBottom)
    {
        QRectF desiredRect = QRectF(mpRectItem->rect().x(), mpRectItem->rect().y(), mpRectItem->rect().width(), max(0.0, mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y()));
        mpRectItem->setRect(desiredRect.united(mpTextItem->boundingRect()));
        this->setPos(mPosBeforeResize);
    }

    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    mpSelectionBox->setActive();
    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
    mpRectItem->setPos(mpRectItem->pen().width()/2.0, mpRectItem->pen().width()/2.0);
}


void TextBoxWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Widget::mouseReleaseEvent(event);
    if(mWidthBeforeResize != mpRectItem->rect().width() || mHeightBeforeResize != mpRectItem->rect().height())
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        mpParentContainerObject->getUndoStackPtr()->registerResizedTextBoxWidget( mWidgetIndex, mWidthBeforeResize, mHeightBeforeResize, mpRectItem->rect().width(), mpRectItem->rect().height(), mPosBeforeResize, this->pos());
        mWidthBeforeResize = mpRectItem->rect().width();
        mHeightBeforeResize = mpRectItem->rect().height();
        mPosBeforeResize = this->pos();
    }
}

void TextBoxWidget::deleteMe(UndoStatusEnumT undoSettings)
{
    mpParentContainerObject->deleteWidget(this, undoSettings);
}


void TextBoxWidget::updateWidgetFromDialog()
{
    Qt::PenStyle selectedStyle;
    if(mpStyleBoxInDialog->currentIndex() == 0)
        selectedStyle = Qt::SolidLine;
    else if(mpStyleBoxInDialog->currentIndex() == 1)
        selectedStyle = Qt::DashLine;
    else if(mpStyleBoxInDialog->currentIndex() == 2)
        selectedStyle = Qt::DotLine;
    else if(mpStyleBoxInDialog->currentIndex() == 3)
        selectedStyle = Qt::DashDotLine;
    else
        selectedStyle = Qt::SolidLine;      //This shall never happen, but will supress a warning message

    mpParentContainerObject->getUndoStackPtr()->newPost();
    mpParentContainerObject->getUndoStackPtr()->registerModifiedTextBoxWidget(mWidgetIndex, mpTextItem->toPlainText(), mpTextItem->font(), mpTextItem->defaultTextColor(),
                                                                              mpTextBoxInDialog->toPlainText(), mSelectedFont, mSelectedColor,
                                                                              mpRectItem->pen().width(), mpRectItem->pen().style(), mpWidthBoxInDialog->value(), selectedStyle,
                                                                              mpRectItem->isVisible(), mpShowBoxCheckBoxInDialog->isChecked());
    mpParentContainerObject->mpModelWidget->hasChanged();

    mpTextItem->setPlainText(mpTextBoxInDialog->toPlainText());
    mpTextItem->setFont(mSelectedFont);
    mpTextItem->setDefaultTextColor(mSelectedColor);

    //this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());

    //Update box
    mpRectItem->setVisible(mpShowBoxCheckBoxInDialog->isChecked());

    QPen tempPen = mpRectItem->pen();
    tempPen.setColor(mSelectedColor);
    tempPen.setWidth(mpWidthBoxInDialog->value());
    tempPen.setStyle(selectedStyle);
    mpRectItem->setPen(tempPen);
    mpRectItem->setRect(mpRectItem->rect().united(mpTextItem->boundingRect()));

    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    if(this->isSelected())
    {
        mpSelectionBox->setActive();
    }
    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    mpEditDialog->close();
}


void TextBoxWidget::openFontDialog()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, mpTextBoxInDialog->font(), gpMainWindow);
    if (ok)
    {
        mSelectedFont = font;
        mpTextBoxInDialog->setFont(font);
    }
}

void TextBoxWidget::openColorDialog()
{
    QColor color;
    color = QColorDialog::getColor(mSelectedColor, gpMainWindow);

    if (color.isValid())
    {
        mSelectedColor = color;
        mpTextBoxInDialog->setTextColor(color);
        QString redString, greenString, blueString;
        redString.setNum(mSelectedColor.red());
        greenString.setNum(mSelectedColor.green());
        blueString.setNum(mSelectedColor.blue());
        mpColorInDialogButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    }
}
