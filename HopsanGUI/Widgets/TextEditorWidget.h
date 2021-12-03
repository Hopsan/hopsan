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
//!
//! @brief Contains the script editor class
//!


#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QFileInfo>
#include <QCompleter>

#include "common.h"
#include "Utilities/HighlightingUtilities.h"


class TextEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    TextEditor(HighlighterTypeEnum language, QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *pEvent);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *pEvent);
    void insertFromMimeData(const QMimeData *pData);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent* e);

public slots:
    void updateAutoCompleteList();

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void insertCompletion(const QString& completion);

private:
    HighlighterTypeEnum mLanguage;
    QWidget *mpLineNumberArea;
    QCompleter *mpCompleter;
    QStringList mAllCompletionWords;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor)
    {
        mpTextEditor = editor;
    }

    QSize sizeHint() const
    {
        return QSize(mpTextEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        mpTextEditor->lineNumberAreaPaintEvent(event);
    }

private:
    TextEditor *mpTextEditor;
};


class TextEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditorWidget(QFileInfo file, HighlighterTypeEnum highlighter, QWidget *parent = nullptr);
    ~TextEditorWidget();

    QFileInfo getFileInfo() const { return mFileInfo; }
    bool isSaved() const { return mIsSaved; }
    QString getSelectedText();
    void replaceSelectedText(QString newText);

public slots:
    void find(QString text, QTextDocument::FindFlags flags);
    void fileChanged(QString filePath);

protected:
    void wheelEvent(QWheelEvent* event);

signals:

private:
    void setHighlighter(HighlighterTypeEnum highlighter);

    TextEditor *mpEditor;
    QFileInfo mFileInfo;
    bool mIsSaved = true;
    bool mIgnoreNextFileChangeNotification = false;
    QString mSavedText;
    HcomHighlighter *mpHcomHighlighter;
    CppHighlighter *mpCppHighlighter;
    XmlHighlighter *mpXmlHighlighter;
    ModelicaHighlighter *mpModelicaHighlighter;
    PythonHighlighter *mpPythonXmlHighlighter;
    QWidget *mpFileChangeNotificationWidget;

public slots:
    void saveAs();
    void saveAndRun();
    void cut();
    void copy();
    void paste();
    void undo();
    void redo();
    void zoomIn();
    void zoomOut();
    void print();
    void save(SaveTargetEnumT saveAsFlag=ExistingFile);
    void reload();

private slots:
    void hasChanged();
};

#endif // SCRIPTEDITOR_H
