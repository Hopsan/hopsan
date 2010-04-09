//!
//! @file   ParameterDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#include <QtGui>
#include <cassert>

#include "ParameterDialog.h"
#include "HopsanCore.h"
#include "mainwindow.h"
#include "GUIObject.h"


//! @class ParameterDialog
//! @brief The ParameterDialog class is a Widget used to interact with component parameters.
//!
//! It read and write parameters to the core components.
//!


//! Constructor.
//! @param coreComponent is a ponter to the core component.
//! @param parent defines a parent to the new instanced object.
ParameterDialog::ParameterDialog(GUIComponent *guiComponent, QWidget *parent)
    : QDialog(parent)
{
    mpGuiComponent = guiComponent;
    mpGUISubsystem = 0;
    mpCoreComponent = mpGuiComponent->getHopsanCoreComponentPtr();
    mpCoreComponentSystem = 0;

    createEditStuff();
}


ParameterDialog::ParameterDialog(GUISubsystem *pGUISubsystem, QWidget *parent)     : QDialog(parent)
{
    mpGuiComponent = 0;
    mpGUISubsystem = pGUISubsystem;
    mpCoreComponentSystem = pGUISubsystem->getHopsanCoreSystemComponentPtr();
    mpCoreComponent = mpGUISubsystem->getHopsanCoreComponentPtr();

    createEditStuff();
}

void ParameterDialog::createEditStuff()
{
    mpNameEdit = new QLineEdit(QString::fromStdString(mpCoreComponent->getName()));

    std::vector<CompParameter>::iterator it;
    vector<CompParameter> paramVector = mpCoreComponent->getParameterVector();
    for ( it=paramVector.begin() ; it !=paramVector.end(); it++ )
    {
        mVarVector.push_back(new QLabel(QString::fromStdString(it->getName())));
        mVarVector.back()->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        mDescriptionVector.push_back(new QLabel(QString::fromStdString(it->getDesc()).append(", ")));
        mUnitVector.push_back(new QLabel(QString::fromStdString(it->getUnit())));

        mValueVector.push_back(new QLineEdit());
        mValueVector.back()->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mValueVector.back()));

        QString valueTxt;
        valueTxt.setNum(it->getValue(), 'g', 6 );
        mValueVector.back()->setText(valueTxt);

        mVarVector.back()->setBuddy(mValueVector.back());
    }

    okButton = new QPushButton(tr("&Ok"));
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setDefault(false);

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    connect(okButton, SIGNAL(pressed()), SLOT(setParameters()));
    connect(cancelButton, SIGNAL(pressed()), SLOT(close()));

    QHBoxLayout *pNameLayout = new QHBoxLayout;
    QLabel *pNameLabel = new QLabel("Name: ");
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);

    QHBoxLayout *pCQSLayout;
    if (mpGUISubsystem != 0)
    {
        mpCQSEdit = new QLineEdit(mpGUISubsystem->getTypeCQS());
        pCQSLayout = new QHBoxLayout;
        QLabel *pCQSLabel = new QLabel("CQS: ");
        pCQSLayout->addWidget(pCQSLabel);
        pCQSLayout->addWidget(mpCQSEdit);
    }

    QVBoxLayout *topLeftLayout1 = new QVBoxLayout;
    QVBoxLayout *topLeftLayout2 = new QVBoxLayout;
    QVBoxLayout *topLeftLayout3 = new QVBoxLayout;
    QVBoxLayout *topLeftLayout4 = new QVBoxLayout;
    for (size_t i=0 ; i <mVarVector.size(); ++i )
    {
        topLeftLayout1->addWidget(mDescriptionVector[i]);
        topLeftLayout2->addWidget(mVarVector[i]);
        topLeftLayout3->addWidget(mValueVector[i]);
        topLeftLayout4->addWidget(mUnitVector[i]);
    }

    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->addLayout(topLeftLayout1);
    leftLayout->addLayout(topLeftLayout2);
    leftLayout->addLayout(topLeftLayout3);
    leftLayout->addLayout(topLeftLayout4);
    leftLayout->addStretch(1);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(buttonBox, lr, 1);
    ++lr;
    if (mpGUISubsystem != 0)
    {
        mainLayout->addLayout(pCQSLayout, lr, 0);
        ++lr;
    }
    mainLayout->addLayout(leftLayout, lr, 0);
    setLayout(mainLayout);

    setWindowTitle(tr("Parameters"));
}


#include "GUIObject.h"
//! Sets the parameters in the core component. Read the values from the dialog and write them into the core component.
void ParameterDialog::setParameters()
{
    mpCoreComponent->setName(mpNameEdit->text().toStdString());
    mpGuiComponent->refreshName();

    qDebug() << mpNameEdit->text();

    for (size_t i=0 ; i < mValueVector.size(); ++i )
    {
        bool ok;
        double newValue = mValueVector[i]->text().toDouble(&ok);
        //std::cout << "i: " << i << qPrintable(labelList[i]->text()) << "  " << mpCoreComponent->getParameterVector().at(i).getName() << std::endl;
        if (!ok)
        {
            MessageWidget *messageWidget = qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent()->parent())->mpMessageWidget;
            messageWidget->printGUIMessage(QString("ParameterDialog::setParameters(): You must give a correct value for '").append(mVarVector[i]->text()).append(QString("', putz. Try again!")));
            return;
        }
        mpCoreComponent->setParameter(mVarVector[i]->text().toStdString(), newValue);
    }
    std::cout << "Parameters updated." << std::endl;
    this->close();
}
