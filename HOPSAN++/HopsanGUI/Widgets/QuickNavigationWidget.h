//!
//! @file   QuickNavigationWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-xx
//! @version $Id$
//!
//! @brief Contains the quick navigation widget that is used to go back after entering into container objects
//!

#ifndef QUICKNAVIGATIONWIDGET_H
#define QUICKNAVIGATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
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
    void refreshVisible();

    QVector<GUIContainerObject*> mContainerObjectPtrs;
    QVector<QPushButton*> mPushButtonPtrs;
    QButtonGroup *mpButtonGroup;
};

#endif // QUICKNAVIGATIONWIDGET_H
