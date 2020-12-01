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

#include "NumHopScriptDialog.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>

#include "GUIObjects/GUIContainerObject.h"
#include "MessageHandler.h"
#include "global.h"

NumHopScriptDialog::NumHopScriptDialog(SystemObject *pSystem, QWidget *pParent) :
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
    QPushButton *pApplyButton = new QPushButton(tr("&Apply"), this);
    QPushButton *pRunButton = new QPushButton(tr("&Run"), this);
    pOkButton->setDefault(true);
    pButtonBox->addButton(pRunButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pApplyButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pRevertButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);

    connect(pApplyButton, SIGNAL(clicked()), this, SLOT(applyPressed()));
    connect(pOkButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(pCancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pRevertButton, SIGNAL(clicked()), this, SLOT(revert()));
    connect(pRunButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(pSystem, SIGNAL(objectDeleted()), this, SLOT(close()));

    pLayout->addWidget(mpTextEdit);
    pLayout->addWidget(pButtonBox);

    // Set text contents
    revert();

}

void NumHopScriptDialog::applyPressed()
{
    if (mpSystem)
    {
        mpSystem->setNumHopScript(mpTextEdit->toPlainText());
    }
}

void NumHopScriptDialog::okPressed()
{
    if (mpSystem)
    {
        mpSystem->setNumHopScript(mpTextEdit->toPlainText());
    }
    close();
}

void NumHopScriptDialog::revert()
{
    if (mpSystem)
    {
        mpTextEdit->setText(mpSystem->getNumHopScript());
    }
    else
    {
        mpTextEdit->setText("Error: System is no longer present!");
    }
}

void NumHopScriptDialog::run()
{
    QString output;
    if (mpSystem)
    {
        mpSystem->runNumHopScript(mpTextEdit->toPlainText(), true, output);
        output.prepend("Running NumHop\n");
        gpMessageHandler->addInfoMessage(output);
    }
    else
    {
        gpMessageHandler->addErrorMessage("NumHopScriptDialog::run() System is no longer present!");
    }
}

