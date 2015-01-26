#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QPlainTextEdit>

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

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *mpLineNumberArea;
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
