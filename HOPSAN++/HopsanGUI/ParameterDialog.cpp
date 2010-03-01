//!
//! @file   ParameterDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#include <QtGui>
#include <vector>
#include <list>
#include <cassert>

#include "ParameterDialog.h"
#include "HopsanCore.h"

ParameterDialog::ParameterDialog(Component *coreComponent, QWidget *parent)
    : QDialog(parent)
{
    mpCoreComponent = coreComponent;

    std::vector<CompParameter>::iterator it;

    vector<CompParameter> paramVector = mpCoreComponent->getParameterVector();

    for ( it=paramVector.begin() ; it !=paramVector.end(); it++ )
    {
        labelList.push_back(new QLabel(QString::fromStdString(it->getName())));
        lineEditList.push_back(new QLineEdit());
        lineEditList.back()->setValidator(new QDoubleValidator(-999.0, 999.0, 2, lineEditList.back()));
        QString valueTxt;
        valueTxt.setNum(it->getValue(), 'g', 6 );
        lineEditList.back()->setText(valueTxt);
        labelList.back()->setBuddy(lineEditList.back());
    }


    findButton = new QPushButton(tr("&Ok"));
    findButton->setDefault(true);

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(findButton, QDialogButtonBox::ActionRole);

    connect(findButton, SIGNAL(pressed()), SLOT(setParameters()));

    QVBoxLayout *topLeftLayout1 = new QVBoxLayout;
    QVBoxLayout *topLeftLayout2 = new QVBoxLayout;
    for (size_t i=0 ; i <labelList.size(); ++i )
    {
        topLeftLayout1->addWidget(labelList[i]);
        topLeftLayout2->addWidget(lineEditList[i]);
    }

    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->addLayout(topLeftLayout1);
    leftLayout->addLayout(topLeftLayout2);
    leftLayout->addStretch(1);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(leftLayout, 0, 0);
    mainLayout->addWidget(buttonBox, 0, 1);
    setLayout(mainLayout);

    setWindowTitle(tr("Parameters"));
}

void ParameterDialog::setParameters()
{
    for (size_t i=0 ; i < lineEditList.size(); ++i )
    {
        bool ok;
        mpCoreComponent->setParameter(labelList[i]->text().toStdString(), lineEditList[i]->text().toDouble(&ok));
        //std::cout << "i: " << i << qPrintable(labelList[i]->text()) << "  " << mpCoreComponent->getParameterVector().at(i).getName() << std::endl;
        if (!ok)
        {
            assert(false);
        }
    }
    std::cout << "Parameters updated." << std::endl;
    this->close();
}
