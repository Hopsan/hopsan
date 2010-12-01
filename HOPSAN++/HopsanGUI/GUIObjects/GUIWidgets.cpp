//$Id$

#include "../common.h"

#include "GUIWidgets.h"
#include "GUISystem.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../MainWindow.h"
#include "UndoStack.h"

#include <QLabel>
#include <QDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFontDialog>
#include <QColorDialog>
#include <QGroupBox>
#include <QSpinBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QComboBox>


using namespace std;


GUIWidget::GUIWidget(QPoint pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, QGraphicsItem *pParent)
    : GUIObject(pos, rot, startSelected, pSystem, pParent)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    pSystem->getContainedScenePtr()->addItem(this);
    this->setPos(pos);
    mIsResizing = false;        //Only used for resizable widgets
}


QVariant GUIWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if(this->isSelected())
        {
            mpParentContainerObject->mSelectedGUIWidgetsList.append(this);
        }
        else
        {
            mpParentContainerObject->mSelectedGUIWidgetsList.removeAll(this);
        }
    }

    return GUIObject::itemChange(change, value);
}


void GUIWidget::deleteMe(undoStatus undoSettings)
{
    assert(1 == 2);
}


void GUIWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QList<GUIWidget *>::iterator it;

        //Loop through all selected widgets and register changed positions in undo stack
    bool alreadyClearedRedo = false;
    for(it = mpParentContainerObject->mSelectedGUIWidgetsList.begin(); it != mpParentContainerObject->mSelectedGUIWidgetsList.end(); ++it)
    {
        if(((*it)->mOldPos != (*it)->pos()) && (event->button() == Qt::LeftButton) && !(*it)->mIsResizing)
        {
                //This check makes sure that only one undo post is created when moving several objects at once
            if(!alreadyClearedRedo)
            {
                if(mpParentContainerObject->mSelectedGUIWidgetsList.size() > 1)
                {
                    mpParentContainerObject->mUndoStack->newPost("movedmultiplewidgets");
                }
                else
                {
                    mpParentContainerObject->mUndoStack->newPost();
                }
                mpParentContainerObject->mpParentProjectTab->hasChanged();
                alreadyClearedRedo = true;
            }

            mpParentContainerObject->mUndoStack->registerMovedWidget((*it), (*it)->mOldPos, (*it)->pos());
        }
        (*it)->mIsResizing = false;
    }

    GUIObject::mouseReleaseEvent(event);
}


//! @brief Constructor for text widget class
//! @param text Initial text in the widget
//! @param pos Position of text widget
//! @param rot Rotation of text widget (should normally be zero)
//! @param startSelected Initial selection status of text widget
//! @param pSystem Pointer to the GUI System where text widget is located
//! @param pParent Pointer to parent object (not required)
GUITextWidget::GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent)
    : GUIWidget(pos, rot, startSelected, pSystem, pParent)
{
    this->mHmfTagName = HMF_TEXTWIDGETTAG;

    mpTextItem = new QGraphicsTextItem(text, this);
    QFont tempFont = mpTextItem->font();
    tempFont.setPointSize(12);
    mpTextItem->setFont(tempFont);
    mpTextItem->setPos(this->boundingRect().center());
    mpTextItem->show();

    this->setTextColor(QColor("darkolivegreen"));

    this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    mpSelectionBox->setSize(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());

    mWidgetIndex = widgetIndex;
}


