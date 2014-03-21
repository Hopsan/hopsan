#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>

class OptionsHandler;

class OptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsWidget(OptionsHandler *pOptionsHandler, QWidget *parent = 0);

signals:

public slots:

private slots:
    void setHopsanPath();
    void setCompilerPath();

private:
    QLineEdit *mpHopsanDirLineEdit;
    QLineEdit *mpLibraryLineEdit;
    QLineEdit *mpIncludeLineEdit;
    QLabel *mpWarningLabel;
    QLineEdit *mpCompilerLineEdit;
    QLabel *mpCompilerWarningLabel;

    OptionsHandler *mpOptionsHandler;
};

#endif // OPTIONSWIDGET_H
