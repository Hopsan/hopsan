/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Based on: HopsanGUI
 */

//!
//! @file   ParameterDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class GUIObject;
class GUIComponent;
class GUISubsystem;
class Component;
class ComponentSystem;

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    ParameterDialog(GUIComponent *pGUIComponent, QWidget *parent = 0);
    ParameterDialog(GUISubsystem *pGUISubsystem, QWidget *parent = 0);

protected slots:
    void setParameters();

private:
    GUIObject    *mpGUIObject;
    bool isGUISubsystem;

    void createEditStuff();

    QLabel *label;
    QLineEdit *lineEdit;

    QLineEdit *mpNameEdit;
    QLineEdit *mpCQSEdit;
    std::vector<QLabel*> mVarVector;
    std::vector<QLabel*> mDescriptionVector;
    std::vector<QLabel*> mUnitVector;
    std::vector<QLineEdit*> mValueVector;
    QDialogButtonBox *buttonBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QWidget *extension;
};

#endif // PARAMETERDIALOG_H
