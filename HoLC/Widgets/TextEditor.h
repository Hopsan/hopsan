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

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QPlainTextEdit>
#include <QCompleter>

class LineNumberArea;


class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditor(QWidget *parent = 0);

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
    QWidget *mpLineNumberArea;
    QCompleter *mpCompleter;
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



#endif // TEXTEDITOR_H
