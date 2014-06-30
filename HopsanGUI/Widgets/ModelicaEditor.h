#ifndef MODELICAEDITOR_H
#define MODELICAEDITOR_H

#include <QWidget>
#include <QTextEdit>

class ModelicaEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ModelicaEditor(QWidget *parent = 0);

signals:

public slots:

private slots:
    void reloadFile();

private:
    QTextEdit *mpEditor;
};

#endif // MODELICAEDITOR_H