//! @brief Defines double click event for text widget
void GUITextWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{

        //Open a dialog where text and font can be selected
    mpEditTextDialog = new QDialog(gpMainWindow);
    mpEditTextDialog->setWindowTitle("Set Text Label");

    mpTextBox = new QTextEdit();
    mpTextBox->setPlainText(mpTextItem->toPlainText());
    mpTextBox->setMaximumHeight(70);
    mpFontInDialogButton = new QPushButton("Change Font");
    mpColorInDialogButton = new QPushButton("Change Color");
    mpExampleLabel = new QLabel("Hopsan is cool!");
    mpExampleLabel->setFont(mpTextItem->font());
    QPalette pal(mSelectedColor);
    pal.setColor( QPalette::Foreground, mSelectedColor );
    mpExampleLabel->setPalette(pal);

    QGridLayout *pTextGroupLayout = new QGridLayout();
    pTextGroupLayout->addWidget(mpTextBox,0,0,1,4);
    pTextGroupLayout->addWidget(mpExampleLabel,1,0,1,4);
    pTextGroupLayout->addWidget(mpFontInDialogButton,2,0);
    pTextGroupLayout->addWidget(mpColorInDialogButton,2,1);
    QGroupBox *pTextGroupBox = new QGroupBox();
    pTextGroupBox->setLayout(pTextGroupLayout);

    mpDoneInDialogButton = new QPushButton("Done");
    mpCancelInDialogButton = new QPushButton("Cancel");
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpDoneInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpCancelInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout();
    pDialogLayout->addWidget(pTextGroupBox,0,0);
    pDialogLayout->addWidget(pButtonBox,1,0);
    mpEditTextDialog->setLayout(pDialogLayout);
    mpEditTextDialog->show();

    mSelectedFont = mpTextItem->font();
    mSelectedColor = mpTextItem->defaultTextColor();

    connect(mpColorInDialogButton,SIGNAL(clicked()),this,SLOT(openColorDialog()));
    connect(mpFontInDialogButton,SIGNAL(clicked()),this,SLOT(openFontDialog()));
    connect(mpDoneInDialogButton,SIGNAL(clicked()),this,SLOT(updateWidgetFromDialog()));
    connect(mpCancelInDialogButton,SIGNAL(clicked()),mpEditTextDialog,SLOT(close()));
}


//! @brief Slot that removes text widget from all lists and then deletes it
void GUITextWidget::deleteMe(undoStatus undoSettings)
{
    if(undoSettings == UNDO)
    {
        mpParentContainerObject->mUndoStack->newPost();
        mpParentContainerObject->mUndoStack->registerDeletedTextWidget(this);
    }
    mpParentContainerObject->mTextWidgetList.removeAll(this);
    mpParentContainerObject->mSelectedGUIWidgetsList.removeAll(this);
    mpParentContainerObject->mWidgetMap.remove(this->mWidgetIndex);
    delete(this);
}


//! @brief Private function that updates the text widget from the selected values in the text edit dialog
void GUITextWidget::updateWidgetFromDialog()
{
    mpParentContainerObject->mUndoStack->newPost();
    mpParentContainerObject->mUndoStack->registerModifiedTextWidget(mWidgetIndex, mpTextItem->toPlainText(), mpTextItem->font(), mpTextItem->defaultTextColor(), mpTextBox->toPlainText(), mSelectedFont, mSelectedColor);

    mpTextItem->setPlainText(mpTextBox->toPlainText());
    mpTextItem->setFont(mSelectedFont);
    mpTextItem->setDefaultTextColor(mSelectedColor);

//    delete(mpSelectionBox);
//    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
//                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setSize(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    mpSelectionBox->setActive();
    this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    mpEditTextDialog->close();
}


//! @brief Sets the text in the text widget
//! @param text String containing the text
void GUITextWidget::setText(QString text)
{
    mpTextItem->setPlainText(text);
//    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
//                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setSize(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    mpSelectionBox->setPassive();
}


//! @brief Changes color in text widget
//! @param color New color that shall be used
void GUITextWidget::setTextColor(QColor color)
{
    mpTextItem->setDefaultTextColor(color);
}


//! @brief Changes font in the text widget
//! @param font New font that shall be used
void GUITextWidget::setTextFont(QFont font)
{
    mpTextItem->setFont(font);
//    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
//                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setSize(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    mpSelectionBox->setPassive();
    this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
}


//! @brief Opens a font dialog, places the selected font in the member variable and updates example text in edit dialog
void GUITextWidget::openFontDialog()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, mpExampleLabel->font(), gpMainWindow);
    if (ok)
    {
        mSelectedFont = font;
        mpExampleLabel->setFont(font);
    }
}


