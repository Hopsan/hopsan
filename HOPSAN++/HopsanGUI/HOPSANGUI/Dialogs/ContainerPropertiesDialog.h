//!
//! @file   ContainerPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-24
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
class GUIContainerObject;

class ContainerPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ContainerPropertiesDialog(GUIContainerObject *pContainerObject, QWidget *pParentWidget);

private:
    GUIContainerObject *mpContainerObject;

    QLineEdit *mpNameEdit;
    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLineEdit *mpNSamplesEdit;
    QLineEdit *mpCQSLineEdit;
    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpDisableUndoCheckBox;

private slots:
    void setValues();
    void browseUser();
    void browseIso();
};

#endif // CONTAINERPROPERTIESDIALOG_H
