#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QDialog>
#include <QString>
#include <QPointer>
#include <QLineEdit>

// Forward declaration
class ContainerObject;

class FindHelper : public QWidget
{
    Q_OBJECT
public:
    FindHelper(QWidget *pParent);
private:
    QLineEdit *mpLineEdit;
private slots:
    void doFind();
signals:
    void find(QString name);
};


class FindWidget : public QDialog
{
    Q_OBJECT
public:
    explicit FindWidget(QWidget *parent = 0);
    void setContainer(ContainerObject *pContainer);

signals:

public slots:
    void findComponent(const QString &rName, const bool centerView=true);
    void findAlias(const QString &rName, const bool centerView=true);
    void findSystemParameter(const QString &rName, const bool centerView=true);
    void findAny(const QString &rName);

private:
    void clearHighlights();
    QPointer<ContainerObject> mpContainer;

};

#endif // FINDWIDGET_H
