/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   QuickNavigationWidget.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-xx
//!
//! @brief Contains the quick navigation widget that is used to go back after entering into container objects
//!
//$Id$

#include "QuickNavigationWidget.h"
#include "GUIObjects/GUIContainerObject.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QDebug>

QuickNavigationWidget::QuickNavigationWidget(QWidget *parent) :
    QWidget(parent)
{
    mpCurrentSysNameLabel = 0;

    QHBoxLayout *pHBoxLayout = new QHBoxLayout(this);  //Create the horizontal layout for this GroupBox
    mpButtonGroup = new QButtonGroup(this);
    connect(mpButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(gotoContainerAndCloseSubcontainers(int)));

    pHBoxLayout->setContentsMargins(0,0,0,0); //Make narrow margins
    pHBoxLayout->setSpacing(0);
    pHBoxLayout->setAlignment(Qt::AlignLeft);

    refreshVisible();
}

//! @brief Append a new opened container to teh navigation widget
//! @param[in] pContainer A pointer to the Container Object
void QuickNavigationWidget::addOpenContainer(ContainerObject* pContainer)
{
    //! @todo we can use the button group as button storage instead of having an extra Qvector
    mContainerObjectPtrs.append(pContainer);                              //Add the container object ptr from storage
    QPushButton *pTmp = new QPushButton(pContainer->getParentContainerObject()->getName()+" ::", this);   //Create new button with parent name
    pTmp->setStyleSheet("padding:0px");
    pTmp->setFlat(true);
    layout()->addWidget(pTmp);                                             //Add the button to the graphics widget
    mPushButtonPtrs.append(pTmp);                                          //Remember the new button in storage
    mpButtonGroup->addButton(pTmp, mContainerObjectPtrs.size()-1);         //Add the button to the buttons group with the
                                                                           //Id that is corresponding to its container pointer
    refreshCurrentLabel();
    refreshVisible();
}

//! @brief Backstep to the given container closing all bellow it on the way
//! @param[in] id The id of the container to go to
void QuickNavigationWidget::gotoContainerAndCloseSubcontainers(int id)
{
    //Reverse close subsystems,one at a time
    for (int i=mContainerObjectPtrs.size()-1; i>=id; --i)
    {
        mContainerObjectPtrs[i]->exitContainer();
        mpButtonGroup->removeButton(this->mPushButtonPtrs.value(i));  //Remove button from button group
        layout()->removeWidget(this->mPushButtonPtrs.value(i));       //Remove button from graphics box
        delete mPushButtonPtrs.last();                                //Delete the actual button
        mPushButtonPtrs.pop_back();                                   //Remove button ptr from storage
        mContainerObjectPtrs.pop_back();                              //Remove the container object ptr from storage
    }

    refreshCurrentLabel();
    refreshVisible();
}

void QuickNavigationWidget::refreshCurrentLabel()
{
    if (mpCurrentSysNameLabel)
    {
        layout()->removeWidget(mpCurrentSysNameLabel);
        mpCurrentSysNameLabel->hide();
        mpCurrentSysNameLabel->deleteLater();
        mpCurrentSysNameLabel = 0;
    }

    if (mContainerObjectPtrs.size() > 0)
    {
        mpCurrentSysNameLabel = new QLabel(mContainerObjectPtrs.last()->getName(), this);
        mpCurrentSysNameLabel->setStyleSheet("padding:2px");
        layout()->addWidget(mpCurrentSysNameLabel);
    }
}

//! @brief Determines if the widget should be visible or not
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


int QuickNavigationWidget::getCurrentId()
{
    return mpButtonGroup->buttons().size();
}
