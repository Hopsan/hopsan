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


