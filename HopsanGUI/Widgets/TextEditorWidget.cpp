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

//!
//! @file   ScriptEditor.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2018-10-01
//! @version $Id$
//!
//! @brief Contains the script editor class
//!


#include "TextEditorWidget.h"
#include "Utilities/HighlightingUtilities.h"
#include "global.h"
#include "Configuration.h"
#include "MessageHandler.h"
#include "Widgets/ProjectTabWidget.h"
#include "HcomHandler.h"
#include "Widgets/HcomWidget.h"

#include <QGridLayout>
#include <QFileDialog>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTextDocumentFragment>
#include <QKeyEvent>
#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QMimeData>
#include <math.h>

TextEditorWidget::TextEditorWidget(QFileInfo scriptFileInfo, HighlighterTypeEnum highlighter, QWidget *parent) : QWidget(parent)
{
    mFileInfo = scriptFileInfo;

    mpEditor = new TextEditor(highlighter, this);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    mpEditor->setFont(font);
    QFontMetrics metrics(font);
    mpEditor->setTabStopWidth(4*metrics.width(' '));

    mpHcomHighlighter = new HcomHighlighter(nullptr);
    mpXmlHighlighter = new XmlHighlighter(nullptr);
    mpCppHighlighter = new CppHighlighter(nullptr);
    mpModelicaHighlighter = new ModelicaHighlighter(nullptr);
    mpPythonXmlHighlighter = new PythonHighlighter(nullptr);
    setHighlighter(highlighter);

    if(mFileInfo.exists())
    {
        QFile scriptFile(mFileInfo.absoluteFilePath());
        if(!scriptFile.open(QFile::ReadOnly | QFile::Text))
        {
            gpMessageHandler->addErrorMessage("Unable to read from text file: "+mFileInfo.absoluteFilePath());
            return;
        }
        mSavedText = scriptFile.readAll();
        mpEditor->setPlainText(mSavedText);
        scriptFile.close();
    }

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpEditor,         0,0,1,2);

    connect(mpEditor, SIGNAL(textChanged()), this, SLOT(hasChanged()));
}

TextEditorWidget::~TextEditorWidget()
{
    mpHcomHighlighter->deleteLater();
    mpCppHighlighter->deleteLater();
    mpXmlHighlighter->deleteLater();
    mpModelicaHighlighter->deleteLater();
    mpPythonXmlHighlighter->deleteLater();
}

void TextEditorWidget::find(QString text, QTextDocument::FindFlags flags)
{
    mpEditor->find(text,flags);
}


void TextEditorWidget::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION >= 0x050000  //zoomIn() and zoomOut() not available in Qt4
   if ((event->modifiers() == Qt::ControlModifier) && (event->delta() > 0))
       mpEditor->zoomIn(2);
   else if ((event->modifiers() == Qt::ControlModifier) && (event->delta() < 0))
       mpEditor->zoomOut(2);
   else
       QWidget::wheelEvent(event);
#else
   QWidget::wheelEvent(event);
#endif
}

void TextEditorWidget::setHighlighter(HighlighterTypeEnum highlighter)
{
    mpHcomHighlighter->setDocument(nullptr);
    mpCppHighlighter->setDocument(nullptr);
    mpXmlHighlighter->setDocument(nullptr);
    mpModelicaHighlighter->setDocument(nullptr);
    mpPythonXmlHighlighter->setDocument(nullptr);
    switch (highlighter) {
        case HighlighterTypeEnum::Hcom:
            mpHcomHighlighter->setDocument(mpEditor->document());
            break;
        case HighlighterTypeEnum::Cpp:
            mpCppHighlighter->setDocument(mpEditor->document());
            break;
        case HighlighterTypeEnum::XML:
            mpXmlHighlighter->setDocument(mpEditor->document());
            break;
        case HighlighterTypeEnum::Modelica:
            mpModelicaHighlighter->setDocument(mpEditor->document());
            break;
        case HighlighterTypeEnum::Python:
            mpPythonXmlHighlighter->setDocument(mpEditor->document());
            break;
        default:
            break;
    }
}