//! @brief Opens a color dialog, places the selected color in the member variable and updates example text in edit dialog
void GUITextWidget::openColorDialog()
{
    QColor color;
    color = QColorDialog::getColor(mSelectedColor, gpMainWindow);

    if (color.isValid())
    {
        mSelectedColor = color;
        QPalette pal(mSelectedColor);
        pal.setColor( QPalette::Foreground, mSelectedColor );
        mpExampleLabel->setPalette(pal);
    }
}


//! @brief Saves the text object into a specified dom element
//! @param rDomElement Reference to dom element to save into
void GUITextWidget::saveToDomElement(QDomElement &rDomElement)
{
    QDomElement xmlObject = appendDomElement(rDomElement, mHmfTagName);

    //Save GUI realted stuff
    QDomElement xmlGuiStuff = appendDomElement(xmlObject,HMF_HOPSANGUITAG);

    QPointF pos = mapToScene(boundingRect().topLeft());

    //xmlGuiStuff.setAttribute(HMF_POSETAG, getEndComponentName());

    QDomElement xmlPose = appendDomElement(xmlGuiStuff, HMF_POSETAG);
    xmlPose.setAttribute("x", pos.x());
    xmlPose.setAttribute("y", pos.y());

    QDomElement xmlText = appendDomElement(xmlGuiStuff, "textobject");
    xmlText.setAttribute("text", mpTextItem->toPlainText());
    xmlText.setAttribute("font", mpTextItem->font().toString());
    xmlText.setAttribute("fontcolor", mpTextItem->defaultTextColor().name());
}


//! @brief Constructor for box widget class
//! @param pos Position of box widget
//! @param rot Rotation of box widget (should normally be zero)
//! @param startSelected Initial selection status of box widget
//! @param pSystem Pointer to the GUI System where box widget is located
//! @param pParent Pointer to parent object (not required)
GUIBoxWidget::GUIBoxWidget(QPoint pos, qreal rot, selectionStatus startSelected, GUIContainerObject *pSystem, size_t widgetIndex, QGraphicsItem *pParent)
    : GUIWidget(pos, rot, startSelected, pSystem, pParent)
{
    this->mHmfTagName = HMF_BOXWIDGETTAG;

    mpRectItem = new QGraphicsRectItem(0, 0, 100, 100, this);
    QPen tempPen = mpRectItem->pen();
    tempPen.setColor(QColor("darkolivegreen"));
    tempPen.setWidth(2);
    tempPen.setStyle(Qt::SolidLine);//Qt::DotLine);
    tempPen.setCapStyle(Qt::RoundCap);
    tempPen.setJoinStyle(Qt::RoundJoin);
    mpRectItem->setPen(tempPen);
    mpRectItem->setPos(mpRectItem->pen().width()/2.0, mpRectItem->pen().width()/2.0);
    mpRectItem->show();

    this->setFlag(QGraphicsItem::ItemAcceptsInputMethod, true);

    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
    mWidgetIndex = widgetIndex;

    mWidthBeforeResize = mpRectItem->rect().width();
    mHeightBeforeResize = mpRectItem->rect().height();
    mPosBeforeResize = this->pos();
}




//! @brief Slot that removes text widget from all lists and then deletes it
void GUIBoxWidget::deleteMe(undoStatus undoSettings)
{
    if(undoSettings == UNDO)
    {
        mpParentContainerObject->mUndoStack->newPost();
        mpParentContainerObject->mUndoStack->registerDeletedBoxWidget(this);
    }
    mpParentContainerObject->mBoxWidgetList.removeAll(this);
    mpParentContainerObject->mSelectedGUIWidgetsList.removeAll(this);
    mpParentContainerObject->mWidgetMap.remove(this->mWidgetIndex);
    delete(this);
}


