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
//! @file   ParameterSettingsLayout.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a Parameter Settings dialog class for changing parameter properties in components and systems
//!
//$Id$

#ifndef PARAMETERSETTINGSLAYOUT_H
#define PARAMETERSETTINGSLAYOUT_H

#include <QtGui>
#include "CoreAccess.h" //!< @todo mayeb should have parameter stuff in h file of its own so that we dont need to include coreaccess whenever we want to work with parameters

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
    QLineEdit mValueLineEdit;
    QToolButton mResetDefaultToolButton;
    QToolButton mSystemParameterToolButton;
    QCheckBox mDynamicEnabledCheckBox;

    QString mName;
};

#endif // PARAMETERSETTINGSLAYOUT_H
