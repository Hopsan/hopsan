#ifndef NUMHOPSCRIPTDIALOG_H
#define NUMHOPSCRIPTDIALOG_H

#include <QDialog>
#include <QPointer>

class ContainerObject;
class QTextEdit;

class NumHopScriptDialog : public QDialog
{
    Q_OBJECT

public:
    NumHopScriptDialog(ContainerObject *pSystem, QWidget *pParent);

protected slots:
    void applyPressed();
    void okPressed();
    void revert();
    void run();

private:
    QTextEdit *mpTextEdit;
    QPointer<ContainerObject> mpSystem;

};

#endif // NUMHOPSCRIPTDIALOG_H