//! @brief Defines double click events (opens the box edit dialog)
void GUIBoxWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{

        //Open a dialog where line width and color can be selected
    mpEditBoxDialog = new QDialog(gpMainWindow);
    mpEditBoxDialog->setWindowTitle("Set Text Label");

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

    QGridLayout *pBoxGroupLayout = new QGridLayout();
    pBoxGroupLayout->addWidget(mpWidthLabelInDialog,0,0);
    pBoxGroupLayout->addWidget(mpWidthBoxInDialog,0,1);
    pBoxGroupLayout->addWidget(mpColorLabelInDialog,1,0);
    pBoxGroupLayout->addWidget(mpColorInDialogButton,1,1);
    pBoxGroupLayout->addWidget(mpStyleLabelInDialog, 2, 0);
    pBoxGroupLayout->addWidget(mpStyleBoxInDialog,2, 1);

    QGroupBox *pBoxGroupBox = new QGroupBox();
    pBoxGroupBox->setLayout(pBoxGroupLayout);

    mpDoneInDialogButton = new QPushButton("Done");
    mpCancelInDialogButton = new QPushButton("Cancel");
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpDoneInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpCancelInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout();
    pDialogLayout->addWidget(pBoxGroupBox,0,0);
    pDialogLayout->addWidget(pButtonBox,1,0);
    mpEditBoxDialog->setLayout(pDialogLayout);
    mpEditBoxDialog->show();

    this->setZValue(0);
    this->setFlags(QGraphicsItem::ItemStacksBehindParent);

    mSelectedColor = mpRectItem->pen().color();

    connect(mpColorInDialogButton,SIGNAL(clicked()),this,SLOT(openColorDialog()));
    connect(mpDoneInDialogButton,SIGNAL(clicked()),this,SLOT(updateWidgetFromDialog()));
    connect(mpCancelInDialogButton,SIGNAL(clicked()),mpEditBoxDialog,SLOT(close()));
}


//! @brief Opens a color dialog and places the selected color in the member variable
void GUIBoxWidget::openColorDialog()
{
    QColor color;
    color = QColorDialog::getColor(mpRectItem->pen().color(), gpMainWindow);

    if (color.isValid())
    {
        mSelectedColor = color;

        QString redString, greenString, blueString;
        redString.setNum(mSelectedColor.red());
        greenString.setNum(mSelectedColor.green());
        blueString.setNum(mSelectedColor.blue());
        mpColorInDialogButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    }
}


//! @brief Private function that updates the text widget from the selected values in the text edit dialog
void GUIBoxWidget::updateWidgetFromDialog()
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

    mpParentContainerObject->mUndoStack->newPost();
    mpParentContainerObject->mUndoStack->registerModifiedBoxWidgetStyle(mWidgetIndex, mpRectItem->pen().width(), mpRectItem->pen().style(), mpRectItem->pen().color(),
                                                                        mpWidthBoxInDialog->value(), selectedStyle, mSelectedColor);

    QPen tempPen = mpRectItem->pen();
    tempPen.setColor(mSelectedColor);
    tempPen.setWidth(mpWidthBoxInDialog->value());
    tempPen.setStyle(selectedStyle);
    mpRectItem->setPen(tempPen);

//    delete(mpSelectionBox);
//    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height(),
//                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    if(this->isSelected())
    {
        mpSelectionBox->setActive();
    }
    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
    mpEditBoxDialog->close();
}


//! @brief Defines what happens when hovering the box widget (changes cursor and defines resize areas)
void GUIBoxWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    GUIObject::hoverMoveEvent(event);

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


//! @brief Defines what happens when clicking on the box (defines start position for resizing)
void GUIBoxWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
    GUIObject::mousePressEvent(event);
}


//! @brief Defines what happens when user is moves (or is trying to move) the object with the mouse. Used for resizing.
void GUIBoxWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    GUIObject::mouseMoveEvent(event);

    if(mResizeLeft && mResizeTop)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x(), mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y());
    }
    else if(mResizeTop && mResizeRight)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x(), mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y());
        this->setX(mPosBeforeResize.x());
    }
    else if(mResizeRight && mResizeBottom)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x(), mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y());
        this->setX(mPosBeforeResize.x());
        this->setY(mPosBeforeResize.y());
    }
    else if(mResizeBottom && mResizeLeft)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x(), mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y());
        this->setY(mPosBeforeResize.y());
    }
    else if(mResizeLeft)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), mWidthBeforeResize+mPosBeforeResize.x()-this->pos().x(), mpRectItem->rect().height());
        this->setY(mPosBeforeResize.y());
    }
    else if(mResizeRight)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), mWidthBeforeResize-mPosBeforeResize.x()+this->pos().x(), mpRectItem->rect().height());
        this->setPos(mPosBeforeResize);
    }
    else if(mResizeTop)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(),  mpRectItem->rect().width(), mHeightBeforeResize+mPosBeforeResize.y()-this->pos().y());
        this->setX(mPosBeforeResize.x());
    }
    else if(mResizeBottom)
    {
        mpRectItem->setRect(mpRectItem->rect().x(), mpRectItem->rect().y(), mpRectItem->rect().width(), mHeightBeforeResize-mPosBeforeResize.y()+this->pos().y());
        this->setPos(mPosBeforeResize);
    }

