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
//! @file   ContainerPortPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a class for manimulation of Container properties
//!
//$Id$

#ifndef CONTAINERPROPERTIESDIALOG_H
#define CONTAINERPROPERTIESDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>

//Forward Declaration
class GUIContainerPort;

//! @todo We have three different properties dialog with basically the same "style", maybe we could have a class hierarky, no big dela right now though
class ContainerPortPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ContainerPortPropertiesDialog(GUIContainerPort *pContainerPort, QWidget *pParentWidget=0);

private:
    GUIContainerPort *mpContainerPort;
    QLineEdit *mpNameEdit;

private slots:
    void setValues();
};

#endif // CONTAINERPROPERTIESDIALOG_H
