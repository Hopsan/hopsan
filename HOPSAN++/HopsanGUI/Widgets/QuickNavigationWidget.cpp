#include "QuickNavigationWidget.h"
#include "GUIObjects/GUIContainerObject.h"

#include <QHBoxLayout>
#include <QDebug>

QuickNavigationWidget::QuickNavigationWidget(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << "---------------------------------------------------------in QuickNavigationWidget Constructor";

    this->mpGroupBox = new QGroupBox(this);
    QHBoxLayout *hboxlayout = new QHBoxLayout(this->mpGroupBox);

//    QPushButton *tmp = new QPushButton("alg", this);
//    hboxlayout->addWidget(tmp);

    this->mpButtonGroup = new QButtonGroup(this);
    connect(this->mpButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(gotoContainerClosingSubcontainers(int)));

//    tmp = new QPushButton("alg2", this);
//    hboxlayout->addWidget(tmp);

    this->mpGroupBox->setFlat(true);

    //this->mpGroupBox->show();


    this->refreshVisible();

}

void QuickNavigationWidget::addOpenContainer(GUIContainerObject* pContainer)
{
    this->mContainerObjectPtrs.append(pContainer);                          //Add the container object ptr from storage
    QPushButton *tmp = new QPushButton(pContainer->mpParentContainerObject->getName(), this);   //Create new button with parent name
    this->mPushButtonPtrs.append(tmp);                                      //Remember the new button in storage
    this->mpGroupBox->layout()->addWidget(tmp);                             //Add the button to the graphics widget
    this->mpButtonGroup->addButton(tmp);                                    //Add the button to the buttons group
    this->mpButtonGroup->setId(tmp, this->mContainerObjectPtrs.size()-1);   //Give the button the same Id as its corresponding container pointer

    this->refreshVisible();
}

void QuickNavigationWidget::gotoContainerClosingSubcontainers(int id)
{
    //Reverse close subsystems,one at a time
    for (int i=this->mContainerObjectPtrs.size()-1; i>=id; --i)
    {
        mContainerObjectPtrs[i]->exitContainer();
        this->mpButtonGroup->removeButton(this->mPushButtonPtrs.value(i));  //Remove button from button group
        mpGroupBox->layout()->removeWidget(this->mPushButtonPtrs.value(i)); //Remove button from graphics box
        delete this->mPushButtonPtrs.last();                                //Delete the actual button
        this->mPushButtonPtrs.pop_back();                                   //Remove button ptr from storage
        this->mContainerObjectPtrs.pop_back();                              //Remove the container object ptr from storage
    }

    this->refreshVisible();
}

void QuickNavigationWidget::refreshVisible()
{
    qDebug() << "PushButtons.size(): " << this->mPushButtonPtrs.size();

    this->mpGroupBox->adjustSize();
    this->adjustSize();


    if (this->mPushButtonPtrs.size() > 0)
    {
        this->show();
    }
    else
    {
        this->hide();
    }
}
