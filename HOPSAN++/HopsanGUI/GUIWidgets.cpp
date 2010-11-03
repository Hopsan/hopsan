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
#include <QColorDIalog>


using namespace std;


GUITextWidget::GUITextWidget(QString text, QPoint pos, qreal rot, selectionStatus startSelected, GUISystem *pSystem, QGraphicsItem *pParent)
    : GUIObject(pos, rot, startSelected, pSystem, pParent)
{
    //! @todo This should not be necessary to do in all GUIWidgets...

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

    mpTextLabel = new QLabel("Text: ");
    mpTextBox = new QLineEdit();
    mpTextBox->setText(mpTextItem->toPlainText());
    mpFontInDialogButton = new QPushButton("Change Font");
    mpColorInDialogButton = new QPushButton("Change Color");
    mpFontLabel = new QLabel("Hopsan is cool!");
    mpFontLabel->setFont(mpTextItem->font());
    QPalette pal(mpSelectedColor);
    pal.setColor( QPalette::Foreground, mpSelectedColor );
    mpFontLabel->setPalette(pal);
    mpDoneInDialogButton = new QPushButton("Done");
    mpCancelInDialogButton = new QPushButton("Cancel");
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpDoneInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpCancelInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout();
    pDialogLayout->addWidget(mpTextLabel,0,0);
    pDialogLayout->addWidget(mpTextBox,0,1);
    pDialogLayout->addWidget(mpFontInDialogButton,1,0);
    pDialogLayout->addWidget(mpColorInDialogButton,1,1);
    pDialogLayout->addWidget(mpFontLabel, 2, 0, 1, 2);
    pDialogLayout->addWidget(pButtonBox,3,0,1,2);
    mpEditTextDialog->setLayout(pDialogLayout);
    mpEditTextDialog->show();

    mpSelectedFont = mpTextItem->font();
    mpSelectedColor = mpTextItem->defaultTextColor();

    connect(mpColorInDialogButton,SIGNAL(clicked()),this,SLOT(getColor()));
    connect(mpFontInDialogButton,SIGNAL(clicked()),this,SLOT(getFont()));
    connect(mpDoneInDialogButton,SIGNAL(clicked()),this,SLOT(setTextFromDialog()));
    connect(mpCancelInDialogButton,SIGNAL(clicked()),mpEditTextDialog,SLOT(close()));
}



void GUITextWidget::setTextFromDialog()
{
    mpTextItem->setPlainText(mpTextBox->text());
    mpTextItem->setFont(mpSelectedFont);
    mpTextItem->setDefaultTextColor(mpSelectedColor);

    delete(mpSelectionBox);
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, mpTextItem->boundingRect().width(), mpTextItem->boundingRect().height(),
                                               QPen(QColor("red"),2*GOLDENRATIO), QPen(QColor("darkRed"),2*GOLDENRATIO), this);
    mpSelectionBox->setActive();
    mpEditTextDialog->close();
}



void GUITextWidget::getFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, mpTextItem->font());
    if (ok)
    {
        mpSelectedFont = font;
        mpFontLabel->setFont(font);
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
        mpFontLabel->setPalette(pal);
    }
}
