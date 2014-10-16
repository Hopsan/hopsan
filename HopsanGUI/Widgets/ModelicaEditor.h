#ifndef MODELICAEDITOR_H
#define MODELICAEDITOR_H

#include <QWidget>
#include <QTextEdit>

class ModelicaEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ModelicaEditor(QString file, QWidget *parent = 0);

signals:

public slots:

private slots:
    void reloadFile();

private:
    QTextEdit *mpEditor;
    QString mFileName;
};

#endif // MODELICAEDITOR_H
