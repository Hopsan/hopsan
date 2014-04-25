#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

class Configuration;

class OptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsWidget(Configuration *pConfiguration, QWidget *parent = 0);

protected:
    void setVisible(bool visible);

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
    QCheckBox* mpAlwaysSaveBeforeCompilingCheckBox;
    QCheckBox* mpUseTextWrappingCheckBox;

    Configuration *mpConfiguration;
};

#endif // OPTIONSWIDGET_H
