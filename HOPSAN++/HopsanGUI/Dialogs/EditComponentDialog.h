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
//! @file   EditComponentDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the component generator dialog
//!
//$Id$

#ifndef EDITCOMPONENTDIALOG_H_INCLUDED
#define EDITCOMPONENTDIALOG_H_INCLUDED

#include <QtGui>

//#include "Utilities/ComponentGeneratorUtilities.h"  //Needed because we define lists with classes declared here

class ModelObjectAppearance;
class QTextDocument;


class EditComponentDialog : public QDialog
{
    Q_OBJECT
public:
    enum SourceCodeEnumT {Cpp, Modelica};
    QVBoxLayout *mpVerticalLayout;
    QTextEdit *mpCodeTextEdit;
    QComboBox *mpSolverComboBox;
    QDialogButtonBox *mpButtonBox;

    EditComponentDialog(QString code, SourceCodeEnumT language);

    void retranslateUi();

    QString getCode();
    int getSolver();

    void openCreateComponentWizard(SourceCodeEnumT language);

private slots:
    //void openCreateComponentWizard(SourceCodeEnumT language);
    void setHighlighter(SourceCodeEnumT language);
};










class CreateComponentWizard : public QWizard
{
    Q_OBJECT

public:
    CreateComponentWizard(EditComponentDialog::SourceCodeEnumT language, EditComponentDialog *parent = 0);

private slots:
    void updatePage(int i);
    void generate();

private:
    EditComponentDialog::SourceCodeEnumT mLanguage;

    EditComponentDialog *mpParent;

    QWizardPage *mpFirstPage;
    QGridLayout *mpFirstPageLayout;
    QLabel *mpTypeNameLabel;
    QLineEdit *mpTypeNameLineEdit;
    QLabel *mpDisplayNameLabel;
    QLineEdit *mpDisplayNameLineEdit;
    QLabel *mpCqsTypeLabel;
    QComboBox *mpCqsTypeComboBox;
    QLabel *mpNumberOfPortsLabel;
    QSpinBox *mpNumberOfPortsSpinBox;
    QLabel *mpNumberOfParametersLabel;
    QSpinBox *mpNumberOfParametersSpinBox;

    QWizardPage *mpSecondPage;
    QGridLayout *mpSecondPageLayout;
    QLabel *mpPortIdTitle;
    QLabel *mpPortNameTitle;
    QLabel *mpPortTypeTitle;
    QLabel *mpNodeTypeTitle;
    QLabel *mpDefaultValueTitle;
    QList<QLabel*> mPortIdPtrs;
    QList<QLineEdit*> mPortNameLineEditPtrs;
    QList<QComboBox*> mPortTypeComboBoxPtrs;
    QList<QDoubleSpinBox*> mPortDefaultSpinBoxPtrs;

    QWizardPage *mpThirdPage;
    QGridLayout *mpThirdPageLayout;
    QLabel *mpParameterNameTitle;
    QLabel *mpParameterUnitTitle;
    QLabel *mpParameterDescriptionTitle;
    QLabel *mpParameterValueTitle;
    QList<QLineEdit*> mParameterNameLineEditPtrs;
    QList<QLineEdit*> mParameterUnitLineEditPtrs;
    QList<QLineEdit*> mParameterDescriptionLineEditPtrs;
    QList<QLineEdit*> mParameterValueLineEditPtrs;

};

#endif // EDITCOMPONENTDIALOG_H_INCLUDED
