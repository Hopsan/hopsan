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


#include "ScriptEditor.h"
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
#include <math.h>

ScriptEditor::ScriptEditor(QFileInfo scriptFileInfo, QWidget *parent) : QWidget(parent)
{
    mScriptFileInfo = scriptFileInfo;

    mpEditor = new HcomEditor(this);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    mpEditor->setFont(font);

    HcomHighlighter *pHighLighter = new HcomHighlighter(mpEditor->document());

    if(mScriptFileInfo.exists())
    {
        QFile scriptFile(mScriptFileInfo.absoluteFilePath());
        if(!scriptFile.open(QFile::ReadOnly | QFile::Text))
        {
            gpMessageHandler->addErrorMessage("Unable to read from HCOM script file: "+mScriptFileInfo.absoluteFilePath());
            return;
        }
        mSavedText = scriptFile.readAll();
        mpEditor->setText(mSavedText);
        scriptFile.close();
    }

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpEditor,         0,0,1,2);

    connect(mpEditor, SIGNAL(textChanged()), this, SLOT(hasChanged()));
}

void ScriptEditor::wheelEvent(QWheelEvent* event)
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


//! Saves the script tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
void ScriptEditor::save(SaveTargetEnumT saveAsFlag)
{
    if(saveAsFlag == NewFile || !mScriptFileInfo.exists())
    {
        QString filePath = QFileDialog::getSaveFileName(this,
                                                        tr("Save Script File"),
                                                        mScriptFileInfo.filePath(),
                                                        tr("Hopsan Script Files (*.hcom)"));
        if(filePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }
        mScriptFileInfo = QFileInfo(filePath);
    }

    QFile file(mScriptFileInfo.absoluteFilePath());
    gpConfig->setStringSetting(CFG_LOADSCRIPTDIR, mScriptFileInfo.absolutePath());

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        gpMessageHandler->addErrorMessage("Unable to open file for writing: "+file.fileName());
        return;
    }

    file.write(mpEditor->toPlainText().toUtf8());
    file.close();

    gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(this), mScriptFileInfo.fileName());

    mIsSaved = true;
}

void ScriptEditor::hasChanged()
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
void ScriptEditor::saveAs()
{
    save(NewFile);
}

void ScriptEditor::cut()
{
    mpEditor->cut();
}

void ScriptEditor::copy()
{
    mpEditor->copy();
}

void ScriptEditor::paste()
{
    mpEditor->paste();
}

void ScriptEditor::undo()
{
    mpEditor->undo();
}

void ScriptEditor::redo()
{
    mpEditor->redo();
}

void ScriptEditor::zoomIn()
{
#if QT_VERSION >= 0x050000
    mpEditor->zoomIn(2);
#endif
}

void ScriptEditor::zoomOut()
{
#if QT_VERSION >= 0x050000
    mpEditor->zoomOut(2);
#endif
}

void ScriptEditor::print()
{
    //! @todo Implement
}

HcomEditor::HcomEditor(QWidget* parent) : QTextEdit(parent)
{
    mpCompleter = new QCompleter(this);
    updateAutoCompleteList();

    mpCompleter->setWidget(this);
    mpCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mpCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(mpCompleter, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

void HcomEditor::keyPressEvent(QKeyEvent* event)
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

    QTextEdit::keyPressEvent(event);

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


//! @brief Updates list of auto complete words
void HcomEditor::updateAutoCompleteList()
{
    mpCompleter->setModel(new QStringListModel(gpTerminalWidget->mpHandler->getAutoCompleteWords(), mpCompleter));
}


void HcomEditor::insertCompletion(const QString& completion)
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
