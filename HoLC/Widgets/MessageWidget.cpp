#include <QGridLayout>
#include <QPushButton>

#include "MessageWidget.h"

MessageWidget::MessageWidget(QWidget *parent) :
    QWidget(parent)
{
    //Create widgets
    mpTextEdit = new QTextEdit(this);
    mpTextEdit->setReadOnly(true);
    QPushButton *pClearButton = new QPushButton("Clear", this);
    pClearButton->setMaximumWidth(100);

    //Create layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTextEdit,  0,0,1,2);
    pLayout->addWidget(pClearButton,1,0,1,1);
    this->setLayout(pLayout);

    //Setup connections
    connect(pClearButton, SIGNAL(clicked()), this, SLOT(clear()));

    clear();
}


void MessageWidget::addText(const QString &text, const QColor color)
{
    mpTextEdit->undo();
    mpTextEdit->setTextColor(color);
    mpTextEdit->append(text);
    mpTextEdit->setTextColor(Qt::black);
    mpTextEdit->append(">> ");
    mpTextEdit->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor);
}


void MessageWidget::clear()
{
    mpTextEdit->clear();
    mpTextEdit->append(">> ");
}
