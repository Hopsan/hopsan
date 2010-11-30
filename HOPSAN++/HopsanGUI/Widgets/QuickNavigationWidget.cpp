#include "QuickNavigationWidget.h"
#include "GUIObjects/GUIContainerObject.h"

#include <QHBoxLayout>
#include <QDebug>

QuickNavigationWidget::QuickNavigationWidget(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << "---------------------------------------------------------in QuickNavigationWidget Constructor";

    QHBoxLayout *hbox = new QHBoxLayout();

    QPushButton *tmp = new QPushButton("alg");
    hbox->addWidget(tmp);
    QPushButton *tmp2 = new QPushButton("alg2");
    hbox->addWidget(tmp2);

    this->mpGroupBox = new QGroupBox("apabox");
    //this->mpButtonGroup = new QButtonGroup(this);

    this->mpGroupBox->setLayout(hbox);
    //this->mpGroupBox->setFlat(true);

    //connect(this->mpButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(gotoContainerClosingSubcontainers(int)));

    this->refreshVisible();
    this->setVisible(true);
}

void QuickNavigationWidget::addOpenContainer(GUIContainerObject* pContainer)
{
//    this->mContainerObjectPtrs.append(pContainer);
//    QPushButton *tmp = new QPushButton(pContainer->getName(), this);
//    this->mpGroupBox->layout()->addWidget(tmp);
//    this->mPushButtons.append(tmp);
//    this->mpButtonGroup->addButton(tmp);
//    this->mpButtonGroup->setId(tmp, this->mContainerObjectPtrs.size()-1);

//    this->refreshVisible();
}

void QuickNavigationWidget::gotoContainerClosingSubcontainers(int id)
{
    //Reverse close subsystems,one at a time
//    for (int i=this->mContainerObjectPtrs.size()-1; i>id; --i)
//    {
//        mContainerObjectPtrs[i]->exitContainer();
//        this->mpButtonGroup->removeButton(this->mPushButtons.value(i));
//    }

//    this->refreshVisible();
}

void QuickNavigationWidget::closeLastContainer()
{

}

void QuickNavigationWidget::refreshVisible()
{
    qDebug() << "PushButtons.size(): " << this->mPushButtons.size();
    //this->setVisible(this->mPushButtons.size() > 0);
}
