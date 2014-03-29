#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>

class Configuration;

class OptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsWidget(Configuration *pConfiguration, QWidget *parent = 0);

signals:

public slots:

private slots:
    void setHopsanPath();
    void setHopsanPath(const QString &path);
    void setCompilerPath();
    void setCompilerPath(const QString &path);

private:
    QLineEdit *mpHopsanDirLineEdit;
    QLineEdit *mpLibraryLineEdit;
    QLineEdit *mpIncludeLineEdit;
    QLabel *mpWarningLabel;
    QLineEdit *mpCompilerLineEdit;
    QLabel *mpCompilerWarningLabel;

    Configuration *mpConfiguration;
};

#endif // OPTIONSWIDGET_H
