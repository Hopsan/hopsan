#ifndef QUICKNAVIGATIONWIDGET_H
#define QUICKNAVIGATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QHBoxLayout>

//Forward Declarations
class GUIContainerObject;
//class MainWindow;

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
    void refreshVisible();

    QVector<GUIContainerObject*> mContainerObjectPtrs;
    QVector<QPushButton*> mPushButtonPtrs;
    QButtonGroup *mpButtonGroup;
    //QHBoxLayout *mpHBoxLayout;
};

#endif // QUICKNAVIGATIONWIDGET_H