//    delete(mpSelectionBox);
//    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height(),
//                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setSize(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    mpSelectionBox->setActive();
    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());
    mpRectItem->setPos(mpRectItem->pen().width()/2.0, mpRectItem->pen().width()/2.0);
}



void GUIBoxWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    GUIWidget::mouseReleaseEvent(event);
    if(mWidthBeforeResize != mpRectItem->rect().width() || mHeightBeforeResize != mpRectItem->rect().height())
    {
        mpParentContainerObject->mUndoStack->newPost();
        mpParentContainerObject->mUndoStack->registerResizedBoxWidget( mWidgetIndex, mWidthBeforeResize, mHeightBeforeResize, mpRectItem->rect().width(), mpRectItem->rect().height(), mPosBeforeResize, this->pos());
        mWidthBeforeResize = mpRectItem->rect().width();
        mHeightBeforeResize = mpRectItem->rect().height();
        mPosBeforeResize = this->pos();
    }
}


//! @brief Saves the box widget into a specified dom element
//! @param rDomElement Reference to dom element to save into
void GUIBoxWidget::saveToDomElement(QDomElement &rDomElement)
{
    QDomElement xmlObject = appendDomElement(rDomElement, mHmfTagName);

    //Save GUI realted stuff
    QDomElement xmlGuiStuff = appendDomElement(xmlObject,HMF_HOPSANGUITAG);

    QPointF pos = mapToScene(mpRectItem->rect().topLeft());

    QDomElement xmlPose = appendDomElement(xmlGuiStuff, HMF_POSETAG);
    xmlPose.setAttribute("x", pos.x());
    xmlPose.setAttribute("y", pos.y());

    QDomElement xmlSize = appendDomElement(xmlGuiStuff, "size");
    xmlSize.setAttribute("width", mpRectItem->rect().width());
    xmlSize.setAttribute("height", mpRectItem->rect().height());

    QDomElement xmlLine = appendDomElement(xmlGuiStuff, "line");
    xmlLine.setAttribute("width", mpRectItem->pen().width());

    QString style;
    if(mpRectItem->pen().style() == Qt::SolidLine)
        style = "solidline";
    else if(mpRectItem->pen().style() == Qt::DashLine)
        style = "dashline";
    else if(mpRectItem->pen().style() == Qt::DotLine)
        style = "dotline";
    else if(mpRectItem->pen().style() == Qt::DashDotLine)
        style = "dashdotline";
    xmlLine.setAttribute("style", style);
    xmlLine.setAttribute("color", mpRectItem->pen().color().name());
}


//! @brief Sets the line width of the box
//! @param value New width of the line
void GUIBoxWidget::setLineWidth(int value)
{
    QPen tempPen;
    tempPen = mpRectItem->pen();
    tempPen.setWidth(value);
    mpRectItem->setPen(tempPen);
}


//! @brief Sets the line style of the box
//! @param style Desired style (enum defined by Qt)
void GUIBoxWidget::setLineStyle(Qt::PenStyle style)
{
    QPen tempPen;
    tempPen = mpRectItem->pen();
    tempPen.setStyle(style);
    mpRectItem->setPen(tempPen);
}


//! @brief Sets the color of the box
//! @param New color
void GUIBoxWidget::setLineColor(QColor color)
{
    QPen tempPen;
    tempPen = mpRectItem->pen();
    tempPen.setColor(color);
    mpRectItem->setPen(tempPen);
}


//! @brief Sets the size of the box
//! @param w New width of the box
//! @param h New height of the box
void GUIBoxWidget::setSize(qreal w, qreal h)
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
