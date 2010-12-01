#ifndef QUICKNAVIGATIONWIDGET_H
#define QUICKNAVIGATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QButtonGroup>

//Forward Declarations
class GUIContainerObject;

class QuickNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    QuickNavigationWidget(QWidget *parent = 0);
    void addOpenContainer(GUIContainerObject* pContainer);

signals:

public slots:
    void gotoContainerClosingSubcontainers(int id);

private:
    void closeLastContainer();
    void refreshVisible();

    QVector<GUIContainerObject*> mContainerObjectPtrs;
    QVector<QPushButton*> mPushButtonPtrs;
    QGroupBox *mpGroupBox;
    QButtonGroup *mpButtonGroup;

};

#endif // QUICKNAVIGATIONWIDGET_H
