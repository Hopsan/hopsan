#include "NumHopScriptDialog.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>

#include "GUIObjects/GUIContainerObject.h"
#include "MessageHandler.h"
#include "global.h"

NumHopScriptDialog::NumHopScriptDialog(ContainerObject *pSystem, QWidget *pParent) :
    QDialog(pParent)
{
    mpSystem = pSystem;

    setObjectName("NumHop Script Dialog");
    resize(640,480);
    setWindowTitle(tr("NumHop Script Dialog"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    //setPalette(QPalette(QColor("gray"), QColor("whitesmoke")));

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    mpTextEdit = new QTextEdit(this);

    QDialogButtonBox *pButtonBox =    new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pRevertButton = new QPushButton(tr("&Revert"), this);
    QPushButton *pOkButton = new QPushButton(tr("&Ok"), this);
    QPushButton *pRunButton = new QPushButton(tr("&Run"), this);
    pOkButton->setDefault(true);
    pButtonBox->addButton(pRunButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pRevertButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);

    connect(pOkButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(pCancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pRevertButton, SIGNAL(clicked()), this, SLOT(revert()));
    connect(pRunButton, SIGNAL(clicked()), this, SLOT(run()));

    pLayout->addWidget(mpTextEdit);
    pLayout->addWidget(pButtonBox);

    // Set text contents
    revert();

}

void NumHopScriptDialog::okPressed()
{
    mpSystem->setNumHopScript(mpTextEdit->toPlainText());
    close();
}

void NumHopScriptDialog::revert()
{
    mpTextEdit->setText(mpSystem->getNumHopScript());
}

void NumHopScriptDialog::run()
{
    QString output;
    mpSystem->runNumHopScript(mpTextEdit->toPlainText(), true, output);
    output.prepend("Running NumHop\n");
    output.chop(1);
    gpMessageHandler->addInfoMessage(output);
}

