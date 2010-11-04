//$Id$

#include "common.h"

#include "GUIWidgets.h"
#include "GUISystem.h"
#include "GUIObject.h"
#include "GraphicsScene.h"

#include <QLabel>
#include <QDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFontDialog>
#include <QColorDialog>
#include <QGroupBox>
#include <QSpinBox>


using namespace std;



//! @brief Constructor for text widget class
//! @param text Initial text in the widget
//! @param pos Position of text widget
//! @param rot Rotation of text widget (should normally be zero)
//! @param startSelected Initial selection status of text widget
//! @param pSystem Pointer to the GUI System where text widget is located
//! @param pParent Pointer to parent object (not required)
GUITextWidget::GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent)
    : GUIObject(pos, rot, startSelected, pSystem, pParent)
{
    this->mHmfTagName = HMF_TEXTWIDGETTAG;

    pSystem->scene()->addItem(this);
    this->setPos(pos);
    mpTextItem = new QGraphicsTextItem(text, this);
    QFont tempFont = mpTextItem->font();
    tempFont.setPointSize(12);
    mpTextItem->setFont(tempFont);
    mpTextItem->setPos(this->boundingRect().center());
    mpTextItem->show();

    this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());

    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                                  QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
}


//! @brief Defines double click event for text widget
void GUITextWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{

        //Open a dialog where text and font can be selected
    mpEditTextDialog = new QDialog();
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


//! Slot that removes text widget from all lists and then deletes it
void GUITextWidget::deleteMe()
{
    mpParentSystem->mTextWidgetList.removeAll(this);
    mpParentSystem->mSelectedGUIObjectsList.removeAll(this);
    delete(this);
}


//! @brief Private function that updates the text widget from the selected values in the text edit dialog
void GUITextWidget::updateWidgetFromDialog()
{
    mpTextItem->setPlainText(mpTextBox->toPlainText());
    mpTextItem->setFont(mSelectedFont);
    mpTextItem->setDefaultTextColor(mSelectedColor);

    delete(mpSelectionBox);
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setActive();
    this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
    mpEditTextDialog->close();
}


//! @brief Sets the text in the text widget
//! @param text String containing the text
void GUITextWidget::setText(QString text)
{
    mpTextItem->setPlainText(text);
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
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
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setPassive();
    this->resize(mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height());
}


//! @brief Opens a font dialog, places the selected font in the member variable and updates example text in edit dialog
void GUITextWidget::openFontDialog()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, mpTextItem->font());
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
    color = QColorDialog::getColor(mSelectedColor);

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
    appendDomValueNode2(xmlGuiStuff, HMF_POSETAG, pos.x(), pos.y());
    appendDomTextNode(xmlGuiStuff, "text", mpTextItem->toPlainText());
    appendDomTextNode(xmlGuiStuff, "font", mpTextItem->font().toString());
    appendDomValueNode(xmlGuiStuff, "fontsize", mpTextItem->font().pointSize());
    appendDomTextNode(xmlGuiStuff, "fontcolor", mpTextItem->defaultTextColor().name());
}












//! @brief Constructor for box widget class
//! @param pos Position of box widget
//! @param rot Rotation of box widget (should normally be zero)
//! @param startSelected Initial selection status of box widget
//! @param pSystem Pointer to the GUI System where box widget is located
//! @param pParent Pointer to parent object (not required)
GUIBoxWidget::GUIBoxWidget(QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent)
    : GUIObject(pos, rot, startSelected, pSystem, pParent)
{
    this->mHmfTagName = HMF_BOXWIDGETTAG;

    pSystem->scene()->addItem(this);
    this->setPos(pos);
    mpRectItem = new QGraphicsRectItem(0, 0, 100, 100, this);
    QPen tempPen = mpRectItem->pen();
    tempPen.setColor(QColor("black"));
    tempPen.setWidth(3);
    mpRectItem->setPen(tempPen);
    mpRectItem->setPos(mpRectItem->pen().width()/2.0, mpRectItem->pen().width()/2.0);
    mpRectItem->show();

    this->resize(mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height());

    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpRectItem->boundingRect().width(), mpRectItem->boundingRect().height(),
                                                  QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
}




//! Slot that removes text widget from all lists and then deletes it
void GUIBoxWidget::deleteMe()
{
    mpParentSystem->mBoxWidgetList.removeAll(this);
    mpParentSystem->mSelectedGUIObjectsList.removeAll(this);
    delete(this);
}




void GUIBoxWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{

    //Open a dialog where line width and color can be selected
mpEditBoxDialog = new QDialog();
mpEditBoxDialog->setWindowTitle("Set Text Label");

mpWidthLabelInDialog = new QLabel("Line Width: ");
mpWidthBoxInDialog = new QSpinBox();
mpWidthBoxInDialog->setMinimum(1);
mpWidthBoxInDialog->setMaximum(100);
mpWidthBoxInDialog->setSingleStep(1);
mpWidthBoxInDialog->setValue(mpRectItem->pen().width());
mpColorInDialogButton = new QPushButton("Change Color");

QGridLayout *pBoxGroupLayout = new QGridLayout();
pBoxGroupLayout->addWidget(mpWidthLabelInDialog,0,0);
pBoxGroupLayout->addWidget(mpWidthBoxInDialog,0,1);
pBoxGroupLayout->addWidget(mpColorInDialogButton,1,0);

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

//mSelectedFont = mpTextItem->font();
//mSelectedColor = mpTextItem->defaultTextColor();

//connect(mpColorInDialogButton,SIGNAL(clicked()),this,SLOT(openColorDialog()));
//connect(mpFontInDialogButton,SIGNAL(clicked()),this,SLOT(openFontDialog()));
//connect(mpDoneInDialogButton,SIGNAL(clicked()),this,SLOT(updateWidgetFromDialog()));
connect(mpCancelInDialogButton,SIGNAL(clicked()),mpEditBoxDialog,SLOT(close()));
}
