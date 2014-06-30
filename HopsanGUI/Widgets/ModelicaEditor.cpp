#include <QTextEdit>
#include <QFile>
#include <QGridLayout>
#include <QPushButton>

#include "ModelicaEditor.h"
#include "ModelicaLibrary.h"
#include "Utilities/HighlightingUtilities.h"
#include "global.h"
#include "DesktopHandler.h"

ModelicaEditor::ModelicaEditor(QWidget *parent) :
    QWidget(parent)
{
    mpEditor = new QTextEdit(this);
    ModelicaHighlighter *pHighlighter = new ModelicaHighlighter(mpEditor->document());

    //QFile moFile(gpDesktopHandler->getDocumentsPath()+"/Models/modelica.mo");   //Hard-coded for now, should not be like this at all
    QFile moFile("/home/robbr48/Documents/Subversion/robbr48/Konferenser/2014/EOOLT2014/models/modelica.mo");   //Hard-coded for now, should not be like this at all
    if(!moFile.open(QFile::ReadOnly | QFile::Text))
    {
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
    gpModelicaLibrary->loadFile(mpEditor->toPlainText());
}


