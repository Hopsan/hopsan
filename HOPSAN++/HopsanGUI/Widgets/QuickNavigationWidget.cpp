#include "QuickNavigationWidget.h"
#include "GUIObjects/GUIContainerObject.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QDebug>

QuickNavigationWidget::QuickNavigationWidget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *pHBoxLayout = new QHBoxLayout(this);  //Create the horizontal layout for this GroupBox
    this->mpButtonGroup = new QButtonGroup(this);
    connect(this->mpButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(gotoContainerClosingSubcontainers(int)));

    pHBoxLayout->setContentsMargins(0,0,0,0); //Make narrow margins
    pHBoxLayout->setSpacing(0);
    pHBoxLayout->setAlignment(Qt::AlignLeft);

    this->refreshVisible();
}

void QuickNavigationWidget::addOpenContainer(GUIContainerObject* pContainer)
{
    //! @todo we can use the button group as button storage instead of having an extra Qvector
    this->mContainerObjectPtrs.append(pContainer);                              //Add the container object ptr from storage
    QPushButton *tmp = new QPushButton(pContainer->getParentContainerObject()->getName()+" ::", this);   //Create new button with parent name
    tmp->setStyleSheet("padding:0px");
    tmp->setFlat(true);

    this->mPushButtonPtrs.append(tmp);                                          //Remember the new button in storage
    this->layout()->addWidget(tmp);                                             //Add the button to the graphics widget
    this->mpButtonGroup->addButton(tmp,this->mContainerObjectPtrs.size()-1);    //Add the button to the buttons group with the
                                                                                //Id that is corresponding to its container pointer
    this->refreshVisible();
}

void QuickNavigationWidget::gotoContainerClosingSubcontainers(int id)
{
    //Reverse close subsystems,one at a time
    for (int i=this->mContainerObjectPtrs.size()-1; i>=id; --i)
    {
        mContainerObjectPtrs[i]->exitContainer();
        this->mpButtonGroup->removeButton(this->mPushButtonPtrs.value(i));  //Remove button from button group
        this->layout()->removeWidget(this->mPushButtonPtrs.value(i));       //Remove button from graphics box
        delete this->mPushButtonPtrs.last();                                //Delete the actual button
        this->mPushButtonPtrs.pop_back();                                   //Remove button ptr from storage
        this->mContainerObjectPtrs.pop_back();                              //Remove the container object ptr from storage
    }

    this->refreshVisible();
}

void QuickNavigationWidget::refreshVisible()
{
    //qDebug() << "PushButtons.size(): " << this->mPushButtonPtrs.size();
    if (this->mPushButtonPtrs.size() > 0)
    {
        this->show();
    }
    else
    {
        this->hide();
    }
}