//! Saves the script tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
void TextEditorWidget::save(SaveTargetEnumT saveAsFlag)
{
    if(saveAsFlag == NewFile || !mFileInfo.exists())
    {
        QString filePath = QFileDialog::getSaveFileName(this,
                                                        tr("Save Script File"),
                                                        mFileInfo.filePath(),
                                                        tr("Hopsan Script Files (*.hcom);;C++ Header Files (*.hpp)"));
        if(filePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }
        mFileInfo = QFileInfo(filePath);
    }

    QFile file(mFileInfo.absoluteFilePath());
    gpConfig->setStringSetting(CFG_LOADSCRIPTDIR, mFileInfo.absolutePath());

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        gpMessageHandler->addErrorMessage("Unable to open file for writing: "+file.fileName());
        return;
    }

    file.write(mpEditor->toPlainText().toUtf8());
    file.close();

    gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(this), mFileInfo.fileName());

    mIsSaved = true;
}

void TextEditorWidget::hasChanged()
{
    mIsSaved = mpEditor->toPlainText() == mSavedText;

    //Add asterisk from tab name
    QString tabName = gpCentralTabWidget->tabText(gpCentralTabWidget->indexOf(this));
    if(mIsSaved && tabName.endsWith("*"))
    {
        tabName.chop(1);
    }
    else if(!mIsSaved && !tabName.endsWith("*"))
    {
        tabName.append("*");
    }
    gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(this), tabName);
}


//! Slot that saves current script to a new model file.
//! @see saveModel(int index)
void TextEditorWidget::saveAs()
{
    save(NewFile);
}

void TextEditorWidget::cut()
{
    mpEditor->cut();
}

void TextEditorWidget::copy()
{
    mpEditor->copy();
}

void TextEditorWidget::paste()
{
    mpEditor->paste();
}

void TextEditorWidget::undo()
{
    mpEditor->undo();
}

void TextEditorWidget::redo()
{
    mpEditor->redo();
}

void TextEditorWidget::zoomIn()
{
#if QT_VERSION >= 0x050000
    mpEditor->zoomIn(2);
#endif
}

void TextEditorWidget::zoomOut()
{
#if QT_VERSION >= 0x050000
    mpEditor->zoomOut(2);
#endif
}

void TextEditorWidget::print()
{
    QPrinter printer;
    printer.setColorMode(QPrinter::Color);
    QPrintDialog *dialog = new QPrintDialog(&printer);
    dialog->setWindowTitle("Print text file");
    if (dialog->exec() != QDialog::Accepted)
        return;
    mpEditor->print(&printer);
}

