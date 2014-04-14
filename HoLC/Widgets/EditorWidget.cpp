#include <QGridLayout>
#include <QEvent>
#include <QKeyEvent>

#include "EditorWidget.h"
#include "Handlers/FileHandler.h"
#include "Utilities/HighlightingUtilities.h"

EditorWidget::EditorWidget(QWidget *parent) :
    QWidget(parent)
{
    //Create widgets
    mpTextEdit = new TextEditor(this);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    mpTextEdit->setFont(font);

    //Create layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTextEdit,  0,0,1,2);
    this->setLayout(pLayout);

    mpXmlHighlighter = new XmlHighlighter(0);
    mpCppHighlighter = new CppHighlighter(0);

    //Create connections
    connect(mpTextEdit, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
}

void EditorWidget::setText(const QString &text, HighlighterTypeEnum type, bool editingEnabled)
{
    disconnect(mpTextEdit, SIGNAL(textChanged()), this, SIGNAL(textChanged()));

    mpTextEdit->setPlainText(text);

    if(type == XML)
    {
        mpCppHighlighter->setDocument(0);
        mpXmlHighlighter->setDocument(mpTextEdit->document());
    }
    else if(type == Cpp)
    {
        mpXmlHighlighter->setDocument(0);
        mpCppHighlighter->setDocument(mpTextEdit->document());
    }

    mpTextEdit->setPlainText(text);
    mpTextEdit->setReadOnly(!editingEnabled);

    connect(mpTextEdit, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
}

QString EditorWidget::getText() const
{
    return mpTextEdit->toPlainText();
}

void EditorWidget::clear()
{
    mpTextEdit->clear();
}
