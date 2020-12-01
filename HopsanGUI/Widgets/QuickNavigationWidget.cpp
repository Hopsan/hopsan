/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

//! @brief Append a new opened container to the navigation widget
//! @param[in] pContainer A pointer to the Container Object
void QuickNavigationWidget::addOpenContainer(SystemObject* pContainer)
{
    //! @todo we can use the button group as button storage instead of having an extra Qvector
    mContainerObjectPtrs.append(pContainer);                              //Add the container object ptr from storage
    QPushButton *pTmp = new QPushButton(pContainer->getParentSystemObject()->getName()+" ::", this);   //Create new button with parent name
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
        mContainerObjectPtrs[i]->setFlag(QGraphicsItem::ItemIsMovable, false); //!< @todo This is a hack to avoid subsystems moving if you exited multiple systems at once and then entered parent system again, that would for some reason trigger a mouse move event when resetting the viewport
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
