//$Id$

#include "common.h"

#include "GUIWidgets.h"
#include "GUISystem.h"
#include "GUIObject.h"
#include "GraphicsScene.h"

#include <QLabel>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFontDialog>
#include <QColorDialog>
#include <QGroupBox>


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
    mpTextItem->setPos(0,0);
    QFont tempFont = mpTextItem->font();
    tempFont.setPointSize(12);
    mpTextItem->setFont(tempFont);
    mpTextItem->show();

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

    QPointF pos = mapToScene(boundingRect().center());
    appendDomValueNode2(xmlGuiStuff, HMF_POSETAG, pos.x(), pos.y());
    appendDomTextNode(xmlGuiStuff, "text", mpTextItem->toPlainText());
    appendDomTextNode(xmlGuiStuff, "font", mpTextItem->font().toString());
    appendDomValueNode(xmlGuiStuff, "fontsize", mpTextItem->font().pointSize());
    appendDomTextNode(xmlGuiStuff, "fontcolor", mpTextItem->defaultTextColor().name());
}
