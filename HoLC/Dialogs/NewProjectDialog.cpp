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

#include <QGridLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>

#include "NewProjectDialog.h"
#include "Handlers/FileHandler.h"

NewProjectDialog::NewProjectDialog(FileHandler *pFileHandler, QWidget *parent)
    : QDialog(parent)
{
    mpFileHandler = pFileHandler;

    this->setWindowTitle("Create New Component Library");
    this->resize(800, this->height());
    QGridLayout *pLayout = new QGridLayout(this);

    QLabel *pDescriptionLabel = new QLabel("Please choose a name an a directory for your new library.");

    QLabel *pProjectNameLabel = new QLabel("Project Name:",this);
    mpProjectNameLineEdit = new QLineEdit(this);

    QLabel *pProjectDirLabel = new QLabel("Project Directory:",this);
    mpProjectDirLineEdit = new QLineEdit(this);
    mpProjectDirLineEdit->setReadOnly(true);
    QToolButton *pProjectDirButton = new QToolButton(this);
    pProjectDirButton->setIcon(QIcon(":graphics/uiicons/Hopsan-Open.png"));

    mpWarningLabel = new QLabel(this);
    mpWarningLabel->setText("<font color='red'>Warning! Directory is not empty!</font>");
    mpWarningLabel->hide();

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(this);
    QPushButton *pOkButton = pButtonBox->addButton(QDialogButtonBox::Ok);
    QPushButton *pCancelButton = pButtonBox->addButton(QDialogButtonBox::Cancel);

    int row = -1;
    pLayout->addWidget(pDescriptionLabel,       ++row,  0, 1, 3);
    pLayout->addWidget(pProjectNameLabel,       ++row,    0, 1, 1);
    pLayout->addWidget(mpProjectNameLineEdit,    row,    1, 1, 2);
    pLayout->addWidget(pProjectDirLabel,        ++row,  0, 1, 1);
    pLayout->addWidget(mpProjectDirLineEdit,     row,    1, 1, 1);
    pLayout->addWidget(pProjectDirButton,       row,    2, 1, 1);
    pLayout->addWidget(mpWarningLabel,          ++row,  1, 1, 2);
    pLayout->addWidget(pButtonBox,              ++row,  0, 1, 3);

    connect(pProjectDirButton, SIGNAL(clicked()),   this, SLOT(setProjectDir()));
    connect(pOkButton,      SIGNAL(clicked()),      this, SLOT(accept()));
    connect(pCancelButton,  SIGNAL(clicked()),      this, SLOT(reject()));
}

void NewProjectDialog::open()
{
    //! @todo Do something (like asking the user) if current project is not saved

    qDebug() << "Open!";

    QDialog::open();
}

void NewProjectDialog::accept()
{
    qDebug() << "Accept!";

    QString libName = mpProjectNameLineEdit->text();
    QString libPath = mpProjectDirLineEdit->text();

    if(libName.contains(" ") || libPath.contains(" "))
    {
        QMessageBox::critical(parentWidget(),"Error","Path or file name must not contain spaces.");
        return;
    }

    //! @todo Check that everything is ok

    mpFileHandler->generateNewXmlAndSourceFiles(libName, libPath);

    QDialog::close();
}

void NewProjectDialog::setProjectDir()
{
    mpWarningLabel->hide();

    QString path = QFileDialog::getExistingDirectory(this, "Select Project Folder");

    if(path.isEmpty()) return;

    QDir dir(path);
    if(!dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).isEmpty())
    {
        mpWarningLabel->show();
    }
    mpProjectDirLineEdit->setText(path);
}
