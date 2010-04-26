//$Id$


#include <QtGui>
#include "PreferenceWidget.h"
#include <QDebug>
#include "ProjectTabWidget.h"

class ProjectTabWidget;

PreferenceWidget::PreferenceWidget(MainWindow *parent)
    : QDialog(parent)
{
    this->mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("PreferenceWidget");
    this->resize(640,480);
    this->setWindowTitle("Model Preferences");

    isoCheckBox = new QCheckBox(tr("Use ISO graphics"));
    isoCheckBox->setCheckable(true);
    isoCheckBox->setChecked(false);

    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setAutoDefault(false);
    okButton = new QPushButton(tr("&Done"));
    okButton->setAutoDefault(true);

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    connect(cancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(okButton, SIGNAL(pressed()), this, SLOT(updateValues()));
    //connect(isoCheckBox, SIGNAL(pressed(bool)), this->mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView, SLOT(setIsoGraphics(bool)));


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    //mainLayout->addLayout(topLeftLayout, 0, 0);
    mainLayout->addWidget(isoCheckBox, 0, 0);
    mainLayout->addWidget(buttonBox, 1, 0);
    //mainLayout->addWidget(extension, 1, 0, 1, 2);
    setLayout(mainLayout);
}

PreferenceWidget::~PreferenceWidget()
{
}



void PreferenceWidget::updateValues()
{
    this->mpParentMainWindow->mpProjectTabs->setIsoGraphics(isoCheckBox->isChecked());
    //this->isoBool = this->isoCheckBox->isChecked();
    //qDebug() << isoBool;
    this->accept();
}
