#include <QGridLayout>
#include <QEvent>
#include <QKeyEvent>

#include "EditorWidget.h"
#include "Handlers/FileHandler.h"
#include "Configuration.h"
#include "Utilities/HighlightingUtilities.h"

EditorWidget::EditorWidget(Configuration *pConfiguration, QWidget *parent) :
    QWidget(parent)
{
    mpConfiguration = pConfiguration;

    //Create widgets
    mpNotEditableLabel = new QLabel("<font color='darkred'><h3><b>This file cannot be edited from within HoLC.</b></h3></font>", this);
    mpNotEditableLabel->hide();

    mpTextEdit = new TextEditor(this);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    mpTextEdit->setFont(font);

    //Create layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpNotEditableLabel,    0,0,1,2);
    pLayout->addWidget(mpTextEdit,          1,0,1,2);
    this->setLayout(pLayout);

    mpXmlHighlighter = new XmlHighlighter(0);
    mpCppHighlighter = new CppHighlighter(0);

    //Create connections
    connect(mpTextEdit, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    connect(mpConfiguration, SIGNAL(configChanged()), this, SLOT(update()));
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
    mpNotEditableLabel->setVisible(!editingEnabled);

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

void EditorWidget::update()
{
    if(mpConfiguration->getUseTextWrapping())
    {
        mpTextEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    }
    else
    {
        mpTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    }
    QWidget::update();
}
