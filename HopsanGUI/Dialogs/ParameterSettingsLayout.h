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
//! @file   ParameterSettingsLayout.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a Parameter Settings dialog class for changing parameter properties in components and systems
//!
//$Id$

#ifndef PARAMETERSETTINGSLAYOUT_H
#define PARAMETERSETTINGSLAYOUT_H

#include <QGridLayout>
#include <QToolButton>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>


#include "CoreAccess.h" //!< @todo maybe should have parameter stuff in h file of its own so that we don't need to include core access whenever we want to work with parameters

class ModelObject;

class ParameterSettingsLayout : public QGridLayout
{
    Q_OBJECT
    friend class ComponentPropertiesDialog;
    friend class ContainerPropertiesDialog;

public:
    ParameterSettingsLayout(const CoreParameterData &rParameterData, ModelObject *pModelObject, QWidget *pParent=0);

    QString getDataName();
    double getDataValue();
    QString getDataValueTxt();
    void setDataValueTxt(QString valueTxt);
    bool cleanAndVerifyParameterValue();

protected slots:
    void setDefaultValue();
    void showListOfSystemParameters();
    void makePort(bool isPort);
    void pickValueTextColor();

protected:
    ModelObject *mpModelObject;
    QLabel mNameLabel;
    QLabel mDescriptionLabel;
    QLabel mUnitLabel;
    QString mParameterType;
    QLineEdit mValueLineEdit;
    QToolButton mResetDefaultToolButton;
    QToolButton mSystemParameterToolButton;
    QCheckBox mDynamicEnabledCheckBox;

    QString mName;
};

#endif // PARAMETERSETTINGSLAYOUT_H
