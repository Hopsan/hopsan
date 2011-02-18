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
