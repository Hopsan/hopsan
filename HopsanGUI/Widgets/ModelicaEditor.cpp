/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#include <QTextEdit>
#include <QFile>
#include <QGridLayout>
#include <QPushButton>

#include "ModelicaEditor.h"
#include "ModelicaLibrary.h"
#include "Utilities/HighlightingUtilities.h"
#include "global.h"
#include "DesktopHandler.h"
#include "MessageHandler.h"

ModelicaEditor::ModelicaEditor(QString fileName, QWidget *parent) :
    QWidget(parent)
{
    mFileName = fileName;

    mpEditor = new QTextEdit(this);
    ModelicaHighlighter *pHighlighter = new ModelicaHighlighter(mpEditor->document());

    QFile moFile(mFileName);
    if(!moFile.open(QFile::ReadOnly | QFile::Text))
    {
        gpMessageHandler->addErrorMessage("Unable to read from Modelica file: "+mFileName);
        return;
    }
    mpEditor->setText(moFile.readAll());
    moFile.close();

    QPushButton *pReloadButton = new QPushButton("Reload", this);
    connect(pReloadButton, SIGNAL(clicked()), this, SLOT(reloadFile()));

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpEditor,         0,0,1,2);
    pLayout->addWidget(pReloadButton,   1,0,1,1);
    this->setLayout(pLayout);
}

void ModelicaEditor::reloadFile()
{
    QFile moFile(mFileName);
    if(!moFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
    {
        gpMessageHandler->addErrorMessage("Unable to write to Modelica file: "+mFileName);
        return;
    }
    moFile.write(mpEditor->toPlainText().toUtf8());
    moFile.close();

    gpModelicaLibrary->reload();
}