TextEditor::TextEditor(HighlighterTypeEnum language, QWidget* parent) : QPlainTextEdit(parent)
{
    mLanguage = language;

    mpLineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    setLineWrapMode(QPlainTextEdit::NoWrap);

    mpCompleter = new QCompleter(this);
    updateAutoCompleteList();

    mpCompleter->setWidget(this);
    mpCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mpCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(mpCompleter, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

//! @brief Paint event for line number area
void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *pEvent)
{
    QPainter painter(mpLineNumberArea);
    painter.fillRect(pEvent->rect(), QColor(240,240,240)/*Qt::lightGray*/);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = int(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + int(blockBoundingRect(block).height());

    while (block.isValid() && top <= pEvent->rect().bottom())
    {
        if (block.isVisible() && bottom >= pEvent->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, mpLineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

//! @brief Computes and returns the width of the line number area
int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

//! @brief Updates width of line number area
void TextEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


//! @brief Highlights current line
void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}



//! @brief Update function for line number area
//! @param rect Rectangle to update
//! @param dy Amount of pixels viewport is scrolled vertically
void TextEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if(dy != 0) {
        mpLineNumberArea->scroll(0, dy);
    }
    else {
        mpLineNumberArea->update(0, rect.y(), mpLineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void TextEditor::keyPressEvent(QKeyEvent* event)
{
    if (mpCompleter->popup()->isVisible())
    {
        //Ignore the following keys when the completer is visible
       switch (event->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            event->ignore();
            return;
       default:
           break;
       }
    }

    //Replace tabs with four whitespaces
    if(event->key() == Qt::Key_Backtab)    //Shift+tab, remove spaces to the left until next tab stop (or as many spaces as possible)
    {
        QString line = this->textCursor().block().text().toLatin1();
        int pos = textCursor().positionInBlock();
        while(pos > 0 && line.at(pos-1) == ' ')
        {
            this->textCursor().deletePreviousChar();
            line = this->textCursor().block().text().toLatin1();
            pos = textCursor().positionInBlock();
            if(pos%2 == 0)
            {
                break;
            }
        }
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        QString line = this->textCursor().block().text().toLatin1();
        QString newStr = "\n";
        while(line.startsWith(" "))
        {
            line.remove(0,1);
            newStr.append(" "); //Append one space to new line for each space on previous line, to keep indentation
        }
        if(mLanguage == HighlighterTypeEnum::Cpp && line.startsWith("{"))
        {
            newStr.append("    ");  //Add an extra tab if previous line is a left bracket
        }
        insertPlainText(newStr);
    }
    else if(event->key() == Qt::Key_Home)
    {
        if((event->modifiers().testFlag(Qt::ShiftModifier)))
        {
            moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor); //Shift+home = select until start of line (Excluding whitespace at beginning)
            moveCursor(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        }
        else
        {
            moveCursor(QTextCursor::StartOfBlock);      //Home = move cursor to start of line, excluding initial whitespace
            moveCursor(QTextCursor::NextWord);
        }
    }
    else if(event->key() == Qt::Key_Tab)
    {
        if(textCursor().anchor() != textCursor().position())    //Selection exists, indent whole selection
        {
            QString text = textCursor().selection().toPlainText();
            text.replace("\n", "\n  ");
            text.prepend("  ");
            textCursor().beginEditBlock();
            textCursor().removeSelectedText();
            insertPlainText(text);
            textCursor().endEditBlock();
        }
        else
        {
            QString line = this->textCursor().block().text().toLatin1();
            int pos = textCursor().positionInBlock();
            qDebug() << line << ", position: " << pos;
            int nSpaces = 0;
            while(line.startsWith(" "))
            {
                nSpaces++;
                line.remove(0,1);
            }
            int nSpacesToInsert = 2;    //Insert two spaces by default
            if(pos < nSpaces)           //If at beginning of line, insert until next tab stop
            {
                while((nSpaces+nSpacesToInsert)%2 != 0)
                {
                    nSpacesToInsert--;
                }
            }
            for(int i=0; i<nSpacesToInsert; ++i)
            {
                this->insertPlainText(" ");
            }
        }
    }
    else if(event->key() == Qt::Key_T && event->modifiers().testFlag(Qt::ControlModifier))     //Ctrl-T = block comment
    {
        if(mLanguage ==  HighlighterTypeEnum::Cpp) {
            textCursor().beginEditBlock();
            QString text = textCursor().selection().toPlainText();
            text.replace("\n", "\n// ");
            if(textCursor().atBlockStart()) {
                text.prepend("// ");
            }
            else
            {
                int pos = textCursor().position();
                int stopPos = textCursor().anchor();
                QTextCursor c;
                c = textCursor();
                c.setPosition(std::min(pos,stopPos));
                setTextCursor(c);
                moveCursor(QTextCursor::StartOfBlock);
                insertPlainText("// ");

                c = textCursor();
                c.setPosition(pos+2);
                c.setPosition(stopPos+2, QTextCursor::KeepAnchor);
                setTextCursor(c);
            }
            textCursor().removeSelectedText();
            insertPlainText(text);
            textCursor().endEditBlock();
        }
        else if(mLanguage == HighlighterTypeEnum::Hcom) {
            textCursor().beginEditBlock();
            QString text = textCursor().selection().toPlainText();
            text.replace("\n", "\n# ");
            if(textCursor().atBlockStart()) {
                text.prepend("# ");
            }
            else
            {
                int pos = textCursor().position();
                int stopPos = textCursor().anchor();
                QTextCursor c;
                c = textCursor();
                c.setPosition(std::min(pos,stopPos));
                setTextCursor(c);
                moveCursor(QTextCursor::StartOfBlock);
                insertPlainText("# ");
                c = textCursor();
                c.setPosition(pos+2);
                c.setPosition(stopPos+2, QTextCursor::KeepAnchor);
                setTextCursor(c);
            }
            textCursor().removeSelectedText();
            insertPlainText(text);
            textCursor().endEditBlock();
        }
    }
    else if(event->key() == Qt::Key_U && event->modifiers().testFlag(Qt::ControlModifier))     //Ctrl-U = uncomment block
    {
        if(mLanguage == HighlighterTypeEnum::Cpp) {
            textCursor().beginEditBlock();
            QString text = textCursor().selection().toPlainText();
            text.replace("\n// ", "\n");
            if(text.startsWith("// ")) {
                text.remove(0,3);
            }
            int pos = textCursor().position();
            int stopPos = textCursor().anchor();
            QTextCursor c;
            c = textCursor();
            textCursor().removeSelectedText();
            insertPlainText(text);

            c.setPosition(std::min(pos,stopPos));
            setTextCursor(c);

            textCursor().endEditBlock();
        }
        else if(mLanguage == HighlighterTypeEnum::Hcom) {
            textCursor().beginEditBlock();
            QString text = textCursor().selection().toPlainText();
            text.replace("\n# ", "\n");
            if(text.startsWith("# ")) {
                text.remove(0,2);
            }
            int pos = textCursor().position();
            int stopPos = textCursor().anchor();
            QTextCursor c;
            c = textCursor();
            textCursor().removeSelectedText();
            insertPlainText(text);

            c.setPosition(std::min(pos,stopPos));
            setTextCursor(c);

            textCursor().endEditBlock();
        }
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);

        updateAutoCompleteList();

        QTextCursor tc = textCursor();
        tc.select(QTextCursor::WordUnderCursor);
        QString prefix = tc.selectedText();
        if((event->key() != Qt::Key_Space || !event->modifiers().testFlag(Qt::ControlModifier)) && prefix.isEmpty())
            return;

        if (prefix != mpCompleter->completionPrefix()) {
            mpCompleter->setCompletionPrefix(prefix);
            mpCompleter->popup()->setCurrentIndex(mpCompleter->completionModel()->index(0, 0));
        }
        QRect cr = cursorRect();
        cr.setWidth(mpCompleter->popup()->sizeHintForColumn(0)
                    + mpCompleter->popup()->verticalScrollBar()->sizeHint().width());
        mpCompleter->complete(cr);
    }
}


void TextEditor::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION >= 0x050000  //zoomIn() and zoomOut() not available in Qt4
   if ((event->modifiers() == Qt::ControlModifier) && (event->delta() > 0))
       zoomIn(2);
   else if ((event->modifiers() == Qt::ControlModifier) && (event->delta() < 0))
       zoomOut(2);
   else
       QPlainTextEdit::wheelEvent(event);
#else
   QPlainTextEdit::wheelEvent(event);
#endif
}


//! @brief Updates list of auto complete words
void TextEditor::updateAutoCompleteList()
{
    if(mLanguage == HighlighterTypeEnum::Hcom) {
        mpCompleter->setModel(new QStringListModel(gpTerminalWidget->mpHandler->getAutoCompleteWords(), mpCompleter));
    }
    else if(mLanguage == HighlighterTypeEnum::Cpp) {
        QStringList lines = toPlainText().split("\n");

        QStringList dataTypes = QStringList() << "size_t" << "double" << "int" << CppHighlighter::getHopsanKeywordPatterns();
        QStringList functions = QStringList() << "addInputVariable" << "addOutputVariable" << "addConstant" << "addPowerPort" << "getSafeNodeDataPtr";

        int bracketCounter=-1;

        QStringList variables;
        foreach(const QString &line, lines)
        {
            if(line.simplified().startsWith("class "))
                bracketCounter = 0;

            bracketCounter += line.count("{");
            bracketCounter -= line.count("}");
            if(bracketCounter != 1)
                continue;

            if(line.contains("()")) //Ignore functions
                continue;

            if(dataTypes.contains(line.simplified().section(" ",0,0)))
            {
                variables.append(line.section("//",0,0).simplified().split(","));
            }
        }
        for(int v=0; v<variables.size(); ++v)
        {
            variables[v] = variables[v].section("=",0,0);
            variables[v].remove("*");
            variables[v].remove("&");
            variables[v].remove("!");
            variables[v].remove(";");
            for(int d=0; d<dataTypes.size(); ++d)
            {
                variables[v].remove(dataTypes[d]+" ");
            }
            variables[v].remove(" ");

            //Remove index brackets from array variables
            while(variables[v].contains("["))
            {
                variables[v].remove("["+variables[v].section("[",1,1).section("]",0,0)+"]");
            }
        }

        QStringList allWords = QStringList() << dataTypes << functions << variables;
        mpCompleter->setModel(new QStringListModel(allWords, mpCompleter));
    }
    else {
        return; //No auto completer for XML/Modelica/Python yet
    }
}


void TextEditor::insertCompletion(const QString& completion)
{
    if (mpCompleter->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - mpCompleter->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}


//! @brief Resize event for text editor
void TextEditor::resizeEvent(QResizeEvent *pEvent)
{
    QPlainTextEdit::resizeEvent(pEvent);

    QRect cr = contentsRect();
    mpLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


//! @brief Intercepts paste operations, to control what is pastable
void TextEditor::insertFromMimeData(const QMimeData *pData)
{
    if(pData->hasText())
    {
        insertPlainText(pData->text());
    }
}
