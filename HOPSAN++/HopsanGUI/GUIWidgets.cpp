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



void GUITextWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    mpEditTextDialog = new QDialog();
    mpEditTextDialog->setWindowTitle("Set Text Label");

    mpTextBox = new QTextEdit();
    mpTextBox->setPlainText(mpTextItem->toPlainText());
    mpTextBox->setMaximumHeight(70);
    mpFontInDialogButton = new QPushButton("Change Font");
    mpColorInDialogButton = new QPushButton("Change Color");
    mpExampleLabel = new QLabel("Hopsan is cool!");
    mpExampleLabel->setFont(mpTextItem->font());
    QPalette pal(mpSelectedColor);
    pal.setColor( QPalette::Foreground, mpSelectedColor );
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

    mpSelectedFont = mpTextItem->font();
    mpSelectedColor = mpTextItem->defaultTextColor();

    connect(mpColorInDialogButton,SIGNAL(clicked()),this,SLOT(getColor()));
    connect(mpFontInDialogButton,SIGNAL(clicked()),this,SLOT(getFont()));
    connect(mpDoneInDialogButton,SIGNAL(clicked()),this,SLOT(updateWidgetFromDialog()));
    connect(mpCancelInDialogButton,SIGNAL(clicked()),mpEditTextDialog,SLOT(close()));
}



void GUITextWidget::updateWidgetFromDialog()
{
    mpTextItem->setPlainText(mpTextBox->toPlainText());
    mpTextItem->setFont(mpSelectedFont);
    mpTextItem->setDefaultTextColor(mpSelectedColor);

    delete(mpSelectionBox);
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setActive();
    mpEditTextDialog->close();
}


void GUITextWidget::setText(QString text)
{
    mpTextItem->setPlainText(text);
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setPassive();
}


void GUITextWidget::setTextColor(QColor color)
{
    mpTextItem->setDefaultTextColor(color);
}


void GUITextWidget::setTextFont(QFont font)
{
    mpTextItem->setFont(font);
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setPassive();
}


void GUITextWidget::getFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, mpTextItem->font());
    if (ok)
    {
        mpSelectedFont = font;
        mpExampleLabel->setFont(font);
    }
}



void GUITextWidget::getColor()
{
    QColor color;
    color = QColorDialog::getColor(mpSelectedColor);

    if (color.isValid())
    {
        mpSelectedColor = color;
        QPalette pal(mpSelectedColor);
        pal.setColor( QPalette::Foreground, mpSelectedColor );
        mpExampleLabel->setPalette(pal);
    }
}



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
