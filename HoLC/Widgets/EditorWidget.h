#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QTextEdit>

class XmlHighlighter;
class CppHighlighter;

class EditorWidget : public QWidget
{
    Q_OBJECT
public:
    enum HighlighterTypeEnum {XML, Cpp};

    explicit EditorWidget(QWidget *parent = 0);
    void setText(const QString &text, HighlighterTypeEnum type, bool editingEnabled=true);
    QString getText() const;
    void clear();

signals:
    void textChanged();

public slots:

private:
    QTextEdit *mpTextEdit;
    XmlHighlighter *mpXmlHighlighter;
    CppHighlighter *mpCppHighlighter;
};

#endif // EDITORWIDGET_H
