/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   TimeOffsetWidget.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   20150917
//!
//! @brief Contains the plot offset widget
//!
//$Id$

#include "TimeOffsetWidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QLabel>
#include <QPushButton>

#include "Configuration.h"
#include "global.h"

TimeOffsetWidget::TimeOffsetWidget(SharedVectorVariableT pToFVector, QWidget *pParent)
    : QWidget(pParent)
{
    mpToFVector = pToFVector;
    if (mpToFVector)
    {
        QHBoxLayout *pHBoxLayout = new QHBoxLayout(this);
        mpOffsetLineEdit = new QLineEdit(this);
        mpOffsetLineEdit->setValidator(new QDoubleValidator(this));

        //! @todo cant know we actually have default unit here
        UnitScale us;
        gpConfig->getUnitScale(mpToFVector->getDataQuantity(), gpConfig->getDefaultUnit(mpToFVector->getDataQuantity()), us);
        pHBoxLayout->addWidget(new QLabel(QString("%1 [%2]: ").arg(us.mQuantity).arg(us.mUnit), this));
        pHBoxLayout->addWidget(new QLabel("Offset: ", this));
        pHBoxLayout->addWidget(mpOffsetLineEdit);
        QPushButton *pResetButton = new QPushButton("0", this);
        pResetButton->setToolTip("Reset to 0");
        pHBoxLayout->addWidget(pResetButton);

        // Set the current offset value
        mpOffsetLineEdit->setText(QString("%1").arg(us.rescale(mpToFVector->getPlotOffset())));

        // Connect signals to update time scale and offset when changing values
        connect(mpOffsetLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setOffset(QString)));
        connect(pResetButton, SIGNAL(clicked(bool)), this, SLOT(zeroOffset()));
    }
}

void TimeOffsetWidget::setOffset(const QString &rOffset)
{
    bool parseOK;
    double val = rOffset.toDouble(&parseOK);
    if (mpToFVector && parseOK)
    {
        UnitScale us;
        gpConfig->getUnitScale(mpToFVector->getDataQuantity(), gpConfig->getDefaultUnit(mpToFVector->getDataQuantity()), us);
        mpToFVector->setPlotOffsetIfTime(us.invRescale(val));
        emit valuesChanged();
    }
}

void TimeOffsetWidget::zeroOffset()
{
    if (mpOffsetLineEdit)
    {
        mpOffsetLineEdit->setText("0");
    }
}

