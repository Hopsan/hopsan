#include "QuickNavigationWidget.h"
#include "GUIObjects/GUIContainerObject.h"

#include <QHBoxLayout>
#include <QDebug>

QuickNavigationWidget::QuickNavigationWidget(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << "---------------------------------------------------------in QuickNavigationWidget Constructor";

    this->mpGroupBox = new QGroupBox("box",this);
    QHBoxLayout *hboxlayout = new QHBoxLayout(this->mpGroupBox);

    QPushButton *tmp = new QPushButton("alg", this);
    hboxlayout->addWidget(tmp);

    this->mpButtonGroup = new QButtonGroup(this);
    connect(this->mpButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(gotoContainerClosingSubcontainers(int)));

    tmp = new QPushButton("alg2", this);
    hboxlayout->addWidget(tmp);

    this->mpGroupBox->setFlat(true);
//    this->mpGroupBox->setVisible(true);


    this->refreshVisible();

}

void QuickNavigationWidget::addOpenContainer(GUIContainerObject* pContainer)
{
    this->mContainerObjectPtrs.append(pContainer);
    QPushButton *tmp = new QPushButton(pContainer->getName(), this);
    this->mpGroupBox->layout()->addWidget(tmp);
    this->mPushButtons.append(tmp);
    this->mpButtonGroup->addButton(tmp);
    this->mpButtonGroup->setId(tmp, this->mContainerObjectPtrs.size()-1);

    this->refreshVisible();
}

void QuickNavigationWidget::gotoContainerClosingSubcontainers(int id)
{
    //Reverse close subsystems,one at a time
    for (int i=this->mContainerObjectPtrs.size()-1; i>id; --i)
    {
        mContainerObjectPtrs[i]->exitContainer();
        this->mpButtonGroup->removeButton(this->mPushButtons.value(i));
    }

    this->refreshVisible();
}

void QuickNavigationWidget::closeLastContainer()
{

}

void QuickNavigationWidget::refreshVisible()
{
    qDebug() << "PushButtons.size(): " << this->mPushButtons.size();
    this->setVisible(this->mPushButtons.size() > 0);

    this->setVisible(true);
    this->show();
}
