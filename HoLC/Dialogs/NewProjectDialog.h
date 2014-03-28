#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>

class FileHandler;

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    NewProjectDialog(FileHandler *pFileHandler, QWidget *parent);

public slots:
    virtual void open();
    virtual void accept();

private slots:
    void setProjectDir();

private:
    QLineEdit *mpProjectNameLineEdit;
    QLineEdit *mpProjectDirLineEdit;
    QLabel *mpWarningLabel;
    FileHandler *mpFileHandler;
};

#endif // NEWPROJECTDIALOG_H
