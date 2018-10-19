#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTextDocument>
#include <QCheckBox>

class FindWidget : public QWidget
{
    Q_OBJECT
public:
    FindWidget(QWidget *parent = 0);

signals:
    void find(QString, QTextDocument::FindFlags);

public slots:
    virtual void setVisible(bool visible);

private slots:
    void findPrevious();
    void findNext();

private:
    QLineEdit *mpFindLineEdit;
    QCheckBox *mpCaseSensitivityCheckBox;
};

#endif // FINDWIDGET_H
